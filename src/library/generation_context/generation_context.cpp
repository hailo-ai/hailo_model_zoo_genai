/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file generation_context.cpp
 * @brief Generation Context implementation
 **/

#include "generation_context/generation_context.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

#include <hailo/genai/llm/llm.hpp>
#include <hailo/vdevice.hpp>
#include <oatpp/base/Log.hpp>

#include "config/static_config.hpp"

using namespace std::string_literals;

namespace {
void set_params(
    const Generation& params,
    hailort::genai::LLMGeneratorParams& generator_params
) {
    if (params.do_sample) {
        const auto status =
            generator_params.set_do_sample(params.do_sample.value());
        if (status != HAILO_SUCCESS) {
            throw hailort::hailort_error(status, "Failed to set do_sample");
        }
    }
    if (params.seed) {
        const auto status = generator_params.set_seed(params.seed.value());
        if (status != HAILO_SUCCESS) {
            throw hailort::hailort_error(status, "Failed to set seed");
        }
    }
    if (params.temperature) {
        const auto status =
            generator_params.set_temperature(params.temperature.value());
        if (status != HAILO_SUCCESS) {
            throw hailort::hailort_error(status, "Failed to set temperature");
        }
    }
    if (params.top_p) {
        const auto status = generator_params.set_top_p(params.top_p.value());
        if (status != HAILO_SUCCESS) {
            throw hailort::hailort_error(status, "Failed to set top_p");
        }
    }
    if (params.top_k) {
        const auto status = generator_params.set_top_k(params.top_k.value());
        if (status != HAILO_SUCCESS) {
            throw hailort::hailort_error(status, "Failed to set top_k");
        }
    }
    if (params.frequency_penalty) {
        const auto status = generator_params.set_frequency_penalty(
            params.frequency_penalty.value()
        );
        if (status != HAILO_SUCCESS) {
            throw hailort::hailort_error(
                status,
                "Failed to set frequency_penalty"
            );
        }
    }
    if (params.max_generated_tokens) {
        // We don't count the last "dummy" token so we need to request 1 more
        const auto status = generator_params.set_max_generated_tokens(
            params.max_generated_tokens.value() + 1
        );
        if (status != HAILO_SUCCESS) {
            throw hailort::hailort_error(
                status,
                "Failed to set max_generated_tokens"
            );
        }
    }
}
}  // namespace

LLMWrapper::LLMWrapper(hailort::genai::LLM&& llm) : m_llm(std::move(llm)) {}

hailort::genai::LLM& LLMWrapper::operator*() {
    return m_llm;
}

hailort::genai::LLM* LLMWrapper::operator->() {
    return &m_llm;
}

GenerationContext::GenerationContext() = default;

void GenerationContext::load_model(
    const std::string& model_name,
    std::filesystem::path model_path,
    std::optional<std::chrono::seconds> keep_alive
) {
    m_model_name = model_name;
    m_last_generation = std::chrono::steady_clock::now();
    if (keep_alive && (!m_keep_alive || *keep_alive < *m_keep_alive)) {
        m_keep_alive_shortened.notify_one();
    }
    m_keep_alive = keep_alive;
    if (model_path != m_last_path) {
        m_last_path = model_path;
        m_llm.reset();
        // we would like to share the VDevice in the future but it's not supported yet
        m_vdevice.reset();
        std::this_thread::sleep_for(
            config::generation_context_device_switch_sleep_time
        );
        m_vdevice = hailort::VDevice::create_shared().expect(
            "Failed to create VDevice"
        );
        auto llm_params = hailort::genai::LLMParams();
        llm_params.set_model(m_last_path, ""s);
        OATPP_LOGi(
            "GenerationThread",
            "replacing model to {}",
            m_last_path.string()
        );
        m_llm = std::make_unique<LLMWrapper>(
            hailort::genai::LLM::create(m_vdevice, llm_params)
                .expect("Failed to create LLM")
        );
        m_last_prompt.clear();
    }
}

hailort::genai::LLMGeneratorCompletion
GenerationContext::generate_one(const Generation& params) {
    OATPP_LOGi("GenerationThread", "got prompt {}", params.prompt);

    load_model(params.model_name, params.model_path, params.keep_alive);
    std::string prompt = std::move(params.prompt);
    // check if this is a continuation of previous prompt
    if (prompt.rfind(m_last_prompt, 0) != std::string::npos
        && prompt.length() != m_last_prompt.length()) {
        prompt = prompt.substr(m_last_prompt.length(), prompt.length());
        m_last_prompt += prompt;
    } else {
        const auto status = (*m_llm)->clear_context();
        if (status != HAILO_SUCCESS) {
            throw hailort::hailort_error(status, "Failed to clear context");
        }
        m_last_prompt = prompt;
    }

    auto generator_params = (*m_llm)->create_generator_params().expect(
        "Failed to create generator params"
    );
    set_params(params, generator_params);
    auto generator = (*m_llm)
                         ->create_generator(generator_params)
                         .expect("Failed to create generator");

    auto status = generator.write(prompt);
    if (HAILO_SUCCESS != status) {
        throw hailort::hailort_error(status, "Failed to write prompt");
    }
    auto generator_completion =
        generator.generate().expect("Failed to generate");

    return generator_completion;
}

void GenerationContext::append_last_prompt(std::string_view last_prompt) {
    m_last_prompt += last_prompt;
}

std::string GenerationContext::get_model_name() const {
    return m_model_name;
}

std::chrono::steady_clock::time_point
GenerationContext::get_expiration() const {
    if (!m_keep_alive) {
        return std::chrono::steady_clock::time_point::max();
    }
    return m_last_generation + *m_keep_alive;
}

void GenerationContext::reset() {
    OATPP_LOGi("generation_context", "reset issued");
    m_model_name = "";
    m_last_path = "";
    m_keep_alive = std::nullopt;
    m_llm.reset();
    m_vdevice.reset();
}

ExpiryWaitStatus
GenerationContext::wait_expiry(std::unique_lock<std::mutex>& lock) {
    auto expiration = get_expiration();
    while (true) {
        const auto status = m_keep_alive_shortened.wait_until(lock, expiration);
        if (m_stop_flag) {
            OATPP_LOGi("generation_context", "stop received");
            return ExpiryWaitStatus::STOP;
        }
        expiration = get_expiration();
        // cv is notified -> we simply update the new expiration time
        if (status == std::cv_status::no_timeout) {
            OATPP_LOGi("generation_context", "got notified");
            continue;
        }
        OATPP_LOGi("generation_context", "timeout expired");
        assert(status == std::cv_status::timeout);
        // timeout reached -> check if we expired
        const auto now = std::chrono::steady_clock::now();
        if (now >= expiration) {
            return ExpiryWaitStatus::SUCCESS;
        }
    }
}

void GenerationContext::stop() {
    m_stop_flag = true;
    m_keep_alive_shortened.notify_one();
}

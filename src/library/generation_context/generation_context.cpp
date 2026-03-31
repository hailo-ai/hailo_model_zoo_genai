/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
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
#include <hailo/hailort_defaults.hpp>
#include <hailo/vdevice.hpp>
#include <oatpp/base/Log.hpp>

#include "config/static_config.hpp"

using namespace std::string_literals;

namespace hailo_ollama
{

LLMWrapper::LLMWrapper(hailort::genai::LLM &&llm) : m_llm(std::move(llm)) {}

hailort::genai::LLM &LLMWrapper::operator*() { return m_llm; }

hailort::genai::LLM *LLMWrapper::operator->() { return &m_llm; }

GenerationContext::GenerationContext() = default;

GenerationContext::GenerationContext(std::optional<std::string> vdevice_group_id)
    : m_vdevice_group_id(std::move(vdevice_group_id))
{}

void GenerationContext::load_model(const std::string &model_name, std::filesystem::path model_path,
    std::optional<std::chrono::seconds> keep_alive)
{
    m_model_name = model_name;
    m_last_generation = std::chrono::steady_clock::now();
    if (keep_alive && (!m_keep_alive || *keep_alive < *m_keep_alive)) {
        m_keep_alive_shortened.notify_one();
    }
    m_keep_alive = keep_alive;

    if (model_path != m_last_path) { // model changed
        OATPP_LOGi("GenerationThread", "loading model '{}'", m_model_name);
        m_llm.reset();
        // we would like to share the VDevice in the future but it's not supported yet
        m_vdevice.reset();

        m_last_path = model_path;
        auto vdevice_params = hailort::HailoRTDefaults::get_vdevice_params();
        if (m_vdevice_group_id) {
            vdevice_params.group_id = m_vdevice_group_id->c_str();
        }
        m_vdevice = hailort::VDevice::create_shared(vdevice_params).expect("Failed to create VDevice");
        auto llm_params = hailort::genai::LLMParams();
        llm_params.set_model(m_last_path.string(), ""s);
        m_llm = std::make_unique<LLMWrapper>(hailort::genai::LLM::create(m_vdevice, llm_params).expect("Failed to create LLM"));
        OATPP_LOGi("GenerationThread", "Finished loading model '{}'", m_model_name);
    }
}

hailort::genai::LLMGeneratorCompletion GenerationContext::generate_one(const Generation &params)
{
    OATPP_LOGi("GenerationThread", "got prompt");

    load_model(params.model_name, params.model_path, params.keep_alive);

    // Check if this is a continuation of the previous conversation
    // by checking if the new messages start with the cached history as a prefix
    const bool is_continuation =
        params.prompt_json_strings.size() > m_conversation_history.size() &&
        std::equal(m_conversation_history.begin(), m_conversation_history.end(), params.prompt_json_strings.begin());

    std::vector<std::string> messages_to_send;

    if (is_continuation) {
        // Send only the new messages (diff)
        messages_to_send.assign(params.prompt_json_strings.begin() + m_conversation_history.size(),
            params.prompt_json_strings.end());
        OATPP_LOGi("GenerationThread", "Continuation detected, sending {} new messages", messages_to_send.size());
    } else {
        // New conversation - clear context and send all messages
        const auto status = (*m_llm)->clear_context();
        if (status != HAILO_SUCCESS) {
            throw hailort::hailort_error(status, "Failed to clear context");
        }
        messages_to_send = params.prompt_json_strings;
        OATPP_LOGi("GenerationThread", "New conversation, clearing context and sending {} messages",
            messages_to_send.size());
    }

    // Generate using the messages
    auto generator_completion =
        (*m_llm)->generate(params.generator_params, messages_to_send).expect("Failed to generate");

    // Update conversation history with the full conversation (not just diff)
    m_conversation_history = params.prompt_json_strings;

    return generator_completion;
}

void GenerationContext::append_assistant_message(const std::string &content)
{
    static constexpr char QUOTE_CHAR = '"';
    static constexpr char BACKSLASH_CHAR = '\\';
    static constexpr std::string_view ESCAPED_QUOTE = "\\\"";
    static constexpr std::string_view ESCAPED_BACKSLASH = "\\\\";

    // Escape quotes in content for JSON
    std::string escaped_content = content;
    size_t pos = 0;
    while ((pos = escaped_content.find(QUOTE_CHAR, pos)) != std::string::npos) {
        escaped_content.replace(pos, 1, ESCAPED_QUOTE);
        pos += ESCAPED_QUOTE.size();
    }
    // Also escape backslashes
    pos = 0;
    while ((pos = escaped_content.find(BACKSLASH_CHAR, pos)) != std::string::npos) {
        if (pos + 1 < escaped_content.size() && escaped_content[pos + 1] != QUOTE_CHAR) {
            escaped_content.replace(pos, 1, ESCAPED_BACKSLASH);
            pos += ESCAPED_BACKSLASH.size();
        } else {
            pos++;
        }
    }

    m_conversation_history.push_back(R"({"role": "assistant", "content": ")" + escaped_content + R"("})");
}

std::string GenerationContext::get_model_name() const { return m_model_name; }

std::chrono::steady_clock::time_point GenerationContext::get_expiration() const
{
    if (!m_keep_alive) {
        return std::chrono::steady_clock::time_point::max();
    }
    return m_last_generation + *m_keep_alive;
}

std::string GenerationContext::get_generation_recovery_sequence() const
{
    if (!m_llm) {
        throw hailort::hailort_error(HAILO_UNINITIALIZED, "LLM not loaded");
    }
    return (*m_llm)->get_generation_recovery_sequence().expect("Failed to get generation recovery sequence");
}

hailort::genai::LLMGeneratorParams GenerationContext::create_generator_params() const
{
    if (!m_llm) {
        throw hailort::hailort_error(HAILO_UNINITIALIZED, "LLM not loaded");
    }
    return (*m_llm)->create_generator_params().expect("Failed to create generator params");
}

void GenerationContext::reset()
{
    OATPP_LOGi("generation_context", "reset issued");
    m_model_name = "";
    m_last_path = "";
    m_conversation_history.clear();
    m_keep_alive = std::nullopt;
    m_llm.reset();
    m_vdevice.reset();
}

ExpiryWaitStatus GenerationContext::wait_expiry(std::unique_lock<std::mutex> &lock)
{
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

void GenerationContext::stop()
{
    m_stop_flag = true;
    m_keep_alive_shortened.notify_one();
}

} // namespace hailo_ollama

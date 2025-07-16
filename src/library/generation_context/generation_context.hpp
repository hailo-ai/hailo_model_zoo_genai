/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file generation_context.hpp
 * @brief Class for managing generation
 **/

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <hailo/genai/llm/llm.hpp>
#include <hailo/vdevice.hpp>
#include <libguarded/cs_plain_guarded.h>

struct Generation {
    std::string model_name;
    std::filesystem::path model_path;
    std::string prompt;
    std::optional<float32_t> temperature;
    std::optional<float32_t> top_p;
    std::optional<uint32_t> top_k;
    std::optional<float32_t> frequency_penalty;
    std::optional<uint32_t> max_generated_tokens;
    std::optional<bool> do_sample;
    std::optional<uint32_t> seed;
    std::optional<std::chrono::seconds> keep_alive;
};

// We want a unique_ptr for LLM but we can't have it so this wrapper is needed
class LLMWrapper {
  public:
    explicit LLMWrapper(hailort::genai::LLM&& llm);

    hailort::genai::LLM& operator*();
    hailort::genai::LLM* operator->();

  private:
    hailort::genai::LLM m_llm;
};

enum class ExpiryWaitStatus { SUCCESS, STOP };

class GenerationContext {
  public:
    GenerationContext();
    hailort::genai::LLMGeneratorCompletion

    generate_one(const Generation& params);

    std::string get_model_name() const;

    std::chrono::steady_clock::time_point get_expiration() const;

    void append_last_prompt(std::string_view last_prompt);
    void load_model(
        const std::string& model_name,
        std::filesystem::path model_path,
        std::optional<std::chrono::seconds> keep_alive
    );
    void reset();

    ExpiryWaitStatus wait_expiry(std::unique_lock<std::mutex>& lock);

    void stop();

  private:
    std::shared_ptr<hailort::VDevice> m_vdevice;
    std::unique_ptr<LLMWrapper> m_llm;
    std::string m_model_name;
    std::filesystem::path m_last_path;
    std::string m_last_prompt;
    std::chrono::steady_clock::time_point m_last_generation;
    std::optional<std::chrono::seconds> m_keep_alive;
    std::condition_variable m_keep_alive_shortened;
    bool m_stop_flag;
};

using SyncGenerationContext = libguarded::plain_guarded<GenerationContext>;

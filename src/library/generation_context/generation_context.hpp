/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
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

namespace hailo_ollama
{

struct Generation {
    std::string model_name;
    std::filesystem::path model_path;
    std::vector<std::string> prompt_json_strings;
    hailort::genai::LLMGeneratorParams generator_params;
    std::optional<std::chrono::seconds> keep_alive;
};

// We want a unique_ptr for LLM but we can't have it so this wrapper is needed
class LLMWrapper
{
public:
    explicit LLMWrapper(hailort::genai::LLM &&llm);

    hailort::genai::LLM &operator*();
    hailort::genai::LLM *operator->();

private:
    hailort::genai::LLM m_llm;
};

enum class ExpiryWaitStatus {
    SUCCESS,
    STOP
};

class GenerationContext
{
public:
    GenerationContext();
    explicit GenerationContext(std::optional<std::string> vdevice_group_id);
    hailort::genai::LLMGeneratorCompletion generate_one(const Generation &params);

    void append_assistant_message(const std::string &content);

    std::string get_model_name() const;

    std::chrono::steady_clock::time_point get_expiration() const;

    std::string get_generation_recovery_sequence() const;
    hailort::genai::LLMGeneratorParams create_generator_params() const;

    void load_model(const std::string &model_name, std::filesystem::path model_path,
        std::optional<std::chrono::seconds> keep_alive);
    void reset();

    ExpiryWaitStatus wait_expiry(std::unique_lock<std::mutex> &lock);

    void stop();

    struct GenerationFinishedReason {
        static constexpr const char *STOP = "stop";
        static constexpr const char *LENGTH = "length";
    };

private:
    std::optional<std::string> m_vdevice_group_id;
    std::shared_ptr<hailort::VDevice> m_vdevice;
    std::unique_ptr<LLMWrapper> m_llm;
    std::string m_model_name;
    std::filesystem::path m_last_path;
    std::vector<std::string> m_conversation_history; // Track conversation as JSON messages
    std::chrono::steady_clock::time_point m_last_generation;
    std::optional<std::chrono::seconds> m_keep_alive;
    std::condition_variable m_keep_alive_shortened;
    bool m_stop_flag;
};

using SyncGenerationContext = libguarded::plain_guarded<GenerationContext>;

} // namespace hailo_ollama

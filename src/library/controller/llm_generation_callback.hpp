/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file llm_generation_callback.hpp
 * @brief Callback for generating
 **/

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <hailo/genai/llm/llm.hpp>
#include <oatpp/data/mapping/ObjectMapper.hpp>
#include <oatpp/data/stream/Stream.hpp>

#include "generation_context/generation_context.hpp"

namespace hailo_ollama
{

inline std::string strip_eos_suffix(const std::string &text, const std::string &eos_token)
{
    if (!eos_token.empty() && text.size() >= eos_token.size() &&
        text.compare(text.size() - eos_token.size(), eos_token.size(), eos_token) == 0) {
        return text.substr(0, text.size() - eos_token.size());
    }
    return text;
}

inline std::string escape_json_quotes(const std::string &text)
{
    static constexpr char QUOTE_CHAR = '"';
    static constexpr std::string_view ESCAPED_QUOTE = "\\\"";

    std::string result = text;
    size_t pos = 0;
    while ((pos = result.find(QUOTE_CHAR, pos)) != std::string::npos) {
        result.replace(pos, 1, ESCAPED_QUOTE);
        pos += ESCAPED_QUOTE.size();
    }
    return result;
}

class LLMGenerationReadCallback : public oatpp::data::stream::ReadCallback
{
public:
    LLMGenerationReadCallback(const std::string &model,
        const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &object_mapper,
        SyncGenerationContext::handle &&generation_context,
        hailort::genai::LLMGeneratorCompletion &&generator_completion, const bool return_as_message,
        const std::string &eos_token);

    oatpp::v_io_size read(void *buffer, v_buff_size bufferSize, oatpp::async::Action &action) override;

private:
    std::string m_model;
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> m_object_mapper;
    SyncGenerationContext::handle m_generation_context;
    hailort::genai::LLMGeneratorCompletion m_generator_completion;
    bool m_return_as_message;
    std::chrono::steady_clock::time_point m_begin;

    uint64_t m_count;
    bool m_done;
    std::string m_response_text; // Accumulate full response for history
    std::string m_eos_token;     // EOS token to strip from responses
};

} // namespace hailo_ollama

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
#include "tool_parsers/tool_call_parser.hpp"

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

class LLMGenerationReadCallback : public oatpp::data::stream::ReadCallback
{
public:
    LLMGenerationReadCallback(const std::string &model,
        const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &object_mapper,
        SyncGenerationContext::handle &&generation_context,
        hailort::genai::LLMGeneratorCompletion &&generator_completion, const bool return_as_message,
        const std::string &eos_token, const ToolCallParser &tool_call_parser, const bool tools_requested);

    oatpp::v_io_size read(void *buffer, v_buff_size bufferSize, oatpp::async::Action &action) override;

private:
    // Returns the portion of m_pending_text that is safe to stream now, holding back the trailing
    // bytes that could still be the start of a tool-call prefix. Once the prefix is seen, all
    // further text is buffered (returns empty) until end-of-generation.
    std::string take_streamable_text();

    // Returns the parsed non-tool content that has not yet been streamed, so the final chunk can
    // flush the held-back tail of a normal response without re-sending already-streamed text.
    std::string residual_streamable_content(const std::string &parsed_content) const;

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

    // Non-owning: the moved generation-context lock-handle keeps the context and its parser alive for
    // this callback's lifetime.
    const ToolCallParser *m_tool_call_parser;
    bool m_tools_requested;           // Tool-call holdback and parsing engage only when tools were requested
    std::string m_pending_text;       // Text accumulated but not yet streamed (delimiter holdback)
    std::string m_streamed_content;   // Non-tool text already streamed to the client
    bool m_buffering_tool_call;       // True once the tool-call prefix has been seen
};

} // namespace hailo_ollama

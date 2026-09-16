/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file llm_generation_callback.cpp
 * @brief LLMGenerationReadCallback implementation
 **/

#include "controller/llm_generation_callback.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <memory>
#include <sstream>
#include <vector>

#include <hailo/genai/llm/llm.hpp>
#include <oatpp/data/mapping/ObjectMapper.hpp>
#include <oatpp/data/stream/Stream.hpp>

#include "config/static_config.hpp"
#include "dto/DTOs.hpp"
#include "generation_context/generation_context.hpp"
#include "utils/dto_json.hpp"
#include "utils/time.hpp"

namespace hailo_ollama
{

LLMGenerationReadCallback::LLMGenerationReadCallback(const std::string &model,
    const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &object_mapper,
    SyncGenerationContext::handle &&generation_context, hailort::genai::LLMGeneratorCompletion &&generator_completion,
    const bool return_as_message, const std::string &eos_token, const ToolCallParser &tool_call_parser,
    const bool tools_requested)
    : m_model(model), m_object_mapper(object_mapper), m_generation_context(std::move(generation_context)),
      m_generator_completion(std::move(generator_completion)), m_return_as_message(return_as_message),
      m_begin(std::chrono::steady_clock::now()), m_count(0ULL), m_done(false), m_response_text(),
      m_eos_token(eos_token), m_tool_call_parser(&tool_call_parser), m_tools_requested(tools_requested),
      m_pending_text(), m_streamed_content(), m_buffering_tool_call(false)
{}

std::string LLMGenerationReadCallback::residual_streamable_content(const std::string &parsed_content) const
{
    // parsed_content is the full non-tool text; m_streamed_content is what already went out
    // token-by-token. Emit only the part beyond the already-streamed prefix. The parser trims the
    // content, so already-streamed leading whitespace may not be a literal prefix of parsed_content;
    // align on the largest streamed prefix that parsed_content still begins with.
    size_t streamed = m_streamed_content.size();
    while ((streamed > 0) && (0 != parsed_content.compare(0, streamed, m_streamed_content, 0, streamed))) {
        --streamed;
    }
    return parsed_content.substr(std::min(streamed, parsed_content.size()));
}

std::string LLMGenerationReadCallback::take_streamable_text()
{
    if (m_buffering_tool_call) {
        return "";
    }

    // Tool calls can only occur when tools were requested. With no marker, take_streamable_text holds
    // back nothing, so plain generation streams verbatim and the final chunk stays empty.
    const auto call_start = m_tools_requested ? m_tool_call_parser->tool_call_start_marker() : std::string();
    if (!call_start.empty()) {
        const auto call_start_pos = m_pending_text.find(call_start);
        if (std::string::npos != call_start_pos) {
            // Opener seen: stream the leading content, buffer everything from the opener onward.
            m_buffering_tool_call = true;
            return m_pending_text.substr(0, call_start_pos);
        }
        // Hold back a tail that could be the start of a split opener so it never leaks.
        if (m_pending_text.size() <= call_start.size()) {
            return "";
        }
        const auto safe_len = m_pending_text.size() - call_start.size();
        auto streamable = m_pending_text.substr(0, safe_len);
        m_pending_text.erase(0, safe_len);
        return streamable;
    }

    auto streamable = std::move(m_pending_text);
    m_pending_text.clear();
    return streamable;
}

oatpp::v_io_size LLMGenerationReadCallback::read(void *buffer, v_buff_size bufferSize, oatpp::async::Action &action)
{
    using GenerationStatus = hailort::genai::LLMGeneratorCompletion::Status;
    (void)action; // ignore action when using SimpleAPI

    if (m_done) {
        const auto stripped_response = strip_eos_suffix(m_response_text, m_eos_token);
        if (m_tools_requested) {
            const auto parse_result = m_tool_call_parser->parse(stripped_response);
            m_generation_context->append_assistant_message(parse_result.content, parse_result.tool_calls);
        } else {
            m_generation_context->append_assistant_message(stripped_response);
        }
        return 0;
    }
    // Buffered tool-call bodies can hold back many tokens before anything is streamable. Loop on the
    // generator instead of recursing so a long buffered body cannot overflow the stack.
    std::string streamable;
    bool is_last_token = false;
    bool encountered_max_tokens = false;
    while (true) {
        std::string token = m_generator_completion.read().expect("read failed!");

        // check status immediately after read to see if it's the last one
        const auto generation_status = m_generator_completion.generation_status();
        is_last_token = (generation_status != GenerationStatus::GENERATING);
        encountered_max_tokens = (generation_status == GenerationStatus::MAX_TOKENS_REACHED);
        if (is_last_token) {
            break;
        }

        m_response_text += token;
        m_pending_text += token;
        ++m_count;

        // Strip EOS token from token if present, then hold back any potential tool-call prefix.
        streamable = strip_eos_suffix(take_streamable_text(), m_eos_token);
        if (!streamable.empty()) {
            break;
        }
    }

    if (is_last_token) {
        const auto stop_reason = encountered_max_tokens ? GenerationContext::GenerationFinishedReason::LENGTH
                                                        : GenerationContext::GenerationFinishedReason::STOP;
        m_done = true;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        const auto total_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_begin).count();

        // Without tools, every token was already streamed verbatim, so the final chunk carries no
        // content. With tools, per-token streaming holds back trailing bytes (tool-call prefix guard)
        // and buffers everything after a detected prefix; the residual that was never streamed is
        // exactly the parsed non-tool content beyond what already went out. Emit it in this final
        // chunk so a normal response keeps its full text and a tool-call response leaks no wrapper text.
        ToolCallParseResult parse_result;
        std::string final_content;
        if (m_tools_requested) {
            parse_result = m_tool_call_parser->parse(strip_eos_suffix(m_response_text, m_eos_token));
            final_content = residual_streamable_content(parse_result.content);
        }

        auto result = GenerationResponseFinal::createShared();
        result->model = m_model;
        result->created_at = get_current_time_formatted();
        if (m_return_as_message) {
            result->message = ChatMessage::createShared();
            result->message->role = "assistant";
            result->message->content = final_content;
            if (!parse_result.tool_calls.empty()) {
                const auto arguments_as_string = false;
                result->message->tool_calls = tool_calls_to_oatpp(parse_result.tool_calls, arguments_as_string,
                    m_object_mapper);
            }
        } else {
            result->response = final_content;
        }
        result->done = true;
        result->done_reason = stop_reason;
        result->total_duration = total_time_ns;
        result->eval_count = m_count;
        const auto response = m_object_mapper->writeToString(result).getValue("") + config::NDJSON_LINE_TERMINATOR;
        if (response.size() > bufferSize) {
            throw std::runtime_error("Buffer too small");
        }
        std::memcpy(buffer, response.data(), response.size());
        return response.size();
    }

    m_streamed_content += streamable;

    auto result = GenerationResponse::createShared();
    result->model = m_model;
    result->created_at = get_current_time_formatted();
    result->done = false;

    if (m_return_as_message) {
        result->message = ChatMessage::createShared();
        result->message->role = "assistant";
        result->message->content = std::move(streamable);
    } else {
        result->response = std::move(streamable);
    }
    const auto response = m_object_mapper->writeToString(result).getValue("") + config::NDJSON_LINE_TERMINATOR;
    if (response.size() > bufferSize) {
        throw std::runtime_error("Buffer too small");
    }
    std::memcpy(buffer, response.data(), response.size());
    return response.size();
}

} // namespace hailo_ollama

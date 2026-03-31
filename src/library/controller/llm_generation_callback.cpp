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
#include "utils/time.hpp"

namespace hailo_ollama
{

LLMGenerationReadCallback::LLMGenerationReadCallback(const std::string &model,
    const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &object_mapper,
    SyncGenerationContext::handle &&generation_context, hailort::genai::LLMGeneratorCompletion &&generator_completion,
    const bool return_as_message, const std::string &eos_token)
    : m_model(model), m_object_mapper(object_mapper), m_generation_context(std::move(generation_context)),
      m_generator_completion(std::move(generator_completion)), m_return_as_message(return_as_message),
      m_begin(std::chrono::steady_clock::now()), m_count(0ULL), m_done(false), m_response_text(), m_eos_token(eos_token)
{
}

oatpp::v_io_size LLMGenerationReadCallback::read(void *buffer, v_buff_size bufferSize, oatpp::async::Action &action)
{
    using GenerationStatus = hailort::genai::LLMGeneratorCompletion::Status;
    (void)action; // ignore action when using SimpleAPI

    if (m_done) {
        // Append assistant response to conversation history (with EOS stripped)
        m_generation_context->append_assistant_message(strip_eos_suffix(m_response_text, m_eos_token));
        return 0;
    }
    std::string token = m_generator_completion.read().expect("read failed!");

    // check status immediately after read to see if it's the last one
    const auto generation_status = m_generator_completion.generation_status();
    const auto is_last_token = (generation_status != GenerationStatus::GENERATING);
    const auto encountered_max_tokens = (generation_status == GenerationStatus::MAX_TOKENS_REACHED);

    if (!is_last_token) {
        m_response_text += token;
    }
    if (is_last_token) {
        const auto stop_reason = encountered_max_tokens ? GenerationContext::GenerationFinishedReason::LENGTH
                                                        : GenerationContext::GenerationFinishedReason::STOP;
        m_done = true;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        const auto total_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_begin).count();
        auto result = GenerationResponseFinal::createShared();
        result->model = m_model;
        result->created_at = get_current_time_formatted();
        if (m_return_as_message) {
            result->message = ChatMessage::createShared();
            result->message->role = "assistant";
            result->message->content = "";
        } else {
            result->response = "";
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

    ++m_count;

    // Strip EOS token from token if present
    token = strip_eos_suffix(token, m_eos_token);
    // If token becomes empty after stripping, skip sending it
    if (token.empty()) {
        return read(buffer, bufferSize, action); // Recursively read next token
    }

    auto result = GenerationResponse::createShared();
    result->model = m_model;
    result->created_at = get_current_time_formatted();
    result->done = false;

    if (m_return_as_message) {
        result->message = ChatMessage::createShared();
        result->message->role = "assistant";
        result->message->content = std::move(token);
    } else {
        result->response = std::move(token);
    }
    const auto response = m_object_mapper->writeToString(result).getValue("") + config::NDJSON_LINE_TERMINATOR;
    if (response.size() > bufferSize) {
        throw std::runtime_error("Buffer too small");
    }
    std::memcpy(buffer, response.data(), response.size());
    return response.size();
}

} // namespace hailo_ollama

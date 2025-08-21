/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
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

#include "dto/DTOs.hpp"
#include "generation_context/generation_context.hpp"
#include "utils/time.hpp"

LLMGenerationReadCallback::LLMGenerationReadCallback(
    const std::string& model,
    const std::vector<std::string>& stop_tokens,
    const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& object_mapper,
    SyncGenerationContext::handle&& generation_context,
    hailort::genai::LLMGeneratorCompletion&& generator_completion,
    const bool return_as_message
) :
    m_model(model),
    m_stop_tokens(stop_tokens),
    m_object_mapper(object_mapper),
    m_generation_context(std::move(generation_context)),
    m_generator_completion(std::move(generator_completion)),
    m_return_as_message(return_as_message),
    m_begin(std::chrono::steady_clock::now()),
    m_count(0ULL),
    m_done(false),
    m_response() {}

oatpp::v_io_size LLMGenerationReadCallback::read(
    void* buffer,
    v_buff_size bufferSize,
    oatpp::async::Action& action
) {
    using GenerationStatus = hailort::genai::LLMGeneratorCompletion::Status;
    (void)action;  // ignore action when using SimpleAPI

    if (m_done) {
        m_generation_context->append_last_prompt(m_response.str());
        return 0;
    }
    std::string token = m_generator_completion.read().expect("read failed!");
    // check max_tokens first because stop_tokens alters the status
    const auto encountered_max_tokens =
        (m_generator_completion.generation_status()
         == GenerationStatus::MAX_TOKENS_REACHED);

    if (!(m_generator_completion.generation_status()
          != GenerationStatus::GENERATING)) {
        m_response << token;
    }
    // check if encountered a stop token
    const auto stop_token_it =
        std::find(m_stop_tokens.cbegin(), m_stop_tokens.cend(), token);
    if (stop_token_it != m_stop_tokens.cend()) {
        // stop token found -> exhaust the completion
        while (
            m_generator_completion.generation_status()
            // we would like to skip these tokens but current API doesn't allow that
            == GenerationStatus::GENERATING
        ) {
            const auto token =
                m_generator_completion.read().expect("read failed!");
            const auto is_last_token =
                (m_generator_completion.generation_status()
                 != GenerationStatus::GENERATING);
            if (!is_last_token) {
                m_response << token;
            }
        }
    }
    // check status immediately after read to see if it's the last one
    const auto generation_status = m_generator_completion.generation_status();
    const auto is_last_token =
        (generation_status != GenerationStatus::GENERATING);
    if (is_last_token) {
        const auto stop_reason = encountered_max_tokens ? "length" : "stop";
        m_done = true;
        std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();
        const auto total_time_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_begin)
                .count();
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
        const auto response =
            m_object_mapper->writeToString(result).getValue("") + "\r\n";
        if (response.size() > bufferSize) {
            throw std::runtime_error("Buffer too small");
        }
        std::memcpy(buffer, response.data(), response.size());
        return response.size();
    }

    ++m_count;
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
    const auto response =
        m_object_mapper->writeToString(result).getValue("") + "\r\n";
    if (response.size() > bufferSize) {
        throw std::runtime_error("Buffer too small");
    }
    std::memcpy(buffer, response.data(), response.size());
    return response.size();
}

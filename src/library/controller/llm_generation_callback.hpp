/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
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

class LLMGenerationReadCallback: public oatpp::data::stream::ReadCallback {
  public:
    LLMGenerationReadCallback(
        const std::string& model,
        const std::vector<std::string>& stop_tokens,
        const std::shared_ptr<oatpp::data::mapping::ObjectMapper>&
            object_mapper,
        SyncGenerationContext::handle&& generation_context,
        hailort::genai::LLMGeneratorCompletion&& generator_completion,
        const bool return_as_message
    );

    oatpp::v_io_size read(
        void* buffer,
        v_buff_size bufferSize,
        oatpp::async::Action& action
    ) override;

  private:
    std::string m_model;
    std::vector<std::string> m_stop_tokens;
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> m_object_mapper;
    SyncGenerationContext::handle m_generation_context;
    hailort::genai::LLMGeneratorCompletion m_generator_completion;
    bool m_return_as_message;
    std::chrono::steady_clock::time_point m_begin;

    uint64_t m_count;
    bool m_done;
    std::stringstream m_response;
};

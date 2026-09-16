/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file dto_json.hpp
 * @brief Conversions between oatpp DTO values and JSON strings / nlohmann::json
 **/

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <oatpp/Types.hpp>
#include <oatpp/data/mapping/ObjectMapper.hpp>

#include "dto/DTOs.hpp"
#include "tool_parsers/tool_call_parser.hpp"

namespace hailo_ollama
{

// Builds the oatpp tool_calls field for the parsed tool calls. The native Ollama shape keeps
// arguments as an object; the OpenAI shape (arguments_as_string == true) serializes them to a string
// and tags each call with {"type": "function"}.
oatpp::Vector<oatpp::Fields<oatpp::Any>> tool_calls_to_oatpp(const std::vector<ToolCall> &tool_calls,
    bool arguments_as_string, const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &mapper);

// Serializes each tool object to one JSON string, preserving the nested
// {"type": "function", "function": {...}} shape the model template expects.
std::vector<std::string> tools_to_json_strings(const oatpp::Vector<oatpp::Fields<oatpp::Any>> &tools,
    const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &mapper);

// Serializes a whole incoming chat message (role, content, tool_calls, ...) to one JSON string.
template <typename Message>
std::string message_to_json_string(const oatpp::Object<Message> &message,
    const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &mapper)
{
    return mapper->writeToString(message).getValue("");
}

} // namespace hailo_ollama

/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file dto_json.cpp
 * @brief Conversions between oatpp DTO values and JSON strings / nlohmann::json
 **/

#include "utils/dto_json.hpp"

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <oatpp/Types.hpp>
#include <oatpp/data/mapping/ObjectMapper.hpp>

namespace hailo_ollama
{

oatpp::Vector<oatpp::Fields<oatpp::Any>> tool_calls_to_oatpp(const std::vector<ToolCall> &tool_calls,
    bool arguments_as_string, const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &mapper)
{
    auto array = nlohmann::json::array();
    for (const auto &tool_call : tool_calls) {
        nlohmann::json function = {{"name", tool_call.name}};
        function["arguments"] = arguments_as_string ? nlohmann::json(tool_call.arguments.dump()) : tool_call.arguments;
        nlohmann::json element = {{"function", std::move(function)}};
        if (arguments_as_string) {
            element["type"] = "function";
        }
        array.push_back(std::move(element));
    }
    return mapper->readFromString<oatpp::Vector<oatpp::Fields<oatpp::Any>>>(array.dump());
}

std::vector<std::string> tools_to_json_strings(const oatpp::Vector<oatpp::Fields<oatpp::Any>> &tools,
    const std::shared_ptr<oatpp::data::mapping::ObjectMapper> &mapper)
{
    std::vector<std::string> tool_json_strings;
    if (!tools) {
        return tool_json_strings;
    }
    tool_json_strings.reserve(tools->size());
    for (const auto &tool : *tools) {
        tool_json_strings.push_back(mapper->writeToString(tool).getValue(""));
    }
    return tool_json_strings;
}

} // namespace hailo_ollama

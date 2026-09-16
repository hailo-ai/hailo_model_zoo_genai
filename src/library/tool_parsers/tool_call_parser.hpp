/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file tool_call_parser.hpp
 * @brief Template-derived tool-call parser and shared types
 **/

#pragma once

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace hailo_ollama
{

struct ToolCall {
    std::string name;
    nlohmann::json arguments;
};

struct ToolCallParseResult {
    std::string content;
    std::vector<ToolCall> tool_calls;
};

// Parses content and tool calls from complete model output. At construction it probes the chat
// template to learn the tool-call wrapper (fallback: <tool_call> / bare-JSON). No-throw: anything
// that doesn't parse as a tool call is returned as content.
class ToolCallParser
{
public:
    explicit ToolCallParser(const std::string &chat_template);

    static std::optional<bool> does_template_support_tools(const std::string &chat_template);

    ToolCallParseResult parse(const std::string &model_output) const;

    // Literal that opens a tool call (e.g. "<tool_call>"); while streaming, a partial match is held back
    // so it never leaks as content. Empty means no wrapper to hold back.
    std::string tool_call_start_marker() const;

private:
    std::string m_call_start;
    std::string m_call_end;
};

} // namespace hailo_ollama

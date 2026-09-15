/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file tool_call_parser.cpp
 * @brief Template-derived tool-call parser
 **/

#include "tool_parsers/tool_call_parser.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <minja/chat-template.hpp>
#include <nlohmann/json.hpp>

namespace hailo_ollama
{

namespace
{

static constexpr const char *DEFAULT_TOOL_CALL_PREFIX = "<tool_call>";
static constexpr const char *DEFAULT_TOOL_CALL_SUFFIX = "</tool_call>";

// Distinct sentinels for each field of the probe tool call so the surrounding literals can be located
// unambiguously in the rendered template, regardless of where the template places name vs arguments.
static constexpr const char *PROBE_TOOL_NAME = "hailo_probe_tool_name";
static constexpr const char *PROBE_ARG_KEY = "hailo_probe_arg_key";
static constexpr const char *PROBE_ARG_VALUE = "hailo_probe_arg_value";

std::string trim(const std::string &text)
{
    const auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
    auto begin = std::find_if_not(text.begin(), text.end(), is_space);
    auto end = std::find_if_not(text.rbegin(), text.rend(), is_space).base();
    if (begin >= end) {
        return "";
    }
    return std::string(begin, end);
}

nlohmann::json parse_json_no_throw(const std::string &text)
{
    return nlohmann::json::parse(text, nullptr, /* allow_exceptions = */ false);
}

// Returns the index of the '}' matching the '{' at open_brace, or std::string::npos if unbalanced.
size_t find_matching_brace(const std::string &text, size_t open_brace)
{
    size_t depth = 0;
    for (size_t i = open_brace; i < text.size(); ++i) {
        if ('{' == text[i]) {
            ++depth;
        } else if ('}' == text[i]) {
            --depth;
            if (0 == depth) {
                return i;
            }
        }
    }
    return std::string::npos;
}

// Returns the index of the '{' opening the innermost JSON object that encloses pos, or npos if pos is
// not inside any object.
size_t find_enclosing_open_brace(const std::string &text, size_t pos)
{
    size_t depth = 0;
    for (size_t i = pos; i-- > 0;) {
        if ('}' == text[i]) {
            ++depth;
        } else if ('{' == text[i]) {
            if (0 == depth) {
                return i;
            }
            --depth;
        }
    }
    return std::string::npos;
}

// Removes "<|...|>" special-token sequences (e.g. "<|python_tag|>", "<|eom_id|>") so that surrounding
// wrapper tokens left after extracting a bare-JSON tool call do not leak into the content. Plain text is
// returned unchanged.
std::string strip_special_tokens(const std::string &text)
{
    static constexpr const char *TOKEN_OPEN = "<|";
    static constexpr const char *TOKEN_CLOSE = "|>";
    std::string stripped;
    size_t cursor = 0;
    while (cursor < text.size()) {
        const auto open = text.find(TOKEN_OPEN, cursor);
        if (std::string::npos == open) {
            stripped.append(text, cursor, std::string::npos);
            break;
        }
        const auto close = text.find(TOKEN_CLOSE, open + std::char_traits<char>::length(TOKEN_OPEN));
        if (std::string::npos == close) {
            stripped.append(text, cursor, std::string::npos);
            break;
        }
        stripped.append(text, cursor, open - cursor);
        cursor = close + std::char_traits<char>::length(TOKEN_CLOSE);
    }
    return stripped;
}

size_t common_prefix_len(const std::string &a, const std::string &b)
{
    const auto limit = std::min(a.size(), b.size());
    size_t i = 0;
    while ((i < limit) && (a[i] == b[i])) {
        ++i;
    }
    return i;
}

// Reads name + arguments from a parsed tool-call JSON object. The arguments may be carried under either
// "arguments" or "parameters", as an object or a JSON string.
std::optional<ToolCall> tool_call_from_json(const nlohmann::json &object)
{
    if (!object.is_object() || !object.contains("name") || !object["name"].is_string()) {
        return std::nullopt;
    }
    ToolCall out;
    out.name = object["name"].get<std::string>();
    out.arguments = nlohmann::json::object();

    const char *args_key = nullptr;
    if (object.contains("arguments")) {
        args_key = "arguments";
    } else if (object.contains("parameters")) {
        args_key = "parameters";
    }
    if (nullptr != args_key) {
        const auto &arguments = object[args_key];
        if (arguments.is_string()) {
            const auto parsed = parse_json_no_throw(arguments.get<std::string>());
            out.arguments = parsed.is_object() ? parsed : nlohmann::json::object();
        } else if (arguments.is_object()) {
            out.arguments = arguments;
        }
    }
    return out;
}

// The generation-prompt common prefix can split a markup token: e.g. a prompt header ending
// "<|Assistant|><think>" while the tool-call render continues "<|Assistant|><|tool_calls_begin|>",
// so the shared leading '<' of the next token is stripped. Back the boundary up to the start of that
// partially-shared token so the emitted region begins with a whole token.
size_t emitted_region_start(const std::string &probe, const std::string &gen_prompt)
{
    auto cut = common_prefix_len(probe, gen_prompt);
    if ((cut > 0) && ('<' == probe[cut - 1])) {
        --cut;
    }
    return cut;
}

std::optional<std::string> render_template(minja::chat_template &tmpl, const nlohmann::ordered_json &messages,
    bool add_generation_prompt)
{
    minja::chat_template_inputs inputs;
    inputs.messages = messages;
    inputs.tools = nlohmann::ordered_json::array();
    inputs.add_generation_prompt = add_generation_prompt;
    try {
        return tmpl.apply(inputs);
    } catch (const std::exception &) {
        return std::nullopt;
    }
}

std::optional<std::string> render_probe(minja::chat_template &tmpl, const std::string &gen_prompt_render)
{
    const nlohmann::ordered_json arguments = {{PROBE_ARG_KEY, PROBE_ARG_VALUE}};

    nlohmann::ordered_json tool_call = {
        {"type", "function"}, {"function", {{"name", PROBE_TOOL_NAME}, {"arguments", arguments}}}};
    nlohmann::ordered_json assistant_message = {
        {"role", "assistant"}, {"content", nullptr}, {"tool_calls", nlohmann::ordered_json::array({tool_call})}};
    nlohmann::ordered_json user_message = {{"role", "user"}, {"content", "hi"}};
    nlohmann::ordered_json messages = nlohmann::ordered_json::array({user_message, assistant_message});

    const auto rendered = render_template(tmpl, messages, /* add_generation_prompt = */ false);
    if (!rendered) {
        return std::nullopt;
    }
    const auto &rendered_str = *rendered;
    if ((std::string::npos == rendered_str.find(PROBE_TOOL_NAME)) ||
        (std::string::npos == rendered_str.find(PROBE_ARG_VALUE))) {
        return std::nullopt;
    }

    return rendered_str.substr(emitted_region_start(rendered_str, gen_prompt_render));
}

// Derives the call_start/call_end literals from the model-emitted probe region. Supported only when the
// name sentinel lies inside the JSON object that also holds the arguments key (Qwen/Llama shape). A
// name-outside-JSON shape returns nullopt here to fall back to the default delimiters plus bare-JSON
// detection.
std::optional<std::pair<std::string, std::string>> derive_from_emitted(const std::string &emitted)
{
    const auto name_pos = emitted.find(PROBE_TOOL_NAME);
    const auto key_pos = emitted.find(PROBE_ARG_KEY);
    if ((std::string::npos == name_pos) || (std::string::npos == key_pos)) {
        return std::nullopt;
    }

    const auto name_object_open = find_enclosing_open_brace(emitted, name_pos);
    const auto name_object_close =
        (std::string::npos == name_object_open) ? std::string::npos : find_matching_brace(emitted, name_object_open);
    const bool name_in_json = (std::string::npos != name_object_close) && (key_pos > name_object_open) &&
        (key_pos < name_object_close);
    if (!name_in_json) {
        return std::nullopt;
    }

    return std::make_pair(emitted.substr(0, name_object_open), emitted.substr(name_object_close + 1));
}

// Returns the number of bytes to skip past pos: the longest prefix of call_end that text matches there.
// The derived call_end may carry a trailing EOS token (e.g. "</tool_call><|im_end|>") that real output
// lacks once the EOS is stripped, so a prefix match still consumes the wrapper closer.
size_t consume_call_end_prefix(const std::string &text, size_t pos, const std::string &call_end)
{
    size_t matched = 0;
    while ((matched < call_end.size()) && ((pos + matched) < text.size()) &&
        (text[pos + matched] == call_end[matched])) {
        ++matched;
    }
    return matched;
}

// Detects a bare JSON tool call (no wrapper): a leading array of objects, or a single name-bearing JSON
// object located behind any leading tokens. Used as the fallback for wrapper-less families (Llama) and
// whenever no delimited span is found. A trailing call_end (e.g. an EOS token captured during derivation)
// is tolerated and stripped. The single-object path scans for the first '{' so a generation-time wrapper
// token (e.g. Llama's "<|python_tag|>" prefix or a trailing "<|eom_id|>") that the template never renders
// does not defeat detection; the surrounding text is returned as content.
ToolCallParseResult parse_bare_json(const std::string &text, const std::string &call_end)
{
    ToolCallParseResult result;
    const auto trimmed_text = trim(text);
    result.content = trimmed_text;

    auto trimmed = trimmed_text;
    if (!call_end.empty() && (trimmed.size() >= call_end.size()) &&
        (0 == trimmed.compare(trimmed.size() - call_end.size(), call_end.size(), call_end))) {
        trimmed = trim(trimmed.substr(0, trimmed.size() - call_end.size()));
    }
    if (trimmed.empty()) {
        return result;
    }

    if ('[' == trimmed.front()) {
        const auto parsed = parse_json_no_throw(trimmed);
        if (parsed.is_discarded() || !parsed.is_array()) {
            return result;
        }
        std::vector<ToolCall> calls;
        for (const auto &element : parsed) {
            auto tool_call = tool_call_from_json(element);
            if (!tool_call) {
                return result;
            }
            calls.push_back(std::move(*tool_call));
        }
        if (!calls.empty()) {
            result.content = "";
            result.tool_calls = std::move(calls);
        }
        return result;
    }

    const auto open_brace = trimmed.find('{');
    if (std::string::npos == open_brace) {
        return result;
    }
    const auto close_brace = find_matching_brace(trimmed, open_brace);
    if (std::string::npos == close_brace) {
        return result;
    }

    const auto object_text = trimmed.substr(open_brace, close_brace - open_brace + 1);
    const auto parsed = parse_json_no_throw(object_text);
    if (!parsed.is_discarded()) {
        auto tool_call = tool_call_from_json(parsed);
        if (tool_call) {
            const auto surrounding = trimmed.substr(0, open_brace) + trimmed.substr(close_brace + 1);
            result.content = trim(strip_special_tokens(surrounding));
            result.tool_calls.push_back(std::move(*tool_call));
        }
    }
    return result;
}

// Probes the chat template to learn the tool-call wrapper. Returns the derived {call_start, call_end} on
// success, or the default delimiters on any failure
std::pair<std::string, std::string> derive_delimiters(const std::string &chat_template)
{
    try {
        minja::chat_template tmpl(chat_template, "", "");

        nlohmann::ordered_json user_message = {{"role", "user"}, {"content", "hi"}};
        nlohmann::ordered_json gen_messages = nlohmann::ordered_json::array({user_message});
        const auto gen_prompt_render = render_template(tmpl, gen_messages, /* add_generation_prompt = */ true);
        if (gen_prompt_render) {
            const auto probe = render_probe(tmpl, *gen_prompt_render);
            if (probe) {
                if (auto derived = derive_from_emitted(*probe)) {
                    return *derived;
                }
            }
        }
    } catch (const std::exception &) {
        // Fall through to the default delimiters.
    }
    return {DEFAULT_TOOL_CALL_PREFIX, DEFAULT_TOOL_CALL_SUFFIX};
}

} // namespace

std::optional<bool> ToolCallParser::does_template_support_tools(const std::string &chat_template)
{
    try {
        minja::chat_template tmpl(chat_template, "", "");
        return tmpl.original_caps().supports_tools;
    } catch (const std::exception &) {
        return std::nullopt;
    }
}

ToolCallParser::ToolCallParser(const std::string &chat_template)
{
    std::tie(m_call_start, m_call_end) = derive_delimiters(chat_template);
}

ToolCallParseResult ToolCallParser::parse(const std::string &model_output) const
{
    ToolCallParseResult result;

    std::string content;
    size_t cursor = 0;
    bool found_wrapped_call = false;

    const std::string &call_start = m_call_start;
    const std::string &call_end = m_call_end;

    while (!call_start.empty()) {
        const auto start_pos = model_output.find(call_start, cursor);
        if (std::string::npos == start_pos) {
            break;
        }
        const auto json_search_start = start_pos + call_start.size();

        const auto open_brace = model_output.find('{', json_search_start);
        if (std::string::npos == open_brace) {
            break;
        }
        const auto close_brace = find_matching_brace(model_output, open_brace);
        if (std::string::npos == close_brace) {
            break;
        }

        const auto json_text = model_output.substr(open_brace, close_brace - open_brace + 1);
        const auto parsed = parse_json_no_throw(json_text);

        auto tool_call = parsed.is_discarded() ? std::optional<ToolCall>{} : tool_call_from_json(parsed);
        if (!tool_call) {
            content += model_output.substr(cursor, (close_brace + 1) - cursor);
            cursor = close_brace + 1;
            continue;
        }

        content += model_output.substr(cursor, start_pos - cursor);
        result.tool_calls.push_back(std::move(*tool_call));
        found_wrapped_call = true;

        size_t next = close_brace + 1;
        if (!call_end.empty()) {
            next += consume_call_end_prefix(model_output, next, call_end);
        }
        cursor = next;
    }

    content += model_output.substr(cursor);
    result.content = trim(content);

    // Bare-JSON fallback is only for wrapper-less families (empty call_start, e.g. Llama). A wrapper family
    // (e.g. Qwen) that emits JSON without its wrapper yields raw content, not a tool call.
    if (found_wrapped_call || !call_start.empty()) {
        return result;
    }

    return parse_bare_json(model_output, call_end);
}

std::string ToolCallParser::tool_call_start_marker() const { return m_call_start; }

} // namespace hailo_ollama

/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file json_utils.hpp
 * @brief JSON string escaping utilities (RFC 8259 compliant)
 **/

#pragma once

#include <cstdint>
#include <string>

namespace hailo_ollama
{

// Escapes a string for safe embedding inside a JSON quoted value, per RFC 8259.
// Handles: \, ", \n, \r, \t, and all control characters (U+0000-U+001F).
inline std::string escape_json_string(const std::string &text)
{
    static constexpr char HEX_DIGITS[] = "0123456789abcdef";

    std::string result;
    result.reserve(text.size());

    for (const char c : text) {
        switch (c) {
        case '\\':
            result += "\\\\";
            break;
        case '"':
            result += "\\\"";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        default:
            // Control characters U+0000 through U+001F (excluding those handled above)
            if (static_cast<unsigned char>(c) < 0x20) {
                result += "\\u00";
                result += HEX_DIGITS[(static_cast<unsigned char>(c) >> 4) & 0x0F];
                result += HEX_DIGITS[static_cast<unsigned char>(c) & 0x0F];
            } else {
                result += c;
            }
            break;
        }
    }

    return result;
}

} // namespace hailo_ollama

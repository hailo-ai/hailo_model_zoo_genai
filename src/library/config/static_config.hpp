/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file static_config.hpp
 * @brief Configuration available at compile time
 **/

#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace hailo_ollama
{
namespace config
{

constexpr size_t HASH_BUFFER_SIZE = 4 * 1024 * 1024; // 4 MB
constexpr int_fast32_t OUTPUT_FILE_STREAM_UPDATE_EVERY = 1000;
constexpr auto HAILO_OLLAMA_DEFAULT_KEEP_ALIVE = std::chrono::minutes(5);

constexpr uint16_t MAX_PORT_NUMBER = std::numeric_limits<uint16_t>::max(); // 65535
constexpr uint16_t MIN_PORT_NUMBER = 1;

// NDJSON line terminator (CRLF per HTTP/1.1 chunked transfer encoding)
static const std::string NDJSON_LINE_TERMINATOR = "\r\n";

// Environment variables
static const std::string OLLAMA_HOST_ENV_VAR = "OLLAMA_HOST";
static const std::string HAILO_OLLAMA_VDEVICE_GROUP_ID_ENV_VAR = "HAILO_OLLAMA_VDEVICE_GROUP_ID";
static const std::string HAILO_OLLAMA_LIBRARY_HOST_ENV_VAR = "HAILO_OLLAMA_LIBRARY_HOST"; // Internal use only

struct ConnectionDetails {
    std::string host;
    uint16_t port;
};

// Default configuration values
static const ConnectionDetails DEFAULT_SERVER_OLLAMA_CONNECTION = { "0.0.0.0", 8000 };
static const ConnectionDetails DEFAULT_LIBRARY_OLLAMA_CONNECTION = { "dev-public.hailo.ai", 443 };

// TODO: HRT-19806 - Remove this and improve poll time logic
constexpr uint16_t DEFAULT_MAIN_POLL_TIME_MS = 200;

} // namespace config
} // namespace hailo_ollama

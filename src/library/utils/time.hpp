/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file time.hpp
 * @brief Utilities for formatting timestamps
 **/

#pragma once

#include <chrono>
#include <filesystem>
#include <string>

namespace hailo_ollama
{

std::string to_iso_8601(std::chrono::time_point<std::chrono::system_clock> t, const std::string &suffix);

std::string to_iso_8601(std::chrono::time_point<std::chrono::steady_clock> t, const std::string &suffix);

std::string to_iso_8601(std::filesystem::file_time_type t, const std::string &suffix);
std::string get_current_time_formatted();
struct tm get_utc_time_struct(time_t epoch_seconds);

} // namespace hailo_ollama

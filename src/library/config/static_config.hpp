/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file static_config.hpp
 * @brief Configuration available at compile time
 **/

#pragma once

#include <chrono>
#include <cstdint>

namespace config {
constexpr size_t hash_buffer_size = 4 * 1024 * 1024;  // 4 MB
constexpr int_fast32_t output_file_stream_update_every = 1000;
constexpr auto generation_context_device_switch_sleep_time =
    std::chrono::seconds(2);
constexpr auto controller_default_keep_alive = std::chrono::minutes(5);
constexpr auto controller_show_parameter_width = 30;
}  // namespace config

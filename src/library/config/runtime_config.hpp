/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file runtime_config.hpp
 * @brief Configuration read during execution
 **/

#pragma once

#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>

struct ConnectionDetails {
    std::string host;
    uint16_t port;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConnectionDetails, host, port)

struct RuntimeConfig {
    ConnectionDetails server;
    ConnectionDetails library;
    uint16_t main_poll_time_ms;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    RuntimeConfig,
    server,
    library,
    main_poll_time_ms
)

/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file path.cpp
 * @brief path utils implementation
 **/

#include "utils/path.hpp"

#include <cstdlib>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

fs::path data_home() {
    const auto value = std::getenv(XDG_DATA_HOME);
    if (value != nullptr && std::string_view(value) != "") {
        return value;
    }

    auto home = std::getenv(HOME);
    return fs::path(home != nullptr ? home : "") / XDG_DATA_HOME_SUFFIX;
}

fs::path config_home() {
    const auto value = std::getenv(XDG_CONFIG_HOME);
    if (value != nullptr && std::string_view(value) != "") {
        return value;
    }

    auto home = std::getenv(HOME);
    return fs::path(home != nullptr ? home : "") / XDG_CONFIG_HOME_SUFFIX;
}

std::string system_config_home() {
    const auto value = std::getenv(XDG_CONFIG_DIRS);
    if (value == nullptr || std::string_view(value) == "") {
        return XDG_CONFIG_DIRS_DEFAULT;
    }
    return value;
}

std::string system_data_home() {
    const auto value = std::getenv(XDG_DATA_DIRS);
    if (value == nullptr || std::string_view(value) == "") {
        return XDG_DATA_DIRS_DEFAULT;
    }
    return value;
}

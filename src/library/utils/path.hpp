/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file path.hpp
 * @brief Utilities for finding data&config directories
 **/

#pragma once

#include <filesystem>
#include <string>

constexpr auto HOME {"HOME"};
constexpr auto XDG_DATA_HOME {"XDG_DATA_HOME"};
constexpr auto XDG_DATA_DIRS {"XDG_DATA_DIRS"};
constexpr auto XDG_CONFIG_HOME {"XDG_CONFIG_HOME"};
constexpr auto XDG_CONFIG_DIRS {"XDG_CONFIG_DIRS"};
constexpr auto XDG_CACHE_HOME {"XDG_CACHE_HOME"};
constexpr auto XDG_RUNTIME_DIR {"XDG_RUNTIME_DIR"};

constexpr auto XDG_DATA_HOME_SUFFIX {".local/share"};
constexpr auto XDG_CONFIG_HOME_SUFFIX {".config"};
constexpr auto XDG_CACHE_HOME_SUFFIX {".cache"};

constexpr auto XDG_CONFIG_DIRS_DEFAULT {"/etc/xdg"};
constexpr auto XDG_DATA_DIRS_DEFAULT {"/usr/share:/usr/local/share"};

constexpr auto HAILO_DIR_NAME {"hailo-ollama"};
constexpr auto HAILO_CONFIG_NAME {"hailo-ollama.json"};
constexpr auto HAILO_MODELS {"models"};
constexpr auto HAILO_BLOB_DIR_NAME {"blob"};
constexpr auto HAILO_MODEL_MANIFEST {"manifests"};

std::filesystem::path data_home();
std::string system_data_home();

std::filesystem::path config_home();
std::string system_config_home();

std::filesystem::path model_manifest();

std::filesystem::path data_dir();

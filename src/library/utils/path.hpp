/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file path.hpp
 * @brief Utilities for finding data directories
 **/

#pragma once

#include <filesystem>
#include <string>

namespace hailo_ollama
{

#ifdef _WIN32
constexpr auto HOME{"USERPROFILE"};
constexpr auto XDG_DATA_HOME{"LOCALAPPDATA"};
constexpr auto XDG_DATA_DIRS{"PROGRAMDATA"};

constexpr auto XDG_DATA_HOME_SUFFIX{"AppData/Local"};
constexpr auto XDG_DATA_DIRS_DEFAULT{"C:\\ProgramData"};

constexpr auto PATH_DELIMITER{";"};
#else
constexpr auto HOME{"HOME"};
constexpr auto XDG_DATA_HOME{"XDG_DATA_HOME"};
constexpr auto XDG_DATA_DIRS{"XDG_DATA_DIRS"};

constexpr auto XDG_DATA_HOME_SUFFIX{".local/share"};
constexpr auto XDG_DATA_DIRS_DEFAULT{"/usr/share:/usr/local/share"};

constexpr auto PATH_DELIMITER{":"};
#endif

constexpr auto HAILO_DIR_NAME{"hailo-ollama"};
constexpr auto HAILO_MODELS{"models"};
constexpr auto HAILO_BLOB_DIR_NAME{"blob"};
constexpr auto HAILO_MODEL_MANIFEST{"manifests"};

std::filesystem::path data_home();
std::string system_data_home();

} // namespace hailo_ollama

/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file store.hpp
 * @brief Interface for managing models
 **/

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "utils/interface.hpp"

namespace hailo_ollama
{

struct ModelInfo {
    std::string name;
    std::string hef_resource;
    std::string details;
    std::string license;
};

class ModelStore : Interface
{
public:
    virtual std::optional<ModelInfo> get_model(const std::string &name) = 0;
    virtual std::vector<std::string> get_model_names() = 0;
};

} // namespace hailo_ollama

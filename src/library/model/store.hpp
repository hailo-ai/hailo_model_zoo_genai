/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
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

struct TemplateParamsInfo {
    std::string chat_template;
    std::string bos_token;
    std::string eos_token;
};

struct GenerationParamsInfo {
    std::optional<float> temperature;
    std::optional<float> top_p;
    std::optional<uint32_t> top_k;
    std::optional<float> frequency_penalty;
    std::vector<std::string> stop_tokens;
};

struct ModelInfo {
    std::string name;
    std::string hef_resource;
    TemplateParamsInfo template_params;
    GenerationParamsInfo generation_params;
    std::string details;
    std::string license;
};

class ModelStore: Interface {
  public:
    virtual std::optional<ModelInfo> get_model(const std::string& name) = 0;
    virtual std::vector<std::string> get_model_names() = 0;
};

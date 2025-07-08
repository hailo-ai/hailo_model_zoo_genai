/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file simple_store.hpp
 * @brief Class for managing models
 **/

#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "model/store.hpp"

class SimpleModelStore: public ModelStore {
  private:
    std::map<std::string, ModelInfo> m_models;

  public:
    explicit SimpleModelStore(const std::filesystem::path& path);

    std::optional<ModelInfo> get_model(const std::string& name) override;
    std::vector<std::string> get_model_names() override;
};

/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file test_model_store.hpp
 * @brief ModelStore implementation for tests
 **/

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "model/store.hpp"

class TestModelStore: public ModelStore {
  public:
    std::optional<ModelInfo> get_model(const std::string& name) override;

    std::vector<std::string> get_model_names() override;
};

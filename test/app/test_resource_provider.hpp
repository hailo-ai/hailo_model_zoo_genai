/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file test_resource_provider.hpp
 * @brief ResourceProvider implementation for tests
 **/

#pragma once

#include <filesystem>
#include <string>

#include "model/resource.hpp"

class TestResourceProvider: public ResourceProvider {
  private:
    std::filesystem::path m_test_hef;

  public:
    explicit TestResourceProvider(const std::filesystem::path& test_hef);

    std::filesystem::path get_resource(const std::string& resource) override;
    void pull_resource(const std::string& resource) override;

    void pull_resource(
        const std::string& resource,
        const std::shared_ptr<PullReadCallback::EventQueue>& queue
    ) override;
};

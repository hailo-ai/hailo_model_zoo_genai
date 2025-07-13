/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file test_resource_provider.cpp
 * @brief TestResourceProvider implementation
 **/

#include "test_resource_provider.hpp"

#include <filesystem>
#include <string>

TestResourceProvider::TestResourceProvider(
    const std::filesystem::path& test_hef
) :
    m_test_hef(test_hef) {}

std::filesystem::path
TestResourceProvider::get_resource(const std::string& resource) {
    if (resource == "qwen2_base.hef") {
        return m_test_hef;
    }
    return "";
}

void TestResourceProvider::pull_resource(const std::string& resource) {}

void TestResourceProvider::pull_resource(
    const std::string& resource,
    const std::shared_ptr<PullReadCallback::EventQueue>& queue
) {}

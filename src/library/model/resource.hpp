/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file resource.hpp
 * @brief Interface for pulling&getting resources
 **/

#pragma once

#include <filesystem>
#include <string>

#include "controller/pull_callback.hpp"
#include "utils/interface.hpp"

namespace hailo_ollama
{

class ResourceProvider : Interface
{
public:
    virtual std::filesystem::path get_resource(const std::string &resource) = 0;
    // Throws std::runtime_error on failure
    virtual void pull_resource(const std::string &resource) = 0;
    virtual void pull_resource(const std::string &resource,
        const std::shared_ptr<PullReadCallback::EventQueue> &queue) = 0;
};

} // namespace hailo_ollama

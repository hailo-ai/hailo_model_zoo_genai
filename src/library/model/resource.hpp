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
    virtual std::filesystem::path get_resource(const std::string &hef_filename) = 0;
    // Throws std::runtime_error on failure
    virtual std::string pull_resource(const std::string &hef_filename) = 0;
    virtual std::string pull_resource(const std::string &hef_filename,
        const std::shared_ptr<PullReadCallback::EventQueue> &queue) = 0;
    // Removes the cached HEF and any local bookkeeping beside it. Returns whether the cached HEF was removed.
    virtual bool remove_resource(const std::string &hef_filename) = 0;
};

} // namespace hailo_ollama

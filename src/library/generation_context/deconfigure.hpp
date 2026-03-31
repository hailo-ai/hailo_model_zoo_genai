/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file deconfigure.hpp
 * @brief Class for releasing Hailo Device when idle
 **/

#pragma once

#include <memory>

#include "generation_context/generation_context.hpp"

namespace hailo_ollama
{

class Deconfigure
{
public:
    explicit Deconfigure(const std::shared_ptr<SyncGenerationContext> &context);
    void deconfigure_loop();

private:
    std::shared_ptr<SyncGenerationContext> m_context;
};

} // namespace hailo_ollama

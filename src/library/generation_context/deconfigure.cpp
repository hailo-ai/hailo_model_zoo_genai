/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file deconfigure.cpp
 * @brief Class for releasing Hailo Device when idle
 **/

#include "generation_context/deconfigure.hpp"

#include <memory>

#include "generation_context/generation_context.hpp"

Deconfigure::Deconfigure(
    const std::shared_ptr<SyncGenerationContext>& context
) :
    m_context(context) {}

void Deconfigure::deconfigure_loop() {
    auto generation_context = m_context->lock();
    auto& lock = generation_context.get_deleter().as_lock();
    while (true) {
        const auto status = generation_context->wait_expiry(lock);
        if (status != ExpiryWaitStatus::SUCCESS) {
            break;
        }
        generation_context->reset();
    }
}

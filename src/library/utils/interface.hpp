/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file interface.hpp
 * @brief Utility for defining interfaces
 **/

#pragma once

namespace hailo_ollama
{

class Interface
{
public:
    Interface() = default;
    Interface(const Interface &) = delete;
    Interface(Interface &&) = delete;
    Interface &operator=(const Interface &) = delete;
    Interface &operator=(Interface &&) = delete;

    virtual ~Interface() = default;
};

} // namespace hailo_ollama

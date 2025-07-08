/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file sha256.hpp
 * @brief Class for calculating SHA256sum
 **/

#pragma once

#include <istream>
#include <memory>
#include <string>

#include <openssl/evp.h>
#include <openssl/sha.h>

class ContextWrapper {
  public:
    ContextWrapper();

    [[nodiscard]] EVP_MD_CTX* get() const;

  private:
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> m_context;
};

class SHA256Hasher {
  public:
    static std::string hash(std::istream& stream);

    SHA256Hasher();

    void update(const char* data, size_t count);

    void update(const std::string& data);

    std::string finalize();

  private:
    ContextWrapper m_context;
};

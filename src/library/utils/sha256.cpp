/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file sha256.cpp
 * @brief SHA256Hasher implementation
 **/

#include "utils/sha256.hpp"

#include <iomanip>
#include <istream>
#include <sstream>
#include <string>
#include <vector>

#include <openssl/evp.h>
#include <openssl/sha.h>

#include "config/static_config.hpp"

ContextWrapper::ContextWrapper() :
    m_context(EVP_MD_CTX_new(), EVP_MD_CTX_free) {
    if (!m_context) {
        throw std::runtime_error("Failed to create context");
    }
}

EVP_MD_CTX* ContextWrapper::get() const {
    return m_context.get();
}

SHA256Hasher::SHA256Hasher() {
    if (EVP_DigestInit_ex(m_context.get(), EVP_sha256(), nullptr) != 1) {
        throw std::runtime_error("Failed to initialize digest");
    }
}

void SHA256Hasher::update(const char* data, size_t count) {
    if (EVP_DigestUpdate(m_context.get(), data, count) != 1) {
        throw std::runtime_error("Failed to update digest");
    }
}

void SHA256Hasher::update(const std::string& data) {
    return update(data.data(), data.size());
}

std::string SHA256Hasher::finalize() {
    std::vector<unsigned char> hash(EVP_MD_size(EVP_sha256()));
    unsigned int length = 0;
    if (EVP_DigestFinal_ex(m_context.get(), hash.data(), &length) != 1) {
        throw std::runtime_error("Failed to finalize digest");
    }

    std::ostringstream oss;
    for (unsigned char byte : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(byte);
    }
    return oss.str();
}

std::string SHA256Hasher::hash(std::istream& stream) {
    SHA256Hasher hasher;
    std::vector<char> buffer(config::hash_buffer_size, '\0');

    while (stream) {
        stream.read(buffer.data(), buffer.size());
        auto count = stream.gcount();
        hasher.update(buffer.data(), count);
    }
    return hasher.finalize();
}

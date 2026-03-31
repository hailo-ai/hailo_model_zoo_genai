/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file blob_resource.cpp
 * @brief BlobResourceProvider implementation
 **/

#include "model/blob_resource.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <utility>

#include <oatpp-openssl/Config.hpp>
#include <oatpp-openssl/client/ConnectionProvider.hpp>
#include <oatpp/data/stream/FileStream.hpp>
#include <oatpp/json/ObjectMapper.hpp>
#include <oatpp/network/tcp/client/ConnectionProvider.hpp>
#include <oatpp/web/client/HttpRequestExecutor.hpp>
#include <oatpp/web/protocol/http/incoming/Response.hpp>

#include "controller/pull_callback.hpp"
#include "controller/writefile_callback.hpp"
#include "download/client.hpp"
#include "oatpp/Types.hpp"
#include "utils/sha256.hpp"

using namespace std::literals::string_literals;
namespace fs = std::filesystem;

#define STATUS_OK (200)

namespace hailo_ollama
{

static const int64_t UNKNOWN_SIZE = -1;
static const std::string EMPTY_DIGEST = "";
static const std::string SUCCESS_MSG = "success";

std::string BlobResourceProvider::get_resource_str(const std::string &resource)
{
    return (m_blob_dir / ("sha256_"s + resource)).string();
}

BlobResourceProvider::BlobResourceProvider(std::filesystem::path blob_dir, const std::string &base_url, uint16_t port)
    : m_blob_dir(std::move(blob_dir)), m_base_url(base_url), m_port(port)
{
}

bool valid_file_exists(const std::string &target, const std::string &resource)
{
    std::ifstream stream(target, std::ifstream::in | std::ifstream::binary);

    if (stream) {
        // file opened successfully -> file already exists; checking hash
        if (SHA256Hasher::hash(stream) == resource) {
            return true;
        }
        stream.close();
        // hash mismatch -> delete file
        fs::remove(target);
    }
    return false;
}

std::shared_ptr<oatpp::web::protocol::http::incoming::Response> BlobResourceProvider::
    request_download_file(const std::string &target, const std::string &source)
{
    /* create connection provider */
    auto config = oatpp::openssl::Config::createDefaultClientConfigShared();
    auto connectionProvider = oatpp::openssl::client::ConnectionProvider::createShared(config, {m_base_url, m_port});

    /* create HTTP request executor */
    auto requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider);

    /* create JSON object mapper */
    auto objectMapper = std::make_shared<oatpp::json::ObjectMapper>();

    /* create API client */
    auto client = DownloadClient::createShared(requestExecutor, objectMapper);

    return client->getDownload(source);
}

void BlobResourceProvider::download_file(const std::string &target, const std::string &source)
{
    auto response = request_download_file(target, source);
    if (response->getStatusCode() != STATUS_OK) {
        throw std::runtime_error("failed to download blob: HTTP " + std::to_string(response->getStatusCode()));
    }
    auto output_stream = oatpp::data::stream::FileOutputStream(target.c_str(), "wb");
    response->transferBodyToStream(&output_stream);
}

void BlobResourceProvider::pull_resource(const std::string &resource)
{
    const auto target = get_resource_str(resource);
    if (valid_file_exists(target, resource)) {
        return;
    }

    auto target_temp_path = target + ".tmp";
    download_file(target_temp_path, "sha256_"s + resource);
    fs::rename(target_temp_path, target);

    std::ifstream stream(target, std::ifstream::in | std::ifstream::binary);
    if (SHA256Hasher::hash(stream) != resource) {
        fs::remove(target);
        throw std::runtime_error("hash verification failed for downloaded blob");
    }
}

void BlobResourceProvider::pull_resource(const std::string &resource,
    const std::shared_ptr<PullReadCallback::EventQueue> &queue)
{
    const auto target = get_resource_str(resource);
    if (valid_file_exists(target, resource)) {
        queue->enqueue(PullEvent::PROGRESS, SUCCESS_MSG, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
        queue->enqueue(PullEvent::DONE, EMPTY_DIGEST, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
        return;
    }

    auto target_temp_path = target + ".tmp";
    auto response = request_download_file(target_temp_path, "sha256_"s + resource);
    if (response->getStatusCode() != STATUS_OK) {
        const std::string error_msg = std::string("failed to download blob: HTTP ") + std::to_string(response->getStatusCode());
        queue->enqueue(PullEvent::PULL_ERROR, error_msg, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
        return;
    }
    response->transferBody(std::make_shared<OutputFileStream>(target_temp_path.c_str(), resource, queue,
        response->getHeader("Content-Length").getValue(std::to_string(UNKNOWN_SIZE))));
    fs::rename(target_temp_path, target);

    const std::string verifying_msg = "verifying sha256 digest";
    queue->enqueue(PullEvent::PROGRESS, verifying_msg, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
    std::ifstream stream(target, std::ifstream::in | std::ifstream::binary);
    if (SHA256Hasher::hash(stream) != resource) {
        fs::remove(target);
        const std::string hash_error_msg = "hash verification failed for downloaded blob";
        queue->enqueue(PullEvent::PULL_ERROR, hash_error_msg, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
        return;
    }
    queue->enqueue(PullEvent::PROGRESS, SUCCESS_MSG, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
    queue->enqueue(PullEvent::DONE, EMPTY_DIGEST, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
}

std::filesystem::path BlobResourceProvider::get_resource(const std::string &resource)
{
    return get_resource_str(resource);
}

} // namespace hailo_ollama

/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
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

std::string
BlobResourceProvider::get_resource_str(const std::string& resource) {
    return m_blob_dir / ("sha256_"s + resource);
}

BlobResourceProvider::BlobResourceProvider(
    std::filesystem::path blob_dir,
    const std::string& base_url,
    uint16_t port
) :
    m_blob_dir(std::move(blob_dir)),
    m_base_url(base_url),
    m_port(port) {}

bool valid_file_exists(const std::string& target, const std::string& resource) {
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

std::shared_ptr<oatpp::web::protocol::http::incoming::Response>
BlobResourceProvider::request_download_file(
    const std::string& target,
    const std::string& source
) {
    /* create connection provider */
    auto connectionProvider =
        oatpp::network::tcp::client::ConnectionProvider::createShared(
            {m_base_url, m_port, oatpp::network::Address::IP_4}
        );

    /* create HTTP request executor */
    auto requestExecutor =
        oatpp::web::client::HttpRequestExecutor::createShared(
            connectionProvider
        );

    /* create JSON object mapper */
    auto objectMapper = std::make_shared<oatpp::json::ObjectMapper>();

    /* create API client */
    auto client = DownloadClient::createShared(requestExecutor, objectMapper);

    return client->getDownload(source);
}

void BlobResourceProvider::download_file(
    const std::string& target,
    const std::string& source
) {
    auto response = request_download_file(target, source);
    auto output_stream = oatpp::data::stream::FileOutputStream(target.c_str());
    response->transferBodyToStream(&output_stream);
}

void BlobResourceProvider::pull_resource(const std::string& resource) {
    const auto target = get_resource_str(resource);
    if (valid_file_exists(target, resource)) {
        return;
    }

    auto target_temp_path = target + ".tmp";
    download_file(target_temp_path, "sha256_"s + resource);
    fs::rename(target_temp_path, target);

    std::ifstream stream(target, std::ifstream::in | std::ifstream::binary);
    if (SHA256Hasher::hash(stream) != resource) {
        throw std::runtime_error("bad hash");
    }
}

void BlobResourceProvider::pull_resource(
    const std::string& resource,
    const std::shared_ptr<PullReadCallback::EventQueue>& queue
) {
    const auto target = get_resource_str(resource);
    if (valid_file_exists(target, resource)) {
        queue->enqueue(PullEvent::PROGRESS, "success", "", -1, -1);
        queue->enqueue(PullEvent::DONE, "", "", -1, -1);
        return;
    }

    auto target_temp_path = target + ".tmp";
    auto response =
        request_download_file(target_temp_path, "sha256_"s + resource);
    response->transferBody(
        std::make_shared<OutputFileStream>(
            target_temp_path.c_str(),
            resource,
            queue,
            response->getHeader("Content-Length").getValue("-1")
        )
    );
    fs::rename(target_temp_path, target);

    queue->enqueue(PullEvent::PROGRESS, "verifying sha256 digest", "", -1, -1);
    std::ifstream stream(target, std::ifstream::in | std::ifstream::binary);
    if (SHA256Hasher::hash(stream) != resource) {
        throw std::runtime_error("bad hash");
    }
    queue->enqueue(PullEvent::PROGRESS, "success", "", -1, -1);
    queue->enqueue(PullEvent::DONE, "", "", -1, -1);
}

std::filesystem::path
BlobResourceProvider::get_resource(const std::string& resource) {
    return get_resource_str(resource);
}

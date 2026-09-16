/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file blob_resource.hpp
 * @brief Class for managing resources by downloading BLOBs
 **/

#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <oatpp/web/client/ApiClient.hpp>
#include <oatpp/web/protocol/http/incoming/Response.hpp>

#include "controller/pull_callback.hpp"
#include "model/resource.hpp"

namespace hailo_ollama
{

using HttpResponse = std::shared_ptr<oatpp::web::protocol::http::incoming::Response>;
using HttpsClient = std::shared_ptr<oatpp::web::client::ApiClient>;
using BlobStatusProbe = std::function<std::optional<int>(const std::string &path)>;

namespace detail {
// Implementation details, declared here so unit tests can exercise them directly.
std::string parse_sha256_companion_file_content(const std::string &body);
std::string compute_fallback_version(const std::string &requested_version);
std::string strip_suffix(const std::string &str, const std::string &suffix);
std::string blob_url_path(const std::string &version, const std::string &filename);
std::string sha256_companion_url_path(const std::string &version, const std::string &hef_filename);
std::string resolve_complete_version(const std::string &requested_version, const std::string &hef_filename,
    const BlobStatusProbe &probe, const std::shared_ptr<PullReadCallback::EventQueue> &queue);

std::filesystem::path cached_version_file_path(const std::filesystem::path &cached_hef_path);
// nullopt when the marker is absent, not a regular file, oversized, or unreadable.
std::optional<std::string> read_cached_version(const std::filesystem::path &cached_hef_path);
bool is_hef_cached_at_version(const std::filesystem::path &cached_hef_path, const std::string &resolved_version);
void write_cached_version(const std::filesystem::path &cached_hef_path, const std::string &resolved_version);
void remove_cached_version_file(const std::filesystem::path &cached_hef_path);
// Publishes a verified download as the cached HEF, marker included. Throws if the bytes cannot be published.
void commit_downloaded_hef(const std::string &temp_path, const std::filesystem::path &cache_path,
    const std::string &resolved_version);
} // namespace detail

class BlobResourceProvider : public ResourceProvider
{
public:
    // probe checks a cloud blob's availability: HTTP status code on response, nullopt on transport failure.
    BlobResourceProvider(std::filesystem::path blob_dir, const std::string &base_url, uint16_t port,
        BlobStatusProbe probe = {});

    std::filesystem::path get_resource(const std::string &hef_filename) override;
    bool remove_resource(const std::string &hef_filename) override;
    std::string pull_resource(const std::string &hef_filename) override;
    std::string pull_resource(const std::string &hef_filename,
        const std::shared_ptr<PullReadCallback::EventQueue> &queue) override;

private:
    std::string fetch_hef(const std::string &hef_filename, const std::shared_ptr<PullReadCallback::EventQueue> &queue);
    std::string fetch_expected_hash(const std::string &resolved_version, const std::string &hef_filename);
    void download_hef_to_path(const std::string &resolved_version, const std::string &hef_filename,
        const std::string &temp_path, const std::string &expected_hash,
        const std::shared_ptr<PullReadCallback::EventQueue> &queue);

    HttpResponse request_get(const std::string &path);
    std::optional<int> probe_blob_status(const std::string &path);

    std::filesystem::path m_blob_dir;
    std::string m_base_url;
    uint16_t m_port;
    BlobStatusProbe m_probe;
};

} // namespace hailo_ollama

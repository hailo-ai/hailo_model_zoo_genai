/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file blob_resource.hpp
 * @brief Class for managing resources by downloading BLOBs
 **/

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include <oatpp/web/protocol/http/incoming/Response.hpp>

#include "controller/pull_callback.hpp"
#include "model/resource.hpp"

class BlobResourceProvider: public ResourceProvider {
  public:
    BlobResourceProvider(
        std::filesystem::path blob_dir,
        const std::string& base_url,
        uint16_t port
    );

    std::filesystem::path get_resource(const std::string& resource) override;
    void pull_resource(const std::string& resource) override;
    void pull_resource(
        const std::string& resource,
        const std::shared_ptr<PullReadCallback::EventQueue>& queue
    ) override;

  private:
    std::string get_resource_str(const std::string& resource);
    std::shared_ptr<oatpp::web::protocol::http::incoming::Response>
    request_download_file(const std::string& target, const std::string& source);
    void download_file(const std::string& target, const std::string& source);

  private:
    std::filesystem::path m_blob_dir;
    std::string m_base_url;
    uint16_t m_port;
};

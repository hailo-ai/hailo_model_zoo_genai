/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file client.hpp
 * @brief Class for downloading BLOBs from http server
 **/

#pragma once

#include <oatpp/Types.hpp>
#include <oatpp/macro/codegen.hpp>
#include <oatpp/web/client/ApiClient.hpp>

/* Begin Api Client code generation */
#include OATPP_CODEGEN_BEGIN(ApiClient)

/**
 * Test API client.
 * Use this client to call application APIs.
 */
class DownloadClient: public oatpp::web::client::ApiClient {
    API_CLIENT_INIT(DownloadClient)

    API_CALL("GET", "/blob/{digest}", getDownload, PATH(String, digest))
};

/* End Api Client code generation */
#include OATPP_CODEGEN_END(ApiClient)

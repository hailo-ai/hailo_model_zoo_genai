/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file api_test_client.hpp
 * @brief Test client for Hailo Ollama server
 **/

#ifndef MyApiTestClient_hpp
#define MyApiTestClient_hpp

#include <oatpp/Types.hpp>
#include <oatpp/macro/codegen.hpp>
#include <oatpp/web/client/ApiClient.hpp>

#include "dto/DTOs.hpp"

/* Begin Api Client code generation */
#include OATPP_CODEGEN_BEGIN(ApiClient)

/**
 * Test API client.
 * Use this client to call application APIs.
 */
class MyApiTestClient: public oatpp::web::client::ApiClient {
    API_CLIENT_INIT(MyApiTestClient)

    API_CALL("GET", "/", getRoot)
    API_CALL("GET", "/api/version", getVersion)
    API_CALL("GET", "/api/tags", getTags)
    API_CALL("GET", "/hailo/v1/list", getAllModels)
    API_CALL("GET", "/api/ps", getRunningModels)
    API_CALL(
        "POST",
        "/api/show",
        postShow,
        BODY_DTO(Object<ShowParams>, show_params)
    )
    API_CALL(
        "POST",
        "/api/generate",
        postGenerate,
        BODY_DTO(Object<GenerationParams>, generation_params)
    )
    API_CALL(
        "POST",
        "/api/chat",
        postChat,
        BODY_DTO(Object<ChatParams>, chat_params)
    )
    API_CALL(
        "POST",
        "/v1/chat/completions",
        postChatCompletions,
        BODY_DTO(Object<CreateChatCompletionParams>, chat_params)
    )
};

/* End Api Client code generation */
#include OATPP_CODEGEN_END(ApiClient)

#endif  // MyApiTestClient_hpp

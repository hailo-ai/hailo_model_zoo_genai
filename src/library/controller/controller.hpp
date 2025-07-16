/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file controller.hpp
 * @brief Entry points supported by Hailo Ollama server
 **/

#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <tuple>

#include <oatpp/data/mapping/ObjectMapper.hpp>
#include <oatpp/macro/codegen.hpp>
#include <oatpp/macro/component.hpp>
#include <oatpp/web/protocol/http/outgoing/StreamingBody.hpp>
#include <oatpp/web/server/api/ApiController.hpp>

#include "dto/DTOs.hpp"
#include "generation_context/generation_context.hpp"
#include "model/resource.hpp"
#include "model/store.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class MyController: public oatpp::web::server::api::ApiController {
    enum class ReturnType {
        RESPONSE,
        MESSAGE,
        COMPLETION,
    };

  public:
    MyController(
        const std::shared_ptr<SyncGenerationContext>& generation_context,
        const std::shared_ptr<ModelStore>& model_store,
        const std::shared_ptr<ResourceProvider>& resource_provider,
        OATPP_COMPONENT(
            const std::shared_ptr<oatpp::web::mime::ContentMappers>,
            apiContentMappers
        )
    );

  public:
    ENDPOINT("GET", "/", root);
    ENDPOINT("GET", "/api/version", version);

    ENDPOINT("GET", "/api/tags", list_models);
    ENDPOINT("GET", "/hailo/v1/list", list_all_models);
    ENDPOINT("GET", "/api/ps", list_running_models);

    ENDPOINT(
        "POST",
        "/api/show",
        show,
        BODY_DTO(Object<ShowParams>, show_params)
    );

    ENDPOINT(
        "POST",
        "/api/pull",
        pull_model,
        BODY_DTO(Object<PullParams>, pull_params)
    );

    ENDPOINT(
        "DELETE",
        "/api/delete",
        delete_model,
        BODY_DTO(Object<DeleteParams>, delete_params)
    );

    ENDPOINT(
        "POST",
        "/api/generate",
        generate,
        BODY_DTO(Object<GenerationParams>, generation_params)
    );

    ENDPOINT(
        "POST",
        "/api/chat",
        chat,
        BODY_DTO(Object<ChatParams>, generation_params)
    );
    ENDPOINT(
        "POST",
        "/v1/chat/completions",
        chat_completions,
        BODY_DTO(Object<CreateChatCompletionParams>, generation_params)
    );

  private:
    std::shared_ptr<OutgoingResponse> handle_completion(
        const ModelInfo& model_data,
        const std::string& raw_prompt,
        const Object<ModelParameters>& options,
        const bool stream,
        const oatpp::Int32& keep_alive,
        const std::string& model,
        const ReturnType return_type
    );

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
    handle_load_unload(
        const std::string& model_name,
        const ModelInfo& model_data,
        const oatpp::Object<ModelParameters>& options,
        const oatpp::Int32& keep_alive,
        const bool return_as_message
    );

    std::optional<std::pair<ModelInfo, std::filesystem::path>>
    get_model_data(const std::string& model_name);

    std::optional<Object<ModelInfoShort>>
    get_model_info(const std::string& model_name);

    static std::optional<std::chrono::seconds>
    convert_keep_alive(const oatpp::Int32& keep_alive);

  private:
    std::shared_ptr<SyncGenerationContext> m_generation_context;
    std::shared_ptr<ModelStore> m_model_store;
    std::shared_ptr<ResourceProvider> m_resource_provider;
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen

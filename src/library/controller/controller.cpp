/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file controller.cpp
 * @brief MyController implementation
 **/

#include "controller.hpp"

#include <cstdint>
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
#include <thread>

#include <hailo/genai/llm/llm.hpp>
#include <oatpp/base/Log.hpp>
#include <oatpp/data/mapping/ObjectMapper.hpp>
#include <oatpp/macro/codegen.hpp>
#include <oatpp/macro/component.hpp>
#include <oatpp/web/protocol/http/outgoing/StreamingBody.hpp>
#include <oatpp/web/server/api/ApiController.hpp>

#include "config/static_config.hpp"
#include "controller/llm_generation_callback.hpp"
#include "controller/pull_callback.hpp"
#include "dto/DTOs.hpp"
#include "generation_context/generation_context.hpp"
#include "model/resource.hpp"
#include "model/store.hpp"
#include "oatpp/Types.hpp"
#include "utils/time.hpp"

namespace fs = std::filesystem;

// manage all long imports  from oatpp
namespace oat
{
using OutgoingResponse = oatpp::web::protocol::http::outgoing::Response;
using OutgoingStreamingBody = oatpp::web::protocol::http::outgoing::StreamingBody;
} // namespace oat

namespace hailo_ollama
{

// Apply user options to generator params (override defaults)
void apply_user_options(const oatpp::Object<ModelParameters> &options,
    hailort::genai::LLMGeneratorParams &generator_params)
{
    if (!options) {
        return;
    }

    if (options->temperature == 0.0F) {
        generator_params.set_do_sample(false);
    } else if (options->temperature != nullptr) {
        generator_params.set_do_sample(true);
        generator_params.set_temperature(options->temperature);
    }

    if (options->seed != nullptr && options->seed != -1) {
        generator_params.set_seed(options->seed);
    }

    if (options->top_k != nullptr) {
        generator_params.set_top_k(options->top_k);
    }
    if (options->top_p != nullptr) {
        generator_params.set_top_p(options->top_p);
    }
    if (options->frequency_penalty != nullptr) {
        generator_params.set_frequency_penalty(options->frequency_penalty);
    }

    if (options->num_predict != nullptr) {
        generator_params.set_max_generated_tokens(options->num_predict);
    }
}

MyController::MyController(const std::shared_ptr<SyncGenerationContext> &generation_context,
    const std::shared_ptr<ModelStore> &model_store, const std::shared_ptr<ResourceProvider> &resource_provider,
    const std::shared_ptr<oatpp::web::mime::ContentMappers> &apiContentMappers)
    : oatpp::web::server::api::ApiController(apiContentMappers), m_generation_context(generation_context),
      m_model_store(model_store), m_resource_provider(resource_provider)
{
}

std::optional<std::pair<ModelInfo, std::filesystem::path>> MyController::get_model_data(const std::string &model_name)
{
    const auto model_data_opt = m_model_store->get_model(model_name);
    if (!model_data_opt) {
        return std::nullopt;
    }
    const auto &model_data = *model_data_opt;
    const auto hef = m_resource_provider->get_resource(model_data.hef_resource);
    if (!fs::is_regular_file(hef)) {
        return std::nullopt;
    }

    return {{model_data, hef}};
}

std::optional<oatpp::Object<ModelInfoShort>> MyController::get_model_info(const std::string &model_name)
{
    const auto model_data_opt = get_model_data(model_name);
    if (!model_data_opt) {
        return std::nullopt;
    }
    const auto &model_data = model_data_opt->first;
    const auto &hef = model_data_opt->second;
    std::error_code error_code;
    const auto modified_at = fs::last_write_time(hef, error_code);
    if (error_code) {
        return std::nullopt;
    }
    const auto file_size = fs::file_size(hef, error_code);
    if (error_code) {
        return std::nullopt;
    }
    auto model_info = ModelInfoShort::createShared();
    model_info->name = model_name;
    model_info->model = model_name;
    model_info->size = file_size;
    model_info->modified_at = to_iso_8601(modified_at, "Z");
    if (!model_data.details.empty()) {
        model_info->details =
            m_contentMappers->getDefaultMapper()->readFromString<oatpp::Object<ModelInfoDetails>>(model_data.details);
    }
    return model_info;
}

std::optional<std::chrono::seconds> MyController::convert_keep_alive(const oatpp::Int32 &keep_alive)
{
    if (!keep_alive) {
        return config::HAILO_OLLAMA_DEFAULT_KEEP_ALIVE; // default value for keep_alive is 5m
    } else if (*keep_alive < 0) {
        return std::nullopt;
    } else {
        return std::chrono::seconds(*keep_alive);
    }
}

std::shared_ptr<oat::OutgoingResponse> MyController::handle_completion(const ModelInfo &model_data,
    const std::vector<std::string> &prompt_json_strings, const oatpp::Object<ModelParameters> &options,
    const bool stream, const oatpp::Int32 &keep_alive, const std::string &model, const ReturnType return_type)
{
    using GenerationStatus = hailort::genai::LLMGeneratorCompletion::Status;

    const auto hef = m_resource_provider->get_resource(model_data.hef_resource);
    OATPP_LOGi("handle_completion", "Got model '{}', path '{}'", model, hef.string());

    // Load model first to get default generator params
    auto generator = m_generation_context->lock();
    generator->load_model(model_data.name, hef, convert_keep_alive(keep_alive));

    // Get EOS token from model to clamp it from responses
    std::string eos_token = generator->get_generation_recovery_sequence();

    // Get default generator params from LLM API
    auto generator_params = generator->create_generator_params();

    // Apply user options (overriding defaults)
    apply_user_options(options, generator_params);

    Generation generation{
        model_data.name,
        hef,
        prompt_json_strings,
        std::move(generator_params),
        convert_keep_alive(keep_alive)
    };

    auto generator_completion = generator->generate_one(std::move(generation));
    if (!stream) {
        const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        uint32_t token_count = 0;
        std::string response_text = "";
        while (generator_completion.generation_status() == GenerationStatus::GENERATING) {
            response_text += generator_completion.read().expect("read failed!");
            token_count++;
        }

        // Strip EOS token from response if present
        response_text = strip_eos_suffix(response_text, eos_token);

        const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        const auto status = generator_completion.generation_status();
        const char *stop_reason = (status == GenerationStatus::LOGICAL_END_OF_GENERATION)
                                      ? GenerationContext::GenerationFinishedReason::STOP
                                      : GenerationContext::GenerationFinishedReason::LENGTH;

        // Append assistant response to conversation history for context continuation
        generator->append_assistant_message(response_text);

        generator.reset(); // Unlock before creating response

        if (return_type == ReturnType::COMPLETION) {
            auto result = CreateChatCompletionResponse::createShared();
            result->id = "chatcmpl-" + std::to_string(std::rand());
            result->object = "chat.completion";
            result->created = std::chrono::system_clock::now().time_since_epoch().count();

            result->model = model;
            result->choices = {};

            auto message = ChatCompletionMessage::createShared();
            message->role = "assistant";
            message->content = response_text;

            auto choice = ChatChoice::createShared();
            choice->index = oatpp::Int64(static_cast<int64_t>(0));
            choice->finish_reason = std::move(stop_reason);
            choice->message = message;

            result->choices->push_back(choice);
            return createDtoResponse(Status::CODE_200, result);
        }
        auto result = GenerationResponseFinal::createShared();
        result->model = model;
        result->created_at = get_current_time_formatted();
        if (return_type == ReturnType::MESSAGE) {
            result->message = ChatMessage::createShared();
            result->message->role = "assistant";
            result->message->content = response_text;
        } else {
            result->response = response_text;
        }

        result->done = true;
        result->done_reason = std::move(stop_reason);
        const auto total_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
        result->total_duration = total_time_ns;
        result->eval_count = token_count;

        return createDtoResponse(Status::CODE_200, result);
    }
    auto body = std::make_shared<oat::OutgoingStreamingBody>(std::make_shared<LLMGenerationReadCallback>(model,
        m_contentMappers->getDefaultMapper(), std::move(generator), std::move(generator_completion),
        return_type == ReturnType::MESSAGE, eos_token));

    auto outgoing_response = OutgoingResponse::createShared(Status::CODE_200, body);
    outgoing_response->putHeader("Content-Type", "application/x-ndjson");
    return outgoing_response;
}

std::shared_ptr<oat::OutgoingResponse> MyController::handle_load_unload(const std::string &model_name,
    const ModelInfo &model_data, const oatpp::Object<ModelParameters> &options, const oatpp::Int32 &keep_alive,
    const bool return_as_message)
{
    (void)options;
    // keep alive is 0 -> should unload the model
    auto result = GenerationResponseFinal::createShared();
    auto generator = m_generation_context->lock();
    if (keep_alive && *keep_alive == 0) {
        generator->reset();

        result->done_reason = "unload";
    } else {
        // Model load
        const auto hef = m_resource_provider->get_resource(model_data.hef_resource);
        generator->load_model(model_name, hef, convert_keep_alive(keep_alive));
        result->done_reason = "load";
    }
    result->model = model_name;
    result->created_at = get_current_time_formatted();
    if (return_as_message) {
        result->message = ChatMessage::createShared();
        result->message->role = "assistant";
        result->message->content = "";
    } else {
        result->response = "";
    }
    result->done = true;
    return createDtoResponse(Status::CODE_200, result);
}

std::shared_ptr<oat::OutgoingResponse> MyController::root()
{
    return ResponseFactory::createResponse(Status::CODE_200, "hailo-ollama is running");
}

std::shared_ptr<oat::OutgoingResponse> MyController::version()
{
    auto result = VersionResponse::createShared();
    // tools might rely on this -> return a version similar to original Ollama
    result->version = "0.5.1";
    return createDtoResponse(Status::CODE_200, result);
}

std::shared_ptr<oat::OutgoingResponse> MyController::list_models()
{
    const auto model_names = m_model_store->get_model_names();
    OATPP_LOGi("list_models", "got {} models in store", model_names.size());
    auto result = TagsResponse::createShared();
    result->models = {};
    for (const auto &model_name : model_names) {
        OATPP_LOGi("list_models", "model: {}", model_name);
        const auto model_info = get_model_info(model_name);
        if (!model_info) {
            continue;
        }
        result->models->push_back(*model_info);
    }
    return createDtoResponse(Status::CODE_200, result);
}

std::shared_ptr<oat::OutgoingResponse> MyController::list_all_models()
{
    const auto model_names = m_model_store->get_model_names();
    OATPP_LOGi("list_models", "got {} models in store", model_names.size());
    auto result = ListAllResponse::createShared();
    result->models = {};
    for (const auto &name : model_names) {
        result->models->push_back(name);
    }

    return createDtoResponse(Status::CODE_200, result);
}

std::shared_ptr<oat::OutgoingResponse> MyController::list_running_models()
{
    auto generator = m_generation_context->lock();
    const auto model_name = generator->get_model_name();
    const auto expiration = generator->get_expiration();
    generator.reset(); // unlock
    auto result = TagsResponse::createShared();
    result->models = {};
    const auto model_info_opt = get_model_info(model_name);
    if (model_info_opt) {
        const auto &model_info = *model_info_opt;
        model_info->expires_at = to_iso_8601(expiration, "Z");
        result->models->push_back(model_info);
    }

    return createDtoResponse(Status::CODE_200, result);
}

std::shared_ptr<oat::OutgoingResponse> MyController::show(const oatpp::Object<ShowParams> &show_params)
{
    const auto model_data_opt = get_model_data(show_params->model);
    if (!model_data_opt) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "model '" + show_params->model + "' not found";
        return createDtoResponse(Status::CODE_404, error_result);
    }
    const auto &model_data = model_data_opt->first;
    auto result = ShowResponse::createShared();
    result->license = model_data.license;
    result->modelfile = "";
    result->parameters = ""; // Stop tokens are now managed by HailoRT

    // Get HEF path and file modification time
    const auto &hef_path = model_data_opt->second;
    std::error_code error_code;
    const auto modified_at = fs::last_write_time(hef_path, error_code);
    if (!error_code) {
        result->modified_at = to_iso_8601(modified_at, "Z");
    }

    if (!model_data.details.empty()) {
        result->details =
            m_contentMappers->getDefaultMapper()->readFromString<oatpp::Object<ModelInfoDetails>>(model_data.details);
    }
    result->model_info = "";
    return createDtoResponse(Status::CODE_200, result);
}

std::shared_ptr<oat::OutgoingResponse> MyController::pull_model(const oatpp::Object<PullParams> &pull_params)
{
    auto model_data = m_model_store->get_model(pull_params->model);
    if (!model_data) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "model '" + pull_params->model + "' not found";
        return createDtoResponse(Status::CODE_404, error_result);
    }

    if (!pull_params->stream) {
        try {
            m_resource_provider->pull_resource(model_data->hef_resource);
        } catch (const std::exception &e) {
            auto error_result = ErrorResponse::createShared();
            error_result->error = e.what();
            return createDtoResponse(Status::CODE_500, error_result);
        }
        auto result = PullResponse::createShared();
        result->status = "success";

        return createDtoResponse(Status::CODE_200, result);
    }

    auto queue = std::make_shared<PullReadCallback::EventQueue>();
    std::thread pull_thread([this, &model_data, queue, hef_resource = model_data->hef_resource]() {
        m_resource_provider->pull_resource(hef_resource, queue);
    });
    auto body =
        std::make_shared<oat::OutgoingStreamingBody>(std::make_shared<PullReadCallback>(m_contentMappers
                                                                                            ->getDefaultMapper(),
            queue, std::move(pull_thread)));

    auto outgoing_response = OutgoingResponse::createShared(Status::CODE_200, body);
    outgoing_response->putHeader("Content-Type", "application/x-ndjson");
    return outgoing_response;
}

std::shared_ptr<oat::OutgoingResponse> MyController::delete_model(const oatpp::Object<DeleteParams> &delete_params)
{
    auto model_data = m_model_store->get_model(delete_params->model);
    if (!model_data) {
        auto error_result = DeleteErrorResponse::createShared();
        error_result->code = "not_found";
        error_result->error = "model not found";
        return createDtoResponse(Status::CODE_404, error_result);
    }
    const auto hef = m_resource_provider->get_resource(model_data->hef_resource);
    std::error_code error_code;
    const auto removed = fs::remove(hef, error_code);
    if (error_code || !removed) {
        auto error_result = DeleteErrorResponse::createShared();
        error_result->code = "not_found";
        error_result->error = "model not found";
        return createDtoResponse(Status::CODE_404, error_result);
    }

    return createResponse(Status::CODE_200);
}

std::shared_ptr<oat::OutgoingResponse> MyController::generate(const oatpp::Object<GenerationParams> &generation_params)
{
    const auto &model = generation_params->model;

    const auto model_data_opt = get_model_data(model);
    if (!model_data_opt) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "model '" + model + "' not found";
        return createDtoResponse(Status::CODE_404, error_result);
    }
    const auto &model_data = model_data_opt->first;

    if (!generation_params->prompt) {
        return handle_load_unload(model, model_data, generation_params->options, generation_params->keep_alive, false);
    }

    const auto &prompt = generation_params->prompt;
    const auto stream = generation_params->stream;

    // Create structured prompt as JSON string using raw string literal
    std::vector<std::string> prompt_json_strings = {R"({"role": "user", "content": ")" + std::string(prompt) + R"("})"};

    return handle_completion(model_data, prompt_json_strings, generation_params->options, stream,
        generation_params->keep_alive, model, ReturnType::RESPONSE);
}

std::shared_ptr<oat::OutgoingResponse> MyController::chat(const oatpp::Object<ChatParams> &generation_params)
{
    const auto &model = generation_params->model;
    const auto model_data_opt = get_model_data(model);
    if (!model_data_opt) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "model '" + model + "' not found";
        return createDtoResponse(Status::CODE_404, error_result);
    }
    const auto &model_data = model_data_opt->first;

    if (!generation_params->messages) {
        return handle_load_unload(model, model_data, generation_params->options, generation_params->keep_alive, true);
    }

    const auto stream = generation_params->stream;

    // Convert messages to JSON strings for structured prompts
    std::vector<std::string> prompt_json_strings;
    for (const auto &message : *generation_params->messages) {
        prompt_json_strings.push_back(R"({"role": ")" + message->role + R"(", "content": ")" +
                                      escape_json_quotes(message->content) + R"("})");
    }

    return handle_completion(model_data, prompt_json_strings, generation_params->options, stream,
        generation_params->keep_alive, model, ReturnType::MESSAGE);
}

std::shared_ptr<oat::OutgoingResponse> MyController::chat_completions(const oatpp::Object<CreateChatCompletionParams>
        &generation_params)
{
    if (!generation_params->messages) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "messages field is required";
        return createDtoResponse(Status::CODE_400, error_result);
    }
    if (generation_params->n != 1) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "only n == 1 is supported";
        return createDtoResponse(Status::CODE_400, error_result);
    }
    if (generation_params->stream) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "streaming not supported on this endpoint. Use /api/chat endpoint instead.";
        return createDtoResponse(Status::CODE_400, error_result);
    }
    const auto &model = generation_params->model;
    const auto model_data_opt = get_model_data(model);
    if (!model_data_opt) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "model '" + model + "' not found";
        return createDtoResponse(Status::CODE_404, error_result);
    }
    const auto &model_data = model_data_opt->first;

    const auto stream = generation_params->stream;

    // Convert messages to JSON strings for structured prompts
    std::vector<std::string> prompt_json_strings;
    for (const auto &message : *generation_params->messages) {
        prompt_json_strings.push_back(R"({"role": ")" + message->role + R"(", "content": ")" +
                                      escape_json_quotes(message->content) + R"("})");
    }

    auto model_options = ModelParameters::createShared();
    model_options->temperature = generation_params->temperature;
    model_options->seed = generation_params->seed;
    model_options->top_p = generation_params->top_p;
    model_options->frequency_penalty = generation_params->frequency_penalty;
    if (generation_params->max_tokens) {
        model_options->num_predict = generation_params->max_tokens;
    }
    if (generation_params->max_completion_tokens) {
        model_options->num_predict = generation_params->max_completion_tokens;
    }

    return handle_completion(model_data, prompt_json_strings, model_options, stream, oatpp::Int32(nullptr), model,
        ReturnType::COMPLETION);
}

} // namespace hailo_ollama

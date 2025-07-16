/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file controller.cpp
 * @brief MyController implementation
 **/

#include "controller.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
#include <thread>

#include <hailo/genai/llm/llm.hpp>
#include <minja/chat-template.hpp>
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

using json = nlohmann::ordered_json;
namespace fs = std::filesystem;

// manage all long imports  from oatpp
namespace oat {
using OutgoingResponse = oatpp::web::protocol::http::outgoing::Response;
using OutgoingStreamingBody =
    oatpp::web::protocol::http::outgoing::StreamingBody;
}  // namespace oat

namespace {
void set_options(
    const oatpp::Object<ModelParameters>& options,
    Generation& generation
) {
    if (!options) {
        return;
    }
    if (options->temperature == 0.0F) {
        generation.do_sample = false;
    } else if (options->temperature != nullptr) {
        generation.do_sample = true;
        generation.temperature = options->temperature;
    }

    if (options->seed != nullptr && options->seed != -1) {
        generation.seed = options->seed;
    }

    if (options->top_k != nullptr) {
        generation.top_k = options->top_k;
    }
    if (options->top_p != nullptr) {
        generation.top_p = options->top_p;
    }
    if (options->frequency_penalty != nullptr) {
        generation.frequency_penalty = options->frequency_penalty;
    }

    if (options->num_predict != nullptr) {
        generation.max_generated_tokens = options->num_predict;
    }
}
}  // namespace

MyController::MyController(
    const std::shared_ptr<SyncGenerationContext>& generation_context,
    const std::shared_ptr<ModelStore>& model_store,
    const std::shared_ptr<ResourceProvider>& resource_provider,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& apiContentMappers
) :
    oatpp::web::server::api::ApiController(apiContentMappers),
    m_generation_context(generation_context),
    m_model_store(model_store),
    m_resource_provider(resource_provider) {}

std::optional<std::pair<ModelInfo, std::filesystem::path>>
MyController::get_model_data(const std::string& model_name) {
    const auto model_data_opt = m_model_store->get_model(model_name);
    if (!model_data_opt) {
        return std::nullopt;
    }
    const auto& model_data = *model_data_opt;
    const auto hef = m_resource_provider->get_resource(model_data.hef_resource);
    if (!fs::is_regular_file(hef)) {
        return std::nullopt;
    }

    return {{model_data, hef}};
}

std::optional<oatpp::Object<ModelInfoShort>>
MyController::get_model_info(const std::string& model_name) {
    const auto model_data_opt = get_model_data(model_name);
    if (!model_data_opt) {
        return std::nullopt;
    }
    const auto& model_data = model_data_opt->first;
    const auto& hef = model_data_opt->second;
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
            m_contentMappers->getDefaultMapper()
                ->readFromString<oatpp::Object<ModelInfoDetails>>(
                    model_data.details
                );
    }
    return model_info;
}

std::optional<std::chrono::seconds>
MyController::convert_keep_alive(const oatpp::Int32& keep_alive) {
    if (!keep_alive) {
        return config::
            controller_default_keep_alive;  // default value for keep_alive is 5m
    } else if (*keep_alive < 0) {
        return std::nullopt;
    } else {
        return std::chrono::seconds(*keep_alive);
    }
}

std::shared_ptr<oat::OutgoingResponse> MyController::handle_completion(
    const ModelInfo& model_data,
    const std::string& raw_prompt,
    const oatpp::Object<ModelParameters>& options,
    const bool stream,
    const oatpp::Int32& keep_alive,
    const std::string& model,
    const ReturnType return_type
) {
    using GenerationStatus = hailort::genai::LLMGeneratorCompletion::Status;

    const auto hef = m_resource_provider->get_resource(model_data.hef_resource);
    OATPP_LOGi("handle_completion", "Got model {}", hef.string());
    Generation generation {
        .model_name = model_data.name,
        .model_path = hef,
        .prompt = raw_prompt,
        .top_p = model_data.generation_params.top_p,
        .top_k = model_data.generation_params.top_k,
        .frequency_penalty = model_data.generation_params.frequency_penalty,
    };
    generation.keep_alive = convert_keep_alive(keep_alive);

    if (model_data.generation_params.temperature) {
        if (*model_data.generation_params.temperature != 0.0F) {
            generation.temperature = *model_data.generation_params.temperature;
        } else {
            generation.do_sample = false;
        }
    }
    set_options(options, generation);
    auto generator = m_generation_context->lock();
    auto generator_completion = generator->generate_one(std::move(generation));
    const auto& stop_tokens = model_data.generation_params.stop_tokens;
    if (!stream) {
        std::stringstream response;

        auto token_count = 0ULL;
        const std::chrono::steady_clock::time_point begin =
            std::chrono::steady_clock::now();
        bool stop_token_encountered = false;
        while (true) {
            const auto status = generator_completion.generation_status();
            if (status == GenerationStatus::LOGICAL_END_OF_GENERATION) {
                stop_token_encountered = true;
                break;
            }
            if (status == GenerationStatus::MAX_TOKENS_REACHED) {
                break;
            }

            const auto output =
                generator_completion.read().expect("read failed!");

            // check status immediately after read to see if it's the last one
            const auto is_last_token =
                (generator_completion.generation_status()
                 != GenerationStatus::GENERATING);
            // the last token is guaranteed to be an end token -> not returning to user or to history
            if (is_last_token) {
                continue;
            }

            const auto stop_token_it =
                std::find(stop_tokens.cbegin(), stop_tokens.cend(), output);
            if (stop_token_it != stop_tokens.cend()) {
                stop_token_encountered = true;
                // Do not return stop tokens to user, but they're still part of the last prompt
                generator->append_last_prompt(output);
                break;
            }
            ++token_count;
            response << output;
        }

        // we'd like to stop the generation on stop_token but it leads to races
        // instead we continue reading until HRT stops the generation
        while (true) {
            const auto status = generator_completion.generation_status();
            if (status != GenerationStatus::GENERATING) {
                break;
            }
            const auto output =
                generator_completion.read().expect("read failed!");

            // check status immediately after read to see if it's the last one
            const auto is_last_token =
                (generator_completion.generation_status()
                 != GenerationStatus::GENERATING);
            // the last token is guaranteed to be an end token -> not returning to user or to history
            if (is_last_token) {
                break;
            }

            generator->append_last_prompt(output);
        }

        const std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();
        std::string stop_reason = stop_token_encountered ? "stop" : "length";

        if (return_type == ReturnType::COMPLETION) {
            auto result = CreateChatCompletionResponse::createShared();
            result->id = "chatcmpl-" + std::to_string(std::rand());
            result->object = "chat.completion";
            result->created =
                std::chrono::system_clock::now().time_since_epoch().count();

            result->model = model;
            result->choices = {};

            auto message = ChatCompletionMessage::createShared();
            message->role = "assistant";
            message->content = response.str();

            auto choice = ChatChoice::createShared();
            choice->index = 0L;
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
            result->message->content = response.str();
        } else {
            result->response = response.str();
        }
        generator->append_last_prompt(response.str());

        result->done = true;
        result->done_reason = std::move(stop_reason);
        const auto total_time_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
                .count();
        result->total_duration = total_time_ns;
        result->eval_count = token_count;

        return createDtoResponse(Status::CODE_200, result);
    }
    auto body = std::make_shared<oat::OutgoingStreamingBody>(
        std::make_shared<LLMGenerationReadCallback>(
            model,
            stop_tokens,
            m_contentMappers->getDefaultMapper(),
            std::move(generator),
            std::move(generator_completion),
            return_type == ReturnType::MESSAGE
        )
    );

    auto outgoing_response =
        OutgoingResponse::createShared(Status::CODE_200, body);
    outgoing_response->putHeader("Content-Type", "application/x-ndjson");
    return outgoing_response;
}

std::shared_ptr<oat::OutgoingResponse> MyController::handle_load_unload(
    const std::string& model_name,
    const ModelInfo& model_data,
    const oatpp::Object<ModelParameters>& options,
    const oatpp::Int32& keep_alive,
    const bool return_as_message
) {
    (void)options;
    // keep alive is 0 -> should unload the model
    auto result = GenerationResponseFinal::createShared();
    auto generator = m_generation_context->lock();
    if (keep_alive && *keep_alive == 0) {
        generator->reset();

        result->done_reason = "unload";
    } else {
        // Model load
        const auto hef =
            m_resource_provider->get_resource(model_data.hef_resource);
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

std::shared_ptr<oat::OutgoingResponse> MyController::root() {
    return ResponseFactory::createResponse(
        Status::CODE_200,
        "hailo-ollama is running"
    );
}

std::shared_ptr<oat::OutgoingResponse> MyController::version() {
    auto result = VersionResponse::createShared();
    // tools might rely on this -> return a version similar to original Ollama
    result->version = "0.5.1";
    return createDtoResponse(Status::CODE_200, result);
}

std::shared_ptr<oat::OutgoingResponse> MyController::list_models() {
    const auto model_names = m_model_store->get_model_names();
    OATPP_LOGi("list_models", "got {} models in store", model_names.size());
    auto result = TagsResponse::createShared();
    result->models = {};
    for (const auto& model_name : model_names) {
        OATPP_LOGi("list_models", "model: {}", model_name);
        const auto model_info = get_model_info(model_name);
        if (!model_info) {
            continue;
        }
        result->models->push_back(*model_info);
    }
    return createDtoResponse(Status::CODE_200, result);
}

std::shared_ptr<oat::OutgoingResponse> MyController::list_all_models() {
    const auto model_names = m_model_store->get_model_names();
    OATPP_LOGi("list_models", "got {} models in store", model_names.size());
    auto result = ListAllResponse::createShared();
    result->models = {};
    for (const auto& name : model_names) {
        result->models->push_back(name);
    }

    return createDtoResponse(Status::CODE_200, result);
}

std::shared_ptr<oat::OutgoingResponse> MyController::list_running_models() {
    auto generator = m_generation_context->lock();
    const auto model_name = generator->get_model_name();
    const auto expiration = generator->get_expiration();
    generator.reset();  // unlock
    auto result = TagsResponse::createShared();
    result->models = {};
    const auto model_info_opt = get_model_info(model_name);
    if (model_info_opt) {
        const auto& model_info = *model_info_opt;
        model_info->expires_at = to_iso_8601(expiration, "Z");
        result->models->push_back(model_info);
    }

    return createDtoResponse(Status::CODE_200, result);
}

std::shared_ptr<oat::OutgoingResponse>
MyController::show(const oatpp::Object<ShowParams>& show_params) {
    const auto model_data_opt = get_model_data(show_params->model);
    if (!model_data_opt) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "model '" + show_params->model + "' not found";
        return createDtoResponse(Status::CODE_200, error_result);
    }
    const auto& model_data = model_data_opt->first;
    auto result = ShowResponse::createShared();
    result->license = model_data.license;
    result->modelfile = "";

    std::stringstream parameters_stream;
    for (const auto& stop_token : model_data.generation_params.stop_tokens) {
        parameters_stream << "stop"
                          << std::setw(config::controller_show_parameter_width)
                          << '"' << stop_token << '"' << '\n';
    }
    std::string parameters_string = parameters_stream.str();
    // remove final \n
    if (!parameters_string.empty()) {
        parameters_string.pop_back();
    }
    result->parameters = std::move(parameters_string);
    result->chat_template = model_data.template_params.chat_template;
    if (!model_data.details.empty()) {
        result->details = m_contentMappers->getDefaultMapper()
                              ->readFromString<oatpp::Object<ModelInfoDetails>>(
                                  model_data.details
                              );
    }
    result->model_info = "";
    std::error_code error_code;
    const auto& hef = model_data_opt->second;
    const auto modified_at = fs::last_write_time(hef, error_code);
    if (!error_code) {
        result->modified_at = to_iso_8601(modified_at, "Z");
    }
    return createDtoResponse(Status::CODE_200, result);
}

std::shared_ptr<oat::OutgoingResponse>
MyController::pull_model(const oatpp::Object<PullParams>& pull_params) {
    auto model_data = m_model_store->get_model(pull_params->model);
    if (!model_data) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "model '" + pull_params->model + "' not found";
        return createDtoResponse(Status::CODE_200, error_result);
    }

    if (!pull_params->stream) {
        m_resource_provider->pull_resource(model_data->hef_resource);
        auto result = PullResponse::createShared();
        result->status = "success";

        return createDtoResponse(Status::CODE_200, result);
    }

    auto queue = std::make_shared<PullReadCallback::EventQueue>();
    std::thread pull_thread(
        [this, &model_data, queue, hef_resource = model_data->hef_resource]() {
            m_resource_provider->pull_resource(hef_resource, queue);
        }
    );
    auto body = std::make_shared<oat::OutgoingStreamingBody>(
        std::make_shared<PullReadCallback>(
            m_contentMappers->getDefaultMapper(),
            queue,
            std::move(pull_thread)
        )
    );

    auto outgoing_response =
        OutgoingResponse::createShared(Status::CODE_200, body);
    outgoing_response->putHeader("Content-Type", "application/x-ndjson");
    return outgoing_response;
}

std::shared_ptr<oat::OutgoingResponse>
MyController::delete_model(const oatpp::Object<DeleteParams>& delete_params) {
    auto model_data = m_model_store->get_model(delete_params->model);
    if (!model_data) {
        auto error_result = DeleteErrorResponse::createShared();
        error_result->code = "not_found";
        error_result->error = "model not found";
        return createDtoResponse(Status::CODE_404, error_result);
    }
    const auto hef =
        m_resource_provider->get_resource(model_data->hef_resource);
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

std::shared_ptr<oat::OutgoingResponse> MyController::generate(
    const oatpp::Object<GenerationParams>& generation_params
) {
    const auto& model = generation_params->model;

    const auto model_data_opt = get_model_data(model);
    if (!model_data_opt) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "model '" + model + "' not found";
        return createDtoResponse(Status::CODE_200, error_result);
    }
    const auto& model_data = model_data_opt->first;

    if (!generation_params->prompt) {
        return handle_load_unload(
            model,
            model_data,
            generation_params->options,
            generation_params->keep_alive,
            false
        );
    }
    minja::chat_template templ(
        model_data.template_params.chat_template,
        model_data.template_params.bos_token,
        model_data.template_params.eos_token
    );

    const auto& prompt = generation_params->prompt;
    const auto stream = generation_params->stream;

    minja::chat_template_inputs inputs;
    inputs.add_generation_prompt = true;
    inputs.messages = json {{{"role", "user"}, {"content", prompt}}};

    const std::string prompt_templ = templ.apply(inputs);

    return handle_completion(
        model_data,
        prompt_templ,
        generation_params->options,
        stream,
        generation_params->keep_alive,
        model,
        ReturnType::RESPONSE
    );
}

std::shared_ptr<oat::OutgoingResponse>
MyController::chat(const oatpp::Object<ChatParams>& generation_params) {
    const auto& model = generation_params->model;
    const auto model_data_opt = get_model_data(model);
    if (!model_data_opt) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "model '" + model + "' not found";
        return createDtoResponse(Status::CODE_200, error_result);
    }
    const auto& model_data = model_data_opt->first;

    if (!generation_params->messages) {
        return handle_load_unload(
            model,
            model_data,
            generation_params->options,
            generation_params->keep_alive,
            true
        );
    }
    minja::chat_template templ(
        model_data.template_params.chat_template,
        model_data.template_params.bos_token,
        model_data.template_params.eos_token
    );

    const auto stream = generation_params->stream;

    minja::chat_template_inputs inputs;
    inputs.add_generation_prompt = true;
    inputs.messages =
        json::parse(m_contentMappers->getDefaultMapper()
                        ->writeToString(generation_params->messages)
                        .getValue(""));

    const std::string prompt_templ = templ.apply(inputs);

    return handle_completion(
        model_data,
        prompt_templ,
        generation_params->options,
        stream,
        generation_params->keep_alive,
        model,
        ReturnType::MESSAGE
    );
}

std::shared_ptr<oat::OutgoingResponse> MyController::chat_completions(
    const oatpp::Object<CreateChatCompletionParams>& generation_params
) {
    if (!generation_params->messages) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "messages field is required";
        return createDtoResponse(Status::CODE_200, error_result);
    }
    if (generation_params->n != 1) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "only n == 1 is supported";
        return createDtoResponse(Status::CODE_200, error_result);
    }
    if (generation_params->stream) {
        auto error_result = ErrorResponse::createShared();
        error_result->error =
            "streaming not supported on this endpoint. Use /api/chat endpoint instead.";
        return createDtoResponse(Status::CODE_200, error_result);
    }
    const auto& model = generation_params->model;
    const auto model_data_opt = get_model_data(model);
    if (!model_data_opt) {
        auto error_result = ErrorResponse::createShared();
        error_result->error = "model '" + model + "' not found";
        return createDtoResponse(Status::CODE_200, error_result);
    }
    const auto& model_data = model_data_opt->first;

    minja::chat_template templ(
        model_data.template_params.chat_template,
        model_data.template_params.bos_token,
        model_data.template_params.eos_token
    );

    const auto stream = generation_params->stream;

    minja::chat_template_inputs inputs;
    inputs.add_generation_prompt = true;
    inputs.messages =
        json::parse(m_contentMappers->getDefaultMapper()
                        ->writeToString(generation_params->messages)
                        .getValue(""));

    const std::string prompt_templ = templ.apply(inputs);

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

    return handle_completion(
        model_data,
        prompt_templ,
        model_options,
        stream,
        oatpp::Int32(nullptr),
        model,
        ReturnType::COMPLETION
    );
}

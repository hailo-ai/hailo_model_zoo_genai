/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file controller_test.cpp
 * @brief Tests for MyController
 **/

#include "controller_test.hpp"

#include <regex>
#include <sstream>
#include <string>

#include <oatpp-test/web/ClientServerTestRunner.hpp>
#include <oatpp/base/Log.hpp>
#include <oatpp/web/client/HttpRequestExecutor.hpp>

#include "app/api_test_client.hpp"
#include "app/test_component.hpp"
#include "app/test_model_store.hpp"
#include "app/test_resource_provider.hpp"
#include "controller/controller.hpp"
#include "dto/DTOs.hpp"
#include "generation_context/deconfigure.hpp"
#include "generation_context/generation_context.hpp"

namespace {
const std::regex& get_time_format_regex() {
    static const std::regex time_format_regex(
        R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{9}Z)"
    );
    return time_format_regex;
}

void test_root(const std::shared_ptr<MyApiTestClient>& client) {
    /* Call server API */
    /* Call root endpoint of MyController */
    auto root_response = client->getRoot();

    /* Assert that server responds with 200 */
    OATPP_ASSERT(root_response)
    OATPP_ASSERT(root_response->getStatusCode() == 200)
}

void test_version(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto version_response = client->getVersion();

    /* Assert that server responds with 200 */
    OATPP_ASSERT(version_response)
    OATPP_ASSERT(version_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto version_message =
        version_response->readBodyToDto<oatpp::Object<VersionResponse>>(
            contentMappers->selectMapperForContent(
                version_response->getHeader("Content-Type")
            )
        );

    /* Assert that received message is as expected */
    OATPP_ASSERT(version_message);
}

void test_list_all_models(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto list_response = client->getAllModels();
    OATPP_ASSERT(list_response);
    OATPP_ASSERT(list_response->getStatusCode() == 200);
    /* Read response body as MessageDto */
    auto list_message =
        list_response->readBodyToDto<oatpp::Object<ListAllResponse>>(
            contentMappers->selectMapperForContent(
                list_response->getHeader("Content-Type")
            )
        );

    OATPP_ASSERT(list_message->models->size() == 2);
    OATPP_ASSERT(list_message->models[0] == "qwen2:1.5b");
    OATPP_ASSERT(list_message->models[1] == "no_model:latest");
}

void test_list_models(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto tags_response = client->getTags();
    OATPP_ASSERT(tags_response);
    OATPP_ASSERT(tags_response->getStatusCode() == 200);
    /* Read response body as MessageDto */
    auto tags_message =
        tags_response->readBodyToDto<oatpp::Object<TagsResponse>>(
            contentMappers->selectMapperForContent(
                tags_response->getHeader("Content-Type")
            )
        );

    OATPP_LOGi("tags", "Got model {}", tags_message->models[0]->name);
    OATPP_ASSERT(tags_message->models->size() == 1);
    OATPP_ASSERT(tags_message->models[0]->name == "qwen2:1.5b");
    OATPP_ASSERT(tags_message->models[0]->model == "qwen2:1.5b");
    OATPP_ASSERT(tags_message->models[0]->size == 1678181695);
    // Regular expression to match the desired format

    OATPP_ASSERT(
        std::regex_match(
            tags_message->models[0]->modified_at.getValue(""),
            get_time_format_regex()
        )
    );
    OATPP_ASSERT(tags_message->models[0]->details);
    OATPP_ASSERT(tags_message->models[0]->details->parent_model == "");
    OATPP_ASSERT(tags_message->models[0]->details->format == "hef");
    OATPP_ASSERT(tags_message->models[0]->details->family == "qwen2");
    OATPP_ASSERT(tags_message->models[0]->details->families);
    OATPP_ASSERT(tags_message->models[0]->details->families->size() == 1);
    OATPP_ASSERT(tags_message->models[0]->details->families[0] == "qwen2");
    OATPP_ASSERT(tags_message->models[0]->details->parameter_size == "1.5B");
    OATPP_ASSERT(
        tags_message->models[0]->details->quantization_level == "Q4_0"
    );
}

void test_show_params_no_model(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto show_params_no_model = ShowParams::createShared();
    show_params_no_model->model = "does_not_exist";

    auto show_no_model_response = client->postShow(show_params_no_model);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(show_no_model_response)
    OATPP_ASSERT(show_no_model_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto show_no_model_message =
        show_no_model_response->readBodyToDto<oatpp::Object<ErrorResponse>>(
            contentMappers->selectMapperForContent(
                show_no_model_response->getHeader("Content-Type")
            )
        );
    OATPP_ASSERT(
        show_no_model_message->error == "model 'does_not_exist' not found"
    );
}

void test_show_model_normal(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto show_params = ShowParams::createShared();
    show_params->model = "qwen2:1.5b";

    auto show_response = client->postShow(show_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(show_response)
    OATPP_ASSERT(show_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto show_message =
        show_response->readBodyToDto<oatpp::Object<ShowResponse>>(
            contentMappers->selectMapperForContent(
                show_response->getHeader("Content-Type")
            )
        );
    OATPP_ASSERT(show_message->license == "Dummy license");
    OATPP_ASSERT(show_message->modelfile == "");
    OATPP_LOGi("tests", show_message->parameters);
    OATPP_ASSERT(
        show_message->parameters
        == "stop                             \"<|endoftext|>\"\nstop                             \"<|im_end|>\""
    );
    OATPP_ASSERT(show_message->model_info == "");
    OATPP_ASSERT(
        show_message->chat_template
        == ("{% for message in messages %}{% if loop.first and messages[0]['role'] != 'system' "
            "%}{{ '<|im_start|>system\nYou are a helpful assistant.<|im_end|>\n' }}{% endif "
            "%}{{'<|im_start|>' + message['role'] + '\n' + message['content'] + '<|im_end|>' + "
            "'\n'}}{% endfor %}{% if add_generation_prompt %}{{ '<|im_start|>assistant\n' }}{% "
            "endif %}")
    );
    OATPP_ASSERT(
        std::regex_match(
            show_message->modified_at.getValue(""),
            get_time_format_regex()
        )
    );
    OATPP_ASSERT(show_message->details);
    OATPP_ASSERT(show_message->details->parent_model == "");
    OATPP_ASSERT(show_message->details->format == "hef");
    OATPP_ASSERT(show_message->details->family == "qwen2");
    OATPP_ASSERT(show_message->details->families);
    OATPP_ASSERT(show_message->details->families->size() == 1);
    OATPP_ASSERT(show_message->details->families[0] == "qwen2");
    OATPP_ASSERT(show_message->details->parameter_size == "1.5B");
    OATPP_ASSERT(show_message->details->quantization_level == "Q4_0");
    OATPP_ASSERT(!show_message->capabilities);
}

void test_ps_no_running_models(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto ps_empty_response = client->getRunningModels();
    OATPP_ASSERT(ps_empty_response);
    OATPP_ASSERT(ps_empty_response->getStatusCode() == 200);
    /* Read response body as MessageDto */
    auto ps_empty_message =
        ps_empty_response->readBodyToDto<oatpp::Object<TagsResponse>>(
            contentMappers->selectMapperForContent(
                ps_empty_response->getHeader("Content-Type")
            )
        );

    OATPP_ASSERT(ps_empty_message->models->size() == 0);
}

void test_generation_no_model(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto generation_params_no_model = GenerationParams::createShared();
    generation_params_no_model->model = "does_not_exist";
    generation_params_no_model->prompt = "Doesn't matter";

    auto generation_no_model_response =
        client->postGenerate(generation_params_no_model);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(generation_no_model_response)
    OATPP_ASSERT(generation_no_model_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto generation_no_model_message =
        generation_no_model_response
            ->readBodyToDto<oatpp::Object<ErrorResponse>>(
                contentMappers->selectMapperForContent(
                    generation_no_model_response->getHeader("Content-Type")
                )
            );
    OATPP_ASSERT(
        generation_no_model_message->error == "model 'does_not_exist' not found"
    );
}

void test_generation_load_and_ps(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto generation_load_params = GenerationParams::createShared();
    // load is a generation/chat without prompt
    generation_load_params->model = "qwen2:1.5b";

    auto generation_load_response =
        client->postGenerate(generation_load_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(generation_load_response)
    OATPP_ASSERT(generation_load_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto generation_load_message =
        generation_load_response
            ->readBodyToDto<oatpp::Object<GenerationResponseFinal>>(
                contentMappers->selectMapperForContent(
                    generation_load_response->getHeader("Content-Type")
                )
            );

    /* Assert that received message is as expected */
    OATPP_ASSERT(generation_load_message);
    OATPP_ASSERT(generation_load_message->model == "qwen2:1.5b");
    OATPP_ASSERT(generation_load_message->done == true);
    OATPP_ASSERT(generation_load_message->done_reason == "load");

    // Check if the output matches the format
    OATPP_ASSERT(
        std::regex_match(
            generation_load_message->created_at.getValue(""),
            get_time_format_regex()
        )
    );

    OATPP_ASSERT(generation_load_message->response == "");

    auto ps_response = client->getRunningModels();
    OATPP_ASSERT(ps_response);
    OATPP_ASSERT(ps_response->getStatusCode() == 200);
    /* Read response body as MessageDto */
    auto ps_message = ps_response->readBodyToDto<oatpp::Object<TagsResponse>>(
        contentMappers->selectMapperForContent(
            ps_response->getHeader("Content-Type")
        )
    );

    OATPP_LOGi("ps", "Got model {}", ps_message->models[0]->name);
    OATPP_ASSERT(ps_message->models->size() == 1);
    OATPP_ASSERT(ps_message->models[0]->name == "qwen2:1.5b");
    OATPP_ASSERT(ps_message->models[0]->model == "qwen2:1.5b");
    OATPP_ASSERT(ps_message->models[0]->size == 1678181695);
    OATPP_ASSERT(
        std::regex_match(
            ps_message->models[0]->modified_at.getValue(""),
            get_time_format_regex()
        )
    );
    OATPP_ASSERT(ps_message->models[0]->details);
    OATPP_ASSERT(ps_message->models[0]->details->parent_model == "");
    OATPP_ASSERT(ps_message->models[0]->details->format == "hef");
    OATPP_ASSERT(ps_message->models[0]->details->family == "qwen2");
    OATPP_ASSERT(ps_message->models[0]->details->families);
    OATPP_ASSERT(ps_message->models[0]->details->families->size() == 1);
    OATPP_ASSERT(ps_message->models[0]->details->families[0] == "qwen2");
    OATPP_ASSERT(ps_message->models[0]->details->parameter_size == "1.5B");
    OATPP_ASSERT(ps_message->models[0]->details->quantization_level == "Q4_0");
    OATPP_ASSERT(
        std::regex_match(
            ps_message->models[0]->expires_at.getValue(""),
            get_time_format_regex()
        )
    );
}

void test_generation(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto generation_params = GenerationParams::createShared();
    generation_params->model = "qwen2:1.5b";
    generation_params->prompt = "Tell me a joke";
    generation_params->stream = false;
    generation_params->options = ModelParameters::createShared();
    generation_params->options->temperature = 0.0F;

    auto generation_response = client->postGenerate(generation_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(generation_response)
    OATPP_ASSERT(generation_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto generation_message =
        generation_response
            ->readBodyToDto<oatpp::Object<GenerationResponseFinal>>(
                contentMappers->selectMapperForContent(
                    generation_response->getHeader("Content-Type")
                )
            );

    /* Assert that received message is as expected */
    OATPP_ASSERT(generation_message);
    OATPP_ASSERT(generation_message->model == "qwen2:1.5b");
    OATPP_ASSERT(generation_message->done == true);
    OATPP_ASSERT(generation_message->done_reason == "stop");

    // Check if the output matches the format
    OATPP_ASSERT(
        std::regex_match(
            generation_message->created_at.getValue(""),
            get_time_format_regex()
        )
    );
    OATPP_ASSERT(generation_message->total_duration)
    OATPP_ASSERT(generation_message->eval_count)
    OATPP_LOGi("test::generation", generation_message->response);
    OATPP_ASSERT(
        generation_message->response
        == "Sure, here's a joke for you:\n\nWhy did the tomato never get into trouble?\n\nBecause it always wore its red hat!"
    );
}

void test_chat(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto chat_params = ChatParams::createShared();
    chat_params->model = "qwen2:1.5b";
    auto message = ChatMessage::createShared();
    message->role = "user";
    message->content = "Tell me a joke";
    chat_params->messages = {message};
    chat_params->stream = false;
    chat_params->options = ModelParameters::createShared();
    chat_params->options->temperature = 0.0F;

    auto chat_response = client->postChat(chat_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(chat_response)
    OATPP_ASSERT(chat_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto chat_message =
        chat_response->readBodyToDto<oatpp::Object<GenerationResponseFinal>>(
            contentMappers->selectMapperForContent(
                chat_response->getHeader("Content-Type")
            )
        );

    /* Assert that received message is as expected */
    OATPP_ASSERT(chat_message);
    OATPP_ASSERT(chat_message->model == "qwen2:1.5b");
    OATPP_ASSERT(chat_message->done == true);
    OATPP_ASSERT(chat_message->done_reason == "stop");
    OATPP_ASSERT(
        std::regex_match(
            chat_message->created_at.getValue(""),
            get_time_format_regex()
        )
    );
    OATPP_ASSERT(chat_message->total_duration)
    OATPP_ASSERT(chat_message->eval_count)
    OATPP_ASSERT(chat_message->message);
    OATPP_LOGi("test::chat", chat_message->message->content);
    OATPP_ASSERT(chat_message->message->role == "assistant");
    OATPP_ASSERT(
        chat_message->message->content
        == "Sure, here's a joke for you:\n\nWhy did the tomato never get into trouble?\n\nBecause it always wore its red hat!"
    );
}

void test_chat_max_tokens(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto chat_params = ChatParams::createShared();
    chat_params->model = "qwen2:1.5b";
    auto message = ChatMessage::createShared();
    message->role = "user";
    message->content = "Tell me a joke";
    chat_params->messages = {message};
    chat_params->stream = false;
    chat_params->options = ModelParameters::createShared();
    chat_params->options->temperature = 0.0F;
    chat_params->options->num_predict = 3;

    auto chat_response = client->postChat(chat_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(chat_response)
    OATPP_ASSERT(chat_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto chat_message =
        chat_response->readBodyToDto<oatpp::Object<GenerationResponseFinal>>(
            contentMappers->selectMapperForContent(
                chat_response->getHeader("Content-Type")
            )
        );

    /* Assert that received message is as expected */
    OATPP_ASSERT(chat_message);
    OATPP_ASSERT(chat_message->model == "qwen2:1.5b");
    OATPP_ASSERT(chat_message->done == true);
    OATPP_ASSERT(chat_message->done_reason == "length");
    OATPP_ASSERT(
        std::regex_match(
            chat_message->created_at.getValue(""),
            get_time_format_regex()
        )
    );
    OATPP_ASSERT(chat_message->total_duration)
    OATPP_ASSERT(chat_message->eval_count)
    OATPP_ASSERT(chat_message->message);
    OATPP_ASSERT(chat_message->message->role == "assistant");
    OATPP_ASSERT(chat_message->message->content == "Sure, here");
}

void test_chat_streaming(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto chat_stream_params = ChatParams::createShared();
    chat_stream_params->model = "qwen2:1.5b";
    auto stream_message = ChatMessage::createShared();
    stream_message->role = "user";
    stream_message->content = "Tell me a joke";
    chat_stream_params->messages = {stream_message};
    chat_stream_params->stream = true;
    chat_stream_params->options = ModelParameters::createShared();
    chat_stream_params->options->temperature = 0.0F;

    auto chat_stream_response = client->postChat(chat_stream_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(chat_stream_response)
    OATPP_ASSERT(chat_stream_response->getStatusCode() == 200)

    OATPP_ASSERT(
        chat_stream_response->getHeader("Content-Type")
        == "application/x-ndjson"
    )
    auto chat_stream_message = chat_stream_response->readBodyToString();
    OATPP_LOGi("test::stream", chat_stream_message);

    std::stringstream chat_stream_body;
    chat_stream_body << chat_stream_message.getValue("");

    std::string line;
    std::string last_line;
    auto json_mapper = contentMappers->getMapper("application/json");
    std::stringstream generation;
    while (std::getline(chat_stream_body, line)) {
        const auto line_response =
            json_mapper->readFromString<oatpp::Object<GenerationResponse>>(
                oatpp::String(line)
            );

        last_line = std::move(line);
        OATPP_ASSERT(line_response->model == "qwen2:1.5b");
        OATPP_ASSERT(
            std::regex_match(
                line_response->created_at.getValue(""),
                get_time_format_regex()
            )
        );
        OATPP_ASSERT(line_response->message);
        OATPP_ASSERT(line_response->message->role == "assistant");
        generation << line_response->message->content.getValue("");
    }

    OATPP_ASSERT(
        generation.str()
        == "Sure, here's a joke for you:\n\nWhy did the tomato never get into trouble?\n\nBecause it always wore its red hat!"
    );

    // check the last line which is the final response
    const auto chat_message =
        json_mapper->readFromString<oatpp::Object<GenerationResponseFinal>>(
            oatpp::String(last_line)
        );
    /* Assert that received message is as expected */
    OATPP_ASSERT(chat_message);
    OATPP_ASSERT(chat_message->model == "qwen2:1.5b");
    OATPP_ASSERT(chat_message->done == true);
    OATPP_ASSERT(chat_message->done_reason == "stop");
    OATPP_ASSERT(
        std::regex_match(
            chat_message->created_at.getValue(""),
            get_time_format_regex()
        )
    );
    OATPP_ASSERT(chat_message->total_duration)
    OATPP_ASSERT(chat_message->eval_count)
    OATPP_ASSERT(chat_message->message);
    OATPP_ASSERT(chat_message->message->role == "assistant");
    OATPP_ASSERT(chat_message->message->content == "");
}

void test_chat_streaming_max_tokens(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto chat_stream_params = ChatParams::createShared();
    chat_stream_params->model = "qwen2:1.5b";
    auto stream_message = ChatMessage::createShared();
    stream_message->role = "user";
    stream_message->content = "Tell me a joke";
    chat_stream_params->messages = {stream_message};
    chat_stream_params->stream = true;
    chat_stream_params->options = ModelParameters::createShared();
    chat_stream_params->options->temperature = 0.0F;
    chat_stream_params->options->num_predict = 3;

    auto chat_stream_response = client->postChat(chat_stream_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(chat_stream_response)
    OATPP_ASSERT(chat_stream_response->getStatusCode() == 200)

    OATPP_ASSERT(
        chat_stream_response->getHeader("Content-Type")
        == "application/x-ndjson"
    )
    auto chat_stream_message = chat_stream_response->readBodyToString();

    std::stringstream chat_stream_body;
    chat_stream_body << chat_stream_message.getValue("");

    std::string line;
    std::string last_line;
    auto json_mapper = contentMappers->getMapper("application/json");
    std::stringstream generation;
    while (std::getline(chat_stream_body, line)) {
        const auto line_response =
            json_mapper->readFromString<oatpp::Object<GenerationResponse>>(
                oatpp::String(line)
            );
        last_line = std::move(line);

        OATPP_ASSERT(line_response->model == "qwen2:1.5b");
        OATPP_ASSERT(
            std::regex_match(
                line_response->created_at.getValue(""),
                get_time_format_regex()
            )
        );
        OATPP_ASSERT(line_response->message);
        OATPP_ASSERT(line_response->message->role == "assistant");
        generation << line_response->message->content.getValue("");
    }

    OATPP_ASSERT(generation.str() == "Sure, here");
    //
    // check the last line which is the final response
    const auto chat_message =
        json_mapper->readFromString<oatpp::Object<GenerationResponseFinal>>(
            oatpp::String(last_line)
        );
    /* Assert that received message is as expected */
    OATPP_ASSERT(chat_message);
    OATPP_ASSERT(chat_message->model == "qwen2:1.5b");
    OATPP_ASSERT(chat_message->done == true);
    OATPP_ASSERT(chat_message->done_reason == "length");
    OATPP_ASSERT(
        std::regex_match(
            chat_message->created_at.getValue(""),
            get_time_format_regex()
        )
    );
    OATPP_ASSERT(chat_message->total_duration)
    OATPP_ASSERT(chat_message->eval_count)
    OATPP_ASSERT(chat_message->message);
    OATPP_ASSERT(chat_message->message->role == "assistant");
    OATPP_ASSERT(chat_message->message->content == "");
}

void test_chat_completions(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto chat_params = CreateChatCompletionParams::createShared();
    chat_params->model = "qwen2:1.5b";
    auto message = ChatCompletionMessage::createShared();
    message->role = "user";
    message->content = "Tell me a joke";
    chat_params->messages = {message};
    chat_params->stream = false;
    chat_params->temperature = 0.0F;
    chat_params->n = 1;

    auto chat_response = client->postChatCompletions(chat_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(chat_response)
    OATPP_ASSERT(chat_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto chat_message =
        chat_response
            ->readBodyToDto<oatpp::Object<CreateChatCompletionResponse>>(
                contentMappers->selectMapperForContent(
                    chat_response->getHeader("Content-Type")
                )
            );

    /* Assert that received message is as expected */
    OATPP_ASSERT(chat_message);
    OATPP_ASSERT(chat_message->id);
    OATPP_ASSERT(chat_message->id.getValue("").find("chatcmpl-", 0) == 0);
    OATPP_ASSERT(chat_message->object == "chat.completion");
    OATPP_ASSERT(chat_message->created);
    OATPP_ASSERT(chat_message->model == "qwen2:1.5b");
    OATPP_ASSERT(chat_message->choices);
    OATPP_ASSERT(chat_message->choices->size() == 1);
    OATPP_ASSERT(chat_message->choices[0]->index == 0);
    OATPP_ASSERT(chat_message->choices[0]->finish_reason == "stop");
    OATPP_ASSERT(chat_message->choices[0]->message->role == "assistant");
    OATPP_ASSERT(
        chat_message->choices[0]->message->content
        == "Sure, here's a joke for you:\n\nWhy did the tomato never get into trouble?\n\nBecause it always wore its red hat!"
    );
}

void test_chat_completions_too_many_choices(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto chat_params = CreateChatCompletionParams::createShared();
    chat_params->model = "qwen2:1.5b";
    auto message = ChatCompletionMessage::createShared();
    message->role = "user";
    message->content = "Tell me a joke";
    chat_params->messages = {message};
    chat_params->stream = false;
    chat_params->temperature = 0.0F;
    chat_params->n = 2;

    auto chat_response = client->postChatCompletions(chat_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(chat_response)
    OATPP_ASSERT(chat_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto chat_message =
        chat_response->readBodyToDto<oatpp::Object<ErrorResponse>>(
            contentMappers->selectMapperForContent(
                chat_response->getHeader("Content-Type")
            )
        );

    /* Assert that received message is as expected */
    OATPP_ASSERT(chat_message);
    OATPP_ASSERT(chat_message->error == "only n == 1 is supported");
}

void test_chat_completions_no_messages(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto chat_params = CreateChatCompletionParams::createShared();
    chat_params->model = "qwen2:1.5b";
    chat_params->stream = false;
    chat_params->temperature = 0.0F;

    auto chat_response = client->postChatCompletions(chat_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(chat_response)
    OATPP_ASSERT(chat_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto chat_message =
        chat_response->readBodyToDto<oatpp::Object<ErrorResponse>>(
            contentMappers->selectMapperForContent(
                chat_response->getHeader("Content-Type")
            )
        );

    /* Assert that received message is as expected */
    OATPP_ASSERT(chat_message);
    OATPP_ASSERT(chat_message->error == "messages field is required");
}

void test_chat_completions_streaming_should_fail(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto chat_params = CreateChatCompletionParams::createShared();
    chat_params->model = "qwen2:1.5b";
    auto message = ChatCompletionMessage::createShared();
    message->role = "user";
    message->content = "Tell me a joke";
    chat_params->messages = {message};
    chat_params->stream = true;
    chat_params->temperature = 0.0F;

    auto chat_response = client->postChatCompletions(chat_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(chat_response)
    OATPP_ASSERT(chat_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto chat_message =
        chat_response->readBodyToDto<oatpp::Object<ErrorResponse>>(
            contentMappers->selectMapperForContent(
                chat_response->getHeader("Content-Type")
            )
        );

    /* Assert that received message is as expected */
    OATPP_ASSERT(chat_message);
    OATPP_ASSERT(
        chat_message->error
        == "streaming not supported on this endpoint. Use /api/chat endpoint instead."
    );
}

void test_unload_and_ps(
    const std::shared_ptr<MyApiTestClient>& client,
    const std::shared_ptr<oatpp::web::mime::ContentMappers>& contentMappers
) {
    auto generation_unload_params = GenerationParams::createShared();
    // unload is a generation/chat without prompt and keep_alive == 0
    generation_unload_params->model = "qwen2:1.5b";
    generation_unload_params->keep_alive = 0;

    auto generation_unload_response =
        client->postGenerate(generation_unload_params);

    /* Assert that server responds with 200 */
    OATPP_ASSERT(generation_unload_response)
    OATPP_ASSERT(generation_unload_response->getStatusCode() == 200)

    /* Read response body as MessageDto */
    auto generation_unload_message =
        generation_unload_response
            ->readBodyToDto<oatpp::Object<GenerationResponseFinal>>(
                contentMappers->selectMapperForContent(
                    generation_unload_response->getHeader("Content-Type")
                )
            );

    /* Assert that received message is as expected */
    OATPP_ASSERT(generation_unload_message);
    OATPP_ASSERT(generation_unload_message->model == "qwen2:1.5b");
    OATPP_ASSERT(generation_unload_message->done == true);
    OATPP_ASSERT(generation_unload_message->done_reason == "unload");

    // Check if the output matches the format
    OATPP_ASSERT(
        std::regex_match(
            generation_unload_message->created_at.getValue(""),
            get_time_format_regex()
        )
    );

    OATPP_ASSERT(generation_unload_message->response == "");

    auto ps_empty_after_unload_response = client->getRunningModels();
    OATPP_ASSERT(ps_empty_after_unload_response);
    OATPP_ASSERT(ps_empty_after_unload_response->getStatusCode() == 200);
    /* Read response body as MessageDto */
    auto ps_empty_after_unload_message =
        ps_empty_after_unload_response
            ->readBodyToDto<oatpp::Object<TagsResponse>>(
                contentMappers->selectMapperForContent(
                    ps_empty_after_unload_response->getHeader("Content-Type")
                )
            );

    OATPP_ASSERT(ps_empty_after_unload_message->models->size() == 0);
}
}  // namespace

MyControllerTest::MyControllerTest(const char* test_hef) :
    UnitTest("TEST[MyControllerTest]"),
    m_test_hef(test_hef) {}

void MyControllerTest::onRun() {
    /* Register test components */
    TestComponent component;

    /* Create client-server test runner */
    oatpp::test::web::ClientServerTestRunner runner;

    /* Add MyController endpoints to the router of the test server */
    auto generation_context = std::make_shared<SyncGenerationContext>();
    Deconfigure deconfigure_loop(generation_context);
    std::thread deconfigure_thread(
        &Deconfigure::deconfigure_loop,
        &deconfigure_loop
    );

    auto model_store = std::make_shared<TestModelStore>();
    auto resource_provider = std::make_shared<TestResourceProvider>(m_test_hef);

    runner.addController(
        std::make_shared<MyController>(
            generation_context,
            model_store,
            resource_provider
        )
    );

    /* Run test */
    runner.run(
        [this, &runner] {
            /* Get client connection provider for Api Client */
            OATPP_COMPONENT(
                std::shared_ptr<oatpp::network::ClientConnectionProvider>,
                clientConnectionProvider
            );

            /* Get object mapper component */
            OATPP_COMPONENT(
                std::shared_ptr<oatpp::web::mime::ContentMappers>,
                contentMappers
            );

            /* Create http request executor for Api Client */
            auto requestExecutor =
                oatpp::web::client::HttpRequestExecutor::createShared(
                    clientConnectionProvider
                );

            /* Create Test API client */
            auto client = MyApiTestClient::createShared(
                requestExecutor,
                contentMappers->getMapper("application/json")
            );

            test_root(client);
            test_version(client, contentMappers);

            test_list_all_models(client, contentMappers);
            test_list_models(client, contentMappers);

            test_show_params_no_model(client, contentMappers);
            test_show_model_normal(client, contentMappers);

            test_ps_no_running_models(client, contentMappers);
            test_generation_no_model(client, contentMappers);
            test_generation_load_and_ps(client, contentMappers);

            test_generation(client, contentMappers);
            test_chat(client, contentMappers);
            test_chat_max_tokens(client, contentMappers);

            test_chat_streaming(client, contentMappers);
            test_chat_streaming_max_tokens(client, contentMappers);

            test_chat_completions_too_many_choices(client, contentMappers);
            test_chat_completions_no_messages(client, contentMappers);
            test_chat_completions_streaming_should_fail(client, contentMappers);
            test_chat_completions(client, contentMappers);

            test_unload_and_ps(client, contentMappers);
        },
        std::chrono::minutes(10) /* test timeout */
    );

    generation_context->lock()->stop();
    deconfigure_thread.join();
    /* wait all server threads finished */
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

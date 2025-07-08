/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file DTOs.hpp
 * @brief Definitions for Data Transfer Objects
 **/

#pragma once

#include <oatpp/Types.hpp>
#include <oatpp/macro/codegen.hpp>

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ModelParameters: public oatpp::DTO {
    DTO_INIT(ModelParameters, DTO)

    DTO_FIELD(Float32, temperature);
    DTO_FIELD(Int32, seed);
    DTO_FIELD(UInt32, top_k);
    DTO_FIELD(Float32, top_p);
    DTO_FIELD(Float32, frequency_penalty);

    DTO_FIELD(UInt32, num_predict);
    // Meaningless when running on hailo hardware but we need it for compatibility
    DTO_FIELD(Boolean, use_mlock);
};

class GenerationParams: public oatpp::DTO {
    DTO_INIT(GenerationParams, DTO)

    DTO_FIELD(String, model);
    DTO_FIELD(String, prompt);
    DTO_FIELD(String, suffix);
    DTO_FIELD(Vector<String>, images);
    DTO_FIELD(String, format);
    DTO_FIELD(Object<ModelParameters>, options);
    DTO_FIELD(String, response_template, "template");
    DTO_FIELD(Boolean, stream) = true;
    DTO_FIELD(Boolean, raw);
    DTO_FIELD(Int32, keep_alive);
};

class ChatMessage: public oatpp::DTO {
    DTO_INIT(ChatMessage, DTO)

    DTO_FIELD(String, role);
    DTO_FIELD(String, content);
};

class ChatParams: public oatpp::DTO {
    DTO_INIT(ChatParams, DTO)

    DTO_FIELD(String, model);
    // These are actually json but we use nlohmann/json for them
    DTO_FIELD(Vector<Object<ChatMessage>>, messages);
    DTO_FIELD(String, tools);

    DTO_FIELD(String, format);
    DTO_FIELD(Object<ModelParameters>, options);
    DTO_FIELD(Boolean, stream) = true;
    DTO_FIELD(Int32, keep_alive);
};

class GenerationResponse: public oatpp::DTO {
    DTO_INIT(GenerationResponse, DTO)

    DTO_FIELD(String, model);
    DTO_FIELD(String, created_at);
    DTO_FIELD(String, response);
    DTO_FIELD(Object<ChatMessage>, message);
    DTO_FIELD(Boolean, done);
};

class GenerationResponseFinal: public oatpp::DTO {
    DTO_INIT(GenerationResponseFinal, DTO)

    DTO_FIELD(String, model);
    DTO_FIELD(String, created_at);
    DTO_FIELD(String, response);
    DTO_FIELD(Object<ChatMessage>, message);
    DTO_FIELD(Boolean, done);
    DTO_FIELD(String, done_reason);
    DTO_FIELD(UInt64, total_duration);
    DTO_FIELD(UInt64, load_duration);
    DTO_FIELD(UInt64, eval_count);
    DTO_FIELD(UInt64, eval_duration);
    DTO_FIELD(UInt64, prompt_eval_count);
    DTO_FIELD(UInt64, prompt_eval_duration);
    DTO_FIELD(Vector<UInt64>, context);
};

class ChatCompletionMessage: public oatpp::DTO {
    DTO_INIT(ChatCompletionMessage, DTO)

    DTO_FIELD(String, role);
    DTO_FIELD(String, content);  // TODO: switch to Any
};

class CreateChatCompletionParams: public oatpp::DTO {
    DTO_INIT(CreateChatCompletionParams, DTO)

    DTO_FIELD(String, model);
    DTO_FIELD(Vector<Object<ChatCompletionMessage>>, messages);
    DTO_FIELD(Float32, frequency_penalty);
    DTO_FIELD(Float32, presence_penalty);
    DTO_FIELD(Int64, max_completion_tokens);
    DTO_FIELD(Int64, max_tokens);
    DTO_FIELD(Float32, temperature);
    DTO_FIELD(Int32, seed);
    DTO_FIELD(Float32, top_p);
    DTO_FIELD(Boolean, stream) = false;
    DTO_FIELD(Int8, n) = 1;
};

class ChatChoice: public oatpp::DTO {
    DTO_INIT(ChatChoice, DTO)

    DTO_FIELD(Int64, index);
    DTO_FIELD(Object<ChatCompletionMessage>, message);
    DTO_FIELD(String, finish_reason);
};

class CreateChatCompletionResponse: public oatpp::DTO {
    DTO_INIT(CreateChatCompletionResponse, DTO)

    DTO_FIELD(String, id);
    DTO_FIELD(String, object);
    DTO_FIELD(Int64, created);
    DTO_FIELD(String, model);
    DTO_FIELD(Vector<Object<ChatChoice>>, choices);
};

class ModelInfoDetails: public oatpp::DTO {
    DTO_INIT(ModelInfoDetails, DTO)

    DTO_FIELD(String, parent_model);
    DTO_FIELD(String, format) = "hef";
    DTO_FIELD(String, family);
    DTO_FIELD(Vector<String>, families);
    DTO_FIELD(String, parameter_size);
    DTO_FIELD(String, quantization_level);
};

class ModelInfoShort: public oatpp::DTO {
    DTO_INIT(ModelInfoShort, DTO)

    DTO_FIELD(String, name);
    DTO_FIELD(String, model);
    DTO_FIELD(String, modified_at);
    DTO_FIELD(UInt64, size);
    DTO_FIELD(String, digest);
    DTO_FIELD(Object<ModelInfoDetails>, details);
    DTO_FIELD(String, expires_at);
};

class ListAllResponse: public oatpp::DTO {
    DTO_INIT(ListAllResponse, DTO)

    DTO_FIELD(Vector<String>, models);
};

class TagsResponse: public oatpp::DTO {
    DTO_INIT(TagsResponse, DTO)

    DTO_FIELD(Vector<Object<ModelInfoShort>>, models);
};

class ShowParams: public oatpp::DTO {
    DTO_INIT(ShowParams, DTO)

    DTO_FIELD(String, model);
};

class ShowResponse: public oatpp::DTO {
    DTO_INIT(ShowResponse, DTO)

    DTO_FIELD(String, license);
    DTO_FIELD(String, modelfile);
    DTO_FIELD(String, parameters);
    DTO_FIELD(String, chat_template, "template");
    DTO_FIELD(Object<ModelInfoDetails>, details);
    DTO_FIELD(String, model_info);
    DTO_FIELD(Vector<String>, capabilities);
    DTO_FIELD(String, modified_at);
};

class PullParams: public oatpp::DTO {
    DTO_INIT(PullParams, DTO)

    DTO_FIELD(String, model);
    DTO_FIELD(Boolean, stream) = true;
};

class DeleteParams: public oatpp::DTO {
    DTO_INIT(DeleteParams, DTO)

    DTO_FIELD(String, model);
};

class DeleteErrorResponse: public oatpp::DTO {
    DTO_INIT(DeleteErrorResponse, DTO)

    DTO_FIELD(String, code);
    DTO_FIELD(String, error);
};

class PullResponse: public oatpp::DTO {
    DTO_INIT(PullResponse, DTO)

    DTO_FIELD(String, status);
    DTO_FIELD(String, digest);
    DTO_FIELD(Int64, total);
    DTO_FIELD(Int64, completed);
};

class VersionResponse: public oatpp::DTO {
    DTO_INIT(VersionResponse, DTO)

    DTO_FIELD(String, version);
};

class ErrorResponse: public oatpp::DTO {
    DTO_INIT(ErrorResponse, DTO)

    DTO_FIELD(String, error);
};

#include OATPP_CODEGEN_END(DTO)

/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file simple_store.cpp
 * @brief SimpleModelStore implementation
 **/

#include "simple_store.hpp"

#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "model/store.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    TemplateParamsInfo,
    chat_template,
    bos_token,
    eos_token
)

template<typename T>
std::optional<T> optional_from_json(const json& parent, std::string_view key) {
    auto j = parent.find(key);
    if (j == parent.end()) {
        return {};
    }
    if (j->is_null()) {
        return {};
    }
    return {j->template get<T>()};
}

// partial specialization (full specialization works too)
NLOHMANN_JSON_NAMESPACE_BEGIN

template<>
struct adl_serializer<GenerationParamsInfo> {
    static void from_json(const json& j, GenerationParamsInfo& info) {
        info.temperature = optional_from_json<float>(j, "temperature");
        info.top_p = optional_from_json<float>(j, "top_p");
        info.top_k = optional_from_json<float>(j, "top_k");
        info.frequency_penalty =
            optional_from_json<float>(j, "frequency_penalty");
        const auto& stop_tokens = j.find("stop_tokens");
        if (stop_tokens != j.end()) {
            info.stop_tokens =
                stop_tokens->template get<std::vector<std::string>>();
        }
    }
};

NLOHMANN_JSON_NAMESPACE_END

ModelInfo model_from_json(const std::string& name, const json& j) {
    const auto& template_params = j.at("template_params");
    const auto& generation_params = j.find("generation_params");
    GenerationParamsInfo generation_params_info = {};
    if (generation_params != j.end()) {
        generation_params_info =
            generation_params->template get<GenerationParamsInfo>();
    }
    const auto& details = j.find("details");
    std::string details_string = details != j.end() ? details->dump() : "";
    return ModelInfo {
        .name = name,
        .hef_resource = j.at("hef_h10h").template get<std::string>(),
        .template_params = template_params.template get<TemplateParamsInfo>(),
        .generation_params = std::move(generation_params_info),
        .details = std::move(details_string),
    };
}

SimpleModelStore::SimpleModelStore(const fs::path& path) : m_models() {
    // filesystem tree:
    // manifests (path variable)
    // |
    // |- model
    //   |- tag
    //      |- manifest.json
    for (const auto& dir_entry : fs::recursive_directory_iterator(path)) {
        // skip directories
        if (!dir_entry.is_regular_file()) {
            continue;
        }

        const auto& file_path = dir_entry.path();
        auto model = file_path.parent_path().parent_path().filename().string();
        auto tag = file_path.parent_path().filename().string();

        std::ifstream stream(file_path);
        const auto model_name = std::move(model) + ":" + std::move(tag);
        m_models.emplace(
            model_name,
            model_from_json(model_name, json::parse(stream))
        );
    }
}

std::optional<ModelInfo> SimpleModelStore::get_model(const std::string& name) {
    const auto model = m_models.find(name);
    if (model != m_models.end()) {
        return model->second;
    }
    return std::nullopt;
}

std::vector<std::string> SimpleModelStore::get_model_names() {
    std::vector<std::string> res;
    res.reserve(m_models.size());
    for (const auto& [model_name, model_info] : m_models) {
        res.push_back(model_name);
    }
    return res;
}

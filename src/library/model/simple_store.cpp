/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file simple_store.cpp
 * @brief SimpleModelStore implementation
 **/

#include "simple_store.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "model/store.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace hailo_ollama
{

ModelInfo model_from_json(const std::string &name, const json &j)
{
    const auto &details = j.find("details");
    std::string details_string = details != j.end() ? details->dump() : "";
    const auto &license = j.find("license");
    std::string license_string =
        (license != j.end() && !license->is_null()) ? license->template get<std::string>() : "";
    return ModelInfo{
        name,
        j.at("hef_h10h").template get<std::string>(),
        std::move(details_string),
        std::move(license_string)
    };
}

SimpleModelStore::SimpleModelStore(const fs::path &path) : m_models()
{
    // filesystem tree:
    // manifests (path variable)
    // |
    // |- model
    //   |- tag
    //      |- manifest.json
    for (const auto &dir_entry : fs::recursive_directory_iterator(path)) {
        // skip directories
        if (!dir_entry.is_regular_file()) {
            continue;
        }

        const auto &file_path = dir_entry.path();
        auto model = file_path.parent_path().parent_path().filename().string();
        auto tag = file_path.parent_path().filename().string();

        std::ifstream stream(file_path);
        const auto model_name = std::move(model) + ":" + std::move(tag);
        m_models.emplace(model_name, model_from_json(model_name, json::parse(stream)));
    }
}

std::optional<ModelInfo> SimpleModelStore::get_model(const std::string &name)
{
    const auto model = m_models.find(name);
    if (model != m_models.end()) {
        return model->second;
    }
    return std::nullopt;
}

std::vector<std::string> SimpleModelStore::get_model_names()
{
    std::vector<std::string> res;
    res.reserve(m_models.size());
    for (const auto &[model_name, model_info] : m_models) {
        res.push_back(model_name);
    }
    return res;
}

} // namespace hailo_ollama

/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file test_model_store.cpp
 * @brief TestModelStore implementation
 **/

#include "test_model_store.hpp"

#include <optional>
#include <string>
#include <vector>

#include "model/store.hpp"

std::optional<ModelInfo> TestModelStore::get_model(const std::string& name) {
    if (name == "no_model:latest") {
        return ModelInfo {.name = name, .hef_resource = "no/such/path"};
    }
    if (name != "qwen2" && name != "qwen2:1.5b") {
        return {};
    }
    return ModelInfo {
        .name = name,
        .hef_resource = "qwen2_base.hef",

        .template_params =
            {
                .chat_template =
                    ("{% for message in messages %}{% if loop.first and messages[0]['role'] != 'system' "
                     "%}{{ '<|im_start|>system\nYou are a helpful assistant.<|im_end|>\n' }}{% endif "
                     "%}{{'<|im_start|>' + message['role'] + '\n' + message['content'] + '<|im_end|>' + "
                     "'\n'}}{% endfor %}{% if add_generation_prompt %}{{ '<|im_start|>assistant\n' }}{% "
                     "endif %}"),
                .bos_token = "",
                .eos_token = "<|im_end|>",
            },
        .generation_params = {.stop_tokens = {"<|endoftext|>", "<|im_end|>"}},
        .details = R"({
                        "parent_model": "",
                        "format": "hef",
                        "family": "qwen2",
                        "families": [
                            "qwen2"
                        ],
                        "parameter_size": "1.5B",
                        "quantization_level": "Q4_0"
                    })",
        .license = "Dummy license"
    };
}

std::vector<std::string> TestModelStore::get_model_names() {
    return {"qwen2:1.5b", "no_model:latest"};
}

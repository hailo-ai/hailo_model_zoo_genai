add_library(hailo-ollama-lib
    config/runtime_config.hpp
    config/static_config.hpp
    controller/controller.cpp
    controller/controller.hpp
    controller/llm_generation_callback.cpp
    controller/llm_generation_callback.hpp
    controller/pull_callback.cpp
    controller/pull_callback.hpp
    controller/writefile_callback.cpp
    controller/writefile_callback.hpp
    dto/DTOs.hpp
    generation_context/deconfigure.cpp
    generation_context/deconfigure.hpp
    generation_context/generation_context.cpp
    generation_context/generation_context.hpp
    download/client.hpp
    model/resource.hpp
    model/store.hpp
    model/blob_resource.cpp
    model/blob_resource.hpp
    model/simple_store.cpp
    model/simple_store.hpp
    utils/path.hpp
    utils/path.cpp
    utils/split.hpp
    utils/split.cpp
    utils/time.hpp
    utils/time.cpp
    utils/sha256.cpp
    utils/sha256.hpp
)

set_target_properties(hailo-ollama-lib PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(hailo-ollama-lib
    PUBLIC oatpp
    PUBLIC oatpp-openssl
    PRIVATE HailoRT::libhailort
    PUBLIC nlohmann_json::nlohmann_json
    PUBLIC OpenSSL::SSL
    PUBLIC OpenSSL::Crypto
    PUBLIC eventpp::eventpp
    PRIVATE minja
    PUBLIC libguarded
)

target_include_directories(hailo-ollama-lib PUBLIC .)


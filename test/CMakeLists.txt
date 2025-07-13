add_executable(hailo-ollama-test
    tests.cpp
    app/test_component.hpp
    app/api_test_client.hpp
    app/test_model_store.cpp
    app/test_model_store.hpp
    app/test_resource_provider.cpp
    app/test_resource_provider.hpp
    controller_test.cpp
    controller_test.hpp
)

set_target_properties(hailo-ollama-test PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(hailo-ollama-test
    oatpp-test
    hailo-ollama-lib
)


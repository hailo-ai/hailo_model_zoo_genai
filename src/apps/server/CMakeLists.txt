add_executable(hailo-ollama
    app_component.hpp
    main.cpp
)

set_target_properties(hailo-ollama PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(hailo-ollama hailo-ollama-lib)

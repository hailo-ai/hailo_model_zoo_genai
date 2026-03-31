cmake_minimum_required(VERSION 3.20)

include(FetchContent)

fetchcontent_declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG        v3.12.0
    GIT_SHALLOW    TRUE
    SOURCE_DIR     ${HAILO_OLLAMA_EXTERNAL_DIR}/json-src
    SUBBUILD_DIR   ${HAILO_OLLAMA_EXTERNAL_DIR}/json-subbuild
)

fetchcontent_getproperties(json)
if(NOT json_POPULATED)
    fetchcontent_populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

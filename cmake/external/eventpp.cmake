cmake_minimum_required(VERSION 3.20)

include(FetchContent)

fetchcontent_declare(
    eventpp
    GIT_REPOSITORY https://github.com/wqking/eventpp
    GIT_TAG        v0.1.3
    GIT_SHALLOW    TRUE
    SOURCE_DIR     ${HAILO_OLLAMA_EXTERNAL_DIR}/eventpp-src
    SUBBUILD_DIR   ${HAILO_OLLAMA_EXTERNAL_DIR}/eventpp-subbuild
)

fetchcontent_getproperties(eventpp)
if(NOT eventpp_POPULATED)
    fetchcontent_populate(eventpp)
    add_subdirectory(${eventpp_SOURCE_DIR} ${eventpp_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

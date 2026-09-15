cmake_minimum_required(VERSION 3.20)

include(FetchContent)

fetchcontent_declare(
    minja
    GIT_REPOSITORY https://github.com/google/minja
    GIT_TAG        58568621432715b0ed38efd16238b0e7ff36c3ba # main
    SOURCE_DIR     ${HAILO_OLLAMA_EXTERNAL_DIR}/minja-src
    SUBBUILD_DIR   ${HAILO_OLLAMA_EXTERNAL_DIR}/minja-subbuild
)

fetchcontent_getproperties(minja)
if(NOT minja_POPULATED)
    fetchcontent_populate(minja)
    add_library(minja INTERFACE)
    target_include_directories(minja INTERFACE ${minja_SOURCE_DIR}/include)
endif()

cmake_minimum_required(VERSION 3.20)

include(FetchContent)

# Fetch oatpp - pinned to specific commit for stability
set(OATPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(OATPP_INSTALL OFF CACHE BOOL "" FORCE)

fetchcontent_declare(
    oatpp
    GIT_REPOSITORY https://github.com/oatpp/oatpp
    GIT_TAG        f83d648fd82dc222ef88aabbafb68efbd7d7bf50  # master as of 2024-12
    SOURCE_DIR     ${HAILO_OLLAMA_EXTERNAL_DIR}/oatpp-src
    SUBBUILD_DIR   ${HAILO_OLLAMA_EXTERNAL_DIR}/oatpp-subbuild
)
fetchcontent_makeavailable(oatpp)

# Set up oatpp module location for oatpp-openssl
# Must be CACHE variables with FORCE, otherwise oatpp-openssl's option() will clear them
set(OATPP_MODULES_LOCATION "CUSTOM" CACHE STRING "Location where to find oatpp modules" FORCE)
set(OATPP_DIR_SRC ${oatpp_SOURCE_DIR} CACHE PATH "oatpp source directory" FORCE)
set(OATPP_DIR_LIB ${oatpp_BINARY_DIR} CACHE PATH "oatpp binary directory" FORCE)

# Fetch oatpp-openssl - pinned to specific commit for stability
fetchcontent_declare(
    oatpp-openssl
    GIT_REPOSITORY https://github.com/oatpp/oatpp-openssl
    GIT_TAG        32c6ff8b59406470dbdff6dd65a21b671052abad  # master as of 2024-12
    SOURCE_DIR     ${HAILO_OLLAMA_EXTERNAL_DIR}/oatpp-openssl-src
    SUBBUILD_DIR   ${HAILO_OLLAMA_EXTERNAL_DIR}/oatpp-openssl-subbuild
)
fetchcontent_makeavailable(oatpp-openssl)

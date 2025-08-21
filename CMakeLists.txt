cmake_minimum_required(VERSION 3.20)

option(HAILO_BUILD_UT "Build Unit Tests" OFF)

project(hailo-ollama)
include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)
find_package(HailoRT 5 REQUIRED)

add_subdirectory(src)
add_subdirectory(thirdparty)

if(HAILO_BUILD_UT)
    add_subdirectory(test)
    enable_testing()
    add_test(project-tests hailo-ollama-test)
endif()
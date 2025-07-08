project(libguarded)

# Header only library, therefore INTERFACE
add_library(libguarded INTERFACE)

# INTERFACE targets only have INTERFACE properties
target_include_directories(libguarded INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

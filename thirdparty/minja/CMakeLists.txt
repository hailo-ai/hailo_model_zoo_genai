project(minja)

# Header only library, therefore INTERFACE
add_library(minja INTERFACE)

# INTERFACE targets only have INTERFACE properties
target_include_directories(minja INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

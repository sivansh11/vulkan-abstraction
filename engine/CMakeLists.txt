cmake_minimum_required(VERSION 3.10)

project(engine)

file(GLOB_RECURSE SRC_FILES ./*.cpp)

find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    message(STATUS "- Found Vulkan ${Vulkan_VERSION}")
    message(STATUS "- Imported includes from ${Vulkan_INCLUDE_DIR}")
    message(STATUS "- Linking Vulkan library from ${Vulkan_LIBRARY}")
else()
    message(WARNING "Could not find Vulkan.")
endif()

add_library(engine ${SRC_FILES})

include_directories(engine
    ../deps
    Vulkan::Headers
)

target_link_libraries(engine
    Vulkan::Vulkan
    glfw
    glm
    spdlog
    glslang
    shaderc
    SPIRV-Headers
    SPIRV-Tools
)

target_include_directories(engine PUBLIC ../deps/shaderc/libshaderc_util/include)

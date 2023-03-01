cmake_minimum_required(VERSION 3.10)

project(engine)

file(GLOB_RECURSE SRC_FILES ./*.cpp)

add_library(engine ${SRC_FILES})

include_directories(engine
    ../deps
)

target_link_libraries(engine
    ${Vulkan_LIBRARIES}
    glfw
    glm
    spdlog
    glad
)
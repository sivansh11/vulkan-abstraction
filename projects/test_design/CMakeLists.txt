cmake_minimum_required(VERSION 3.10)

project(test_design)

file(GLOB_RECURSE SRC_FILES ./*.cpp)

add_executable(test_design ${SRC_FILES})

include_directories(test_design
    ../../engine
    ../../deps/glfw/include
)

target_link_libraries(test_design
    engine
)
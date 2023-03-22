cmake_minimum_required(VERSION 3.10)

project(test-3)

file(GLOB_RECURSE SRC_FILES ./*.cpp)

add_executable(test-3 ${SRC_FILES})

include_directories(test-3
    ../../engine
    ../../deps/glfw/include
)

target_link_libraries(test-3
    engine
)
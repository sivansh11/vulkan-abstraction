cmake_minimum_required(VERSION 3.10)

project(test-2)

file(GLOB_RECURSE SRC_FILES ./*.cpp)

add_executable(test-2 ${SRC_FILES})

include_directories(test-2
    ../../engine
    ../../deps/glfw/include
)

target_link_libraries(test-2
    engine
)
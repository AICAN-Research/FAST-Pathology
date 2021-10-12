cmake_minimum_required(VERSION 3.12)
project(pipelineRuntime)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
find_package(FAST REQUIRED)
include(${FAST_USE_FILE})

add_executable(pipelineRuntime pipelineRuntime.cpp)
add_dependencies(pipelineRuntime fast_copy)
target_link_libraries(pipelineRuntime ${FAST_LIBRARIES})

set_target_properties(pipelineRuntime PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        )

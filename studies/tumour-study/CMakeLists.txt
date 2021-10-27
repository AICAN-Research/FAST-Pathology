cmake_minimum_required(VERSION 3.12)
project(measureRuntime)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
find_package(FAST REQUIRED)
include(${FAST_USE_FILE})

add_executable(measureRuntime measureRuntime.cpp)
add_dependencies(measureRuntime fast_copy)
target_link_libraries(measureRuntime ${FAST_LIBRARIES})

set_target_properties(measureRuntime PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        )

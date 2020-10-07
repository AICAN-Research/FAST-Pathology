#cmake_minimum_required(VERSION 3.15)
cmake_minimum_required(VERSION 3.12) # 3.5
project(measurePipelinePerformance)

set(CMAKE_CXX_STANDARD 17) # 14
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

include_directories(.)
find_package(FAST REQUIRED)
include(${FAST_USE_FILE})

add_executable(measurePipelinePerformance
        measurePipelinePerformance.cpp)
add_dependencies(measurePipelinePerformance fast_copy)
target_link_libraries(measurePipelinePerformance ${FAST_LIBRARIES})

set_target_properties(measurePipelinePerformance PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        )

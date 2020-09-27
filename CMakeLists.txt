#cmake_minimum_required(VERSION 3.15)
cmake_minimum_required(VERSION 3.15) # 3.5
project(FastPathology)

set(CMAKE_CXX_STANDARD 17) # 14
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

include_directories(.)
find_package(FAST REQUIRED)
include(${FAST_USE_FILE})

add_executable(fastPathology
        main.cpp
        MainWindow.cpp)
add_dependencies(fastPathology fast_copy)
target_link_libraries(fastPathology ${FAST_LIBRARIES})

set_target_properties(fastPathology PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        )

include(cmake/Package.cmake)

cmake_minimum_required(VERSION 3.15)
project(FAST_Pathology)

set(CMAKE_CXX_STANDARD 14)

#include_directories(.)
find_package(FAST REQUIRED)
include(${FAST_USE_FILE})

add_executable(fastPathology
        main.cpp
        MainWindow.cpp
        MainWindow.hpp)
target_link_libraries(fastPathology ${FAST_LIBRARIES})

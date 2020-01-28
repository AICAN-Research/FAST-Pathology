cmake_minimum_required(VERSION 3.1)
project(FAST-Pathology)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE -no-pie)

find_package(FAST REQUIRED)
include(${FAST_USE_FILE})

add_executable(fastPathology
    main.cpp
    MainWindow.cpp
    MainWindow.hpp)
target_link_libraries(fastPathology ${FAST_LIBRARIES})

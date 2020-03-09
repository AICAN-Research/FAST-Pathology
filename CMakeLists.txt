cmake_minimum_required(VERSION 3.5)
project(FAST-Pathology)

find_package(FAST REQUIRED)
include(${FAST_USE_FILE})

add_executable(fastPathology
    main.cpp
    MainWindow.cpp
    MainWindow.hpp)
add_dependencies(fastPathology fast_copy)
target_link_libraries(fastPathology ${FAST_LIBRARIES}) # nlohmann_json::nlohmann_json) # json)

include(cmake/Package.cmake)

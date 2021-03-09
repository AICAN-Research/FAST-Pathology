cmake_minimum_required(VERSION 3.12) # 3.15
project(fastpathology)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
find_package(FAST REQUIRED)
include(${FAST_USE_FILE})
include_directories(${FAST_BINARY_DIR}/../include/QtNetwork)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

add_executable(fastpathology
        source/main.cpp
        source/MainWindow.cpp
		misc/AppIcon.rc
		misc/qtres.qrc  # TODO: is this needed if qtres.cpp is added?
		misc/qtres.cpp)

add_definitions(-D_USE_MATH_DEFINES)
add_dependencies(fastpathology fast_copy)
target_link_libraries(fastpathology ${FAST_LIBRARIES})

set_target_properties(fastpathology PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        )

include(cmake/Package.cmake)

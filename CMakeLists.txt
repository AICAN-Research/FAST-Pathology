cmake_minimum_required(VERSION 3.12) # 3.15
project(fastpathology)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
find_package(FAST REQUIRED)
include(${FAST_USE_FILE})

set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(fastpathology
        main.cpp
        MainWindow.cpp
		AppIcon.rc
		qtres.qrc
		qtres.cpp
		qtres.rcc
		cproject.pro)

set(CMAKE_AUTORCC ON)

add_dependencies(fastpathology fast_copy)
target_link_libraries(fastpathology ${FAST_LIBRARIES})

set_target_properties(fastpathology PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        )

include(cmake/Package.cmake)

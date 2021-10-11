cmake_minimum_required(VERSION 3.12)  # 3.15
# set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
project(fastpathology)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
find_package(FAST REQUIRED)
include(${FAST_USE_FILE})
include_directories(${FAST_BINARY_DIR}/../include/QtNetwork)

# Set up RPATH with relative path so that binaries will find libraries in the lib folder
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

add_executable(fastpathology
        source/main.cpp
        source/MainWindow.cpp
		misc/AppIcon.rc
		misc/qtres.qrc  # TODO: is this needed if qtres.cpp is added?
		misc/qtres.cpp)

add_definitions(-D_USE_MATH_DEFINES)
add_dependencies(fastpathology fast_copy)

if (WIN32)
	target_link_libraries(fastpathology ${FAST_LIBRARIES})
elseif(APPLE)
	# find_library(FAST_LIBRARIES FAST "${CMAKE_CURRENT_SOURCE_DIR}/some-path-here"
	target_link_libraries(fastpathology ${FAST_LIBRARIES} pthread)  # @TODO: Is this sufficient on macOS?
else()
	target_link_libraries(fastpathology ${FAST_LIBRARIES} pthread)
endif()

set_target_properties(fastpathology PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        )

include(cmake/Package.cmake)

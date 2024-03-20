cmake_minimum_required(VERSION 3.12) # 3.15
if(APPLE)
	message(STATUS "APPLE DETECTED. Set CMAKE_OSX_ARCHITECTURES to arm64 to build for Apple Silicon")
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
endif()
project(fastpathology)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 1)
set(VERSION_PATCH 3)
set(FP_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

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
        source/gui/MainWindow.cpp
		misc/AppIcon.rc
		misc/qtres.qrc  # TODO: is this needed if qtres.cpp is added?
		misc/qtres.cpp
		source/utils/utilities.h
		source/utils/qutilities.h
		source/gui/ProjectTab/ProjectThumbnailPushButton.cpp
		source/gui/ProjectTab/ProjectThumbnailPushButton.h
		source/gui/ProjectTab/ProjectWidget.cpp
		source/gui/ProjectTab/ProjectWidget.h
		source/gui/ProcessTab/ProcessWidget.cpp
		source/gui/ProcessTab/ProcessWidget.h
		source/gui/ProcessTab/PipelineScriptEditorWidget.cpp
		source/gui/ProcessTab/PipelineScriptEditorWidget.h
		source/gui/ViewTab/ViewWidget.cpp
		source/gui/ViewTab/ViewWidget.h
		source/gui/StatsTab/StatsWidget.cpp
		source/gui/StatsTab/StatsWidget.h
		source/gui/ExportTab/ExportWidget.cpp
		source/gui/ExportTab/ExportWidget.h
		source/gui/MainSidePanelWidget.cpp
		source/gui/MainSidePanelWidget.h
		source/logic/WholeSlideImage.cpp
		source/logic/WholeSlideImage.h
		source/logic/Project.cpp
		source/logic/Project.h
		source/gui/SplashWidget.cpp
		source/gui/SplashWidget.hpp
)

add_definitions(-DFAST_PATHOLOGY_VERSION="${FP_VERSION}")
add_dependencies(fastpathology fast_copy)
target_link_libraries(fastpathology ${FAST_LIBRARIES})

include(cmake/Package.cmake)

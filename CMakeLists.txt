cmake_minimum_required(VERSION 3.12) # 3.15
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
project(fastpathology)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
find_package(FAST REQUIRED)
include(${FAST_USE_FILE})
include_directories(${FAST_BINARY_DIR}/../include/QtNetwork)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

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

add_definitions(-D_USE_MATH_DEFINES)
add_dependencies(fastpathology fast_copy)

if (WIN32)
	target_link_libraries(fastpathology ${FAST_LIBRARIES})
endif (WIN32)

if (UNIX)
	target_link_libraries(fastpathology ${FAST_LIBRARIES} pthread)
endif (UNIX)

include(cmake/Package.cmake)

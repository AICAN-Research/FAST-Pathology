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
                source/gui/ViewTab/ViewWidget.cpp
                source/gui/ViewTab/ViewWidget.h
                source/gui/StatsTab/StatsWidget.cpp
                source/gui/StatsTab/StatsWidget.h
                source/gui/MainSidePanelWidget.cpp
                source/gui/MainSidePanelWidget.h
                source/logic/DataManager.cpp
                source/logic/DataManager.h
                source/logic/WholeSlideImage.cpp
                source/logic/WholeSlideImage.h
                source/logic/ProcessManager.cpp
                source/logic/ProcessManager.h
                source/logic/SegmentationProcess.cpp
                source/logic/SegmentationProcess.h
                source/logic/InferenceModel.cpp
                source/logic/InferenceModel.h)

add_definitions(-D_USE_MATH_DEFINES)
add_dependencies(fastpathology fast_copy)

if (WIN32)
	target_link_libraries(fastpathology ${FAST_LIBRARIES})
endif (WIN32)

if (UNIX)
	target_link_libraries(fastpathology ${FAST_LIBRARIES} pthread)
endif (UNIX)

set_target_properties(fastpathology PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        )

include(cmake/Package.cmake)

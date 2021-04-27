cmake_minimum_required(VERSION 3.15)
project(ImageViewer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets DataVisualization REQUIRED)

add_library(ImageViewer
        ImageViewerWidget.cpp)
target_include_directories(ImageViewer PUBLIC
        Include)
target_link_libraries(ImageViewer Qt5::Widgets)

add_library(PointCloudViewer
        PointCloudViewerWidget.cpp)
target_include_directories(PointCloudViewer PUBLIC
        Include)
target_link_libraries(PointCloudViewer Qt5::Widgets)

add_executable(main main.cpp ImageViewerWidget.cpp
        Resources/shareddata.qrc
        )
target_include_directories(main PUBLIC Include)
target_link_libraries(main
        Qt5::Widgets
        ImageViewer
        )

add_executable(Scatter Scatter.cpp     scatterdatamodifier.cpp    )

target_link_libraries(Scatter
        Qt5::Widgets
        Qt5::DataVisualization)
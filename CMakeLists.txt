cmake_minimum_required(VERSION 3.1)
project(GraphPicker CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
#find_package(Qt4 REQUIRED QtCore QtGui)
#include(${QT_USE_FILE})
#QT4_WRAP_CPP(GraphPicker_MOC_HEADERS  GraphPicker.h)
#QT4_ADD_RESOURCES(GraphPicker_RESOURCES_RCC GraphPicker.qrc)

add_executable (graph_picker
  ${GraphPicker_MOC_HEADERS}
  GraphPicker.cc
  GraphPicker.h
  main.cc
  ${GraphPicker_RESOURCES_RCC}
)


#target_link_libraries(graph_picker ${QT_LIBRARIES})
target_link_libraries(graph_picker Qt5::Core Qt5::Widgets)
cmake_minimum_required(VERSION 3.16)
project(AxlFlash VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets SerialPort Network REQUIRED)

include_directories(src)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/crc.c
    src/crc.h
    src/packet_serial.c
    src/packet_serial.h
    src/serialworker.cpp
    src/serialworker.h
)

add_executable(AxlFlash ${PROJECT_SOURCES})

target_link_libraries(AxlFlash PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Network
)

if(WIN32)
    set_target_properties(AxlFlash PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

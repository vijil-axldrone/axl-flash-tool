cmake_minimum_required(VERSION 3.16)
project(AxlFlash VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets SerialPort Network REQUIRED)

include(FetchContent)
FetchContent_Declare(
  esp-serial-flasher
  GIT_REPOSITORY https://github.com/espressif/esp-serial-flasher.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(esp-serial-flasher)

include_directories(
    src
    stm32_api/include
    ${CMAKE_BINARY_DIR}/_deps/esp-serial-flasher-src/include
)

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
    src/esp_flasher/EspFlashWorker.cpp
    src/esp_flasher/esp_loader_port.cpp
)

add_executable(AxlFlash ${PROJECT_SOURCES})



# On Linux, link libCubeProgrammer_API.so explicitly to avoid Qt library conflicts.
if(WIN32)
    target_link_libraries(AxlFlash PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::SerialPort
        Qt6::Network
        flasher
    )
else()
    target_link_libraries(AxlFlash PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::SerialPort
        Qt6::Network
        flasher
    )
endif()

if(WIN32)
    set_target_properties(AxlFlash PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

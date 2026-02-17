cmake_minimum_required(VERSION 3.16)

project(Logger VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(Logger
    main.cpp
    LoggerApp.cpp
    LoggerApp.h
    PacketConsumer.cpp
    PacketConsumer.h
    EmulatorHost.cpp
    EmulatorHost.h
    ControlData.h
    loggerui.h loggerui.cpp loggerui.ui

)

target_link_libraries(Logger PRIVATE Qt6::Core Qt6::Widgets)

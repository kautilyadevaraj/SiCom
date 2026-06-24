cmake_minimum_required(VERSION 3.16)
project(SiCom LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)   # processes Q_OBJECT macros
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)   # auto-compiles .ui → ui_mainwindow.h

find_package(Qt6 REQUIRED COMPONENTS Widgets SerialPort)

add_executable(SiCom
    main.cpp
    SerialConfig.h
    serialmanager.h
    serialmanager.cpp
    dataconvertor.h
    dataconvertor.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    sendscheduler.h sendscheduler.cpp


)

target_link_libraries(SiCom PRIVATE Qt6::Widgets Qt6::SerialPort)
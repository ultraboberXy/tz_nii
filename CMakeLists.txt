cmake_minimum_required(VERSION 3.16)
project(RiaMapParser VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql)

qt_add_executable(RiaMapParser
    main.cpp
    event.h
    mainwindow.h
    mainwindow.cpp
    datafetcher.h
    datafetcher.cpp
    dataparser.h
    dataparser.cpp
    databasemanager.h
    databasemanager.cpp
    eventrepository.h
    eventrepository.cpp
    mainstate.h
    mainbloc.h
    mainbloc.cpp
)

target_link_libraries(RiaMapParser PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
)

set_target_properties(RiaMapParser PROPERTIES
    WIN32_EXECUTABLE $<CONFIG:Release>
    MACOSX_BUNDLE TRUE
)
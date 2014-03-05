
project(mediarepeater)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
set(QT_USE_QTWEBKIT 1)
set(QT_USE_PHONON 1)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

qt4_wrap_cpp(mp_mocs mainwindow.h seekslider.h videowidget.h audiowidget.h)
#qt4_wrap_ui(mp_uis mainwindow.ui)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif(NOT CMAKE_BUILD_TYPE)

if(MINGW AND (${CMAKE_BUILD_TYPE} STREQUAL "Release"))
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif(MINGW AND (${CMAKE_BUILD_TYPE} STREQUAL "Release"))

add_executable(mediarepeater
    main.cpp
    mainwindow.cpp
    seekslider.cpp
    videowidget.cpp
    audiowidget.cpp
    ${mp_mocs})
target_link_libraries(mediarepeater ${QT_LIBRARIES})


# vim: sw=4 ts=4 tw=80

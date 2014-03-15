
project(mediarepeater)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
set(QT_USE_QTWEBKIT 1)
set(QT_USE_PHONON 1)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

qt4_wrap_cpp(mp_mocs mainwindow.h seekslider.h videowidget.h)
#qt4_wrap_ui(mp_uis mainwindow.ui)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif(NOT CMAKE_BUILD_TYPE)

if(MINGW AND (${CMAKE_BUILD_TYPE} STREQUAL "Release"))
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif(MINGW AND (${CMAKE_BUILD_TYPE} STREQUAL "Release"))

add_executable(mediarepeater
    version.cpp
    main.cpp
    mainwindow.cpp
    seekslider.cpp
    videowidget.cpp
    ${mp_mocs})
target_link_libraries(mediarepeater ${QT_LIBRARIES})

install(TARGETS mediarepeater RUNTIME DESTINATION bin)

set(CPACK_PACKAGE_NAME "MediaRepeater")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")

include(CPack)

# vim: sw=4 ts=4

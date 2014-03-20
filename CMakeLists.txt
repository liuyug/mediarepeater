
project(mediarepeater)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED qtcore qtgui qtwebkit)
set(QT_USE_PHONON 1)
include(${QT_USE_FILE})
set(LIBS ${LIBS} ${QT_LIBRARIES})

if(NOT QT_PHONON_FOUND)
    find_package(Phonon REQUIRED)
    if(${Phonon_FOUND})
        message(STATUS "Found Standalone PHONON: ${PHONON_LIBRARIES}")
    else(${Phonon_FOUND})
        return()
    endif(${Phonon_FOUND})
    include_directories(${PHONON_INCLUDE_DIR})
    link_directories(${PHONON_LIBRARY_DIR})
    set(LIBS ${LIBS} ${PHONON_LIBRARIES})
endif(NOT QT_PHONON_FOUND)

qt4_wrap_cpp(mp_mocs mainwindow.h seekslider.h videowidget.h)
#qt4_wrap_ui(mp_uis mainwindow.ui)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif(NOT CMAKE_BUILD_TYPE)

if(MINGW AND (${CMAKE_BUILD_TYPE} STREQUAL "Release"))
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows -static")
endif(MINGW AND (${CMAKE_BUILD_TYPE} STREQUAL "Release"))

add_executable(mediarepeater
    version.cpp
    main.cpp
    mainwindow.cpp
    seekslider.cpp
    videowidget.cpp
    ${mp_mocs})
target_link_libraries(mediarepeater ${LIBS})

if(NOT WIN32)
    install(TARGETS mediarepeater RUNTIME DESTINATION bin)
endif(NOT WIN32)

if(WIN32)
    install(TARGETS mediarepeater RUNTIME DESTINATION ./ COMPONENT Application)

    if(MINGW)
        install(FILES mingwm10.dll
            libgcc_s_dw2-1.dll
            DESTINATION ./ COMPONENT "Cores")
    endif(MINGW)

    install(FILES ${QT_BINARY_DIR}/QtCore4.dll
        ${QT_BINARY_DIR}/QtGui4.dll
        ${QT_BINARY_DIR}/QtNetwork4.dll
        ${QT_BINARY_DIR}/QtWebKit4.dll
        ${QT_BINARY_DIR}/phonon4.dll
        DESTINATION ./ COMPONENT "Cores")

    install(DIRECTORY ${QT_PLUGINS_DIR}/phonon_backend DESTINATION plugins COMPONENT "Plugins")
    install(DIRECTORY ${QT_PLUGINS_DIR}/iconengines DESTINATION plugins COMPONENT "Plugins")
    install(DIRECTORY ${QT_PLUGINS_DIR}/imageformats DESTINATION plugins COMPONENT "Plugins")
endif(WIN32)

set(CPACK_PACKAGE_NAME "MediaRepeater")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "MediaRepeater")
set(CPACK_PACKAGE_EXECUTABLES "mediarepeater" "Media Repeater")
set(CPACK_COMPONENTS_ALL Application Cores Plugins)

include(CPack)

# vim: sw=4 ts=4

Media Repeater
==============

For learning language repeat playback media files.

requirement
-----------
+ only support Qt4 with Phonon

Linux Install
-------------
::

    mkdir build
    cmake ../
    make
    make install

Windows Install
---------------
1. Download Qt4.8 compiled by MinGW from `qt-project`_
2. Download CMake_
3. Download `GCC4.4`_, Qt4.8 compiled by MinGW only support MinGW 4.4
4. Install above toolchain and compile.

   Run in Window::

     ``cmake -G"MinGW Makefiles" ../``

5. copy below dlls from MinGW and QT directory into running directory:

  + libgcc_s_dw2-1.dll
  + mingwm10.dll
  + phonon4.dll
  + QtCore4.dll
  + QtGui4.dll
  + QtNetwork4.dll
  + QtWebKit4.dll
  + phonon_backend
  + imageformats
  + iconengines


.. _`qt-project`: http://download.qt-project.org/official_releases/qt/4.8/4.8.5/qt-win-opensource-4.8.5-mingw.exe
.. _CMake: http://www.cmake.org/
.. _`GCC4.4`: https://docs.google.com/file/d/0B4D8x6CJEmtuczdiQklwMEs4RUU/edit?pli=1

ToDo
----
+ Use QtMultimedia to support Qt5
+ Add sound wave window for audio output and input

License
-------
Please follow GPLv3. Thanks.

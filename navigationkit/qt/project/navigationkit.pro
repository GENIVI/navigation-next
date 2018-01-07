LTK_ROOT = ../../..
include ($$LTK_ROOT/build/qt/build_library.pro)


######### Project setting ############
win32{
DEFINES += LTK_PLATFORM_WIN32
}

INCLUDEPATH += ../include
INCLUDEPATH += ../include/private
INCLUDEPATH += ../include/signal
INCLUDEPATH += ../include/internal
INCLUDEPATH += ../include/data

SOURCES += ../src/*.cpp

HEADERS += ../include/*.h
HEADERS += ../include/private/*.h
HEADERS += ../include/signal/*.h
HEADERS += ../include/internal/*.h
HEADERS += ../include/data/*.h

QT += gui

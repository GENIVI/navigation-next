CCC_ROOT = ../../..
PROJ_ROOT = $$CCC_ROOT/nbpal
THIRDPARTY_ROOT = $$CCC_ROOT/thirdparty

HEADERS += $$PROJ_ROOT/include/*.h
HEADERS += $$PROJ_ROOT/include/qt/*.h
HEADERS += $$PROJ_ROOT/src/qt/threading/*.h
HEADERS += $$PROJ_ROOT/src/qt/timer/*.h
HEADERS += $$PROJ_ROOT/src/qt/net/*.h
HEADERS += $$PROJ_ROOT/include/private/*.h

INCLUDEPATH += $$PROJ_ROOT/include
INCLUDEPATH += $$PROJ_ROOT/include/qt
INCLUDEPATH += $$PROJ_ROOT/include/private

INCLUDEPATH += $$PROJ_ROOT/src/qt/timer
INCLUDEPATH += $$PROJ_ROOT/src/qt/display
INCLUDEPATH += $$PROJ_ROOT/src/qt/net
INCLUDEPATH += $$PROJ_ROOT/src/qt/pal
INCLUDEPATH += $$PROJ_ROOT/src/qt/uitaskmanager
INCLUDEPATH += $$PROJ_ROOT/src/qt/synchronization

INCLUDEPATH += $$PROJ_ROOT/src/util
INCLUDEPATH += $$PROJ_ROOT/src/common/finishfunction
INCLUDEPATH += $$PROJ_ROOT/src/common/gl
INCLUDEPATH += $$PROJ_ROOT/src/common/unzip
INCLUDEPATH += $$PROJ_ROOT/src/common/database
INCLUDEPATH += $$PROJ_ROOT/src/common/taskqueue
INCLUDEPATH += $$PROJ_ROOT/src/common/freetypefont
INCLUDEPATH += $$PROJ_ROOT/src/common/net

linux{
    INCLUDEPATH += $$[QT_SYSROOT]/usr/include/freetype2/
}

SOURCES += $$PROJ_ROOT/src/qt/pal/*.cpp
SOURCES += $$PROJ_ROOT/src/qt/threading/*.cpp
SOURCES += $$PROJ_ROOT/src/qt/synchronization/*.cpp
SOURCES += $$PROJ_ROOT/src/qt/triangulation/*.cpp
SOURCES += $$PROJ_ROOT/src/qt/util/palstdlib.cpp
SOURCES += $$PROJ_ROOT/src/qt/util/paldebuglog.cpp
SOURCES += $$PROJ_ROOT/src/qt/clock/*.cpp
SOURCES += $$PROJ_ROOT/src/qt/timer/*.cpp
SOURCES += $$PROJ_ROOT/src/qt/imageblender/*.cpp
SOURCES += $$PROJ_ROOT/src/qt/display/*.cpp
SOURCES += $$PROJ_ROOT/src/qt/testlog/*.cpp

SOURCES += $$PROJ_ROOT/src/qt/net/*.cpp
SOURCES += $$PROJ_ROOT/src/qt/file/*.cpp

SOURCES += $$PROJ_ROOT/src/util/*.c
SOURCES += $$PROJ_ROOT/src/common/nbtaskqueue/*.cpp
SOURCES += $$PROJ_ROOT/src/common/freetypefont/*.cpp
SOURCES += $$PROJ_ROOT/src/common/unzip/*.cpp
SOURCES += $$PROJ_ROOT/src/common/database/*.cpp
SOURCES += $$PROJ_ROOT/src/common/taskqueue/*.cpp
SOURCES += $$PROJ_ROOT/src/common/net/*.cpp
SOURCES += $$PROJ_ROOT/src/common/finishfunction/*.cpp

win32{
    INCLUDEPATH += $$THIRDPARTY_ROOT/sqlite/include
    INCLUDEPATH += $$THIRDPARTY_ROOT/freetype-2.4.4/include
    INCLUDEPATH += $$THIRDPARTY_ROOT/zlib
    INCLUDEPATH += $$THIRDPARTY_ROOT/glew-1.7.0/include
    SOURCES += $$PROJ_ROOT/src/qt/gl2/*.cpp
}else{
    SOURCES += $$PROJ_ROOT/src/qt/gles2/*.cpp
}

debug:linux{
    DEFINES += MPERF
    DEFINES += DTHREAD
}

QT = core gui network qml quick
TEMPLATE = lib
TARGET = nbpal
CONFIG += staticlib
CONFIG += debug_and_release debug_and_release_target
DEFINES += MAKE_STATIC_LIBRARIES
DEFINES += NOMINMAX _CRT_SECURE_NO_WARNINGS
debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/

include ($$CCC_ROOT/build/projects/qt/config.pro)


OS_PLATFORM_SPECIFIC = linux
OS_PLATFORM_SPECIFIC_AUTIO = linux

CCC_ROOT = ../../..
PROJ_ROOT = $$CCC_ROOT/abpal
NIM_NBPAL_ROOT = $$CCC_ROOT/nbpal
NIM_THIRDPARTY_ROOT = $$CCC_ROOT/thirdparty

INCLUDEPATH += $$NIM_NBPAL_ROOT/include
INCLUDEPATH += $$NIM_NBPAL_ROOT/include/qt
win32 {
    OS_PLATFORM_SPECIFIC = win32
    OS_PLATFORM_SPECIFIC_AUTIO = win32
}
unix {
    OS_PLATFORM_SPECIFIC = linux
    OS_PLATFORM_SPECIFIC_AUTIO = qt
    HEADERS += \
        ../../include/linux/abpalexp.h \
        ../../include/linux/gpsprovider.h \
        ../../include/linux/hsgpsprovider.h \
        ../../include/linux/locationlistener.h
    HEADERS += \
        ../../src/qt/audio/abpalttsengineimpl.h
}
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/flite-1.4-release/include
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/flite-1.4-release/lang/cmu_us_rms
INCLUDEPATH += $$PROJ_ROOT/include/$$OS_PLATFORM_SPECIFIC
INCLUDEPATH += $$PROJ_ROOT/include/private
INCLUDEPATH += $$PROJ_ROOT/include
INCLUDEPATH += $$PROJ_ROOT/src/$$OS_PLATFORM_SPECIFIC_AUTIO/audio
INCLUDEPATH += $$PROJ_ROOT/src/$$OS_PLATFORM_SPECIFIC/gps

HEADERS += $$PROJ_ROOT/include/$$OS_PLATFORM_SPECIFIC/*.h
HEADERS += $$PROJ_ROOT/include/private/*.h
HEADERS += $$PROJ_ROOT/include/*.h
HEADERS += $$PROJ_ROOT/src/$$OS_PLATFORM_SPECIFIC_AUTIO/audio/*.h
HEADERS += $$PROJ_ROOT/src/$$OS_PLATFORM_SPECIFIC/gps/*.h

SOURCES += $$PROJ_ROOT/src/qt/audio/*.cpp
SOURCES += $$PROJ_ROOT/src/$$OS_PLATFORM_SPECIFIC_AUTIO/audio/*.cpp
SOURCES += $$PROJ_ROOT/src/$$OS_PLATFORM_SPECIFIC/gps/*.cpp
SOURCES += $$PROJ_ROOT/src/util/*.cpp
SOURCES += $$PROJ_ROOT/src/util/*.c
SOURCES += $$PROJ_ROOT/src/util/combineraac/*.cpp

QT       -= gui
TEMPLATE = lib
TARGET = abpal
CONFIG += staticlib
DEFINES += UNUSED_NDEFINED
DEFINES += MAKE_STATIC_LIBRARIES

include ($$CCC_ROOT/build/projects/qt/config.pro)


CCC_ROOT = ../../..
PROJ_ROOT = $$CCC_ROOT/coreservices
NIM_NBPAL_ROOT = $$CCC_ROOT/nbpal

INCLUDEPATH += $$NIM_NBPAL_ROOT/include
INCLUDEPATH += $$NIM_NBPAL_ROOT/include/qt
INCLUDEPATH += $$PROJ_ROOT/include/qt
INCLUDEPATH += $$PROJ_ROOT/include/private
INCLUDEPATH += $$PROJ_ROOT/include/protected
INCLUDEPATH += $$PROJ_ROOT/include

HEADERS += $$PROJ_ROOT/include/qt/*.h
HEADERS += $$PROJ_ROOT/include/private/*.h
HEADERS += $$PROJ_ROOT/include/protected/*.h
HEADERS += $$PROJ_ROOT/include/*.h

SOURCES += ../../src/network/cslnetwork.c
SOURCES += $$PROJ_ROOT/src/tps/*.c
SOURCES += ../../src/logging/*.c
SOURCES += $$PROJ_ROOT/src/qalog/*.c
SOURCES += $$PROJ_ROOT/src/util/*.c

TEMPLATE = lib
TARGET = coreservices
CONFIG += staticlib
CONFIG += debug_and_release debug_and_release_target
DEFINES += UNUSED_NDEFINED
DEFINES += MAKE_STATIC_LIBRARIES
debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/

include ($$CCC_ROOT/build/projects/qt/config.pro)


######################################################################
# Automatically generated by qmake (2.01a) Thu Oct 4 19:01:12 2007
######################################################################
PROJ_ROOT            = ../../../libxml2
INCLUDEPATH          += $$PROJ_ROOT
INCLUDEPATH          += $$PROJ_ROOT/include
INCLUDEPATH          += ../../../zlib
win32: INCLUDEPATH          += $$PROJ_ROOT/include/platforms/win32
unix: INCLUDEPATH          += $$PROJ_ROOT/include/platforms/linux

SOURCES              += $$PROJ_ROOT/*.c

CCC_ROOT             = ../../../..

TEMPLATE             = lib
TARGET               = xml2
CONFIG               += staticlib
CONFIG               += debug_and_release debug_and_release_target
debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/

include ($$CCC_ROOT/build/projects/qt/config.pro)

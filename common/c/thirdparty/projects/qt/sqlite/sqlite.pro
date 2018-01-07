
INCLUDEPATH += ../../../sqlite/include

SOURCES += ../../../sqlite/Sources/sqlite3.c

CCC_ROOT             = ../../../..

QT       -= core gui

TARGET = sqlite3
TEMPLATE = lib
CONFIG += staticlib

#CONFIG += debug_and_release debug_and_release_target
#debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
#release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/

include ($$CCC_ROOT/build/projects/qt/config.pro)

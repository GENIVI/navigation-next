#-------------------------------------------------
#
# Project created by QtCreator 2016-04-28T15:13:41
#
#-------------------------------------------------

BRANCH = $$dirname(PWD)
BRANCH = $$dirname(BRANCH)
BRANCH = $$dirname(BRANCH)
BRANCH = $$basename(BRANCH)

CCC_ROOT = ../../../../..
BUILD_ROOT = $$CCC_ROOT/build/$$BRANCH

message(branch: $$BRANCH)
message(ccc_root: $$CCC_ROOT)
message(build_root: $$BUILD_ROOT)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = searchtest
TEMPLATE = app

NCDB_LIBS = -L$$BUILD_ROOT/ncdb/libs -lncdb

INCLUDEPATH += $$BUILD_ROOT/ncdb/include

SOURCES += main.cpp\
        searchwindow.cpp

HEADERS  += $$BUILD_ROOT/ncdb/include/*.h
HEADERS  += searchwindow.h

FORMS    += searchwindow.ui

LIBS += $$NCDB_LIBS



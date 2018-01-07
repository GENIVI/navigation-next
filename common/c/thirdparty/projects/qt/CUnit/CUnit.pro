QT       -= core gui

TARGET = CUnit
TEMPLATE = lib
CONFIG += staticlib
CONFIG += debug_and_release debug_and_release_target

INCLUDEPATH += ../../../CUnit/include

SOURCES += ../../../CUnit/Sources/Automated/Automated.c \
	../../../CUnit/Sources/Basic/Basic.c \
	../../../CUnit/Sources/Framework/CUError.c \
	../../../CUnit/Sources/Framework/TestDB.c \
	../../../CUnit/Sources/Framework/TestRun.c \
	../../../CUnit/Sources/Framework/Util.c

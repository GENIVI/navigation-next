BRANCH = $$dirname(PWD)
BRANCH = $$dirname(BRANCH)
BRANCH = $$dirname(BRANCH)
BRANCH = $$basename(BRANCH)

CCC_ROOT = ../../../../..
BUILD_ROOT = $$CCC_ROOT/build/$$BRANCH

message(branch: $$BRANCH)
message(ccc_root: $$CCC_ROOT)
message(build_root: $$BUILD_ROOT)

QT += core
QT -= gui

TARGET = testSingleSearch
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

NCDB_LIBS = -L$$BUILD_ROOT/ncdb/libs -lncdb


LIBS += $$NCDB_LIBS
LIBS += -L/usr/local/libpng/lib -lpng15

INCLUDEPATH += $$BUILD_ROOT/ncdb/include

SOURCES += *.cpp

HEADERS  += $$BUILD_ROOT/ncdb/include/*.h
HEADERS += *.h

qnx {
  target.path = /root/dev
  INSTALLS += target
}

CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    BUILD_CONFIG = release
}


qnx {
  BUILD_PLATFORM=qnx
} else:win32 {
  BUILD_PLATFORM=win32
} else:unix {
  BUILD_PLATFORM=unix
} else {
  BUILD_PLATOFRM=unk
}

HW_ROOT = ../
MOC_DIR += ./moc
OBJECTS_DIR += ./obj/$$BUILD_PLATFORM/$$BUILD_CONFIG
DESTDIR = $$HW_ROOT/output/$$BUILD_PLATFORM/$$BUILD_CONFIG
CONFIG += debug_and_release debug_and_release_target

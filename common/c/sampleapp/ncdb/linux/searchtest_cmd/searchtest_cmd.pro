BRANCH = $$dirname(PWD)
BRANCH = $$dirname(BRANCH)
BRANCH = $$dirname(BRANCH)
BRANCH = $$basename(BRANCH)

CCC_ROOT = ../../../../..
BUILD_ROOT = $$CCC_ROOT/build/$$BRANCH

message(branch: $$BRANCH)
message(ccc_root: $$CCC_ROOT)
message(build_root: $$BUILD_ROOT)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
QT = core

NCDB_LIBS = -L$$BUILD_ROOT/ncdb/libs -lncdb #-L/usr/lib -lpng

qnx {
    LIBS += -L/home/tom/qnx660/target/qnx6/armle-v7/usr/lib -lpng -lfreetype
}
LIBS += $$NCDB_LIBS

INCLUDEPATH += $$BUILD_ROOT/ncdb/include

HEADERS += searchtest.h \
    searchparam.h
SOURCES += main.cpp searchtest.cpp

qnx {
   DEPLOY_PATH = /nav/dev/ltk_adaptor/
   target.path = $$DEPLOY_PATH
   INSTALLS += target
}


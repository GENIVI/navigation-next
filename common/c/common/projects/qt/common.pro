CCC_ROOT = ../../..
PROJ_ROOT = $$CCC_ROOT/common
NIM_NBPAL_ROOT = $$CCC_ROOT/nbpal
NIM_ABPAL_ROOT = $$CCC_ROOT/abpal
NIM_NBSERVICES_ROOT = $$CCC_ROOT/nbservices
NIM_ABSERVICES_ROOT = $$CCC_ROOT/abservices
NIM_CORESERVICES_ROOT = $$CCC_ROOT/coreservices
NIM_THIRDPARTY_ROOT = $$CCC_ROOT/thirdparty
BUILD_ROOT = $$CCC_ROOT/build

INCLUDEPATH += $$PROJ_ROOT/include
INCLUDEPATH += $$PROJ_ROOT/include/qt
INCLUDEPATH += $$PROJ_ROOT/include/private
INCLUDEPATH += $$PROJ_ROOT/include/protected
INCLUDEPATH += $$NIM_ABPAL_ROOT/include
INCLUDEPATH += $$NIM_ABPAL_ROOT/include/qt
INCLUDEPATH += $$NIM_NBPAL_ROOT/include
INCLUDEPATH += $$NIM_NBPAL_ROOT/include/qt
INCLUDEPATH += $$NIM_NBPAL_ROOT/include/protected
INCLUDEPATH += $$NIM_ABSERVICES_ROOT/include
INCLUDEPATH += $$NIM_ABSERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/protected
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/generated
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/private
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include/protected
INCLUDEPATH += $$BUILD_ROOT
INCLUDEPATH += $$BUILD_ROOT/ncdb/include

HEADERS += $$PROJ_ROOT/include/*.h
HEADERS += $$PROJ_ROOT/include/qt/*.h
HEADERS += $$PROJ_ROOT/include/private/*.h
HEADERS += $$PROJ_ROOT/include/protected/*.h
HEADERS += $$BUILD_ROOT/ncdb/include/*.h

SOURCES += $$PROJ_ROOT/src/util/*.c
SOURCES += $$PROJ_ROOT/src/*.cpp

TEMPLATE = lib
TARGET = common
CONFIG += staticlib
CONFIG += debug_and_release debug_and_release_target
CONFIG += exceptions
DEFINES += MAKE_STATIC_LIBRARIES
debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/

include ($$CCC_ROOT/build/projects/qt/config.pro)


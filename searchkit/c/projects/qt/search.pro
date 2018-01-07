CCC_ROOT = ../../../../common/c
PROJ_ROOT = $$CCC_ROOT/search
NIM_NBPAL_ROOT = $$CCC_ROOT/nbpal
NIM_NBSERVICES_ROOT = $$CCC_ROOT/nbservices
NIM_CORESERVICES_ROOT = $$CCC_ROOT/coreservices
NIM_COMMON_ROOT = $$CCC_ROOT/common
NIM_THIRDPARTY_ROOT = $$CCC_ROOT/thirdparty
BUILD_ROOT = $$CCC_ROOT/build

INCLUDEPATH += $$PROJ_ROOT/include
INCLUDEPATH += $$PROJ_ROOT/include/private
INCLUDEPATH += $$PROJ_ROOT/src
INCLUDEPATH += $$PROJ_ROOT/src/datamanager
INCLUDEPATH += $$PROJ_ROOT/src/datamanager/db
INCLUDEPATH += $$PROJ_ROOT/src/datamanager/dao/dbdao
INCLUDEPATH += $$PROJ_ROOT/src/datamanager/dao/interface
INCLUDEPATH += $$NIM_NBPAL_ROOT/include
INCLUDEPATH += $$NIM_NBPAL_ROOT/include/qt
INCLUDEPATH += $$NIM_ABSERVICES_ROOT/include
INCLUDEPATH += $$NIM_ABSERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/protected
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/generated
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/private
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include/protected
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_COMMON_ROOT/include
INCLUDEPATH += $$NIM_COMMON_ROOT/include/protected
INCLUDEPATH += $$BUILD_ROOT
INCLUDEPATH += $$BUILD_ROOT/ncdb/include


HEADERS += $$PROJ_ROOT/include/*.h
HEADERS += $$PROJ_ROOT/include/private/*.h
HEADERS += $$PROJ_ROOT/src/offboard/*.h
HEADERS += $$PROJ_ROOT/src/onboard/*.h
HEADERS += $$BUILD_ROOT/ncdb/include/*.h

SOURCES +=  $$PROJ_ROOT/src/*.cpp
SOURCES +=  $$PROJ_ROOT/src/datamanager/*.cpp
SOURCES +=  $$PROJ_ROOT/src/datamanager/db/*.cpp
SOURCES +=  $$PROJ_ROOT/src/datamanager/dao/dbdao/*.cpp
SOURCES +=  $$PROJ_ROOT/src/datamanager/dao/interface/*.cpp
SOURCES +=  $$PROJ_ROOT/src/offboard/*.cpp
SOURCES +=  $$PROJ_ROOT/src/onboard/*.cpp

win32:INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/sqlite/include

TEMPLATE = lib
CONFIG += staticlib
CONFIG += debug_and_release debug_and_release_target
TARGET = search
DEFINES += MAKE_STATIC_LIBRARIES
debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/

include ($$CCC_ROOT/build/projects/qt/config.pro)


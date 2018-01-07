CCC_ROOT = ../../../../common/c
PROJ_ROOT = $$CCC_ROOT/map
NIM_NBPAL_ROOT = $$CCC_ROOT/nbpal
NIM_ABPAL_ROOT = $$CCC_ROOT/abpal
NIM_NBSERVICES_ROOT = $$CCC_ROOT/nbservices
NIM_CORESERVICES_ROOT = $$CCC_ROOT/coreservices
NIM_NBGM_ROOT = $$CCC_ROOT/nbgm
NIM_NBUI_ROOT = $$CCC_ROOT/nbui
NIM_THIRDPARTY_ROOT = $$CCC_ROOT/thirdparty

message ($$CCC_ROOT)
message ($$PROJ_ROOT)

INCLUDEPATH += $$PROJ_ROOT/include
INCLUDEPATH += $$NIM_NBPAL_ROOT/include
INCLUDEPATH += $$NIM_NBPAL_ROOT/include/qt
INCLUDEPATH += $$NIM_NBPAL_ROOT/include/protected
INCLUDEPATH += $$NIM_NBGM_ROOT/nbgmmain/include
INCLUDEPATH += $$NIM_NBGM_ROOT/nbgmmain/include/qt
INCLUDEPATH += $$NIM_NBUI_ROOT/include/protected
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/protected
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/src/map/layermanager
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include/protected
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/tinyxml

HEADERS += $$PROJ_ROOT/include/*.h
SOURCES += $$PROJ_ROOT/src/*.cpp

TEMPLATE = lib
TARGET = map
CONFIG += staticlib
CONFIG += debug_and_release debug_and_release_target
DEFINES += MAKE_STATIC_LIBRARIES
debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/

message ($$CCC_ROOT)
message ($$DESTDIR)
message ($$PWD)

include ($$CCC_ROOT/build/projects/qt/config.pro)


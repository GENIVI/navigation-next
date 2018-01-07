CCC_ROOT = ../../../../common/c
PROJ_ROOT = $$CCC_ROOT/nav
NIM_NBPAL_ROOT = $$CCC_ROOT/nbpal
NIM_ABPAL_ROOT = $$CCC_ROOT/abpal
NIM_NBSERVICES_ROOT = $$CCC_ROOT/nbservices
NIM_CORESERVICES_ROOT = $$CCC_ROOT/coreservices
NIM_NBGM_ROOT = $$CCC_ROOT/nbgm
NIM_THIRDPARTY_ROOT = $$CCC_ROOT/thirdparty


INCLUDEPATH += $$PROJ_ROOT/include
INCLUDEPATH += $$NIM_ABPAL_ROOT/include
INCLUDEPATH += $$NIM_ABPAL_ROOT/include/qt
INCLUDEPATH += $$NIM_NBPAL_ROOT/include
INCLUDEPATH += $$NIM_NBPAL_ROOT/include/qt
INCLUDEPATH += $$NIM_ABSERVICES_ROOT/include
INCLUDEPATH += $$NIM_ABSERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/protected
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/generated
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include/protected
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/ArgTable/include
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/CUnit/include
INCLUDEPATH += $$NIM_NBGM_ROOT/nbgmmain/include
INCLUDEPATH += $$NIM_NBGM_ROOT/nbgmmain/include/qt
INCLUDEPATH += $$PROJ_ROOT/include

HEADERS += $$PROJ_ROOT/include/*.h
SOURCES +=  $$PROJ_ROOT/src/*.cpp

TEMPLATE = lib
CONFIG += staticlib
CONFIG += debug_and_release debug_and_release_target
TARGET = nav
DEFINES += MAKE_STATIC_LIBRARIES
debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/

include ($$CCC_ROOT/build/projects/qt/config.pro)


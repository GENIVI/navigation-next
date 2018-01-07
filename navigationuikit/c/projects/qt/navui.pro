CCC_ROOT = ../../../../common/c
PROJ_ROOT = $$CCC_ROOT/navui
NIM_NBPAL_ROOT = $$CCC_ROOT/nbpal
NIM_ABPAL_ROOT = $$CCC_ROOT/abpal
NIM_NBSERVICES_ROOT = $$CCC_ROOT/nbservices
NIM_CORESERVICES_ROOT = $$CCC_ROOT/coreservices
NIM_NBGM_ROOT = $$CCC_ROOT/nbgm
NIM_THIRDPARTY_ROOT = $$CCC_ROOT/thirdparty
NIM_NAV_ROOT = $$CCC_ROOT/nav

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
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/tinyxml
INCLUDEPATH += $$NIM_NBGM_ROOT/nbgmmain/include
INCLUDEPATH += $$NIM_NBGM_ROOT/nbgmmain/include/qt
INCLUDEPATH += $$NIM_NAV_ROOT/include
INCLUDEPATH += $$PROJ_ROOT/include/
INCLUDEPATH += $$PROJ_ROOT/include/private
INCLUDEPATH += $$PROJ_ROOT/include/widgets
INCLUDEPATH += $$PROJ_ROOT/src/presenters
INCLUDEPATH += $$PROJ_ROOT/src/presenters/map
INCLUDEPATH += $$PROJ_ROOT/src/states
INCLUDEPATH += $$PROJ_ROOT/src/utils

HEADERS +=  $$PROJ_ROOT/include/*.h
HEADERS +=  $$PROJ_ROOT/include/private/*.h
HEADERS +=  $$PROJ_ROOT/include/widgets/*.h
HEADERS += $$PROJ_ROOT/src/presenters/*.h
HEADERS += $$PROJ_ROOT/src/presenters/map/*.h
HEADERS += $$PROJ_ROOT/src/states/*.h
HEADERS += $$PROJ_ROOT/src/utils/*.h

SOURCES +=  $$PROJ_ROOT/src/*.cpp
SOURCES +=  $$PROJ_ROOT/src/presenters/*.cpp
SOURCES +=  $$PROJ_ROOT/src/presenters/map/*.cpp
SOURCES +=  $$PROJ_ROOT/src/states/*.cpp
SOURCES +=  $$PROJ_ROOT/src/utils/*.cpp

TEMPLATE = lib
CONFIG += staticlib
CONFIG += debug_and_release debug_and_release_target
TARGET = navui
DEFINES += MAKE_STATIC_LIBRARIES
debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/

include ($$CCC_ROOT/build/projects/qt/config.pro)


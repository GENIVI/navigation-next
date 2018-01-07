CCC_ROOT = ../../..
PROJ_ROOT = $$CCC_ROOT/abservices
NIM_NBPAL_ROOT = $$CCC_ROOT/nbpal
NIM_ABPAL_ROOT = $$CCC_ROOT/abpal
NIM_NBSERVICES_ROOT = $$CCC_ROOT/nbservices
NIM_CORESERVICES_ROOT = $$CCC_ROOT/coreservices
NIM_THIRDPARTY_ROOT = $$CCC_ROOT/thirdparty

INCLUDEPATH += $$PROJ_ROOT/include
INCLUDEPATH += $$PROJ_ROOT/include/qt
INCLUDEPATH += $$PROJ_ROOT/include/private
INCLUDEPATH += $$NIM_ABPAL_ROOT/include
INCLUDEPATH += $$NIM_ABPAL_ROOT/include/qt
INCLUDEPATH += $$NIM_NBPAL_ROOT/include
INCLUDEPATH += $$NIM_NBPAL_ROOT/include/qt
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/protected
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include/protected
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/speex/include

HEADERS += $$PROJ_ROOT/include/*.h
HEADERS += $$PROJ_ROOT/include/qt/*.h
HEADERS += $$PROJ_ROOT/include/private/*.h

SOURCES += $$PROJ_ROOT/src/data/*.c
SOURCES += $$PROJ_ROOT/src/data/*.cpp
SOURCES += $$PROJ_ROOT/src/datastore/*.c
SOURCES += $$PROJ_ROOT/src/ers/*.c
SOURCES += $$PROJ_ROOT/src/fileset/*.c
SOURCES += $$PROJ_ROOT/src/license/*.c
SOURCES += $$PROJ_ROOT/src/motd/*.c
SOURCES += $$PROJ_ROOT/src/msg/*.c
SOURCES += $$PROJ_ROOT/src/oneshotasr/*.cpp
SOURCES += $$PROJ_ROOT/src/profile/*.c
SOURCES += $$PROJ_ROOT/src/qalog/*.c
SOURCES += $$PROJ_ROOT/src/sms/*.c
SOURCES += $$PROJ_ROOT/src/speech/*.c
SOURCES += $$PROJ_ROOT/src/subscription/*.c
SOURCES += $$PROJ_ROOT/src/sync/*.c
SOURCES += $$PROJ_ROOT/src/util/*.c
SOURCES += $$PROJ_ROOT/src/version/*.c

TEMPLATE = lib
TARGET = abservices
CONFIG += staticlib
DEFINES += MAKE_STATIC_LIBRARIES

include ($$CCC_ROOT/build/projects/qt/config.pro)


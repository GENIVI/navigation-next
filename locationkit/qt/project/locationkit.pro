LTK_ROOT = ../../..
include ($$LTK_ROOT/build/qt/build_library.pro)

NIM_ROOT = ../../../../../..

######### Project setting ############

INCLUDEPATH += ../include
INCLUDEPATH += ../include/private

SOURCES += \
    ../src/filelocationprovider.cpp \
    ../src/locationprovider.cpp \
    ../src/internallistenermanager.cpp \
    ../src/internallocationproviderimpl.cpp


HEADERS += ../include/*.h
HEADERS += ../include/private/*.h

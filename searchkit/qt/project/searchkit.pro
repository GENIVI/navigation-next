######### Environment setting ############
LTK_ROOT = ../../..
include ($$LTK_ROOT/build/qt/build_library.pro)


######### Project setting ############
INCLUDEPATH += ../include
INCLUDEPATH += ../include/private

HEADERS += ../include/*.h
HEADERS += ../include/private/*.h

SOURCES += \
    ../src/singlesearchrequest.cpp \
    ../src/searchutli.cpp \
    ../src/searchrequestimpl.cpp \
    ../src/suggestionsearchrequest.cpp \
    ../src/searchreversegeocoderequestimpl.cpp \
    ../src/searchreversegeocoderequest.cpp \
    ../src/addresssearchrequest.cpp

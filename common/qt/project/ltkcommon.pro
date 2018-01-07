LTK_ROOT = ../../..
include ($$LTK_ROOT/build/qt/build_library.pro)

######### Project setting ############
INCLUDEPATH += ../include
INCLUDEPATH += ../include/qa
INCLUDEPATH += ../include/private
INCLUDEPATH += ../include/private/qt
INCLUDEPATH += ../include/data

SOURCES += \
    ../src/ltkcontextimpl.cpp \
    ../src/segmentattribute.cpp \
    ../src/colorsegment.cpp \
    ../src/place.cpp \
    ../src/patternsegment.cpp \
    ../src/qaloglistenerdelegate.cpp \
    ../src/qaloglistenerimpl.cpp \
    ../src/ltkhybridmanagerimpl.cpp \
    ../src/ltkerror.cpp


HEADERS += ../include/*.h
HEADERS += ../include/qa/*.h
HEADERS += ../include/private/*.h
HEADERS += ../include/private/qt/*.h
HEADERS += ../include/data/*.h

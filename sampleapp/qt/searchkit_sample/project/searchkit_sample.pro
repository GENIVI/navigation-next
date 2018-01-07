#-------------------------------------------------
#
# Project created by QtCreator 2016-02-29T20:28:11
#
#-------------------------------------------------

LTK_ROOT = ../../../library
include ($$LTK_ROOT/locationtoolkit.pro)

TARGET = searchkit_sample
TEMPLATE = app


CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    BUILD_CONFIG = release
}

MOC_DIR += ../src/moc

unix {
  QMAKE_CXXFLAGS += -std=c++11
  QMAKE_CXXFLAGS += -Wno-unused-parameter
  QMAKE_CXXFLAGS += -Wno-unused-variable
}

ltkdev {
    qnx {
      PLATFORM=qnx
    } else:unix {
      PLATFORM=unix
    } else {
      PLATFORM=unknown
    }
    OBJECTS_DIR += ./$$PLATFORM/obj/$$BUILD_CONFIG/$$QT_VERSION
    DESTDIR = ./$$PLATFORM/$$BUILD_CONFIG/$$QT_VERSION
} else {
    OBJECTS_DIR += ./obj/$$BUILD_CONFIG
    DESTDIR = ./$$BUILD_CONFIG
}


INCLUDEPATH += ../include  \
               ../../util/include

SOURCES += ../src/*.cpp \
           ../../util/src/*.cpp


HEADERS  += ../include/*.h \
            ../../util/include/*.h

FORMS    += ../form/*.ui \
            ../../util/Forms/*.ui

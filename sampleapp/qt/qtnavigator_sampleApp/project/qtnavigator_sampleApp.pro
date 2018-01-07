TEMPLATE = app

QT += qml quick
CONFIG += c++11
TARGET = MainViewLoader

LTK_ROOT=../../../library
include ($$LTK_ROOT/locationtoolkit.pro)

INCLUDEPATH += 	../include \
        ../../util/include

CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    BUILD_CONFIG = release
}

OUT_PWD = $$OUT_PWD/$$BUILD_CONFIG

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

SOURCES += ../src/main.cpp

RESOURCES +=

DISTFILES += \
    ../resource/main.qml

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_resource.target = $$OUT_PWD/resource
    copy_resource.depends = $$_PRO_FILE_PWD_/../resource
    copy_resource.commands = $(COPY_DIR) \"$$replace(copy_resource.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_resource.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_resource
    PRE_TARGETDEPS += $$copy_resource.target
}


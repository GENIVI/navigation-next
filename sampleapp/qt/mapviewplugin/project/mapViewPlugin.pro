TEMPLATE = lib
TARGET = mapviewplugin
QT += qml quick dbus
CONFIG += qt plugin

LTK_ROOT=../../../library
include ($$LTK_ROOT/locationtoolkit.pro)

INCLUDEPATH += 	../include \
		../../util/include

TARGET = $$qtLibraryTarget($$TARGET)
uri = com.locationstudio.mapviewplugin

CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    DEFINES += QT_NO_DEBUG_OUTPUT
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

# Input
SOURCES += \
    ../src/mapview.cpp \
    ../src/mapViewPlugin.cpp \
    ../../util/src/util.cpp \
    ../../util/src/globalsetting.cpp


HEADERS += \
    ../include/mapview.h \
    ../include/mapViewPlugin.h \
    ../../util/include/globalsetting.h \
    ../../util/include/util.h \
    ../../util/include/servertoken.h \
    ../../util/include/inifile.h


DISTFILES = qmldir

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}

TEMPLATE = lib
TARGET = qtnavigatorconsoleplugin
QT += qml quick dbus
CONFIG += qt plugin
CONFIG += c++11

LTK_ROOT=../../../library
include ($$LTK_ROOT/locationtoolkit.pro)

INCLUDEPATH += 	../include \
		../../util/include

TARGET = $$qtLibraryTarget($$TARGET)
uri = com.locationstudio.qtnavigator.console

CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    DEFINES += QT_NO_DEBUG_OUTPUT
    BUILD_CONFIG = release
    message($$[QT_INSTALL_QML])
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
    ../src/qtnavigatorconsoleplugin.cpp \
    ../src/mapviewcontroller.cpp \
    ../../util/src/util.cpp \
    ../../util/src/globalsetting.cpp \
    ../../util/src/datatransformer.cpp \
    ../../util/src/dataobject.cpp \
    ../../util/src/nearmedataobject.cpp \
    ../src/placemanager.cpp \
    ../src/interpolator.cpp \
    ../src/gpsfile.cpp \
    ../src/hamburgermenucontroller.cpp \
    ../src/mainviewcontroller.cpp \
    ../src/titlebarcontroller.cpp \
    ../src/settingscontroller.cpp \
    ../src/categorysearchcontroller.cpp \
    ../src/searchdetailscontroller.cpp \
    ../src/waitdialog.cpp \
    ../src/nearmecontroller.cpp \
    ../src/messageboxcontroller.cpp \
    ../src/callviewcontroller.cpp \
    ../src/confirmboxcontroller.cpp \
    ../src/routeinfo.cpp \
    ../src/custombubble.cpp \
    ../src/navigationroutecontroller.cpp \
    ../src/navigationsession.cpp \
    ../src/maneuverdetailcontroller.cpp \
    ../src/maneuverslistcontroller.cpp \
    ../../util/src/GPSFile.cpp \
    ../../util/src/maneuverdataobject.cpp

HEADERS += \
    ../include/qtnavigatorconsoleplugin.h \
    ../include/mapviewcontroller.h \
    ../../util/include/util.h \
    ../../util/include/dataobject.h \
    ../../util/include/globalsetting.h \
    ../../util/include/nearmedataobject.h \
    ../include/placemanager.h \
    ../include/interpolator.h \
    ../include/gpsfile.h \
    ../include/geolocation.h \
    ../include/mainviewcontroller.h \
    ../include/titlebarcontroller.h \
    ../include/hamburgermenucontroller.h \
    ../include/categorysearchcontroller.h \
    ../include/settingscontroller.h \
    ../include/searchdetailscontroller.h \
    ../include/mercator.h \
    ../include/waitdialog.h \
    ../include/confirmboxcontroller.h \
    ../include/nearmecontroller.h\
    ../include/callviewcontroller.h\
    ../include/messageboxcontroller.h \
    ../include/routeinfo.h \
    ../include/custombubble.h \
    ../include/mercator.h \
    ../include/navigationroutecontroller.h \
    ../include/navigationsession.h \
    ../include/maneuverdetailcontroller.h \
    ../include/pixmapprovider.h \
    ../include/maneuverslistcontroller.h \
    ../../util/include/GPSFile.h \
    ../../util/include/maneuverdataobject.h

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

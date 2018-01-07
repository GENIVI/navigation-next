#-------------------------------------------------
#
# Project created by QtCreator 2014-10-27T15:37:03
#
#-------------------------------------------------
LTK_ROOT = ../../../library
NIM_ROOT = ../../../../../../..
include ($$LTK_ROOT/locationtoolkit.pro)
QT       += core gui declarative
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = navigationuikit_sample
TEMPLATE = app
#############################
MOC_DIR += ../src/moc
TEMPLATE = app
TARGET = navigationuikit_sample
CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    BUILD_CONFIG = release
}
OBJECTS_DIR += ./obj/$$BUILD_CONFIG
#####################################################
unix {
  QMAKE_CXXFLAGS += -std=c++11
  QMAKE_CXXFLAGS += -Wno-unused-parameter
  QMAKE_CXXFLAGS += -Wno-unused-variable
}

INCLUDEPATH += ../include
INCLUDEPATH += ../../util/include
SOURCES += \
    ../src/main.cpp\
    ../src/mainwindow.cpp \
    ../src/navigationuimapimpl.cpp \
    ../../util/src/globalsetting.cpp \
    ../../util/src/util.cpp \
    ../src/navigationuimapcontroller.cpp \
    ../src/ltknkuitypeimpl.cpp \
    ../src/routeoptionwidget.cpp \
    ../src/interpolator.cpp \
    ../../util/src/devtooldialog.cpp \
    ../../util/src/aboutdialog.cpp \
    ../src/layeroptionwidget.cpp \
    ../src/positioninputdialog.cpp \
    ../../util/src/modeselectordialog.cpp \
    ../../util/src/GPSFile.cpp

HEADERS  += \
    ../include/mainwindow.h \
    ../include/navigationuimapimpl.h \
    ../../util/include/globalsetting.h \
    ../../util/include/inifile.h \
    ../../util/include/servertoken.h \
    ../../util/include/util.h \
    ../include/navigationuimapcontroller.h \
    ../include/ltknkuitypeimpl.h \
    ../include/routeoptionwidget.h \
    ../include/interpolator.h \
    ../../util/include/devtooldialog.h \
    ../../util/include/aboutdialog.h \
    ../include/layeroptionwidget.h \
    ../include/positioninputdialog.h \
    ../../util/include/modeselectordialog.h \
    ../../util/include/GPSFile.h

FORMS    += \
    ../Forms/mainwindow.ui \
    ../../util/Forms/devtooldialog.ui \
    ../../util/Forms/aboutdialog.ui \
    ../Forms/positioninputdialog.ui \
    ../../util/Forms/modeselectordialog.ui

OTHER_FILES += \
    ../resource/qml/CheckBox.qml \
    ../resource/qml/LayerOption.qml \
    ../resource/qml/RouteOption.qml

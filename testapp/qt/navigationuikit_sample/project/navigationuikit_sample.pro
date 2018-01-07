#-------------------------------------------------
#
# Project created by QtCreator 2014-10-27T15:37:03
#
#-------------------------------------------------
LTK_ROOT = ../../../../sampleapp/library
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
    ../src/GPSFile.cpp

HEADERS  += \
    ../include/mainwindow.h \
    ../include/navigationuimapimpl.h \
    ../../util/include/globalsetting.h \
    ../../util/include/inifile.h \
    ../../util/include/servertoken.h \
    ../../util/include/util.h \
    ../include/navigationuimapcontroller.h \
    ../include/ltknkuitypeimpl.h \
    ../src/GPSFile.h

FORMS    += \
    ../Forms/mainwindow.ui

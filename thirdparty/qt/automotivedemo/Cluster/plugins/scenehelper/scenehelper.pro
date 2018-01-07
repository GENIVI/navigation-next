CXX_MODULE = qml
TARGET = clusterscenehelperplugin
TARGETPATH = com/qtcompany/clusterscenehelper
IMPORT_VERSION = 1.0

QT += qml quick 3dcore 3dextras 3drender 3dquick 3dquick-private

SOURCES += \
    plugin.cpp \
    scenehelper.cpp

HEADERS += \
    scenehelper.h

load(qml_plugin)

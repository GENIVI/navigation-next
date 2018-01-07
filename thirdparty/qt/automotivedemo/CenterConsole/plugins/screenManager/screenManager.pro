CXX_MODULE = qml
TARGET = screenmanagerplugin
TARGETPATH = com/pelagicore/ScreenManager
IMPORT_VERSION = 1.0

QT += qml quick
CONFIG += c++11

SOURCES += \
    plugin.cpp \
    screenmanager.cpp \

HEADERS += \
    screenmanager.h \

load(qml_plugin)

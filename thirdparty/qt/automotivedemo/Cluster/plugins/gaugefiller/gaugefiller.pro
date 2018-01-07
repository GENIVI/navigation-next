CXX_MODULE = qml
TARGET = clustergaugefillerplugin
TARGETPATH = com/qtcompany/clustergaugefiller
IMPORT_VERSION = 1.0

QT += qml quick

SOURCES += \
    plugin.cpp \
    gauge.cpp \
    gaugenode.cpp

HEADERS += \
    gauge.h

load(qml_plugin)
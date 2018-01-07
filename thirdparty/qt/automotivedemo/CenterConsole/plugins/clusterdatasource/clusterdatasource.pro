CXX_MODULE = qml
TARGET = clusterdatasourceplugin
TARGETPATH = com/qtcompany/clusterdemodata
IMPORT_VERSION = 1.0

CONFIG += c++11
QT += qml

SOURCES += \
    plugin.cpp \
    qtiviclusterdata.cpp \
    clusterdatabackend.cpp

HEADERS += \
    qtiviclusterdata.h \
    clusterdatabackend.h
    
#qtHaveModule(serialbus):!simulation_only {
#QT += serialbus
#} else: {
DEFINES += CLUSTERDATA_SIMULATION
#}

load(qml_plugin)
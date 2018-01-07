CXX_MODULE = qml
TARGET = qmldatasources
TARGETPATH = com/pelagicore/datasource
IMPORT_VERSION = 1.0

QT += qml quick sql
CONFIG += c++11

SOURCES += \
    sqlquerydatasource.cpp \
    plugin.cpp \
    sqlquerymodel.cpp \
    sqltablemodel.cpp \
    sqltabledatasource.cpp

HEADERS += \
    sqlquerydatasource.h \
    plugin.h \
    sqlquerymodel.h \
    sqltablemodel.h \
    sqltabledatasource.h

load(qml_plugin)

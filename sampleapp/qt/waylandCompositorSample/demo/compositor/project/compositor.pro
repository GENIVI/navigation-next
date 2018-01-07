QT += core gui qml

QT += waylandcompositor-private

CONFIG += wayland-scanner
CONFIG += c++11
TARGET = compositor

WAYLANDSERVERSOURCES += \
            ../../protocol/custom.xml

contains(QT_CONFIG, no-pkg-config) {
    LIBS += -lwayland-server
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += wayland-server
}


target.path = install/
INSTALLS += target

DISTFILES += \
    ../resource/Screen.qml \
    ../resource/Main.qml

HEADERS += \
    ../include/customextension.h

SOURCES += \
    ../src/main.cpp \
    ../src/customextension.cpp

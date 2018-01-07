CONFIG += wayland-scanner
CONFIG += link_pkgconfig

TARGET = externalApplication

QT += quick waylandclient-private

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-client
} else {
    LIBS += -lwayland-client
}

WAYLANDCLIENTSOURCES += ../../protocol/custom.xml



target.path = install/
INSTALLS += target

HEADERS += \
    ../../client-common/include/customextension.h

SOURCES += \
    ../../client-common/src/customextension.cpp \
    ../src/main.cpp

DISTFILES += \
    ../resource/Main.qml

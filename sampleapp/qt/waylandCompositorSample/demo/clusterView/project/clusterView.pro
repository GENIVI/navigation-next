CONFIG += wayland-scanner
CONFIG += link_pkgconfig

TARGET = clusterView

QT += quick waylandclient-private

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += wayland-client
} else {
    LIBS += -lwayland-client
}

WAYLANDCLIENTSOURCES += ../../protocol/custom.xml

target.path = install/
INSTALLS += target

SOURCES += \
    ../src/main.cpp \
    ../../client-common/src/customextension.cpp

DISTFILES += \
    ../resource/Main.qml

HEADERS += \
    ../../client-common/include/customextension.h \
    ../../client-common/include/ \


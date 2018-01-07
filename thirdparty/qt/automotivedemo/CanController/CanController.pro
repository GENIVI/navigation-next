QT = core gui quick qml
DISTFILES += \
    CarUI.qml \
    Button.qml \
    Slide.qml \
    Blinks.qml \
    Car.qml \
    Gear.qml \
    GearAutomatic.qml \
    ButtonHolder.qml \
    SlideHolder.qml \
    ViewChange.qml

DEFINES += TCPCLUSTERDATACONNECTION
SOURCES += \
    tcpclusterdatacontrol.cpp
HEADERS += \
    tcpclusterdatacontrol.h

SOURCES += \
    main.cpp \
    gpssender.cpp \
    nmea.cpp

HEADERS += \
    clusterdatacontrolinterface.h \
    gpssender.h \
    nmea.h \
    valuetypes.h

RESOURCES += carui.qrc

DEFINES += MAIN_QML_FILE_NAME=\\\"qrc:/CarUI.qml\\\" DEFAULT_ROUTE_FILE=\\\":/route.txt\\\"

TEMPLATE = app
TARGET = carui

target.path = /data/user/qt/$$TARGET
INSTALLS += target

TEMPLATE = subdirs
SUBDIRS = plugins

OTHER_FILES += \
    sysui/* \
    sysui/Climage/* \
    sysui/Cloud/* \
    sysui/CenterItems/* \
    sysui/Launcher/* \
    sysui/Statusbar/* \
    sysui/BottomBar/* \
    apps/com.pelagicore.media/* \
    apps/com.qtcompany.calendar/* \
    apps/com.qtcompany.carinfo/* \
    apps/com.qtcompany.contacts/* \
    apps/com.qtcompany.map/* \
    apps/* \

PREFIX = /opt
qml.files = apps sysui
qml.path = $$PREFIX/automotivedemo/CenterConsole
INSTALLS += qml

TEMPLATE = subdirs

SUBDIRS = \
        CenterConsole \
        Cluster

qml.files = imports kms_config.json Main*.qml am-config.yaml start.sh
qml.path = /opt/automotivedemo
startupscript.files = scripts/neptune.service
startupscript.path = /lib/systemd/system
INSTALLS += qml startupscript

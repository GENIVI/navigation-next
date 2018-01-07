TEMPLATE = subdirs
SUBDIRS = plugins

PREFIX = /opt
qml.files = qml images
qml.path = $$PREFIX/automotivedemo/Cluster
INSTALLS += qml

#!/bin/sh
export QT_QPA_EGLFS_KMS_CONFIG=kms_config.json
#cd /opt/automotivedemo
#/usr/bin/appcontroller /usr/#bin/appman -r -c /opt/am/config.yaml -c am-config.yaml --dbus none MainWithCluster.qml

export QT_XCB_GL_INTEGRATION=xcb_egl
export QT_IM_MODULE=qtvirtualkeyboard
export SVGA_VGPU10=0
export PATH=/opt/Qt5.8.0/bin:$PATH
export LD_LIBRARY_PATH=/opt/locationstudio/libs/png:/opt/locationstudio/libs/ncdb:$LD_LIBRARY_PATH
#appman --force-single-process -r -c /opt/am/config.yaml -c am-config.yaml --dbus none MainWithCluster.qml

appman -c /opt/am/config.yaml -c am-config.yaml -r --no-fullscreen

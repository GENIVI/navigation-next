#!/bin/bash         

#install dependencies for LTK

BUILD_TYPE=$1

if [ "$BUILD_TYPE" = "all" ]; then
    sudo apt update
    sudo apt install libgbm1 libegl1-mesa libegl1-mesa-drivers libgles2-mesa libgles2-mesa-dev libgl1-mesa-dri
    sudo apt install libasound2-dev alsa-oss libphonon-dev libwayland-client0 libwayland-server0
    sudo apt install libsqlite3-dev libfreetype6-dev

    #install dependencies for automotive demo

    sudo apt install libgstreamer1.0-dev
    sudo apt install libwayland-dev
    sudo apt install libwayland-egl1-mesa*
    sudo apt install libgstreamer1.0-0
    sudo apt install libssl-dev
    sudo apt install libarchive-dev
    sudo apt install libyaml-dev
    sudo apt install libfontconfig1-dev

    #install libdbus 1.6

    sudo apt install libdbus-1-dev libdbus-1-3
    sudo apt install libdbus-glib-1-dev

    # TODO: Install Qt 5.8.0

    echo "Install dependencies completed"
fi

#Cleaning the exisiting folder

sudo rm -rf /opt/locationstudio/res/*
sudo rm -rf /opt/locationstudio/libs/*
sudo rm -rf /opt/am/*
sudo rm -rf /opt/automotivedemo/*
sudo rm -rf /opt/neptuneui/*

if [ "$BUILD_TYPE" = "all" ]; then
    sudo rm -rf /opt/locationstudio/*
fi

#Copy Qt5.8.0, AppManager, AutomotiveDemo binaries

sudo cp -r bin/thirdparty/* /opt/
sudo chown $(whoami) -R /opt/am
sudo chmod -R ugo+rw /opt/am
sudo chown $(whoami) -R /opt/locationstudio/res
sudo rm -rf /opt/qtapplicationmanager

echo "Copy AppManager, AutomotiveDemo binaries completed"

#Copy plugins

export PATH=/opt/Qt5.8.0/bin:$PATH

qmlPath=`qmake -query QT_INSTALL_QML`

consolePluginPath="$qmlPath/com/locationstudio/qtnavigator/console"
clusterPluginPath="$qmlPath/com/locationstudio/qtnavigator/cluster"
mapviewPluginPath="$qmlPath/com/locationstudio/mapviewplugin"

sudo mkdir -p $consolePluginPath
sudo mkdir -p $clusterPluginPath
sudo mkdir -p $mapviewPluginPath

sudo cp -f bin/qtnavigator_console_plugin/libqtnavigatorconsoleplugin.so $consolePluginPath
sudo cp -f bin/qtnavigator_console_plugin/qmldir $consolePluginPath

sudo cp -f bin/qtnavigator_cluster_plugin/libqtnavigatorclusterplugin.so $clusterPluginPath
sudo cp -f bin/qtnavigator_cluster_plugin/qmldir $clusterPluginPath

sudo cp -f bin/mapviewplugin/libmapviewplugin.so $mapviewPluginPath
sudo cp -f bin/mapviewplugin/qmldir $mapviewPluginPath

echo "Automotive installation Successful"


# Overview

You are going to need a data source. The CanController synthesizes events and can deliver them via CAN or TCP. TCP is strongly recommended
(unless you know better) as the CAN interface can produce 8-bit overflow issues and other unnoted problems, which should be investigated later.

The CanController (client) synthesizes CAN events, while the automotivedemo (server) responds to said events.

If you really want to use CAN, you will probably want to use the SocketCAN driver, which can be set up on a Linux system using the following
commands:

    sudo modprobe vcan
    sudo ip link add dev can0 type vcan
    sudo ip link set up can0

At least Qt 5.8 is required to build automotivedemo. Currently only TCP is supported in this automotivedemo.

# Walkthrough
NOTE: MAPS ARE NOT WORKING OFFLINE, bug in https://bugreports.qt.io/browse/QTBUG-57011
In case offline maps are needed, checkout following commit from qtlocation and recompile mapbox plugin:
  https://codereview.qt-project.org/#/c/176591/

Automotivedemo uses appman, neptune-ui and QtIvi. If you are using automotive suite, those are already installed but for desktop you need to build
1. QtIvi
    git clone git://code.qt.io/qt/qtivi.git

2. QtApplicationManager
    git clone git://code.qt.io/qt/qtapplicationmanager.git

For all:

1. Clone automotivedemo repository:

    git clone -b master git@git.qt.io:playground/automotivedemo.git

2. Chose your data sources (using TCP, or create your CAN devices as necessary).

3. Build plugins from automotivedemo repository

        qmake CenterConsole/plugins.pro -r CONFIG+=simulation_only
        make

        qmake Cluster/plugins.pro -r
        make

4. Copy media and database to homepath.

5. Run apps

        If TCP connection is used, get the IP address from the device where automotivedemo is installed.
        Then use the IP address in CanController on runtime by defining QT_CLUSTER_SIMU env variable
        e.g. QT_CLUSTER_SIMU=192.168.0.1 by default it is localhost 127.0.0.1.

        In device set the following environment variable:
        export QT_QPA_EGLFS_KMS_CONFIG=<deployed kms_config.json file from automotivedemo git>

        Run both apps in desktop:
        appman.exe -r -c \qtapplicationmanager\template-opt\am\config.yaml -c am-config.yaml --dbus none MainWithCluster.qml

        Run only center console in desktop
        appman.exe -r -c \qtapplicationmanager\template-opt\am\config.yaml -c am-config.yaml --dbus none Main.qml

        In device use:
        /usr/bin/appcontroller /usr/bin/appman -r -c /opt/am/config.yaml -c am-config.yaml --dbus none MainWithCluster.qml

# Making it run automatically in HW

    Replace neptune.service under /lib/systemd/system with the version included in the project, under scripts folder:
        adb push scripts/neptune.service /lib/systemd/system

    Give the scripts execution rights:
        adb shell
        chmod +x /lib/systemd/system/neptune.service
        chmod +x /opt/automotivedemo/start.sh

    If you have trouble making the CanController work, there is a fully automatic demo mode, which
    can be activated by setting the "fullDemo" property to true in ValueSource.qml:

    //
    // ENABLE FOR FULLY AUTOMATIC DEMO MODE (in case there is no CanController)
    //
    property bool fullDemo: true//false

    You can find the ValueSource.qml file under /opt/automotivedemo/imports/shared/service/valuesource
    in HW, or automotivedemo\imports\shared\service\valuesource in project hierarchy.

Typical errors:
    * If you are not seeing the route in the map after you have selected destination, make sure that openssl libraries are found. Route feature is available
      only from Qt5.8 onwards.
    * "QtQuick.VirtualKeyboard is not installed" -> set environmentvariable QT_IM_MODULE=qtvirtualkeyboard
    * If you are seeing red car rotating, then you are using too old qt3d version. Car should be white. Use 5.8 branch from qt3d repository.

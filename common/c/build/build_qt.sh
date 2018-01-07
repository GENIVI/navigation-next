#!/bin/bash

case "$1" in
    -c)
        CONFIGURATION=$2
    ;;
    *)
        echo "Usage: build_qt [OPTION...]" 
        echo "    -c     specify build configuration, could be 'qnx_arm', 'qnx_x86', 'linux_armhf', 'linux_x86', 'linux_x86_64"
        echo "    -?     give this help list"
        exit 1
    ;;
esac

CONFIGURATION_FILE=./configuration/qt_${CONFIGURATION}.txt
if [ ! -f $CONFIGURATION_FILE ] ; then
    echo "Can't find configuration file '${CONFIGURATION_FILE}'"
    echo "Configuration should be one of 'qnx_arm', 'qnx_x86', 'linux_armhf', 'linux_x86'"
    exit 1
fi

#config.txt
. $CONFIGURATION_FILE

echo $NCDB_LIBRARY_PATH

#prepare NBM library
rm -rf nbm
NBM_INCLUDE_FROM=${NBM_INCLUDE_PATH}
if [ -d ${NBM_INCLUDE_PATH}/output ] ; then
    NBM_INCLUDE_FROM=${NBM_INCLUDE_PATH}/output
fi

NBM_INCLUDE_FROM=${NBM_INCLUDE_FROM}/include/NBM
cp -rf ${NBM_INCLUDE_FROM} ./nbm
mkdir -p ./nbm/include
mv ./nbm/*.* ./nbm/include
#if [ -d ./nbm/libs/64bit ] ; then
#    mv ./nbm/libs/64bit/*.a ./nbm/libs
#    rmdir ./nbm/libs/64bit
#fi

#prepare NCDB library
rm -rf ncdb
cp -rf ${NCDB_LIBRARY_PATH}/output ./ncdb
if [ -d ./ncdb/libs/32bit ] ; then
    mv ./ncdb/libs/32bit/*.so ./ncdb/libs
    rmdir ./ncdb/libs/32bit
else
    mv ./ncdb/libs/64bit/*.so ./ncdb/libs
    rmdir ./ncdb/libs/64bit
fi

#prepare version file
cp -f ${VERSION_FILE} version_qt.txt

#create links for map, search, nav and navui

SAVEPATH=$PWD
cd ..

if [ -d search ]; then
    rm search
fi
ln -s ../../searchkit/c/ search

if [ -d map ]; then
    rm map
fi
ln -s ../../mapkit3d/c/ map

if [ -d nav ]; then
    rm nav
fi
ln -s ../../navigationkit/c/ nav

if [ -d navui ]; then
    rm navui
fi
ln -s ../../navigationuikit/c/ navui

cd $SAVEPATH

python util/build_qt.py offboard release ${TARGET} ${PLATFORM}

#!/bin/sh
#export environment variable
#########################################
export NIMNB_HOME=${NIMNB_INC}/..
#########################################
#build nbservices
echo "building nbservices..."
cd $NIMNB_HOME/projects/android_ndk
$ANDROID_NDK_HOME/ndk-build
echo "build nbservices finished"

#!/bin/sh
#export environment variable
#########################################
export NIMCORE_HOME=${NIMCORE_INC}/..
#########################################
#build coreservices
echo "building coreservices..."
cd $NIMCORE_HOME/projects/android_ndk
$ANDROID_NDK_HOME/ndk-build
echo "build coreservices finished"

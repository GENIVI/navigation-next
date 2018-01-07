#!/bin/sh
#export environment variable
#########################################
export NIMABPAL_HOME=${NIMABPAL_INC}/..
#########################################
#build abpal
echo "building abpal..."
cd $NIMABPAL_HOME/projects/android_ndk
$ANDROID_NDK_HOME/ndk-build
echo "build abpal finished"

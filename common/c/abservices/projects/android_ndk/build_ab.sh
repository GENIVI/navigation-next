#!/bin/sh
#export environment variable
#########################################
export NIMAB_HOME=${NIMAB_INC}/..
#########################################
#build abservices
echo "building abservices..."
cd $NIMAB_HOME/projects/android_ndk
$ANDROID_NDK_HOME/ndk-build
echo "build abservices finished"

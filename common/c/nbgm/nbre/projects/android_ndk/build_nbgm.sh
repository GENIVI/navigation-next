#!/bin/sh
#export environment variable
#########################################
export NIMNBRE_HOME=$NIMNBRE_INC/..
#########################################
#build nbgm
echo "building nbgm..."
cd $NIMNBRE_HOME/projects/android_ndk
$ANDROID_NDK_HOME/ndk-build
echo "build nbgm finished"

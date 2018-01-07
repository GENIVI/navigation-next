#!/bin/sh
#export environment variable
#########################################
export NIMJNI_HOME=${NIMJNI_INC}/..
#########################################
#build jniwrapper
echo "building jni..."
cd $NIMJNI_HOME/projects/android_ndk
$ANDROID_NDK_HOME/ndk-build
echo "build jni finished"

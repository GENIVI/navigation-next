#!/bin/sh
#export environment variable
#########################################
echo {NIMNBIPAL_INC}: ${NIMNBIPAL_INC}
export NIMNBIPAL_HOME=${NIMNBIPAL_INC}/..
echo NIMNBIPAL_HOME: $NIMNBIPAL_HOME
#########################################
#build nbpal
echo "building nbpal..."
cd $NIMNBIPAL_HOME/projects/android_ndk
$ANDROID_NDK_HOME/ndk-build NDK_DEBUG=$NDK_DEBUG V=1 -j 4
echo "build nbpal finished"

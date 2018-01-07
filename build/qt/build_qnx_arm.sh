#!/bin/sh
CORE_SDK_VERSION=$(sed -ne '1p' "coresdk_qt_qnx_arm_hybrid_version.txt")

cp locationtoolkit_unified.pro locationtoolkit.pro

rm -rf CoreSDK
cp -rf ../../../../c/releases/sdk/qt_qnx_arm/CoreSDK_${CORE_SDK_VERSION}_qt_qnx_arm_internal CoreSDK

rm -f version.txt
cp -f version_qt_qnx_arm.txt version.txt

sh ./build_tpl.sh

python ./util/build.py qnx

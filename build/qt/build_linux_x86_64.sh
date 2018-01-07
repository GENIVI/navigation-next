#!/bin/sh

LTK_BUILD_HYBRID_TYPE=$1
TOOLCHAIN_SHORTNAME=$2
ARCHITECTURE=$3

CORE_SDK_VERSION=$(sed -ne '1p' "coresdk_qt_linux_x86_64_hybrid_version.txt")

if [ "$LTK_BUILD_HYBRID_TYPE" = "hybrid" ]; then
CORE_SDK_VERSION=$(sed -ne '1p' "coresdk_qt_linux_x86_64_hybrid_version.txt")
fi

cp locationtoolkit_unified.pro locationtoolkit.pro

rm -rf CoreSDK
cp -rf ../../common/c/build/CoreSDK_${CORE_SDK_VERSION}_qt_linux_x86_64_internal CoreSDK

rm -f version.txt
cp -f version_qt_linux_x64.txt version.txt

sh ./build_tpl.sh

python ./util/build.py linux ${LTK_BUILD_HYBRID_TYPE} ${TOOLCHAIN_SHORTNAME} ${ARCHITECTURE}

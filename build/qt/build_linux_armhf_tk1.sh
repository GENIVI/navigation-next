#!/bin/sh

LTK_BUILD_HYBRID_TYPE=$1

#ln -s /usr/local/home/parabuild/tegra-jetson-toolchains/rootfs-jetson-tk1/lib/arm-linux-gnueabihf /lib/arm-linux-gnueabihf
#ln -s /usr/local/home/parabuild/tegra-jetson-toolchains/rootfs-jetson-tk1/usrlib/arm-linux-gnueabihf /usr/lib/arm-linux-gnueabihf

CORE_SDK_VERSION=$(sed -ne '1p' "coresdk_qt_linux_armhf_version.txt")

if [ "$LTK_BUILD_HYBRID_TYPE" = "hybrid" ]; then
CORE_SDK_VERSION=$(sed -ne '1p' "coresdk_qt_linux_armhf_hybrid_version.txt")
fi

rm -rf CoreSDK
cp -rf ../../../../c/releases/sdk/qt_linux_armhf/CoreSDK_${CORE_SDK_VERSION}_qt_linux_armhf_internal CoreSDK

rm -f version.txt
cp -f version_qt_linux_armhf.txt version.txt

sh ./build_tpl.sh

python ./util/build.py linux ${LTK_BUILD_HYBRID_TYPE}

#rm -f /lib/arm-linux-gnueabihf
#rm -f /usr/lib/arm-linux-gnueabihf

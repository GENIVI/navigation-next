#!/bin/bash
QT_VER=""
TOOLCHAIN=""
DOCLEAN=""
TOOLCHAIN_SHORTNAME="ubuntu"
ARCHITECTURE="x86_64"
#QT_HOST_BIN=`qmake -query QT_HOST_BINS`
#QT_INSTALL_LIBS=`qmake -query QT_INSTALL_LIBS`

case "$1" in
    -t)
        TOOLCHAIN_SHORTNAME=$2
        if [ "$TOOLCHAIN_SHORTNAME" != "ubuntu" ] && [ "$TOOLCHAIN_SHORTNAME" != "nuc" ] && [ "$TOOLCHAIN_SHORTNAME" != "nvidia" ] && [ "$TOOLCHAIN_SHORTNAME" != "imx6" ]
        then
            echo "Usage: build_ccc_ltk_qt_demo [OPTION...]"
            echo "  -t ubuntu, nuc, nvidia, imx6"
            exit 1
        fi
    ;;

    -c)
	DOCLEAN=yes
    ;;

    *)
        echo "Usage: build_ccc_ltk_qt_demo [OPTION...]" 
        echo "  -c linux_x86_64, intel_corei7_64, jetson_tx1, nitrogen6x"
        exit 1
    ;;
esac
case "$2" in
    -t)
        TOOLCHAIN_SHORTNAME=$3
        if [ "$TOOLCHAIN_SHORTNAME" != "ubuntu" ] && [ "$TOOLCHAIN_SHORTNAME" != "nuc" ] && [ "$TOOLCHAIN_SHORTNAME" != "nvidia" ] && [ "$TOOLCHAIN_SHORTNAME" != "imx6" ]
        then
            echo "Usage: build_ccc_ltk_qt_demo [OPTION...]"
            echo "  -t ubuntu, nuc, nvidia, imx6"
            exit 1
        fi
    ;;

    -c)
	DOCLEAN=yes
    ;;

    *)
        echo "Usage: build_ccc_ltk_qt_demo [OPTION...]" 
        echo "  -c linux_x86_64, intel_corei7_64, jetson_tx1, nitrogen6x"
        exit 1
    ;;
esac
case "$3" in
    -c)
	DOCLEAN=yes
    ;;
esac
	
if [ "$TOOLCHAIN_SHORTNAME" = "ubuntu" ]
then
	TOOLCHAIN=linux_x86_64
	ARCHITECTURE=x86_64

elif [ "$TOOLCHAIN_SHORTNAME" = "nuc" ]
then
	TOOLCHAIN=intel_corei7_64
	ARCHITECTURE=x86_64

elif [ "$TOOLCHAIN_SHORTNAME" = "nvidia" ]
then
	TOOLCHAIN=jetson_tx1
	ARCHITECTURE=ARM_64

elif [ "$TOOLCHAIN_SHORTNAME" = "imx6" ]
then
	TOOLCHAIN=nitrogen6x
	ARCHITECTURE=ARM_32

fi

export DOCLEAN=$DOCLEAN

QTDIR=/opt/Qt5.9.2

TOOLCHAIN_QT_PATH="$( echo "$TOOLCHAIN" | tr  '_' '-'  )"

if [ "$TOOLCHAIN" = "linux_x86_64" ]
then
	export PATH=$QTDIR/bin:$PATH
else
	export PATH=/opt/Qt/5.9.2/Boot2Qt/$TOOLCHAIN_QT_PATH/toolchain/sysroots/x86_64-pokysdk-linux/usr/bin:$PATH
	echo $PATH
fi

#Build CCC
cd ../../common/c/build
sh build_qt.sh -c ${TOOLCHAIN}

#Build LTK
cd ../../../build/qt
sh build_linux_x86_64.sh hybrid ${TOOLCHAIN_SHORTNAME} ${ARCHITECTURE}

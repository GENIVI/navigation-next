#!/bin/sh

cd ../../common/c/build

export QTDIR=/opt/Qt5.8.0
export PATH=$QTDIR/bin:$PATH

#Build CCC

sh build_qt.sh -c linux_x86_64

cd ../../../build/qt

#Build LTK

sh build_linux_x86_64.sh hybrid

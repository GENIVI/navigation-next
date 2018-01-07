#config.txt
. ./configuration/qt_linux_x86.txt

CCC_MODE="offboard"
if [ $# -gt 0 ]; then
   CCC_MODE=$1
fi

#prepare NBM library
rm -rf nbm
cp -rf ${NBM_LIBRARY_PATH} ./nbm

#prepare NCDB library
rm -rf ncdb
cp -rf ${NCDB_LIBRARY_PATH} ./ncdb

#prepare version file
cp -f ${VERSION_FILE} version_qt.txt

#build
python util/build_qt.py ${CCC_MODE} all ${target} ${platform}


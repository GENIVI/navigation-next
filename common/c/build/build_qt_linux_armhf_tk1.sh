NBM_VERSION=$(sed -ne '1p' nbm_linux_armhf_version.txt)
rm -rf nbm
cp -rf ../../../../../core/releases/nbm/armhf/nbm_${NBM_VERSION}_armhf ./nbm

CCC_MODE="offboard"
if [ $# -gt 0 ]; then
   CCC_MODE=$1
fi
if [ ${CCC_MODE} = "onboard" ]; then
    NCDB_VERSION=$(sed -ne '1p' ncdb_linux_armhf_version.txt)
    rm -rf ncdb
    cp -rf ../../../../../core/releases/ncdb/ubuntuarm/${NCDB_VERSION} ./ncdb
fi

cp -f version_qt_linux_armhf.txt version_qt.txt

python util/build_qt.py ${CCC_MODE} all linux_armhf linux

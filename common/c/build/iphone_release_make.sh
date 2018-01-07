#!/bin/sh

# update_xcode_env -- Utility to update environment for XCode
# it accepts two arguments:
#   1. name
#   2. path
function update_xcode_env ()
{
    defaults write com.apple.dt.Xcode IDEApplicationwideBuildSettings -dict-add $1 "$2"
    defaults write com.apple.dt.Xcode IDESourceTreeDisplayNames -dict-add $1 $1
}

# pack_library -- pack library of debug version and release version.
#                 Library of release version will be stripped.
# It accepts three parameters:
#   1. library name.
#   2. path of debug version library.
#   3. path of release version library.
function pack_library ()
{
    echo "Packaging library: $1"
    lipo -create $2 $3 -output $DEST_DIR_LIB_DEBUG/$1
    strip -S -x $3 > /dev/null 2>&1
    lipo -create $3 -output $DEST_DIR_LIB/$1
}

# Init
SCRIPT_DIR=$(pwd -P)
CCC_ROOT=${SCRIPT_DIR}/../..
BRANCH=$(basename ${SCRIPT_DIR})
CCC_VERSION=`sed -ne '1p' version_iphone.txt`
CCC_MODE='offboard'

# configure necessary environment variables
NBM_VERSION=`head nbm_iphone_version.txt`
NBM_PATH=$(PWD)/../../../../../core/releases/nbm/iphone/nbm_"$NBM_VERSION"_iphone
update_xcode_env NBM_RELEASE $NBM_PATH

# Dest dir is
DEST_DIR_NAME=coreSDK_"$CCC_VERSION"_iphone_internal
DEST_DIR_PATH=$CCC_ROOT/build/$BRANCH

# set build machine
# How to set build machine
# 1. iphone_release_make.sh           - it is for all dev machine
# 2. iphone_release_make.sh local     - it is for all dev machine
# 3. iphone_release_make.sh parabuild - it is for parabuild

rm -r -f $DEST_DIR_PATH/$DEST_DIR_NAME
mkdir -p $DEST_DIR_PATH/$DEST_DIR_NAME/{lib,include/iphone,release_pal}
mkdir -p $DEST_DIR_PATH/$DEST_DIR_NAME/lib-debug
DEST_DIR_LIB=$DEST_DIR_PATH/$DEST_DIR_NAME/lib
DEST_DIR_LIB_DEBUG=$DEST_DIR_PATH/$DEST_DIR_NAME/lib-debug
DEST_DIR_INCLUDE=$DEST_DIR_PATH/$DEST_DIR_NAME/include
DEST_DIR_INCLUDE_IPHONE=$DEST_DIR_INCLUDE/iphone

echo "-= Generate Mapkit 3D Code. =-"
#python2.4 util/build_iphone.py

python2.4 util/codegenerator.py $CCC_VERSION iphone

if [ $? -ne 0 ]; then
    echo "Failed to generate files ..."
    exit 1
fi
python2.4 util/makemodeinfo.py $CCC_MODE

echo "-= Mapkit 3D Code Generate Complete. =-"
echo
echo

# Build
./iphone_build_xcode.sh debug && ./iphone_build_xcode.sh release
if [ $? -ne 0 ]; then
    echo "Failed to build libraries, refer to logs for more information.!"
    exit 1
fi

echo "-=Copying libraries-="
echo ""
# LIPO with version number
pack_library libnbpal.a \
    $CCC_ROOT/nbpal/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libnbpal.a \
    $CCC_ROOT/nbpal/$BRANCH/projects/iphone/build/Release-iphoneos/libnbpal.a \

pack_library libcoreservices.a \
    $CCC_ROOT/coreservices/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libcoreservices.a \
    $CCC_ROOT/coreservices/$BRANCH/projects/iphone/build/Release-iphoneos/libcoreservices.a

pack_library libnbservices.a \
    $CCC_ROOT/nbservices/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libnbservices.a \
    $CCC_ROOT/nbservices/$BRANCH/projects/iphone/build/Release-iphoneos/libnbservices.a

pack_library libabpal.a \
    $CCC_ROOT/abpal/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libabpal.a \
    $CCC_ROOT/abpal/$BRANCH/projects/iphone/build/Release-iphoneos/libabpal.a \

pack_library libabservices.a \
    $CCC_ROOT/abservices/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libabservices.a \
    $CCC_ROOT/abservices/$BRANCH/projects/iphone/build/Release-iphoneos/libabservices.a

pack_library libnbui.a \
    $CCC_ROOT/nbui/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libnbui.a \
    $CCC_ROOT/nbui/$BRANCH/projects/iphone/build/Release-iphoneos/libnbui.a

pack_library libnbgm.a \
    $CCC_ROOT/nbgm/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libnbgm.a \
    $CCC_ROOT/nbgm/$BRANCH/projects/iphone/build/Release-iphoneos/libnbgm.a

pack_library libnbcommon.a \
    $CCC_ROOT/common/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libnbcommon.a \
    $CCC_ROOT/common/$BRANCH/projects/iphone/build/Release-iphoneos/libnbcommon.a

pack_library libnav.a \
    $CCC_ROOT/nav/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libnav.a \
    $CCC_ROOT/nav/$BRANCH/projects/iphone/build/Release-iphoneos/libnav.a

pack_library libnavui.a \
    $CCC_ROOT/navui/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libnavui.a \
    $CCC_ROOT/navui/$BRANCH/projects/iphone/build/Release-iphoneos/libnavui.a

pack_library libmap.a \
    $CCC_ROOT/map/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libmap.a \
    $CCC_ROOT/map/$BRANCH/projects/iphone/build/Release-iphoneos/libmap.a

pack_library libsearch.a \
    $CCC_ROOT/search/$BRANCH/projects/iphone/build/Debug-iphonesimulator/libsearch.a \
    $CCC_ROOT/search/$BRANCH/projects/iphone/build/Release-iphoneos/libsearch.a

echo "-=Copying libraries Complete-="
echo ""
echo ""


# Collect header files
echo "-=Copying Headers-="
echo ""
find $CCC_ROOT/nbpal/$BRANCH/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE
find $CCC_ROOT/nbpal/$BRANCH/include/iphone -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE_IPHONE
find $CCC_ROOT/abpal/$BRANCH/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE
find $CCC_ROOT/abpal/$BRANCH/include/iphone -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE_IPHONE
find $CCC_ROOT/coreservices/$BRANCH/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE
find $CCC_ROOT/coreservices/$BRANCH/include/iphone -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE_IPHONE
find $CCC_ROOT/nbservices/$BRANCH/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE
find $CCC_ROOT/nbservices/$BRANCH/include -type f -depth 1 -name \*.hpp | xargs -J % cp % $DEST_DIR_INCLUDE
find $CCC_ROOT/nbservices/$BRANCH/include/iphone -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE_IPHONE
find $CCC_ROOT/abservices/$BRANCH/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE
find $CCC_ROOT/abservices/$BRANCH/include/iphone -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE_IPHONE
find $CCC_ROOT/nbui/$BRANCH/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE
find $CCC_ROOT/nbui/$BRANCH/include/iphone -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE_IPHONE
find $CCC_ROOT/nbgm/$BRANCH/nbgmmain/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE
find $CCC_ROOT/nbgm/$BRANCH/nbgmmain/include/iphone -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE_IPHONE
find $CCC_ROOT/common/$BRANCH/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE
find $CCC_ROOT/common/$BRANCH/include/iphone -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE
find $CCC_ROOT/map/$BRANCH/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE

# TODO: each component should provide interface for this build script to copy headers!
mkdir  $DEST_DIR_INCLUDE/navui/
find $CCC_ROOT/navui/$BRANCH/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE/navui/
cp -aRf "$CCC_ROOT/navui/$BRANCH/include/widgets" $DEST_DIR_INCLUDE/navui/

rm -rf $DEST_DIR_INCLUDE/search/
mkdir $DEST_DIR_INCLUDE/search/
find $CCC_ROOT/search/$BRANCH/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE/search/

rm -rf $DEST_DIR_INCLUDE/nav/
mkdir $DEST_DIR_INCLUDE/nav/
find $CCC_ROOT/nav/$BRANCH/include -type f -depth 1 -name \*.h | xargs -J % cp % $DEST_DIR_INCLUDE/nav/
echo "-=Copying Headers Complete-="

echo "-=Copying Release Pal-="
echo ""
DEST_DIR_RELEASE_PAL=$DEST_DIR_PATH/$DEST_DIR_NAME/release_pal
sh ./release_package_pal_iphone.sh $DEST_DIR_PATH/$DEST_DIR_NAME nbpal $DEST_DIR_RELEASE_PAL $CCC_ROOT $BRANCH
sh ./release_package_pal_iphone.sh $DEST_DIR_PATH/$DEST_DIR_NAME abpal $DEST_DIR_RELEASE_PAL $CCC_ROOT $BRANCH
echo "-=Copying Release Pal Complete-="

echo ""

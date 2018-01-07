#!/bin/sh
#
# iphone_build_xcode.sh
#
# How to build CCC in batch way:
# 1. iphone_build_xcode.sh debug
# 2. iphone_build_xcode.sh release

if [ $# -eq 0 ]; then
    echo "Usage: iphone_build_xcode.sh [debug|release]"
    exit 1
fi

if [ -z $SDK_VERSION ]; then
    SDK_VERSION=8.0
fi

DEPLOYMENT_TARGET=7.0


SCRIPT_DIR=$(pwd -P)
CCC_ROOT=${SCRIPT_DIR}/../..
BRANCH=$(basename ${SCRIPT_DIR})

BUILD_TYPE=$1
DEVICE="iphonesimulator$SDK_VERSION"

if [ "$BUILD_TYPE" == "debug" ]; then
	BUILD_TYPE="Debug"
    DEVICE="iphonesimulator$SDK_VERSION"
elif [ "$BUILD_TYPE" == "release" ]; then
	BUILD_TYPE="Release"
    DEVICE="iphoneos$SDK_VERSION"
else
	echo "input debug or release to make the build."
	exit 1
fi

# The pathes of projects
ARGTABLE_PROJ_FILE="../../thirdparty/$BRANCH/projects/iphone/ArgTable/ArgTable.xcodeproj"
CUNIT_PROJ_FILE="../../thirdparty/$BRANCH/projects/iphone/CUnit/cunit.xcodeproj"
FREETYPE_PROJ_FILE="../../thirdparty/$BRANCH/projects/iphone/freetype.xcodeproj"
GLUES_PROJ_FILE="../../thirdparty/$BRANCH/projects/iphone/glues.xcodeproj"

NBPAL_PROJ_FILE="../../nbpal/$BRANCH/projects/iphone/nbpal.xcodeproj"
ABPAL_PROJ_FILE="../../abpal/$BRANCH/projects/iphone/abpal.xcodeproj"
CORESERVICES_PROJ_FILE="../../coreservices/$BRANCH/projects/iphone/coreservices.xcodeproj"
NBSERVICES_PROJ_FILE="../../nbservices/$BRANCH/projects/iphone/nbservices.xcodeproj"
ABSERVICES_PROJ_FILE="../../abservices/$BRANCH/projects/iphone/abservices.xcodeproj"
NBUI_PROJ_FILE="../../nbui/$BRANCH/projects/iphone/nbui.xcodeproj"

NBGM_PROJ_FILE="../../nbgm/$BRANCH/projects/iphone/nbgm.xcodeproj"

NBCOMMON_PROJ_FILE="../../common/$BRANCH/projects/iphone/nbcommon.xcodeproj"
NAV_PROJ_FILE="../../nav/$BRANCH/projects/iphone/nav.xcodeproj"
NAVUI_PROJ_FILE="../../navui/$BRANCH/projects/iphone/navui.xcodeproj"
MAP_PROJ_FILE="../../map/$BRANCH/projects/iphone/map.xcodeproj"
SEARCH_PROJ_FILE="../../search/$BRANCH/projects/iphone/search.xcodeproj"

NBPAL_TEST_PROJ_FILE="../../nbpal/$BRANCH/projects/iphone/unittest.xcodeproj"
ABPAL_TEST_PROJ_FILE="../../abpal/$BRANCH/projects/iphone/abpal-test.xcodeproj"
SYSTEMTESTS_PROJ_FILE="../../systemtests/$BRANCH/projects/iphone/SystemTest.xcodeproj"

#TOOLS_PROJECTS="$ARGTABLE_PROJ_FILE $CUNIT_PROJ_FILE"
#TESTS_PROJECTS="$NBPAL_TEST_PROJ_FILE $ABPAL_TEST_PROJ_FILE $SYSTEMTESTS_PROJ_FILE"
CCC_PROJECTS="$NBPAL_PROJ_FILE $ABPAL_PROJ_FILE $CORESERVICES_PROJ_FILE \
  $NBSERVICES_PROJ_FILE $ABSERVICES_PROJ_FILE $NBUI_PROJ_FILE $NBGM_PROJ_FILE \
  $NBCOMMON_PROJ_FILE $NAV_PROJ_FILE $MAP_PROJ_FILE $SEARCH_PROJ_FILE $NAVUI_PROJ_FILE"
TOOLS_PROJECTS="$LIBPNG_PROJ_FILE $FREETYPE_PROJ_FILE $GLUES_PROJ_FILE"
ALL_PROJECTS="$TOOLS_PROJECTS $CCC_PROJECTS $TESTS_PROJECTS"
#ALL_PROJECTS="$CCC_PROJECTS"

# version.h
python2.4 $CCC_ROOT/nbpal/$BRANCH/build/version.py $CCC_ROOT/nbpal/$BRANCH/. iphone
python2.4 $CCC_ROOT/abpal/$BRANCH/build/version.py $CCC_ROOT/abpal/$BRANCH/. iphone
python2.4 $CCC_ROOT/coreservices/$BRANCH/build/version.py $CCC_ROOT/coreservices/$BRANCH/. iphone
python2.4 $CCC_ROOT/nbservices/$BRANCH/build/version.py $CCC_ROOT/nbservices/$BRANCH/. iphone
python2.4 $CCC_ROOT/abservices/$BRANCH/build/version.py $CCC_ROOT/abservices/$BRANCH/. iphone
python2.4 $CCC_ROOT/nbui/$BRANCH/build/version.py $CCC_ROOT/nbui/$BRANCH/. iphone
python2.4 $CCC_ROOT/nbgm/$BRANCH/nbgmmain/build/version.py $CCC_ROOT/nbgm/$BRANCH/nbgmmain/. iphone
python2.4 $CCC_ROOT/common/$BRANCH/build/version.py $CCC_ROOT/common/$BRANCH/. iphone
python2.4 $CCC_ROOT/nav/$BRANCH/build/version.py $CCC_ROOT/nbui/$BRANCH/. iphone
python2.4 $CCC_ROOT/navui/$BRANCH/build/version.py $CCC_ROOT/nbui/$BRANCH/. iphone

echo "DEVICE = $DEVICE"

echo "-= Begin Clean =-"
for PROJ in $ALL_PROJECTS
do
    echo "--= Clean $PROJ =--"
	xcodebuild -project $PROJ clean > /dev/null
done
echo "-= Clean Complete =-"
echo
echo

echo "-= Begin Build =-"
for PROJ in $ALL_PROJECTS
do
    echo "--= Build $PROJ $BUILD_TYPE =--"
    xcodebuild -project $PROJ -configuration $BUILD_TYPE -sdk $DEVICE IPHONEOS_DEPLOYMENT_TARGET=${DEPLOYMENT_TARGET} build >null
    if [ $? -ne 0 ]; then
        echo "Failed to build project: $PROJ..."
        exit 1
    fi
done
echo "-= Build Complete =-"
echo
echo

exit 0

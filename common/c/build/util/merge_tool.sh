#!/bin/bash
# --------------------------------------------------------------------------
#    @file        merge_tool.sh
#    @defgroup    build_utility
#
#    (C) Copyright 2014 by TeleCommunication Systems, Inc.

#    The information contained herein is confidential, proprietary to
#    TeleCommunication Systems, Inc., and considered a trade secret as defined
#    in section 499C of the penal code of the State of California. Use of this
#    information by anyone other than authorized employees of TeleCommunication
#    Systems is granted only under a written non-disclosure agreement, expressly
#    prescribing the scope and manner of such use.
# --------------------------------------------------------------------------


CUR_DIR=$PWD
BUILD_DIR=`realpath ../`
CCC_TOP_DIR=`realpath $BUILD_DIR/../../`
CUR_BRANCH=`basename $BUILD_DIR`

OLD_BRANCH=

# get old branch name from stdin.
while getopts r: var; do
    case $var in
        r)
            OLD_BRANCH="$OPTARG"
            ;;
        *)
            ;;
    esac
done

shift $(($OPTIND - 1))

if [ -z $OLD_BRANCH ]; then
    cat <<EOF
Usage: sh merge_tool.sh -r old_branch
EOF

    exit 1
fi


# replace all old_branch with current branch
cd $BUILD_DIR
echo "Replacing branch names for $PWD..."
find . -type f -exec sed -r -i "s/\<$OLD_BRANCH/$CUR_BRANCH/g" {} \;

cd $CCC_TOP_DIR
echo "Replacing branch names for $PWD..."
find . -name "project.pbxproj" -exec sed -i "s/\<$OLD_BRANCH/$CUR_BRANCH/g" {} \;


# update version your self...
cat <<EOF


You need to increase version number by yourself. Here is the rule:

Build MAJOR.MINOR.SUB.BUILD

*	When feature branches are made (like for LTK) the SUB version should be incremented from the main branch number it branched from.

*	When feature branches are completed and integrated back to main, main’s MINOR should increment.

*	When release branches are made the branch version number should be set to the next main branch BUILD number, and the main branch BUILD number should increment by 50 or so to leave room for multiple builds.

*	The major number is meant to indicate backward compatibility.

Enjoy yourself.

EOF

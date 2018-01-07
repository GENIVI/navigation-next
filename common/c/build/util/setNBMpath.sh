#!/bin/bash
#
# Set the NBM path in the xcode environment variables.
#


if [ $1 == "" ]; then
  echo -e "Invalid home path. ABORTED!!"
  exit
fi

NBM_PATH=$2

echo -e "Home: " $1
echo -e "Local machine home: " $HOME
echo -e "NBM release path: " $NBM_PATH

# iOS 5
xcodePlist="$HOME/Library/Preferences/com.apple.dt.Xcode.plist"
echo -e "Plist file name:" ${xcodePlist} 

killall Xcode
killall cfprefsd

#Update variable in plist
#Add the bictory for build settings
/usr/libexec/PlistBuddy -c "add :IDEApplicationwideBuildSettings dict" "${xcodePlist}"

#Add the NBM_release path, if exists update the path
/usr/libexec/PlistBuddy -c "add :IDEApplicationwideBuildSettings:NBM_RELEASE string ${NBM_PATH}" "${xcodePlist}"
if [ $? -gt 0 ]; then 
    /usr/libexec/PlistBuddy -c "Set :IDEApplicationwideBuildSettings:NBM_RELEASE ${NBM_PATH}" "${xcodePlist}"
fi

#Add the bictory for display
/usr/libexec/PlistBuddy -c "add :IDESourceTreeDisplayNames dict" "${xcodePlist}"

#Add the NBM_release display, if exists update the path
/usr/libexec/PlistBuddy -c "add :IDESourceTreeDisplayNames:NBM_RELEASE string NBM_RELEASE" "${xcodePlist}"
if [ $? -gt 0 ]; then 
    /usr/libexec/PlistBuddy -c "Set :IDESourceTreeDisplayNames:NBM_RELEASE NBM_RELEASE" "${xcodePlist}"
fi



#iOS 4
xcodePlist="$HOME/Library/Preferences/com.apple.Xcode.plist"

echo -e "NBM Release path:" $2

#Update variable in plist
#Add the bictory for build settings
/usr/libexec/PlistBuddy -c "add :PBXApplicationwideBuildSettings dict" "${xcodePlist}"

#Add the NBM_release path, if exists update the path
/usr/libexec/PlistBuddy -c "add :PBXApplicationwideBuildSettings:NBM_RELEASE string ${NBM_PATH}" "${xcodePlist}"
if [ $? -gt 0 ]; then 
    /usr/libexec/PlistBuddy -c "Set :PBXApplicationwideBuildSettings:NBM_RELEASE ${NBM_PATH}" "${xcodePlist}"
fi

#Add the bictory for display
/usr/libexec/PlistBuddy -c "add :PBXSourceTreeDisplayNames dict" "${xcodePlist}"

#Add the NBM_release display, if exists update the path
/usr/libexec/PlistBuddy -c "add :PBXSourceTreeDisplayNames:NBM_RELEASE string NBM_RELEASE" "${xcodePlist}"
if [ $? -gt 0 ]; then 
    /usr/libexec/PlistBuddy -c "Set :PBXSourceTreeDisplayNames:NBM_RELEASE NBM_RELEASE" "${xcodePlist}"
fi

echo -e "Complete"

@ECHO OFF

SET BRANCH=%NIM_CCC_BRANCH%
IF "%NIM_CCC_BRANCH%" == "" SET BRANCH=feature_140523_hybrid_phase2

SET /p ANDROID_NDK_VERSION= < version_android_ndk.txt
SET ANDROID_NDK_PKG_DIR=CoreSDK_%ANDROID_NDK_VERSION%_android_ndk_arm_internal
SET ANDROID_NDK_PKG_LIB_DEBUG_DIR=%ANDROID_NDK_PKG_DIR%\lib\debug
SET ANDROID_NDK_PKG_LIB_RELEASE_DIR=%ANDROID_NDK_PKG_DIR%\lib\release
SET ANDROID_NDK_PKG_INCLUDE_DIR=%ANDROID_NDK_PKG_DIR%\include
SET ANDROID_NDK_PKG_RESOURCE_DIR=%ANDROID_NDK_PKG_DIR%\resource

IF NOT EXIST %ANDROID_NDK_PKG_DIR% mkdir %ANDROID_NDK_PKG_DIR%
IF NOT EXIST %ANDROID_NDK_PKG_LIB_DEBUG_DIR% mkdir %ANDROID_NDK_PKG_LIB_DEBUG_DIR%
IF NOT EXIST %ANDROID_NDK_PKG_LIB_RELEASE_DIR% mkdir %ANDROID_NDK_PKG_LIB_RELEASE_DIR%
IF NOT EXIST %ANDROID_NDK_PKG_INCLUDE_DIR% mkdir %ANDROID_NDK_PKG_INCLUDE_DIR%
IF NOT EXIST %ANDROID_NDK_PKG_INCLUDE_DIR%\android_ndk mkdir %ANDROID_NDK_PKG_INCLUDE_DIR%\android_ndk
IF NOT EXIST %ANDROID_NDK_PKG_RESOURCE_DIR% mkdir %ANDROID_NDK_PKG_RESOURCE_DIR%

xcopy /S /R /Y android\obj\local\armeabi-v7a\*.so %ANDROID_NDK_PKG_LIB_DEBUG_DIR%\
xcopy /S /R /Y android\libs\*.so %ANDROID_NDK_PKG_LIB_RELEASE_DIR%\
copy ..\..\nbpal\%BRANCH%\include\*.h %ANDROID_NDK_PKG_INCLUDE_DIR%\
copy ..\..\nbpal\%BRANCH%\include\android_ndk\*.h %ANDROID_NDK_PKG_INCLUDE_DIR%\android_ndk\
copy ..\..\abpal\%BRANCH%\include\*.h %ANDROID_NDK_PKG_INCLUDE_DIR%\
copy ..\..\abpal\%BRANCH%\include\android_ndk\*.h %ANDROID_NDK_PKG_INCLUDE_DIR%\android_ndk\

set release_pal_dir=%~dp0%ANDROID_NDK_PKG_DIR%\release_pal
mkdir %release_pal_dir%
call package_pal_android_ndk %~dp0%ANDROID_NDK_PKG_DIR% nbpal %release_pal_dir% ..\.. %BRANCH%
call package_pal_android_ndk %~dp0%ANDROID_NDK_PKG_DIR% abpal %release_pal_dir% ..\.. %BRANCH%

ECHO SUCCESS


rem coreSDK
set VERSION_FILE=coresdk_qt_win32_version.txt
for /f "delims=" %%i in (%VERSION_FILE%) do set CORE_SDK_VERSION=%%~nxi
rd /s /q CoreSDK

@xcopy   ..\..\..\..\c\releases\sdk\qt_win32\CoreSDK_%CORE_SDK_VERSION%_qt_win32_internal CoreSDK\ /S /Y /F /K /R
@xcopy win32libs CoreSDK\lib\qt\release /Y

rem version setting
del version.txt
copy version_qt_win32.txt version.txt

rem build
python ./build_tpl.py
python ./util/build.py


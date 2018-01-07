@ECHO OFF
SET VERSION_FILE=nbm_win32_version.txt
for /f "delims=" %%i in (%VERSION_FILE%) do set NBM_VERSION=%%~nxi
rd /s /q nbm
@xcopy  ..\..\..\..\..\core\releases\nbm\qt_win32\nbm_%NBM_VERSION%_win32 nbm\ /S /Y /F /K /R
copy version_qt_win32.txt version_qt.txt

if "%1"=="" (python util\build_qt.py all all win32)
if not "%1"=="" if "%2"=="" (python util\build_qt.py %1 all win32)
if not "%1"=="" if not "%2"=="" (python util\build_qt.py %1 %2 win32)

for /f "delims=" %%i in (version_qt_win32.txt) do set SDK_VERSION=%%~nxi
@xcopy  ..\..\..\..\..\User\jni\qt_ltk\client\c\releases\sdk\qt\lib CoreSDK_%SDK_VERSION%_qt_win32_internal\lib\ /S /Y /F /K /R
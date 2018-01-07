@ECHO OFF

set CWD=%cd%

cd /d %~dp0

REM build debug (1) or release (0)
rem SET NDK_DEBUG=1

rem SET CYGWIN_HOME=D:\Cygwin
rem set PATH=C:\Python24;%PATH%

SET CCC_MODE="offboard"

if "%1"=="" goto regen

if "%1"=="--onboard" goto set_ccc_mode

if "%1"=="--nogen" goto nogen

goto usage

:set_ccc_mode
SET CCC_MODE="onboard"

:regen
SET /p ANDROID_NDK_VERSION= < version_android_ndk.txt
python util\makeversioninfo.py nbpal
python util\makeversioninfo.py abpal
python util\makeversioninfo.py abservices
python util\makeversioninfo.py coreservices
python util\makeversioninfo.py nbservices
python util\makeversioninfo.py nbgm
python util\makeversioninfo.py nbui
python util\makeversioninfo.py common
python util\codegenerator.py %ANDROID_NDK_VERSION% android_ndk_arm
python util\makemodeinfo.py %CCC_MODE%
CALL android\build.bat --regen
goto copy_files

:nogen
CALL android\build.bat
if %ERRORLEVEL% GEQ 1 goto build_failed
goto copy_files

:usage
echo "Usage: build_android_ndk.bat --nogen # do not generate Makefile for android"
echo "Usage: build_android_ndk.bat --onboard # generate Makefile for android with onboard mode"
goto end

:copy_files
SET NDK_LIBS=android\libs
MKDIR dist\android_ndk
xcopy /r /s /y %NDK_LIBS%\*.* dist\android_ndk

echo BUILD SUCCESSFUL
goto end

:build_failed
echo BUILD FAILED
cd %CWD%
EXIT /B 1

:end
cd %CWD%

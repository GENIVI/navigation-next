set CWD=%cd%

cd %~dp0

set NIM_CCC_BRANCH=feature_140523_hybrid_phase2
set NIM_APP_COMMON_BRANCH=feature_140523_hybrid_phase2
set NIM_CCC_CPU_ARCH=armeabi-v7a
set PROJECT_PATH=%cd%

@echo off

if "%1"=="" goto make

if "%1"=="--regen" goto regen

goto usage

:regen
echo "regen"
python ndkdev.py --stlport

:make
echo "make"
call %CYGWIN_HOME%\bin\bash -login -c "cd $PROJECT_PATH && ./make.sh"
if %ERRORLEVEL% GEQ 1 goto error
goto done

:usage
echo "Usage: build.bat --regen # regenerate Makefile for android"

:error
cd %CWB
exit /B 1

:done
cd %CWD%

@ECHO OFF
SET TARGET_PLATFORM=%1
SET PROFILE=%2
SET PARABUILD=%3
SET CCC_MODE=%4
python util\configure.py %TARGET_PLATFORM% %PROFILE% %PARABUILD% %CCC_MODE%

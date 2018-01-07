@ECHO OFF

@set INTERFACE=%1
@set PLATFORM=%2

@set INC_DIR=%NIMCORE_INC%

@set TOOLS_PATH=%CORETECH_ROOT%\build\main\doxygen

python %TOOLS_PATH%\gen_doc.py %TOOLS_PATH% %INC_DIR% "Core Services " %INTERFACE% %PLATFORM%

@set INTERFACE=
@set PLATFORM=
@set INC_DIR=

@ECHO OFF
SET TARGET_DIR=%1

ECHO %TARGET_DIR%

if "%TARGET_DIR%" == "" SET TARGET_DIR=%CD%\

CD %TARGET_DIR%

ECHO ----------------...--------...----------------
ECHO CURRENT REG KEY VALUES 
ECHO ----------------...--------...----------------
REG QUERY "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v NIMCORE_INC
REG QUERY "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v NIMCORE_LIB
REG QUERY "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v NIMCORE_BIN


ECHO ----------------...--------...----------------
ECHO UPDATING REG KEY VALUES
ECHO ----------------...--------...----------------

:CHECK_INC
IF NOT "%NIMCORE_INC%" == "%TARGET_DIR%headers\core" GOTO INC

:CHECK_LIB
IF NOT "%NIMCORE_LIB%" == "%TARGET_DIR%lib" GOTO LIB

:CHECK_BIN
IF NOT "%NIMCORE_BIN%" == "%TARGET_DIR%bin" GOTO BIN

GOTO END

:INC 
SET NIMCORE_INC=%TARGET_DIR%headers\core
REG ADD "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v NIMCORE_INC /t REG_SZ /d %NIMCORE_INC%"
GOTO CHECK_LIB

:LIB
SET NIMCORE_LIB=%TARGET_DIR%lib
REG ADD "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v NIMCORE_LIB /t REG_SZ /d %NIMCORE_LIB%"
GOTO CHECK_BIN

:BIN
SET NIMCORE_BIN=%TARGET_DIR%bin
REG ADD "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v NIMCORE_BIN /t REG_SZ /d %NIMCORE_BIN%"
GOTO END

:END
CALL %TARGET_DIR%setenv.exe
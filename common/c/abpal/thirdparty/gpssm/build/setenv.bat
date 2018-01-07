@ECHO OFF
SET PROJECT=%1
SET TARGET_INC=%2
SET TARGET_LIB=%3
SET TARGET_BIN=%4

ECHO ----------------...--------...----------------
ECHO UPDATING REG KEY VALUES
ECHO ----------------...--------...----------------

SET %PROJECT%_INC=%TARGET_INC%
REG ADD "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v %PROJECT%_INC /t REG_SZ /d %TARGET_INC% /f

SET %PROJECT%_LIB=%TARGET_LIB%
REG ADD "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v %PROJECT%_LIB /t REG_SZ /d %TARGET_LIB% /f

SET %PROJECT%_BIN=%TARGET_BIN%
REG ADD "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v %PROJECT%_BIN /t REG_SZ /d %TARGET_BIN% /f

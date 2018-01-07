SET filename=bld.inf
SET file=%filename%

ECHO //>%file%
ECHO //>>%file%
ECHO.>>%file%
ECHO PRJ_PLATFORMS>>%file%
ECHO DEFAULT>>%file%
ECHO.>>%file%
ECHO PRJ_MMPFILES>>%file%
ECHO ArgTable.mmp>>%file%
ECHO CUnit.mmp>>%file%
ECHO.>>%file%
ECHO PRJ_EXPORTS>>%file%

PUSHD ..\..\ArgTable\include
SET file=..\..\projects\symbian\%filename%
DIR /B /ON *.h > FileList.txt
FOR /F %%f IN (FileList.txt) DO ECHO ..\..\ArgTable\include\%%f			\Epoc32\include\NIM\CoreTech\ArgTable\%%f>>%file%
DEL FileList.txt
POPD

PUSHD ..\..\CUnit\include
SET file=..\..\projects\symbian\%filename%
DIR /B /ON *.h > FileList.txt
ECHO.>>%file%
FOR /F %%f IN (FileList.txt) DO ECHO ..\..\CUnit\include\%%f			\Epoc32\include\NIM\CoreTech\CUnit\%%f>>%file%
DEL FileList.txt
POPD
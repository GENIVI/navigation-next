@ECHO OFF
SET INTERFACE=%1
SET PLATFORM=%2
SET COPY_DIR=%CORETECH_ROOT%\dox
for %%* in (.) do SET BRANCH=%%~n*

IF "%INTERFACE%" == "" GOTO ERROR
IF "%PLATFORM%" == "" GOTO ERROR

IF NOT EXIST %COPY_DIR% mkdir %COPY_DIR%

@call cd ..\..\nbpal\%BRANCH%\build
@call gen_doc.bat %INTERFACE% %PLATFORM%
IF NOT EXIST %COPY_DIR%\nbpal mkdir %COPY_DIR%\nbpal
XCOPY ..\output\doxygen\* %COPY_DIR%\nbpal /s /i /R /Y
rem @call cp -R "..\output\doxygen" %COPY_DIR%\nbpal

SET INTERFACE=%1
SET PLATFORM=%2
SET COPY_DIR=%CORETECH_ROOT%\dox
@call cd ..\..\..\abpal\%BRANCH%\build
@call gen_doc.bat %INTERFACE% %PLATFORM%
IF NOT EXIST %COPY_DIR%\abpal mkdir %COPY_DIR%\abpal
XCOPY ..\output\doxygen\* %COPY_DIR%\abpal /s /i /R /Y
rem @call cp -R ..\output\doxygen %COPY_DIR%\abpal

SET INTERFACE=%1
SET PLATFORM=%2
SET COPY_DIR=%CORETECH_ROOT%\dox
@call cd ..\..\..\coreservices\%BRANCH%\build
@call gen_doc.bat %INTERFACE% %PLATFORM%
IF NOT EXIST %COPY_DIR%\core mkdir %COPY_DIR%\core
XCOPY ..\output\doxygen\* %COPY_DIR%\core /s /i /R /Y
rem @call cp -R ..\output\doxygen %COPY_DIR%\core

SET INTERFACE=%1
SET PLATFORM=%2
SET COPY_DIR=%CORETECH_ROOT%\dox
@call cd ..\..\..\nbservices\%BRANCH%\build
@call gen_doc.bat %INTERFACE% %PLATFORM%
IF NOT EXIST %COPY_DIR%\nb mkdir %COPY_DIR%\nb
XCOPY ..\output\doxygen\* %COPY_DIR%\nb /s /i /R /Y
rem @call cp -R ..\output\doxygen %COPY_DIR%\nb

SET INTERFACE=%1
SET PLATFORM=%2
SET COPY_DIR=%CORETECH_ROOT%\dox
@call cd ..\..\..\abservices\%BRANCH%\build
@call gen_doc.bat %INTERFACE% %PLATFORM%
IF NOT EXIST %COPY_DIR%\ab mkdir %COPY_DIR%\ab
XCOPY ..\output\doxygen\* %COPY_DIR%\ab /s /i /R /Y
rem @call cp -R ..\output\doxygen %COPY_DIR%\ab

@call cd ..\..\..\build\%BRANCH%

GOTO END

:ERROR
@echo Missing parameter - Usage: generate_dox.bat interface platform

:END


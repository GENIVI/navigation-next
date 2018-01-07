cd..
cd..
SET PWD=%CD%
set CORETECH_ROOT_SAVE=%CORETECH_ROOT%
set CORETECH_ROOT=%CD%
cd "%CORETECH_ROOT%"

set NIMABPAL_INC_SAVE=%NIMABPAL_INC%
set NIMAB_INC_SAVE=%NIMAB_INC%
set NIMCORE_INC_SAVE=%NIMCORE_INC%
set NIMNBIPAL_INC_SAVE=%NIMNBIPAL_INC%
set NIMNB_INC_SAVE=%NIMNB_INC%

set NIMABPAL_INC=%CORETECH_ROOT%\abpal\feature_140523_hybrid_phase2\include
set NIMAB_INC=%CORETECH_ROOT%\abservices\feature_140523_hybrid_phase2\include
set NIMCORE_INC=%CORETECH_ROOT%\coreservices\feature_140523_hybrid_phase2\include
set NIMNBIPAL_INC=%CORETECH_ROOT%\nbpal\feature_140523_hybrid_phase2\include
set NIMNB_INC=%CORETECH_ROOT%\nbservices\feature_140523_hybrid_phase2\include
set NIMNBGM_INC=%CORETECH_ROOT%\nbgm\feature_140523_hybrid_phase2\nbgmmain\include
set NIMNBGMMAIN_INC=%CORETECH_ROOT%\nbgm\feature_140523_hybrid_phase2\nbgmmain\include

rd /s /q dox

@echo ---- GENERATE DOX ----
cd "%CORETECH_ROOT%\build\feature_140523_hybrid_phase2"
call generate_dox.bat Public win32
cd "%CORETECH_ROOT%\build\feature_140523_hybrid_phase2"
call generate_dox.bat Protected win32
cd "%CORETECH_ROOT%\build\feature_140523_hybrid_phase2"
call generate_dox.bat Private win32

@echo ---- GENERATE FOLDER FOR FTP ----
cd "%CORETECH_ROOT%"
rd /s /q ccc

SET FTP_DIR=%CORETECH_ROOT%\ccc
IF NOT EXIST %FTP_DIR% mkdir %FTP_DIR%
IF NOT EXIST %FTP_DIR%\private mkdir %FTP_DIR%\private
IF NOT EXIST %FTP_DIR%\public mkdir %FTP_DIR%\public
IF NOT EXIST %FTP_DIR%\protected mkdir %FTP_DIR%\protected

rem Copy abpal
XCOPY %CORETECH_ROOT%\dox\abpal\Private\* %FTP_DIR%\private\abpal\* /s /i /R /Y
XCOPY %CORETECH_ROOT%\dox\abpal\Public\* %FTP_DIR%\public\abpal /s /i /R /Y
XCOPY %CORETECH_ROOT%\dox\abpal\Protected\* %FTP_DIR%\protected\abpal /s /i /R /Y

rem Copy ab services
XCOPY %CORETECH_ROOT%\dox\ab\Private\* %FTP_DIR%\private\ab /s /i /R /Y
XCOPY %CORETECH_ROOT%\dox\ab\Public\* %FTP_DIR%\public\ab /s /i /R /Y
XCOPY %CORETECH_ROOT%\dox\ab\Protected\* %FTP_DIR%\protected\ab /s /i /R /Y

rem Copy core services 
XCOPY %CORETECH_ROOT%\dox\core\Private\* %FTP_DIR%\private\core /s /i /R /Y
XCOPY %CORETECH_ROOT%\dox\core\Public\* %FTP_DIR%\public\core /s /i /R /Y
XCOPY %CORETECH_ROOT%\dox\core\Protected\* %FTP_DIR%\protected\core /s /i /R /Y

rem Copy nbpal
XCOPY %CORETECH_ROOT%\dox\nbpal\Private\* %FTP_DIR%\private\nbpal /s /i /R /Y
XCOPY %CORETECH_ROOT%\dox\nbpal\Public\* %FTP_DIR%\public\nbpal /s /i /R /Y
XCOPY %CORETECH_ROOT%\dox\nbpal\Protected\* %FTP_DIR%\protected\nbpal /s /i /R /Y

rem Copy nb servies
XCOPY %CORETECH_ROOT%\dox\nb\Private\* %FTP_DIR%\private\nb /s /i /R /Y
XCOPY %CORETECH_ROOT%\dox\nb\Public\* %FTP_DIR%\public\nb /s /i /R /Y
XCOPY %CORETECH_ROOT%\dox\nb\Protected\* %FTP_DIR%\protected\nb /s /i /R /Y

cd %CORETECH_ROOT%\ccc
del /S *.rtf
cd "%CORETECH_ROOT%"

echo Build succeeded
echo SUCCESS

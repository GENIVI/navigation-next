@echo off

rem ----------------------------------------------------
rem -- gendoxygen.cmd
rem ----------------------------------------------------

rem -- check command line params

if '%1'=='' goto usage
if '%2'=='' goto usage

set TITLE=%1
set SOURCE_PATH=%2
for /f "useback tokens=*" %%a in ('%SOURCE_PATH%') do set SOURCE_PATH=%%~a
set TEMP_PATH=%3
set DEST_PATH=%4
if not '%4'=='' goto setpaths0
set DEST_PATH=.\doc

:setpaths0
if not '%DOXYGEN_PATH%'=='' goto setpaths1
set DOXYGEN_PATH="C:\Program Files\doxygen\bin"
:setpaths1
if not '%DOT_PATH%'=='' goto setpaths2
set DOT_PATH="C:\Program Files (x86)\Graphviz2.34\bin"
:setpaths2
if not '%TOOLS_PATH%'=='' goto setpaths3
set TOOLS_PATH=%CORETECH_ROOT%\build\release_150406_ltk_v1_5_1\doxygen
:setpaths3

echo Generating documentation...
echo     TITLE:       %TITLE%
echo     SOURCE_PATH: %SOURCE_PATH%
echo     TEMP_PATH:   %TEMP_PATH%
echo     DEST_PATH:   %DEST_PATH%
echo     DOXYGEN_PATH %DOXYGEN_PATH% 
echo     DOT_PATH     %DOT_PATH%
echo     TOOLS_PATH   %TOOLS_PATH%

rem -- clear destination dir & working files
del doxyfile                                    1>nul 2>nul
rd /s /q %TEMP_PATH%\html                       1>nul 2>nul


rem -- create doxygen project file with overrides
copy %TOOLS_PATH%\doxyfile-template doxyfile    1>nul 2>nul


echo #---------------------------------------------------------------------------   >> doxyfile
echo # OVERRIDES added by %0                                                        >> doxyfile
echo #---------------------------------------------------------------------------   >> doxyfile

echo PROJECT_NAME=%TITLE%           >> doxyfile
echo INPUT=%SOURCE_PATH%            >> doxyfile
echo OUTPUT_DIRECTORY=%TEMP_PATH%   >> doxyfile

if not '%5'=='' goto internaldocs

:externaldocs
echo RECURSIVE=NO               >> doxyfile
echo EXTRACT_PRIVATE=NO         >> doxyfile
echo EXTRACT_STATIC=NO 	        >> doxyfile
echo EXTRACT_LOCAL_CLASSES=NO   >> doxyfile
echo SHOW_USED_FILES=NO         >> doxyfile
echo SHOW_FILES=NO              >> doxyfile
echo SOURCE_BROWSER=NO          >> doxyfile
echo INLINE_SOURCES=NO          >> doxyfile
echo REFERENCES_LINK_SOURCE=NO  >> doxyfile
echo VERBATIM_HEADERS=NO        >> doxyfile
echo HAVE_DOT=NO                >> doxyfile
echo DOT_PATH=                  >> doxyfile
goto rundoxygen

:internaldocs
echo RECURSIVE=YES              >> doxyfile
echo EXTRACT_PRIVATE=YES        >> doxyfile
echo EXTRACT_STATIC=YES	        >> doxyfile
echo EXTRACT_LOCAL_CLASSES=YES  >> doxyfile
echo SHOW_USED_FILES=YES        >> doxyfile
echo SHOW_FILES=YES             >> doxyfile
echo SOURCE_BROWSER=NO          >> doxyfile
echo INLINE_SOURCES=NO          >> doxyfile
echo REFERENCES_LINK_SOURCE=YES >> doxyfile
echo VERBATIM_HEADERS=YES       >> doxyfile
echo HAVE_DOT=YES               >> doxyfile
echo DOT_PATH=%DOT_PATH%        >> doxyfile

:rundoxygen
%DOXYGEN_PATH%\doxygen doxyfile

rem -- publish generated documents
rd /s /q %DEST_PATH%                                        1>nul 2>nul
xcopy /s %TEMP_PATH%\html\*.* %DEST_PATH%\                  1>nul 2>nul
rd /s /q %TEMP_PATH%\html                                   1>nul 2>nul
copy %TEMP_PATH%\rtf\refman.rtf %DEST_PATH%\%TITLE%.rtf     1>nul 2>nul
rd /s /q %TEMP_PATH%\rtf                                    1>nul 2>nul
del doxyfile                                                1>nul 2>nul

goto done

:usage
echo Usage: %0 {Title} {Source} [{Dest}] [ALL]
echo    Required:   {Title}     desired project title
echo                {Source}    source path of source to document
echo    Optional:   {Dest}      destination path for generated document
echo                ALL         ALL generates documentation on everything (for internal use only)

:done
set TITLE=
set SOURCE_PATH=
set TEMP_PATH=
set DEST_PATH=
set TOOLS_PATH=
set DOXYGEN_PATH=
set DOT_PATH=

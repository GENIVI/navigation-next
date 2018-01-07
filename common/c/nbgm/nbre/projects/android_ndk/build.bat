%cd%\..\..\build\version.py %cd%/../.. android_ndk 
call copy build_nbgm.sh %CYGWIN_HOME%\tmp\build_nbgm.sh
call %CYGWIN_HOME%\bin\bash  --login -c "cd /tmp && ./build_nbgm.sh"

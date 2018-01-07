%cd%\..\../build/version.py %cd%\..\.. android_ndk 
call copy %cd%\..\..\..\..\build\%BRANCH%\android\Application.mk jni\
call python.exe nbpal.py
call copy build_nbpal.sh %CYGWIN_HOME%\tmp\build_nbpal.sh
call %CYGWIN_HOME%\bin\bash  --login -c "cd /tmp && ./build_nbpal.sh"

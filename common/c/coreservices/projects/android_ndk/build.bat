%cd%\..\../build/version.py %cd%\..\.. android_ndk
call copy %cd%\..\..\..\..\build\%BRANCH%\android\Application.mk jni\
call copy build_core.sh %CYGWIN_HOME%\tmp\build_core.sh
call %CYGWIN_HOME%\bin\bash  --login -c "cd /tmp && ./build_core.sh"

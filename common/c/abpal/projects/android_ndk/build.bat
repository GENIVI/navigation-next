%cd%\..\../build/version.py %cd%\..\.. android_ndk
call copy %cd%\..\..\..\..\build\%BRANCH%\android\Application.mk jni\
call copy build_abpal.sh %CYGWIN_HOME%\tmp\build_abpal.sh
call %CYGWIN_HOME%\bin\bash  --login -c "cd /tmp && ./build_abpal.sh"

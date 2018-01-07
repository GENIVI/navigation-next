@echo off

set ccc_src_dir=%1
set component_name=%2
set output_dir=%3
set ccc_root=%4
set ccc_branch=%5

set ccc_des_dir=%output_dir%\%component_name%
set ccc_des_dir_include=%ccc_des_dir%\include
set ccc_des_dir_include_platform=%ccc_des_dir_include%\android_ndk
set ccc_des_dir_lib=%ccc_des_dir%\lib
set ccc_des_dir_lib_debug=%ccc_des_dir_lib%\debug
set ccc_des_dir_lib_release=%ccc_des_dir_lib%\release\armeabi-v7a
set ccc_des_dir_unitests=%ccc_des_dir%\unitests
set ccc_des_dir_unitests_android=%ccc_des_dir_unitests%\android_ndk

mkdir %ccc_des_dir%
mkdir %ccc_des_dir_include%
mkdir %ccc_des_dir_include_platform%
mkdir %ccc_des_dir_lib%
mkdir %ccc_des_dir_lib_debug%
mkdir %ccc_des_dir_lib%\release
mkdir %ccc_des_dir_lib_release%
mkdir %ccc_des_dir_unitests%
mkdir %ccc_des_dir_unitests_android%

copy %ccc_root%\%component_name%\%ccc_branch%\include\*.h %ccc_des_dir_include%\
copy %ccc_root%\%component_name%\%ccc_branch%\include\android_ndk\*.h %ccc_des_dir_include_platform%
copy %ccc_src_dir%\lib\debug\lib%component_name%.so %ccc_des_dir_lib_debug%
copy %ccc_src_dir%\lib\release\armeabi-v7a\lib%component_name%.so %ccc_des_dir_lib_release%
copy %ccc_root%\%component_name%\%ccc_branch%\src\unittests\* %ccc_des_dir_unitests%
xcopy /E %ccc_root%\%component_name%\%ccc_branch%\src\unittests\android_ndk %ccc_des_dir_unitests_android%
echo Done!

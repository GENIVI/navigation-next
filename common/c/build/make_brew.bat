@ECHO OFF
SET BREWVERSION=%1
SET CLEAN=%2

IF "%CLEAN%" == "clean" GOTO CLEAN

cd ..
cd ..
cd thirdparty/release_150406_ltk_v1_5_1/projects/make/sqlite
make PLATFORM=brew MK_BREW_VERSION=%BREWVERSION%
cd ..
cd ..
cd ..
cd ..
cd ..

cd nbpal/release_150406_ltk_v1_5_1/projects/make
make PLATFORM=brew MK_BREW_VERSION=%BREWVERSION%
cd ..
cd ..
cd ..
cd ..

cd abpal/release_150406_ltk_v1_5_1/projects/make
make PLATFORM=brew MK_BREW_VERSION=%BREWVERSION%
cd ..
cd ..
cd ..
cd ..

cd coreservices/release_150406_ltk_v1_5_1/projects/make
make PLATFORM=brew MK_BREW_VERSION=%BREWVERSION%
cd ..
cd ..
cd ..
cd ..

cd nbservices/release_150406_ltk_v1_5_1/projects/make
make PLATFORM=brew MK_BREW_VERSION=%BREWVERSION%
cd ..
cd ..
cd ..
cd ..

cd abservices/release_150406_ltk_v1_5_1/projects/make
make PLATFORM=brew MK_BREW_VERSION=%BREWVERSION%
cd ..
cd ..
cd ..
cd ..

cd build

GOTO END

:CLEAN
cd ..
cd ..
cd nbipal/release_150406_ltk_v1_5_1/projects/make
make clean PLATFORM=brew
cd ..
cd ..
cd ..
cd ..

cd abpal/release_150406_ltk_v1_5_1/projects/make
make clean PLATFORM=brew 
cd ..
cd ..
cd ..
cd ..

cd coreservices/release_150406_ltk_v1_5_1/projects/make
make clean PLATFORM=brew 
cd ..
cd ..
cd ..
cd ..

cd nbservices/release_150406_ltk_v1_5_1/projects/make
make clean PLATFORM=brew 
cd ..
cd ..
cd ..
cd ..

cd abservices/release_150406_ltk_v1_5_1/projects/make
make clean PLATFORM=brew 
cd ..
cd ..
cd ..
cd ..

cd build

:END

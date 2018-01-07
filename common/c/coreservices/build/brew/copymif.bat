SET SOL_DIR=%1
SET CONFIG=%2

IF EXIST %SOL_DIR%/output/brew/%CONFIG%/unittests.mif GOTO END
cp %SOL_DIR%/resources/brew/unittests.mif %SOL_DIR%/output/brew/%CONFIG%/unittests.mif
:END
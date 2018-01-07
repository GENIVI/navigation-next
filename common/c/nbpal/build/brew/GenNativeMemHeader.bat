@echo off
SET BASE_DIR=%1
SET FILE=%BASE_DIR%include\brew\palnativemem.h

IF NOT EXIST %FILE% (
echo #ifndef _PAL_NATIVE_MEM_H_ >> %FILE%
echo #define _PAL_NATIVE_MEM_H_ >> %FILE%
echo #endif >> %FILE%
)
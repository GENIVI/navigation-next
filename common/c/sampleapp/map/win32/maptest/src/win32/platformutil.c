/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!--------------------------------------------------------------------------

    @file     PlatformUtil.c
    @date     02/06/2009
    @defgroup PLATFORM_UTIL Platform-specific Utility Functions

    Implementation of platform-specific utilities used for system test.

    This file contains the Windows platform implementation of the platform-
    specific utilities needed for system testing.
    */
    /*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "platformutil.h"
#include "palstdlib.h"
#include "palconfig.h"
#include "palfile.h"
#include "tchar.h"
#include "cunit.h"
#include "windows.h"
#include "crtdbg.h"
#include "dbghelp.h"
#include <stdarg.h>
#include "protected\\csdict.h"

static uint8 g_reportOutputDetected = 0;

#define TEST_BASE_PATH      "NB_UNITTEST_DATA\\"
#define CSL_DICT_MIN_SIZE   16

const wchar_t DELIMITER = L'\\';
const wchar_t DELIMITER_STRING[] = L"\\";

typedef struct {
    PAL_File*              file;
    LARGE_INTEGER          startTime;
    struct CSL_Dictionary* extraTimers;
} PerformanceMeasurement;

Test_Options g_TestOptions;

static PerformanceMeasurement  s_pm = {0};
static uint32                  s_threadAffinityMaskCounter = 0;
static DWORD_PTR               s_oldThreadAffinityMask = 0;

static PAL_Error PmPrintArgList(PAL_File* file, const char* format, va_list arg_list);
static void replaceChars(char* str, const char* chars, char replaceWith);
static void ApplyThreadAffinityMask();
static void ResetThreadAffinityMask();

static void ApplyThreadAffinityMask()
{
    if (s_threadAffinityMaskCounter == 0)
    {
        s_oldThreadAffinityMask = SetThreadAffinityMask(GetCurrentThread(), 1);
    }

    s_threadAffinityMaskCounter++;
}

static void ResetThreadAffinityMask()
{
    if (s_threadAffinityMaskCounter == 0)
    {
        return;
    }

    if (s_threadAffinityMaskCounter == 1)
    {
        SetThreadAffinityMask(GetCurrentThread(), s_oldThreadAffinityMask);
        s_oldThreadAffinityMask = 0;
    }

    s_threadAffinityMaskCounter--;
}

void*
CreateCallbackCompletedEvent(void)
{
    return CreateEvent(NULL, FALSE, FALSE, NULL);
}

void
ProcessPendingMessages(void)
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

boolean
WaitForCallbackCompletedEvent(void* callbackCompletedEvent, uint32 timeoutMsec)
{
    const uint32 waitTimeSlice = 250;
    DWORD startTime = GetTickCount();

    boolean success = TRUE;
    
    while (success)
    {
        // wait for signalled callback event, timeout or message to process
        DWORD waitResult = MsgWaitForMultipleObjects(1, &callbackCompletedEvent, FALSE, waitTimeSlice, QS_ALLINPUT);
        
        ProcessPendingMessages();

        if (waitResult == WAIT_OBJECT_0)
        {
            // exit successfully if callback event signalled
            break;
        }
        else if (waitResult == WAIT_FAILED)
        {
            // exit unsuccesfully if wait failed
            success = FALSE;
        }
        else
        {
            // exit unsuccesfully if timeout time elapsed
            success = (boolean)((GetTickCount() - startTime) < timeoutMsec);
        }
    }
    
    return success;
}

void
SetCallbackCompletedEvent(void* callbackCompletedEvent)
{
    SetEvent(callbackCompletedEvent);
}

void
ResetCallbackCompletedEvent(void* callbackCompletedEvent)
{
    ResetEvent(callbackCompletedEvent);
}

void
DestroyCallbackCompletedEvent(void* callbackCompletedEvent)
{
    CloseHandle(callbackCompletedEvent);
}

PAL_Instance* PAL_CreateInstance()
{
    PAL_Config palConfig = { 0 };

    return PAL_Create(&palConfig);
}

PAL_Instance* PAL_CreateInstanceMultiThread()
{
    PAL_Config palConfig = { 1 };

    return PAL_Create(&palConfig);
}

void PAL_DestroyInstance(
    PAL_Instance* pal)
{
    if (!pal)
    {
        return;
    }

    PAL_Destroy(pal);   
}

Test_Options* Test_OptionsCreate()
{
    nsl_memset(&g_TestOptions, 0, sizeof(g_TestOptions));

    g_TestOptions.logLevel = LogLevelNone;
    g_TestOptions.networkType = TestNetworkTcp;
    g_TestOptions.testLevel = TestLevelIntermediate;
    g_TestOptions.overwriteFlag = TestOverwriteFlagFalse;
    g_TestOptions.startFromParticularTest = FALSE;

    return &g_TestOptions;
}

Test_Options* Test_OptionsGet()
{
    return &g_TestOptions;
}

void Test_OptionsDestroy()
{
}

void OutputMessage(const char* format, ...)
{
    va_list args;
    int len = 0;
    char* buffer;

    va_start(args, format);
    len = _vscprintf(format, args) + 1;
    buffer = malloc(len * sizeof(char));

    vsprintf(buffer, format, args);

    OutputDebugStringA(buffer);
    printf(buffer);

    free(buffer);
}

int ReportHook(int type, char* message, int* result)
{
    g_reportOutputDetected = 1;

    // Show message in debugger output
    OutputDebugStringA(message);

    // Do not attach debugger
    *result = 0;

    // Continue processing hooks
    return 0;
}

void EnableDebugHeap()
{
    int heapFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    Test_Options* testOptions =  Test_OptionsGet();

    heapFlags |= _CRTDBG_LEAK_CHECK_DF;         // Dump leaks on exit

    if (testOptions->overwriteFlag == TestOverwriteFlagTrue)    
    {
        heapFlags |= _CRTDBG_DELAY_FREE_MEM_DF;     // Reserve freed memory blocks
    }

    heapFlags &= ~_CRTDBG_CHECK_ALWAYS_DF;      // Don't consistency check heap every operation
    heapFlags &= ~_CRTDBG_CHECK_CRT_DF;         // Ignore internal C stdlib leaks

    _CrtSetDbgFlag(heapFlags);
}

void RedirectCrtReportOutput(int type)
{
    _CrtSetReportMode(type, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(type, _CRTDBG_FILE_STDERR);
}

const wchar_t* GetDumpFileName()
{
    static wchar_t filename[FILENAME_MAX] = { 0 };

    DWORD filenameLength = GetModuleFileName(NULL, filename, FILENAME_MAX);
    if (filenameLength > 0)
    {
        wchar_t* delimiter = wcsrchr(filename, '\\');
        if (delimiter != 0)
        {
            delimiter++;
            *delimiter = '\0';
        }
        
    }

    wcscat(filename, L"minidump.dmp");

    return filename;
}


LONG WINAPI SystemTestExceptionFilter(EXCEPTION_POINTERS* exceptions)
{
    HMODULE debugHelp = NULL;
    const wchar_t* minidumpFilename = GetDumpFileName();

    fwprintf(stderr, L"Exception filter called, attempting to write minidump file '%s' ...\n", minidumpFilename);

    debugHelp = LoadLibrary(L"dbghelp.dll");
    if (debugHelp != NULL)
    {
        FARPROC writeDump = GetProcAddress(debugHelp, "MiniDumpWriteDump");
        if (writeDump != NULL)
        {
            HANDLE file = CreateFile(minidumpFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if ((file != INVALID_HANDLE_VALUE ))
            {
                MINIDUMP_TYPE dumpType = MiniDumpWithFullMemory;
                MINIDUMP_EXCEPTION_INFORMATION mdei;
                BOOL rc;

                mdei.ThreadId           = GetCurrentThreadId(); 
                mdei.ExceptionPointers  = exceptions; 
                mdei.ClientPointers     = FALSE; 

                rc = writeDump(GetCurrentProcess(), GetCurrentProcessId(), file, dumpType, (exceptions != 0) ? &mdei : 0, 0, 0); 
                if (!rc)
                {
                    fprintf(stderr, "MiniDumpWriteDump failed. Error: 0x%08x\n", GetLastError());
                }
                else
                {
                    fprintf(stderr, "minidump created\n");
                }

                CloseHandle(file);
            }
            else 
            {
                fprintf(stderr, "CreateFile failed. Error: 0x%08x\n", GetLastError());
            }
        }
        else
        {
            fprintf(stderr, "unable to get address of MiniDumpWriteDump. Error: 0x%08x\n", GetLastError());
        }
        FreeLibrary(debugHelp);
    }
    else
    {
        fprintf(stderr, "unable to load dbghelp.dll. Error: 0x%08x\n", GetLastError());
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

void InitializeHeapCheck()
{
    SetUnhandledExceptionFilter(SystemTestExceptionFilter);

    EnableDebugHeap();

    RedirectCrtReportOutput(_CRT_ASSERT);
    RedirectCrtReportOutput(_CRT_ERROR);
    RedirectCrtReportOutput(_CRT_WARN);

    _CrtSetReportHook(ReportHook);
}


void TestHeapCheck(void)
{
    int rc = _CrtCheckMemory();
    if (!rc)
    {
        CU_FAIL("Heap corruption detected");
    }

    if (g_reportOutputDetected)
    {
        g_reportOutputDetected = 0;
        CU_FAIL("CRT report output detected");
    }
}




uint64 GetMobileDirectoryNumber(PAL_Instance* pal)
{
    return GetMobileDirectoryNumberFromFile(pal);
}

uint64 GetMobileInformationNumber(PAL_Instance* pal)
{
    return DEFAULT_MOBILE_IDENTIFIER_NUMBER;
}

nb_boolean RunningInEmulator()
{
    return FALSE;
}

nb_boolean GetDeviceName(PAL_Instance* pal, char* buffer, nb_size bufferSize)
{
    nsl_strncpy(buffer, "win32", bufferSize);
    return TRUE;
}

const char* GetBasePath()
{
    wchar_t moduleFilename[MAX_PATH] = {0};

    // get the executable path from the system and convert it to multibyte
    DWORD filenameLength = GetModuleFileName(NULL, moduleFilename, (sizeof(moduleFilename) / sizeof(moduleFilename[0])));
    if (filenameLength > 0)
    {
        // Remove the executable name from the path
        wchar_t* delimiter = wcsrchr(moduleFilename, DELIMITER);
        if (delimiter != 0)
        {
            *++delimiter = '\0';
        }
    }
    SetCurrentDirectoryW(moduleFilename);

    return TEST_BASE_PATH;
}

PAL_Error PlatformLoadFile(PAL_Instance* pal, const char* filename, unsigned char** data, uint32* dataSize)
{
    if (data == NULL || dataSize == NULL)
    {
        return PAL_ErrBadParam;
    }

    return PAL_FileLoadFile(pal, filename, data, dataSize);

}

static PAL_Error PmPrintArgList(PAL_File* file, const char* format, va_list arg_list)
{
    PAL_Error err = PAL_Ok;
    char buf[1024] = {0};
    int len = 0;
    uint32 bytesWritten = 0;

    len = nsl_vsnprintf(buf, sizeof(buf), format, arg_list);

    if (len > 0)
    {
        err = PAL_FileWrite(file, (uint8*)buf, len, &bytesWritten);
    }
    else if (len < 0)
    {
        return PAL_Failed;
    }

    return err;
}

static void replaceChars(char* str, const char* chars, char replaceWith)
{
    char* currentPosition = str;

    while(NULL != (currentPosition = strpbrk(currentPosition, chars)))
    {
        *currentPosition = replaceWith;
         currentPosition++;
    }
}

void PerformanceMeasurement_Begin()
{
    PAL_Error err = PAL_Ok;
    char fileName[256] = {0};
    PAL_Instance* pal = NULL;
    char resultsFolder[] = "PerformanceMeasurementResults";

    if (s_pm.file)
    {
        PerformanceMeasurement_End();
    }

    nsl_memset(&s_pm, 0, sizeof(PerformanceMeasurement));

    pal = PAL_CreateInstance();
    if (!pal)
    {
        goto errexit;
    }

    nsl_snprintf(fileName, sizeof(fileName), "%s\\%s.csv", resultsFolder, CU_get_current_test()->pName);
    replaceChars(fileName + sizeof(resultsFolder), "\\/:*?\"<>|", '_');

    PAL_FileCreateDirectory(pal, resultsFolder);

    err = PAL_FileOpen(pal, fileName, PFM_Append, &s_pm.file);
    if (err)
    {
        err = PAL_FileOpen(pal, fileName, PFM_Create, &s_pm.file);
        if (err)
        {
            PAL_Destroy(pal);
            goto errexit;
        }
    }

    PAL_Destroy(pal);

    s_pm.extraTimers = CSL_DictionaryAlloc(CSL_DICT_MIN_SIZE);
    if (!s_pm.extraTimers)
    {
        goto errexit;
    }

    ApplyThreadAffinityMask();
    QueryPerformanceCounter(&s_pm.startTime);

    return;

errexit:
    if (pal)
    {
        PAL_Destroy(pal);
    }
    if (s_pm.file)
    {
        PAL_FileClose(s_pm.file);
        s_pm.file = NULL;
    }
    if (s_pm.extraTimers)
    {
        CSL_DictionaryDealloc(s_pm.extraTimers);
        s_pm.extraTimers = NULL;
    }
}

void PerformanceMeasurement_End()
{
    if (s_pm.extraTimers)
    {
        CSL_DictionaryDealloc(s_pm.extraTimers);
        s_pm.extraTimers = NULL;
    }

    if (s_pm.file)
    {
        ResetThreadAffinityMask();
        PAL_FileClose(s_pm.file);
        s_pm.file = NULL;
    }
}

nb_boolean PerformanceMeasurement_StartTimer(const char* key)
{
    LARGE_INTEGER* pStartTime = NULL;

    if (!s_pm.file)
    {
        return FALSE;
    }

    if (!key)
    {
        QueryPerformanceCounter(&s_pm.startTime);
        return TRUE;
    }

    pStartTime = (LARGE_INTEGER*)CSL_DictionaryGet(s_pm.extraTimers, key, NULL);
    if (pStartTime)
    {
        QueryPerformanceCounter(pStartTime);
    }
    else
    {
        LARGE_INTEGER startTime = {0};
        QueryPerformanceCounter(&startTime);
        if (!CSL_DictionarySet(s_pm.extraTimers, key, (const char*)&startTime, sizeof(LARGE_INTEGER)))
        {
            return FALSE;
        }
    }

    return TRUE;
}

double PerformanceMeasurement_GetElapsedTime(const char* key)
{
    LARGE_INTEGER frequency = {0}, currentTime = {0};

    if (!s_pm.file)
    {
        return -1.0;
    }

    QueryPerformanceCounter(&currentTime);
    QueryPerformanceFrequency(&frequency);

    if (!key)
    {
        return (double)(currentTime.QuadPart - s_pm.startTime.QuadPart) / (double)frequency.QuadPart;
    }
    else if (s_pm.extraTimers)
    {
        LARGE_INTEGER* pStartTime = (LARGE_INTEGER*)CSL_DictionaryGet(s_pm.extraTimers, key, NULL);
        if (!pStartTime)
        {
            return -1.0;
        }
        return (double)(currentTime.QuadPart - pStartTime->QuadPart) / (double)frequency.QuadPart;
    }

    return -1.0;
}

PAL_Error PerformanceMeasurement_Print(const char* format, ...)
{
    PAL_Error err = PAL_Ok;
    va_list arg_list;

    if (!s_pm.file || !format || !nsl_strlen(format))
    {
        return PAL_ErrBadParam;
    }

    va_start(arg_list, format);
    err = err ? err : PmPrintArgList(s_pm.file, format, arg_list);
    va_end(arg_list);

    return err;
}

/*! @} */

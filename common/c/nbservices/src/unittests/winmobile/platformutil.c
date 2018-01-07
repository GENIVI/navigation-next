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
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#pragma warning(disable : 4115)

#include "platformutil.h"
#include "pal.h"
#include "palstdlib.h"
#include "main.h"
#include <windows.h>

#ifdef WINCE
#include <winbase.h>
#else
#include "dbghelp.h"
#include "tchar.h"
#endif

/*! Default MIN value */
#define DEFAULT_MOBILE_IDENTIFIER_NUMBER    999999999999999ULL

/*! Default MDN value */
#define DEFAULT_MOBILE_DIRECTORY_NUMBER     9999990020ULL

/*! Test base path */
#define TEST_BASE_PATH                      "NB_UNITTEST_DATA"

const wchar_t DELIMITER = L'\\';
const wchar_t DELIMITER_STRING[] = L"\\";

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
    uint32 elapsedTime = 0;

    boolean success = TRUE;
    
    while (success)
    {
        DWORD waitResult = 0;

        // wait a bit for signalled callback event or timeout
        waitResult = WaitForSingleObject(callbackCompletedEvent, waitTimeSlice);
        elapsedTime += waitTimeSlice;
        
        ProcessPendingMessages();

        // exit successfully if callback event signalled
        if (waitResult == WAIT_OBJECT_0)
        {
            break;
        }
        // exit unsuccesfully if timeout time elapsed or wait resulted in error
        else
        {
            success = (boolean)(elapsedTime < timeoutMsec && waitResult == WAIT_TIMEOUT);
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
DestroyCallbackCompletedEvent(void* callbackCompletedEvent)
{
    CloseHandle(callbackCompletedEvent);
}

PAL_Instance* PAL_CreateInstance()
{
    PAL_Config palConfig = { 0 };

	return PAL_Create(&palConfig);
}


#ifdef WINCE
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	return test_main();
}
#endif

#if !defined(WINCE)

#if defined(_DEBUG)
/// @todo Need to somehow make the debug heap and exception handler code common to all unit tests and system tests

int ReportHook(int type, char* message, int* result)
{
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

    heapFlags |= _CRTDBG_LEAK_CHECK_DF;         // Dump leaks on exit
    heapFlags &= ~_CRTDBG_CHECK_ALWAYS_DF;      // Don't consistency check heap every operation
    heapFlags &= ~_CRTDBG_CHECK_CRT_DF;         // Ignore internal C stdlib leaks

    _CrtSetDbgFlag(heapFlags);
}


void RedirectCrtReportOutput(int type)
{
    _CrtSetReportMode(type, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(type, _CRTDBG_FILE_STDERR);
}

#endif

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

    wcscat(filename, L"csut-minidump.dmp");

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

int _tmain(int argc, _TCHAR* argv[])
{
    SetUnhandledExceptionFilter(SystemTestExceptionFilter);

#if defined(_DEBUG)
    EnableDebugHeap();

    RedirectCrtReportOutput(_CRT_ASSERT);
    RedirectCrtReportOutput(_CRT_ERROR);
    RedirectCrtReportOutput(_CRT_WARN);

    _CrtSetReportHook(ReportHook);
#endif

    return test_main();
}

#endif

uint64 GetMobileDirectoryNumber(PAL_Instance* pal)
{
    return DEFAULT_MOBILE_DIRECTORY_NUMBER;
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

/* See header file for description */
char*
GetResourcePath(const char* filename)
{
    // @todo: Return the file path.
    return NULL;
}

/*! @} */

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

    @file     utility.h
    @date     03/20/2012
    @defgroup Common Utility Functions

    Function declarations for common utilities used for NBGM test

    */
    /*
    (C) Copyright 2012 by TCS, Inc.                

    The information contained herein is confidential, proprietary 
    to TCS, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
#include "utility.h"
#include "palstdlib.h"
#include "palconfig.h"
#include "palfile.h"
#include "tchar.h"
#include "cunit.h"
#include "windows.h"
#include "crtdbg.h"
#include "dbghelp.h"
#include <stdarg.h>

static uint8 g_reportOutputDetected = 0;
void OutputMessage(const char* format, ...)
{
	va_list args;
	int len = 0;
	char* buffer;

	va_start(args, format);
	len = _vscprintf(format, args) + 1;
	buffer = (char*)nsl_malloc(len * sizeof(char));

	vsprintf(buffer, format, args);

	OutputDebugStringA(buffer);
	printf(buffer);

	nsl_free(buffer);
}

PAL_Instance* CreatePal()
{
	PAL_Config palConfig = { 0 };

	return PAL_Create(&palConfig);
}

void DestroyPal(PAL_Instance* pal)
{
	if (!pal)
	{
		return;
	}

	PAL_Destroy(pal);   
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
		}
		*delimiter = '\0';
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
void EnableDebugHeap()
{
	int heapFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

	heapFlags |= _CRTDBG_LEAK_CHECK_DF;         // Dump leaks on exit
	heapFlags |= _CRTDBG_DELAY_FREE_MEM_DF;     // Reserve freed memory blocks
	heapFlags &= ~_CRTDBG_CHECK_ALWAYS_DF;      // Don't consistency check heap every operation
	heapFlags &= ~_CRTDBG_CHECK_CRT_DF;         // Ignore internal C stdlib leaks

	_CrtSetDbgFlag(heapFlags);
}

void RedirectCrtReportOutput(int type)
{
	_CrtSetReportMode(type, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(type, _CRTDBG_FILE_STDERR);
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
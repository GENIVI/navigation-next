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

@file paltestlog.c
@defgroup PALTESTLOG_H PAL

@brief Provides the PAL interface

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

#include "paltestlog.h"
#include "palstdlib.h"
#include "paltypes.h"
#include "palfile.h"
#include "palclock.h"
#include "pallock.h"

#include <stdio.h>
#include <errno.h>

#define LOG_BUFFER_SIZE 1024
#define REFRESH_IMPORTANT_INFO_THRESHOLD (27 * 1024)
#define LOG_HEADER_LENGTH 17
#define VERSION_STRING_LENGTH 15
#define ERROR_LOG_NUMBER 3


static int errorBufferLogLength = 0;
static char versionString[VERSION_STRING_LENGTH] = {0};
static int init_status = -1;

PAL_DEF void
PAL_SetVersionString(char *version)
{
    if (version == NULL)
        return;

    nsl_snprintf(versionString, VERSION_STRING_LENGTH, "%s", version);
}

PAL_DEF void
PAL_TestLog(PAL_LogBufferNumber bufferNumber, PAL_LogLevel level, const char *fmt, ...)
{
    int stringLength = 0;
    va_list argumentList;
    char buffer[LOG_BUFFER_SIZE] = {0};

    if (bufferNumber >= PAL_LogBuffer_Invalid)
    {
        return;
    }

    va_start(argumentList, fmt);
    stringLength = vsnprintf(buffer, LOG_BUFFER_SIZE, fmt, argumentList);

    switch(level)
    {
    case PAL_LogLevelCritical:
    {
        PAL_WriteTestLog("Critical", fmt, argumentList);
        break;
    }
    case PAL_LogLevelError:
    {
        PAL_WriteTestLog("Error", fmt, argumentList);
        break;
    }
    case PAL_LogLevelWarning:
    {
        PAL_WriteTestLog("Warning", fmt, argumentList);
        break;
    }
    case PAL_LogLevelInfo:
    {
        PAL_WriteTestLog("Info", fmt, argumentList);
        break;
    }
    case PAL_LogLevelDebug:
    {
        PAL_WriteTestLog("Debug", fmt, argumentList);
        break;
    }
    case PAL_LogLevelVerbose:
    {
        PAL_WriteTestLog("Verbose", fmt, argumentList);
        break;
    }
    case PAL_LogLevelNone:
    default:
        break;

    }

    va_end(argumentList);
}

PAL_DEC void PAL_SetStartTime(char *time)
{

}

PAL_DEC void PAL_WriteTestLog(const char *logLevel, const char* format, va_list va)
{
    // Preprend date and time
    PAL_ClockDateTime time = {0};
    char timeString[128] = {0};
    if (PAL_ClockGetDateTime(&time) == PAL_Ok)
    {
        nsl_sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d.%06d ",
                    time.year, time.month, time.day, time.hour, time.minute, time.second, time.milliseconds);
    }


    fprintf(stdout,"%s:%d(%s):(%d)[%s]: ",
            timeString, logLevel, __FILE__, __LINE__, __FUNCTION__);
    vfprintf(stdout, format, va);
    fprintf(stdout, "\n");
}

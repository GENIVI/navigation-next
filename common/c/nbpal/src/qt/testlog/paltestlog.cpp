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

#include <QDebug>
#include <iostream>

#define LOG_BUFFER_SIZE 1024
#define REFRESH_IMPORTANT_INFO_THRESHOLD (27 * 1024)
#define LOG_HEADER_LENGTH 17
#define VERSION_STRING_LENGTH 15
#define ERROR_LOG_NUMBER 3


static int errorBufferLogLength = 0;
static char versionString[VERSION_STRING_LENGTH] = {0};
static int init_status = -1;
static void PAL_WriteTestLog(const char *logLevel, const char* logStr);

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
    if (bufferNumber >= PAL_LogBuffer_Invalid)
    {
        return;
    }

    char buffer[LOG_BUFFER_SIZE] = {0};

    va_list argumentList;
    va_start(argumentList, fmt);
    stringLength = vsnprintf(buffer, LOG_BUFFER_SIZE, fmt, argumentList);
    va_end(argumentList);

    switch(level)
    {
    case PAL_LogLevelCritical:
    {
        qCritical()<<buffer;
        break;
    }
    case PAL_LogLevelError:
    case PAL_LogLevelWarning:
    {
        qWarning()<<buffer;
        break;
    }
    case PAL_LogLevelInfo:
    case PAL_LogLevelDebug:
    case PAL_LogLevelVerbose:
    {
        qDebug()<<buffer;
        break;
    }
    case PAL_LogLevelNone:
    default:
        break;
    }
}

PAL_DEC void PAL_SetStartTime(char* /*time*/)
{

}

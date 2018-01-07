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
//#include <sys/slog2.h>
#include <stdio.h>
#include <errno.h>

//#include <sys/proxyinfo.h>
#include <stdio.h>
//#include <sys/slog.h>
#include <android/log.h>


#define LOG_BUFFER_SIZE 1024
#define REFRESH_IMPORTANT_INFO_THRESHOLD (27 * 1024)
#define LOG_HEADER_LENGTH 17
#define VERSION_STRING_LENGTH 15
#define ERROR_LOG_NUMBER 3


static int errorBufferLogLength = 0;
/*
static slog2_buffer_set_config_t  log_config = {4, (char*)"maps", SLOG2_INFO, {{(char*)"tmaps0", 7}, {(char*)"tmaps1", 7}, {(char*)"tmaps2", 7}, {(char*)"tmapserror", 7}}};
static slog2_buffer_t slog2Buffer[SLOG2_MAX_BUFFERS];
*/
static char versionString[VERSION_STRING_LENGTH] = {0};
static int init_status = -1;

PAL_DEC PAL_Error PAL_LogInit()
{
/*
	if (init_status == 0)
	{
        return PAL_Ok;
	}

    if (slog2_register(&log_config, slog2Buffer, 0) == 0)
    {
        init_status = 0;
        return PAL_Ok;
    }
    else
    {
        init_status = -1;
        return PAL_Failed;
    }
    */
    return PAL_Ok;
}

PAL_DEF uint8
PAL_SwitchLogLevel(PAL_LogLevel level)
{
/*
	switch (level)
	{
        case PAL_LogLevelError:
            return SLOG2_ERROR;

        case PAL_LogLevelWarning:
            return SLOG2_WARNING;

        case PAL_LogLevelInfo:
            return SLOG2_INFO;

        case PAL_LogLevelDebug:
            return SLOG2_DEBUG1;

        case PAL_LogLevelNone:
        case PAL_LogLevelVerbose:
        default:
            return SLOG2_DEBUG2;
	}*/
    return 0;
}

PAL_DEF uint16
PAL_GetLogNumber(PAL_LogLevel level)
{
    return pthread_self() + (int)level * 1000;
}

PAL_DEF void
PAL_SetVersionString(char *version)
{
    if (version == NULL)
        return;

    nsl_snprintf(versionString, VERSION_STRING_LENGTH, "%s", version);
}

PAL_DEF void
PAL_TestLogRefreshImportantLog()
{/*
    char buffer[LOG_BUFFER_SIZE] = {0};

    snprintf(buffer, LOG_BUFFER_SIZE, "version: %s", versionString);

    slog2c(slog2Buffer[ERROR_LOG_NUMBER], PAL_GetLogNumber(PAL_LogLevelError), PAL_SwitchLogLevel(PAL_LogLevelError), buffer);*/
}


static inline android_LogPriority ConvertToNativeLogLevel(PAL_LogLevel logLevel)
{
    switch (logLevel)
    {
        case  PAL_LogLevelInfo: // this is mostly used...
        {
            return ANDROID_LOG_INFO;
        }
        case PAL_LogLevelNone:
        {
            return ANDROID_LOG_UNKNOWN;
        }
        case PAL_LogLevelCritical:
        {
            return ANDROID_LOG_FATAL;
        }
        case PAL_LogLevelError:
        {
            return ANDROID_LOG_ERROR;
        }
        case PAL_LogLevelWarning:
        {
            return ANDROID_LOG_WARN;
        }
        case PAL_LogLevelDebug:
        {
            return ANDROID_LOG_DEBUG;
        }
        case PAL_LogLevelVerbose:
        {
            return ANDROID_LOG_VERBOSE;
        }
        default:
        {
            return ANDROID_LOG_DEFAULT;
        }
    }
}

PAL_DEF void
PAL_TestLog(PAL_LogBufferNumber bufferNumber, PAL_LogLevel level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
#ifdef NDEBUG // Release build, logs are recorded only when log level is higher than Warning.
    if (level > PAL_LogLevelNone  && level <= PAL_LogLevelWarning)
    {
#endif
        __android_log_vprint(ConvertToNativeLogLevel(level), "PAL_TestLog", fmt, ap);

#ifdef NDEBUG
    }
#endif
    va_end(ap);
}


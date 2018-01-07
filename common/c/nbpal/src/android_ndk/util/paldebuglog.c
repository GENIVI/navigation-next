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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2008 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * nimdebuglog.h: created 2008/09/04 by Mark Goddard.
 */
#include <stdio.h>
#include <stdarg.h>
#include <android/log.h>
#include "paldebuglog.h"

char* getsubsystem_str(long subsystem);
int getseverity_val(int severity);


PAL_DEF void
debugf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	__android_log_vprint(ANDROID_LOG_INFO, "nbpal", fmt, ap);
	va_end(ap);
}

PAL_DEF char*
extract_message(const char *fmt, ...)
{
	va_list ap;
	int logBufferSize = 256;
	char* buffer = (char*)malloc(logBufferSize);
	memset(buffer,0,logBufferSize);
	va_start(ap, fmt);
	vsnprintf (buffer, logBufferSize-1, fmt, ap);
	va_end(ap);

	return buffer;
}


PAL_DEF void
logcat(long subsystem, int severity, const char *buffer)
{
	char* subsystem_str;
	subsystem_str = getsubsystem_str(subsystem);
	int severity_val = getseverity_val(severity);
	__android_log_print(severity_val, subsystem_str, "%s",buffer);
	free((void*)buffer);
}

char* getsubsystem_str(long subsystem)
{
	switch (subsystem)
	{
	case LOG_SS_MEMORY:
		return "LOG_SS_MEMORY";
	case LOG_SS_FILE:
		return "LOG_SS_FILE";
	case LOG_SS_NET:
		return "LOG_SS_NET";
	case LOG_SS_GPS:
		return "LOG_SS_GPS";
	case LOG_SS_TPS:
		return "LOG_SS_TPS";
	case LOG_SS_DATA:
		return "LOG_SS_DATA";
	case LOG_SS_RP:
		return "LOG_SS_RP";
	case LOG_SS_FAV:
		return "LOG_SS_FAV";
	case LOG_SS_MYSEARCH:
		return "LOG_SS_MYSEARCH";
	case LOG_SS_UI:
		return "LOG_SS_UI";
	case LOG_SS_NAVUI:
		return "LOG_SS_NAVUI";
	case LOG_SS_REF:
		return "LOG_SS_REF";
	case LOG_SS_CACHE:
		return "LOG_SS_CACHE";
	case LOG_SS_DBGNET:
		return "LOG_SS_DBGNET";
	case LOG_SS_YP:
		return "LOG_SS_YP";
	case LOG_SS_CALL:
		return "LOG_SS_CALL";
	case LOG_SS_NB_NET:
		return "LOG_SS_NB_NET";
	case LOG_SS_NB_GEOCODE:
		return "LOG_SS_NB_GEOCODE";
	case LOG_SS_NB_MAP:
		return "LOG_SS_NB_MAP";
	case LOG_SS_NB_POI:
		return "LOG_SS_NB_POI";
	case LOG_SS_NB_NAV:
		return "LOG_SS_NB_NAV";
	case LOG_SS_NB_FILESET:
		return "LOG_SS_NB_FILESET";
	case LOG_SS_NB_DIR:
		return "LOG_SS_NB_DIR";
	case LOG_SS_CORE:
		return "LOG_SS_CORE";
	case LOG_SS_MEDIA:
		return "LOG_SS_MEDIA";
	case LOG_SS_NET2:
		return "LOG_SS_NET2";
	case LOG_SS_GPS2:
		return "LOG_SS_GPS2";
	case LOG_SS_NB_PAL:
		return "LOG_SS_NB_PAL";
	case LOG_SS_JNI:
		return "LOG_SS_JNI";
	default:
		return "INVALID SUBSYSTEM";
	}
}

int getseverity_val(int severity)
{
	switch(severity)
	{
	case LOG_SEV_CRITICAL:
		return ANDROID_LOG_FATAL;
	case LOG_SEV_MAJOR:
	case LOG_SEV_IMPORTANT:
		return ANDROID_LOG_ERROR;
	case LOG_SEV_MINOR:
		return ANDROID_LOG_WARN;
	case LOG_SEV_INFO:
	case LOG_SEV_MINOR_INFO:
		return ANDROID_LOG_INFO;
	case LOG_SEV_DEBUG:
		return ANDROID_LOG_VERBOSE;
	default:
		return ANDROID_LOG_UNKNOWN;
	}
}

PAL_DEC void dummyf(const char* fmt, ...)
{
}

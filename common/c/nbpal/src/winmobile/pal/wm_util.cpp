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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include <cstdio>
#include <ctime>
#include "windows.h"

#include "wm_util.h"

time_t pal_time( time_t *inTT )
{
	SYSTEMTIME sysTimeStruct;
	FILETIME fTime;
	ULARGE_INTEGER int64time;
	time_t locTT = 0;

	if ( inTT == NULL ) {
		inTT = &locTT;
	}

	GetSystemTime( &sysTimeStruct );
	if ( SystemTimeToFileTime( &sysTimeStruct, &fTime ) ) {
		memcpy( &int64time, &fTime, sizeof( FILETIME ) );		
		int64time.QuadPart -= 0x19db1ded53e8000; //Subtract the value for 1970-01-01 00:00 (UTC) 
		int64time.QuadPart /= 10000000; // Convert to seconds.
		*inTT = (time_t)int64time.QuadPart;
	}

	return *inTT;
} 
unsigned long long convertUTCtoSeconds( SYSTEMTIME t )
{
	FILETIME fTime;
    ULARGE_INTEGER int64time = { 0 };
	if ( SystemTimeToFileTime( &t, &fTime ) ) {
		memcpy( &int64time, &fTime, sizeof( FILETIME ) );		
		int64time.QuadPart -= 0x19db1ded53e8000; //Subtract the value for 1970-01-01 00:00 (UTC) 
		int64time.QuadPart /= 10000000; // Convert to seconds.
	}
	return int64time.QuadPart;
}

void ShowMsg( TCHAR* msg_param )
{  
	//TCHAR msg[120];
	//_stprintf(msg, _T("Timer Scheduled: %d, %d "), te->timerID, millisecs);
	//MessageBox(NULL, msg_param, NULL, MB_OK);
}

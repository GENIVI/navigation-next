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

/*--------------------------------------------------------------------------

(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "windows.h"
#include "paltypes.h"
#include "palstdlib.h"
#include "palclock.h"
#include "time.h"

uint32 SecondFromTime(SYSTEMTIME epoch);

SYSTEMTIME gpsEpoch;
SYSTEMTIME unixEpoch;

nb_gpsTime
PAL_ClockGetGPSTime()
{
    gpsEpoch.wYear = 1980;
    gpsEpoch.wMonth = 1;
    gpsEpoch.wDay = 6;
    gpsEpoch.wHour = 0;
    gpsEpoch.wMinute = 0;
    gpsEpoch.wSecond = 0;
    gpsEpoch.wMilliseconds = 0;
    gpsEpoch.wDayOfWeek = 0;

    return SecondFromTime(gpsEpoch);
}

nb_unixTime
PAL_ClockGetUnixTime()
{
    unixEpoch.wYear = 1970;
    unixEpoch.wMonth = 1;
    unixEpoch.wDay = 1;
    unixEpoch.wHour = 0;
    unixEpoch.wMinute = 0;
    unixEpoch.wSecond = 0;
    unixEpoch.wMilliseconds = 0;
    unixEpoch.wDayOfWeek = 0;

    return SecondFromTime(unixEpoch);
}

uint32
SecondFromTime(SYSTEMTIME epoch)
{

    SYSTEMTIME systemTime ={0};

    FILETIME epochFT;
    FILETIME currentFT;

    ULARGE_INTEGER epochULI;
    ULARGE_INTEGER currentULI;
    ULARGE_INTEGER difference;

    GetSystemTime( &systemTime );

    SystemTimeToFileTime(&epoch, &epochFT);

    epochULI.HighPart = epochFT.dwHighDateTime;
    epochULI.LowPart = epochFT.dwLowDateTime;

    SystemTimeToFileTime(&systemTime, &currentFT);

    currentULI.HighPart = currentFT.dwHighDateTime;
    currentULI.LowPart = currentFT.dwLowDateTime;

    difference.QuadPart = currentULI.QuadPart - epochULI.QuadPart;
    difference.QuadPart = difference.QuadPart / 10000000; // convert from 100 ns blocks to seconds

    return difference.LowPart;
}

PAL_Error PAL_ClockGetDateTime(PAL_ClockDateTime* pDateTime)
{
    SYSTEMTIME curTime;

    // Ensure that we have a valid pointer to copy date/time into
    if(!pDateTime)
    {
        return PAL_ErrBadParam;
    }

    // Get date/time from the system
    GetSystemTime(&curTime);

    // Copy the date/time to the PAL_TimeDateTime structure
    pDateTime->year             = curTime.wYear;
    pDateTime->month            = curTime.wMonth;
    pDateTime->day              = curTime.wDay;
    pDateTime->dayOfWeek        = curTime.wDayOfWeek;
    pDateTime->hour             = curTime.wHour;
    pDateTime->minute           = curTime.wMinute;
    pDateTime->second           = curTime.wSecond;
    pDateTime->milliseconds     = curTime.wMilliseconds;

    time_t t = time(NULL);
    tm local;
    localtime_s(&local, &t);
    tm gmTime;
    gmtime_s(&gmTime, &t);                        /* Get the GM Time (UTC) from the local time */
    time_t gmTimeT = mktime(&gmTime);                                /* Convert gm time to time_t (allows calculations)*/

    pDateTime->daylightSaving = static_cast<nb_boolean>(local.tm_isdst);
    pDateTime->timezone = static_cast<int16>(difftime(t, gmTimeT) / 3600);    /* get the difference and divide to 1 hour, to get the time zone */


    return PAL_Ok;
}

uint32 PAL_ClockGetTimeMs()
{
    return GetTickCount();
}

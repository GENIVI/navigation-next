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

@file palclock.h
@defgroup PALCLOCK_H PAL Clock Functions

@brief Provides a platform-dependent time interface

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

#ifndef PALCLOCK_H
#define	PALCLOCK_H

#include "pal.h"

/*! PAL_TimeDateTime structure */
typedef struct _PAL_TimeDateTime {
    uint16 year;                /*! 4-digit year */
    uint16 month;               /*! Month 1-12 */
    uint16 dayOfWeek;           /*! Day of the week; Sunday=0, Monday=1, ... */
    uint16 day;                 /*! Day of the month */
    uint16 hour;                /*! Hour of the day; 0-23 */
    uint16 minute;              /*! Minutes past the hour; 0-59 */
    uint16 second;              /*! Seconds past the minute; 0-59 */
    uint16 milliseconds;        /*! Milliseconds past the seconds; 0-999 */
    nb_boolean daylightSaving;      /*! Daylight Savings Time flag 0/1 */
    int16 timezone;             /*! time zone (-12)-(+12)*/
} PAL_ClockDateTime;

/*! Get current GPS Time in UTC

This function returns the current GPS Time in seconds (nb_unixTime).
GPS Time is defined as the number of seconds that have elapsed since
January 6, 1980 12:00:00 AM.

@return nb_unixTime
*/
PAL_DEC nb_gpsTime PAL_ClockGetGPSTime();

/*! @brief Get current Unix time in UTC

This function returns the current Unix Time in seconds (nb_unixTime).
Unix Time is defined as the number of seconds that have elapsed since
January 1, 1970 12:00:00 AM.

@return nb_unixTime
*/
PAL_DEC nb_unixTime PAL_ClockGetUnixTime();

/*! Get current Date and Time, in UTC

This function returns the current date and time in the pDateTime parameter.

@return  PAL_Ok on success and current date/time values in the pDateTime parameter 
*/
PAL_DEC PAL_Error PAL_ClockGetDateTime(PAL_ClockDateTime* pDateTime);

/*! Get time with milliseconds precision

    This function should be used for time intervals measurement ONLY. It can return different
    values on different platforms because of different platform APIs. Use it to get time
    difference with high precision, for example in performance metrics.

@return  value in milliseconds
*/
PAL_DEC uint32 PAL_ClockGetTimeMs();

#endif

/*! @} */

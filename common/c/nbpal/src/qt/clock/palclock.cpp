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
#include "paltypes.h"
#include "palclock.h"
#include <QDateTime>

#define GPS_TO_UNIX_OFFSET 315964800
#define NMSEC_PER_SEC      1000

static int GetLocalTimezone()
{
    return 24 - QDateTime(QDate(1970, 1, 2), QTime(0, 0, 0), Qt::LocalTime).toTime_t()/3600;
}

static void ConvertDataTime(const QDateTime& from, PAL_ClockDateTime& to)
{
    QDate date = from.date();
    to.year = date.year();
    to.month = date.month();
    to.dayOfWeek = date.dayOfWeek();
    to.day = date.day();

    QTime time = from.time();
    to.hour = time.hour();
    to.minute = time.minute();
    to.second = time.second();
    to.milliseconds = time.msec();

    to.daylightSaving = from.isDaylightTime();
    to.timezone = GetLocalTimezone();
}

PAL_DEF nb_gpsTime
PAL_ClockGetGPSTime()
{
    return QDateTime::currentDateTime().toTime_t() - GPS_TO_UNIX_OFFSET;
}

PAL_DEF nb_unixTime
PAL_ClockGetUnixTime()
{

    return QDateTime::currentDateTime().toTime_t();
}

PAL_DEF PAL_Error
PAL_ClockGetDateTime(PAL_ClockDateTime* pDateTime)
{
    QDateTime curr = QDateTime::currentDateTime();
    ConvertDataTime(curr, *pDateTime);
    return PAL_Ok;
}

PAL_DEF uint32
PAL_ClockGetTimeMs()
{
    return QDateTime::currentMSecsSinceEpoch();
}


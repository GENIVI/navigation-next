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
 @file         FormatController.cpp
 @defgroup     nkui
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary to
 TeleCommunication Systems, Inc., and considered a trade secret as defined
 in section 499C of the penal code of the State of California. Use of this
 information by anyone other than authorized employees of TeleCommunication
 Systems is granted only under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 --------------------------------------------------------------------------*/

/*! @{ */

#include <math.h>
#include <vector>
#include "palstdlib.h"
#include "paltypes.h"
#include "FormatController.h"

using namespace std;
static const float  MeterToFeetConstant    = 3.28083989501f;
static const int32  FeetToMileConstant     = 5280;
static const int32  ToFeetSwitchThreshold  = 975; // 1000 - (FeetRound / 2) : Rounding can add upto 25.
static const int32  ToMeterSwitchThreshold = 990;
static const uint32 FeetRound              = 50;
static const uint32 MeterRound             = 20;
static const uint32 FeetsPerYard           = 3;

FormatController::FormatController()
{
}

FormatController::FormatController(NKUIStringHelperPtr stringHelper)
    : m_pStringHelper(stringHelper)
{
}

FormatController::~FormatController()
{
}

std::string
FormatController::GetSuperStringByKey(const std::string& key)
{
    return m_pStringHelper->GetLocalizedString(key);
}

void FormatController::FormatDistance(nbnav::Preferences::Measurement distanceFormat, double distance, std::string& result)
{
    std::string distance_text;
    std::string distance_unit;
    FormatDistance(distanceFormat, distance, distance_text, distance_unit);
    result = distance_text + " " + distance_unit;
}

void
FormatController::FormatDistance(nbnav::Preferences::Measurement distanceFormat,
                                 double distance,
                                 std::string& distanceString,
                                 std::string& distanceUnit)
{
    switch(distanceFormat)
    {
        case nbnav::Preferences::NonMetric:
        case nbnav::Preferences::NonMetricYards:
        {
            double feet = distance * MeterToFeetConstant;
            feet = FeetRound * (uint32)((feet + (FeetRound / 2)) / FeetRound);
            if(feet >= ToFeetSwitchThreshold)
            {
                double mile = feet / FeetToMileConstant;
                if(mile < 0.2)
                {
                    mile = 0.2;
                }
                if(mile >= 100)
                {
                    distanceString = ConvertToString((int)(mile + 0.5));
                }
                else
                {
                    distanceString = RetainOneDecimalPlace(mile);
                }
                distanceUnit = GetSuperStringByKey("IDS_MI");
            }
            else
            {
                if(distanceFormat == nbnav::Preferences::NonMetric)
                {
                    distanceString = ConvertToString((int)feet);
                    distanceUnit = GetSuperStringByKey("IDS_FT");
                }
                else
                {
                    distanceString = ConvertToString(int(feet/FeetsPerYard));
                    distanceUnit = GetSuperStringByKey("IDS_YD");
                }
            }
            break;
        }
        case nbnav::Preferences::Metric:
        {
            // @todo: Could floor be used in CCI?
            uint32 meter = (uint32)floor(distance);
            meter = MeterRound * (uint32)((meter + (MeterRound / 2)) / MeterRound);
            if (meter >= ToMeterSwitchThreshold)
            {
                double kiloMeter = (double)(meter / 1000.0);
                if (kiloMeter >= 100)
                {
                    distanceString = ConvertToString((int)(kiloMeter + 0.5));
                }
                else
                {
                    distanceString = RetainOneDecimalPlace(kiloMeter);
                }
                distanceUnit = GetSuperStringByKey("IDS_KM");
            }
            else
            {
                distanceString = ConvertToString((int)meter);
                distanceUnit = GetSuperStringByKey("IDS_METER_SMALL");
            }
            break;
        }
        default:
        {
            nsl_assert(FALSE);
            break;
        }
    }
}

/**
 *  format time to hours
 *
 *  @param seconds    gps time
 *  @param hourString hourstring
 */
void
FormatController::FormatTimeToHour(uint32 seconds, std::string& hourString)
{
    uint32 hours = (seconds / (60 * 60));
    hourString = ConvertToString(hours) + " " + GetSuperStringByKey("IDS_HRS");
}

/*! Get formatted time from secondes.

    There are 3 types result returned:
    1) Only hours with unit, e.g. "14 hr";
    2) Hours and minutes with unit, e.g. "8 h 39 m";
    3) Only minutes with unit, e.g. "39 min".

    For case (1) and (3), only timeString and timeUnit will set string value, timeString2 and timeUnit2 will be empty,
    for case (2), timeString, timeUnit, timeString2, timeUnit2 will set string value.

    @return None
 */
void
FormatController::FormatTime(uint32 seconds, std::string& timeString, std::string& timeUnit, std::string& timeString2, std::string& timeUnit2)
{
    timeString = "";
    timeUnit = "";
    timeString2 = "";
    timeUnit2 = "";

    // Format navigation duration time:
    // 1) >= 10 hours, only show hours, with "hr" unit;
    // 2) >= 1 hours, show hours and minutes, with "h" and "m" unit;
    // 3) < 1 hours, only show minutes with "min" unit;

    // Get minutes, rounded up seconds
    uint32 minutes = seconds / 60;
    if (seconds % 60 >= 30)
    {
        minutes += 1;
    }
    if (minutes >= 60)
    {
        const uint32 MinHoursWithoutMinutes = 10;
        // Get hours
        uint32 hours = (minutes / 60);
        minutes = minutes % 60;
        if (hours >= MinHoursWithoutMinutes || minutes == 0)
        {
            // Only hours case, need rounded up minutes
            if (minutes % 60 >= 30)
            {
                hours += 1;
            }
            timeString = ConvertToString(hours);
            timeUnit = GetSuperStringByKey("IDS_HRS");
        }
        else
        {
            // Hours + minutes case
            timeString = ConvertToString(hours);
            timeUnit = GetSuperStringByKey("IDS_H");
            // Show minutes always
            timeString2 = ConvertToString(minutes);
            timeUnit2 = GetSuperStringByKey("IDS_MIN_SMALL");
        }
    }
    else
    {
        // Only minutes case
        timeString = ConvertToString(minutes);
        timeUnit = GetSuperStringByKey("IDS_MIN");
    }
}

std::string
FormatController::GetAttributionStringByID(const std::string& attributionID)
{
    if(!attributionID.empty())
    {
        return GetSuperStringByKey(attributionID);
    }
    else
    {
        return "";
    }
}


std::string
FormatController::DistanceConversion(double distance, double factor, bool needRound)
{
    std::string distanceString("");
    if(factor != 0.0)
    {
        double _distance = distance / factor;
        if(std::abs((int)_distance) > 100 || needRound)
        {
            int distanceOfInt = (int)(_distance + 0.5);
            distanceString = ConvertToString(distanceOfInt);
        }
        else
        {
            distanceString = RetainOneDecimalPlace(_distance);
        } 
    }
    return distanceString; 
}


std::string
FormatController::RetainOneDecimalPlace(double number)
{
    // Rounding on 2nd decmial point.
    std::string numberString = ConvertToString(number + 0.05);
    int index = (int)numberString.find('.');
    if(index != string::npos)
    {
        numberString = numberString.substr(0, index + 2);
    }
    else
    {
        if (numberString.length() == 1)
        {
            numberString.append(".0");
        }
    }
    return numberString;
}

/*! @} */

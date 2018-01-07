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

#ifndef NCDB_SPEEDLIMITS_H
#define NCDB_SPEEDLIMITS_H

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2014 by TeleCommunication Systems, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to TeleCommunication Systems, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of TeleCommunication Systems is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/
#include "NcdbTypes.h"      // LinkDirectionOW
#include "WorldPoint.h"
#include "AutoArray.h"      
#include "UtfString.h"      
#include "AutoSharedPtr.h"

namespace Ncdb
{

typedef enum NCDB_API Speed_Type
{
    day_monday = 1,
    day_tuesday = 2,
    day_wednesday = 4,
    day_thursday = 8,
    day_friday =0x10,
    day_saturday = 0x20,
    day_sunday = 0x40,
    day_all = 0x7f,
    special_speed_zone = 0x80
}SPEED_TYPE;

typedef enum NCDB_API SpecialSpeedZoneType
{ 
    zone_none = 0, 
    zone_school
} SPECIAL_SPEED_ZONE_TYPE;

struct NCDB_API SpeedType
{
    SpeedType():Speed_Zone_Type(zone_none),Warn_Ahead(0.){}

    SPECIAL_SPEED_ZONE_TYPE GetSpeedZoneType(){return Speed_Zone_Type;}
    SPECIAL_SPEED_ZONE_TYPE GetSpeedZoneType() const{return Speed_Zone_Type;}
    double GetWarnAhead(){return Warn_Ahead;}
    double GetWarnAhead()const {return Warn_Ahead;}

    SPECIAL_SPEED_ZONE_TYPE Speed_Zone_Type;    // special zone  "School Area"
    double Warn_Ahead;                          // warning ahead  "200 meter ahead  is school"
};

struct NCDB_API SpeedSelector  //this is the definition of the active-selector
{
    char m_Day_Type;//applied day(s) 11111111   SPEED_TYPE;
    int m_Time_Start;       
    int m_Time_End;
    // sign_id  should be provide by server-let.
    char GetDayType() const {return m_Day_Type;}
    int GetTimeStart() const {return m_Time_Start;}
    int GetTimeEnd() const {return m_Time_End;}
};

struct NCDB_API SpeedLimited
{				
    SpeedSelector m_SpeedSelector;      
    double m_speed;                        // speed limited value;
    const SpeedSelector& GetSpeedSelector() const {return m_SpeedSelector;}
    double GetSpeedLimitedValue() const {return m_speed;}
};

typedef AutoArray< SpeedLimited > SpeedLimitedList;
#ifdef _MSC_VER
template class NCDB_API				AutoArray<SpeedLimited>;
template class NCDB_API				AutoSharedPtr<SpeedLimitedList>;
#endif

class NCDB_API SpeedRegion
{
public:
    SpeedRegion();
    ~SpeedRegion();
    unsigned int     GetStartManeuverIndex() const;
    unsigned int     GetEndManeuverIndex() const;
    double           GetStartManeuverOffset() const;
    double           GetEndManeuverOffset() const;
    double           GetRegionLength() const;
    const UtfString& GetCountryCode()const;
    const SpeedType& GetSpeedType() const{return m_SpeedType;}
    AutoSharedPtr<SpeedLimitedList> GetSpeedLimitedList()const;

private:
    unsigned int   m_startManIndex;
    unsigned int   m_endManIndex;
    double         m_startManOffset;
    double         m_endManOffset;
    double         m_length;
    UtfString	   m_CCode;     // add country code
    SpeedType      m_SpeedType; // special speed limited type
    AutoSharedPtr<SpeedLimitedList> mp_SpeedLimitedList; // different speed limited conditions for current Region.
    friend class RouteEngine;
};

typedef AutoArray< SpeedRegion > SpeedRegionList;
#ifdef _MSC_VER
template class NCDB_API AutoArray<SpeedRegion>;
template class NCDB_API AutoSharedPtr<SpeedRegionList>;
#endif

} // namespace Ncdb

#endif //NCDB_SPEEDLIMITS_H

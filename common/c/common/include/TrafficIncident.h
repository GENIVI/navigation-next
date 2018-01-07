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
 @file     TrafficIncident.h
 @defgroup nbcommon
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __TRAFFICINCIDENT_H__
#define __TRAFFICINCIDENT_H__

#include "paltypes.h"

namespace nbcommon
{

/*! Severity of the incident.
 */
typedef enum
{
    TIS_Critical = 0,
    TIS_Major,
    TIS_Minor,
    TIS_LowImpact,
    TIS_None
} TrafficIncidentSeverity;

/*! Type of incident.
 */
typedef enum
{
    TIT_None = 0,
    TIT_Accident,
    TIT_Congestion,
    TIT_DisabledVehicle,
    TIT_RoadHazard,
    TIT_UnscheduledConstruction,
    TIT_ScheduledConstruction,
    TIT_PlannedEvent,
    TIT_MassTransit,
    TIT_OtherNews,
    TIT_Misc
} TrafficIncidentType;

class TrafficIncident
{
public:
    TrafficIncident() : m_description(""), m_road(""), m_startTime(0), m_endTime(0), m_entryTime(0), m_type(TIT_None), m_severity(TIS_None) {}
    ~TrafficIncident() {}

    TrafficIncidentSeverity m_severity;   /*!< See TrafficIncidentCriticality define. */
    TrafficIncidentType m_type;              /*!< See the define of TrafficIncidentType. */
    uint32 m_entryTime;        /*!< Time this incident was entered into the database. */
    uint32 m_startTime;        /*!< Time this incident began. */
    uint32 m_endTime;          /*!< Time this incident is expected to end. */
    std::string m_description; /*!< The details of this incident. */
    std::string m_road;        /*!< The road this incident is on. */
};

}

#endif // __TRAFFICINCIDENT_H__

/*! @} */
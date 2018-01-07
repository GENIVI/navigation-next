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

 @file     NBSpeedLimitTypes.h
 */
/*
 (C) Copyright 2011 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

#ifndef NB_SPEED_LIMIT_TYPES_H
#define NB_SPEED_LIMIT_TYPES_H

/*!
 @addtogroup
 @{
 */

#include "vec.h"
#include "nbcontext.h"
#include "paltypes.h"
#include "nbnavigationstateprivate.h"
#include "nbspeedlimitprocessor.h"

// Constants ....................................................................................


// Types ........................................................................................

typedef struct NB_SpeedRegion
{
    double m_speedLimit;
	NB_SpeedZoneType m_speedZoneType;
	double m_warnAhead;
	char* m_signId;
	char* m_signidHighlighted;
	char* m_version;
	char* m_versionHighlighted;
	char* m_datasetid;
    uint32 m_startManeuver;
	double m_startManeuverOffset;
	uint32 m_endManeuver;
	double m_endManeuverOffset;
    nb_boolean m_haveAlert;
    nb_boolean m_qalogEnter;

} NB_SpeedRegion;

typedef struct NB_SpeedLimitInformation
{
	NB_Context* m_context;
	struct CSL_Vector * m_speedRegions;
} NB_SpeedLimitInformation;

// QA Log need
typedef NB_SpeedZoneType NB_QaLogSpecialZoneType;

typedef enum
{
    QALOG_WARN_TYPE_WarnAhead = 1,
    QALOG_WARN_TYPE_Speeding = 2,
} QALOG_WARN_TYPE;

struct NB_QaLogSpeedLimitStateData
{
    uint8   m_warnType;
    char    m_signId[NB_NAVIGATION_STATE_SPEED_LIMIT_SIGN_ID_LEN];
    double  m_latitude;
    double  m_longitude;
    uint32  m_maneuverId;
    double  m_offset;
    double  m_speedLimit;
    uint8   m_zoneType;
};

struct NB_QaLogSpeedLimitInformation
{
    nb_boolean  m_speedLimitEnable;
    nb_boolean  m_displayEnable;
    nb_boolean  m_speedAlertenable;
    uint8       m_speedWarningSoundType;
    double      m_speedWarningRange;
    nb_boolean  m_zoneAlertEnable;
    uint8       m_zoneWarningSoundType;
    nb_boolean  m_debugEnable;
    uint32      m_debugOffset;
};

// Functions ....................................................................................

/*! @} */

#endif // NB_SPEED_LIMIT_TYPES_H

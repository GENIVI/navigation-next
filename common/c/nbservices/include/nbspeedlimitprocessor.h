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
 @file     nbspeedlimitprocessor.h
 @defgroup speedlimit

 */
/*
 (C) Copyright 2011 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef nbservices_nbspeedlimitprocessor_h
#define nbservices_nbspeedlimitprocessor_h

#include "paltypes.h"

typedef enum NB_SpeedZoneType
{
    NB_SpeedZoneType_None = 0,
    NB_SpeedZoneType_SchoolZone,
    NB_SpeedZoneType_ConstructionZone,
    NB_SpeedZoneType_SafetyEnhancedZone
} NB_SpeedZoneType;

/*! this struct is send by speed limit callback to uplayer.
*/
typedef struct NB_SpeedLimitStateData
{
    nb_boolean showSpeedLimit;          /*!< show or not show speedlimit sign. */
    nb_boolean showSchoolZone;          /*!< show or not show school. */
    double     limitSpeed;              /*!< current speed limit value. */
    uint8*     speedlimitPictureData;   /*!< speedlimit sign data. */
    uint32     pictureSize;             /*!< speedlimit data size. */
    NB_SpeedZoneType type;              /*!< speed zone types. */
} NB_SpeedLimitStateData;

/*! Type of speed warning
*/
typedef enum
{
    SP_Tone,                /*!< alert sound is a tone. */
    SP_Caution              /*!< play a "Caution" alert. */
} NB_SpeedWarningType;

/*! Speed Limit configure
*/
typedef struct NB_SpeedLimitConfiguration
{
    nb_boolean              displaySpeedLimit;   /*!< control speed limit icon ON or OFF. */
    nb_boolean              speedAlert;          /*!< play speed exceed warning ON or OFF. */
    nb_boolean              schoolAlert;         /*!< school zone warning ON or OFF. */
    NB_SpeedWarningType     speedWarningType;    /*!< speed exceed warning type. */
    double                  speedWarningLevel;   /*!< the warning level, if currentspeed-speedlimit > warninglevel then play alert. the
                                                  unit is m/s. */
    NB_SpeedWarningType     schoolWarningType;   /*!< school zone warning type. */
    nb_boolean              speedingDisplayAlert;/*!< show or not show speedlimit hightlight sign. */
}NB_SpeedLimitConfiguration;

#endif
/*! @} */

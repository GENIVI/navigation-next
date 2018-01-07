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
 @file     nbspecialregionprocessor.h
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

#ifndef NBSPECIALREGIONPROCESSOR_H
#define NBSPECIALREGIONPROCESSOR_H

#include "paltypes.h"

/*! struct of special region state data.
 */
typedef struct NB_SpecialRegionStateData
{
    nb_boolean  showSpecialRegion;        /*!< show or hide special region. */
    char*       regionType;               /*!< special region type. */
    char*       regionDescription;        /*!< special region description. */
    uint8*      imageData;                /*!< special region image data. */
    uint32      imageDataSize;            /*!< special region image data size. */
    uint32      startManeuverIndex;       /*!< start maneuver index of special region. */
    double      startManeuverOffset;      /*!< the offset distance from the begin of start maneuver. */
    uint32      endManeuverIndex;         /*!< end maneuver index of special region. */
    double      endManeuverOffset;        /*!< the offset distance from the begin of end maneuver. */
    uint32      nextOccurStartTime;       /*!< the next occur start time of special region. */
    uint32      nextOccurEndTime;         /*!< the next occur end time of special region. */
} NB_SpecialRegionStateData;


#endif
/*! @} */
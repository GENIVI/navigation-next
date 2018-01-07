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

    @file nbgmcommon.h
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBGM_COMMON_H_
#define _NBGM_COMMON_H_

#include "nbrecolor.h"
#include "palmath.h"
#include "nbremath.h"
#include "nbgm.h"

static double RADIUS_EARTH_METERS = 6378137.0;

inline float METER_TO_MERCATOR(float x);
inline float MERCATER_TO_METER(float x);
inline double METER_TO_MERCATOR(double x);
inline double MERCATER_TO_METER(double  x);

inline int8 CalcZoomLevel(float distance);
inline float CalcZoomLevelFloat(float distance);
inline float CalcDistanceFromZoomLevel(double zoomLevel);
inline float CalcPixelResolution(int8 zoomlevel);
inline nb_boolean CheckNearFarVisibility(float distance, float nearVisibility, float farVisibility);

inline float MM_TO_PIXEL(float mm, float dpi);

inline nb_boolean VERIFY_NBM_INDEX(uint16 index);
inline nb_boolean VERIFY_NBM_INDEX(uint32 index);
inline nb_boolean VERIFY_NBM_INTERNAL_INDEX(uint16 index);
inline uint16 MAKE_NBM_INDEX(uint16 index);

inline float DP_TO_PIXEL(float dp);
inline float PIXEL_TO_DP(float pixel);

inline NBRE_String GenerateID(const NBRE_String& categoryName, const NBRE_String& name);

inline float GetPolylineScaleFactor(int8 zoomLevel, nb_boolean isGEOPolyline);

#include "nbgmcommon.inl"
#endif

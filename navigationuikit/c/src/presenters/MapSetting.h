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
    @file         MapSetting.h
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
    --------------------------------------------------------------------------
*/

/*! @{ */

#ifndef navui_MapSetting_h
#define navui_MapSetting_h

#include "paltypes.h"
#include "palstdlib.h"
#include "NKUIMapDataType.h"

namespace nkui
{

enum NKUIZoomingToAllType
{
    NZTAT_None = 0
    , NZTAT_FitSingleRoute
    , NZTAT_FitMutipleRoute
    , NZTAT_FitPOI
};

enum NKUICameraAngleMode
{
    NCM_Remain = 0,     /** remain current mode */
    NCM_MapMode,        /** map mode */
    NCM_NavMode,        /** navigation mode */
    NCM_PedestrianMode, /** pedestrian mode */
};

enum NKUICameraTrackingMode
{
    NCTM_Remain = 0,            /** remain current mode */
    NCTM_LocationAndHeading,    /** camera tracks avatar location and heading */
    NCTM_Location,              /** camera tracks avatar location only */
    NCTM_Free                   /** camera moves free */
};

struct MapSetting
{
    NKUIAvatarMode          avatarMode;
    NKUIZoomingToAllType    zoomType;
    NKUINavigationMode      polylineType;
    NKUICameraAngleMode     cameraMode;
    NKUICameraTrackingMode  trackingMode;
    bool                    useFixedGPS;
    bool                    showManeuverArrow;

    bool operator == (const MapSetting& mapSetting) const
    {
        return nsl_memcmp(this, &mapSetting, sizeof(MapSetting)) == 0;
    }

    bool operator != (const MapSetting& mapSetting) const
    {
        return !(*this == mapSetting);
    }
};

}

#endif

/*! @} */

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

    @file nbgmnavdata.h
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

#ifndef _NBGM_NAV_DATA_H_
#define _NBGM_NAV_DATA_H_
#include "nbgmtypes.h"
#include "nbgmavatarsetting.h"
#include <string>
#include <vector>

/*! \addtogroup NBGM_Manager
*  @{
*/

/*! Poi for navigation
*/
struct NBGM_Poi
{
    std::string             poiId;          /*!< poi id */
    NBGM_Point3d64          position;       /*!< poi position*/
    std::string             image;          /*!< poi image path*/
};

enum NBGM_NavCameraUsage
{
    NBGM_NCU_ECM_MAX_PORTRAIT,
    NBGM_NCU_ECM_MAX_LANDSCAPE,
    NBGM_NCU_ECM_MIDDLE_PORTRAIT,
    NBGM_NCU_ECM_MIDDLE_LANDSCAPE,
    NBGM_NCU_ECM_MIN_PORTRAIT,
    NBGM_NCU_ECM_MIN_LANDSCAPE,
    NBGM_NCU_MJO_PORTRAIT,
    NBGM_NCU_MJO_LANDSCAPE,
    NBGM_NCU_VECTOR_3D_PORTRAIT,
    NBGM_NCU_VECTOR_3D_LANDSCAPE,
    NBGM_NCU_VECTOR_2D_PORTRAIT,
    NBGM_NCU_VECTOR_2D_LANDSCAPE,
    NBGM_NCU_LOOK_AHEAD_PORTRAIT,
    NBGM_NCU_LOOK_AHEAD_LANDSCAPE,
    NBGM_NCU_START_UP_PORTRAIT,
    NBGM_NCU_START_UP_LANDSCAPE
};

enum NBGM_CameraProjectionType
{
    NBGM_CPT_PERSPECTIVE,
    NBGM_CPT_ORTHO
};

struct NBGM_NavCameraSetting
{
    float                       cameraHeight;
    float                       cameraToAvatar;
    float                       avatarToHorizon;
    float                       hFov;
    float                       avatarToBottom;
    float                       avatarScale;
    NBGM_CameraProjectionType   projectionType;
};

enum NBGM_NavViewMode
{
    NBGM_NVM_START,
    NBGM_NVM_ARRIVAL,
    NBGM_NVM_MANEUVER,
    NBGM_NVM_NAV
};

enum NBGM_NavViewWorkMode
{
    NBGM_NVWM_STANDARD, /*!< Standard mode, show vector tile, such as BR, DVR,
                        DVA, LM3D, B3D, etc. */
    NBGM_NVWM_ENHANCED  /*!< Enhanced mode, show ECM, MJO, etc. */
};

/*! nbgm spline binary data

This struct defined a spline binary data
*/

struct NBGM_SplineData
{
    uint8* data;                 /*!< Pointer to a data buffer */
    uint32 size;                 /*!< Buffer size */
};


struct NBGM_EcmManeuver
{
    uint32        id;
    NBGM_Point3d  position;
};

struct NBGM_VectorRouteData
{
    float*          data;
    double          offset[2];
    uint32          count;
    uint32          maneuverID;
};

struct NBGM_VectorNavData
{
    ///loc
    NBGM_Location64                     avatarLocation;
    float                               viewHeading;
    ///route
    std::vector<NBGM_VectorRouteData*>  routePolyline;
    ///flag
    NBGM_Location64                     startFlagLoc;
    NBGM_Location64                     endFlagLoc;
};

struct NBGM_EcmNavData
{
    ///loc
    NBGM_Location64                     avatarLocation;
    float                               viewHeading;
    ///route
    std::vector<NBGM_SplineData*>       splines;
    std::vector<NBGM_EcmManeuver>       maneuvers;

    NBGM_Location64                     startFlagLoc;
    NBGM_Location64                     endFlagLoc;
};

enum NBGM_NavDataField
{
    NBGM_NDF_LOCATION           = 0x001,
    NBGM_NDF_ECM_ROUTE          = 0x002,
    NBGM_NDF_VECTOR_ROUTE       = 0x004,
    NBGM_NDF_POI                = 0x008,
    NBGM_NDF_CURRENT_MANEUVER   = 0x010,
    NBGM_NDF_RESET_VECTOR_ROUTE = 0x020,
    NBGM_NDF_START_FLAG         = 0x040,
    NBGM_NDF_END_FLAG           = 0x080,
    NBGM_NDF_AVATAR_MODE_FLAG   = 0x100,
    NBGM_NDF_ALL                = 0x0ff,
};

struct NBGM_NavData
{
    NBGM_VectorNavData                  vectorNavData;
    NBGM_EcmNavData                     ecmNavData;

    NBGM_NavViewMode                    viewMode;
    NBGM_AvatarMode                     avatarMode;
    ///current maneuver
    uint32                              currentManeuver;
    ///poi list
    std::vector<NBGM_Poi*>              poiList;

    uint32                              mChangedFlag;
};

/*! @} */
#endif

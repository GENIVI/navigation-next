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

    @file nbgmcamerasetting.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBGM_CAMERA_STATE_H_
#define _NBGM_CAMERA_STATE_H_
#include "nbretypes.h"


/*! \addtogroup NBGM_Manager
*  @{
*/

typedef enum
{
    NBGM_RS_NAV_MAINVIEW,
    NBGM_RS_MANUEVER_LOOKAHEAD
}NBGM_RenderState;

typedef enum
{
    NBGM_TS_ON_ROUTE,
    NBGM_TS_START_UP,
    NBGM_TS_OFF_ROUTE
}NBGM_TripState;

typedef enum
{
    NBGM_RC_MJO,
    NBGM_RC_VECTOR,
    NBGM_RC_ECM
}NBGM_RenderContent;

typedef enum
{
    NBGM_PT_PERSPECTIVE_2D,
    NBGM_PT_PERSPECTIVE_3D
}NBGM_PerspectiveType;

typedef enum
{
    NBGM_EZL_MAX,
    NBGM_EZL_MIDDLE,
    MBGM_EZL_MIN
}NBGM_ECMZoomLevel;

typedef enum
{
    NBGM_RT_VECTOR,
    NBGM_RT_ECM
}NBGM_RendererType;


struct NBGM_CameraState
{
    nb_boolean              isLandscape;
    nb_boolean              isNeedUpdate;
    nb_boolean              isAvatarBlocked;
    NBGM_RenderState        renderState;
    NBGM_TripState          tripState;
    NBGM_RenderContent      renderContent;
    NBGM_PerspectiveType    perspectiveType;
    NBGM_ECMZoomLevel       zoomLevel;
};

/*! @} */

#endif



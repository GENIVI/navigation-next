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

    @file nbgmcompass.h
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
#ifndef _NBGM_COMPASS_H_
#define _NBGM_COMPASS_H_

#include "nbrenode.h"
#include "nbreentity.h"
#include "nbgmdefaultoverlaymanager.h"
#include "nbgmdraworder.h"

struct NBGM_Context;
class NBRE_Overlay;
class NBRE_SurfaceSubView;

/*! \addtogroup NBGM_Service
*  @{
*/

/** A compass is displayed on the top of the map for indicating the direction.
*/
class NBGM_Compass
{
public:
    NBGM_Compass(NBGM_Context& nbgmContext, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, const DrawOrderStruct& drawOrder);
    virtual ~NBGM_Compass();

public:
    void OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h);
    void SetPosition(float screenX, float screenY);
    void Rotate(float angle);
    void SetRotateAngle(float angle);
    void SetEnable(nb_boolean value);
    nb_boolean IsTapped(float screenX, float screenY);
    void Update();

    void SetDayNightMode(nb_boolean isDayMode);
    void UpdateIcons(const NBRE_String& dayIcon, const NBRE_String& nightIcon);
    NBRE_AxisAlignedBox2f GetBoundingBox() const;
    void UpdateDPI();

private:
    void GetDefaultIconPath(NBRE_String& dayIconPath, NBRE_String& nightIconPath, NBRE_String& shadowPath);
    NBRE_ShaderPtr CreateShader(const NBRE_String& icon, NBRE_Vector2f& iconSize);
    NBRE_EntityPtr CreateEntity(NBRE_ShaderPtr shader, const NBRE_Vector2f& size);

    void UpdateBoundingBox2D(const NBRE_Vector2f& size, NBRE_AxisAlignedBox2f& box);
    nb_boolean IsNorth();
    void UpdateIcons();

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_Compass);

private:
    NBGM_Context&           mNBGMContext;
    NBRE_SurfaceSubView*    mSubView;
    NBRE_DefaultOverlayManager<DrawOrderStruct>& mOverlayManager;
    NBRE_CameraPtr          mCamera;
    NBRE_Overlay*           mOverlay;
    int32                   mOverlayID;
    NBRE_NodePtr            mNode;

    NBRE_EntityPtr          mDayEntity;
    NBRE_Vector2f           mDayIconSize;
    NBRE_AxisAlignedBox2f   mDayBoundingBox;

    NBRE_EntityPtr          mNightEntity;
    NBRE_Vector2f           mNightIconSize;
    NBRE_AxisAlignedBox2f   mNightBoundingBox;

    NBRE_NodePtr            mShadowNode;
    NBRE_EntityPtr          mShadowEntity;

    nb_boolean              mIsDefault;
    nb_boolean              mEnable;
    nb_boolean              mViewPortUpdated;
    nb_boolean              mIsDayMode;
    nb_boolean              mUpdateIcons;
    nb_boolean              mVisible;
    NBRE_OrthoConfig        mConfig;

    NBRE_Point3d            mPosition;
    float                   mRotateAngle;

    NBRE_String             mDayIconName;
    NBRE_String             mNightIconName;
};

/*! @} */

#endif

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

    @file nbgmcircleimpl.h
*/
/*
    (C) Copyright 2014 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBGM_CIRCLE_IMPL_H_
#define _NBGM_CIRCLE_IMPL_H_
#include "nbgmcircle.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "nbreentity.h"
#include "nbreshader.h"
#include "nbgmcontext.h"
#include "nbgmcommon.h"
#include "nbresurfacesubview.h"
#include "nbgmmaplayer.h"
#include "nbgmdefaultoverlaymanager.h"

class NBRE_SceneManager;
class NBRE_SurfaceSubView;

/*! \addtogroup NBGM_Service
*  @{
*/

/** A label displayed in the center of anchor point.
such as area & building labels.
*/
class NBGM_CircleImpl: public NBGM_Circle
{
public:
    NBGM_CircleImpl(NBGM_Context& nbgmContext, NBRE_SceneManager* sceneManager, NBRE_SurfaceSubView* subview, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, DrawOrderStruct drawOrder, int circleId, const NBGM_CircleParameters &circlePara);
    virtual ~NBGM_CircleImpl();

public:
    /// Update circle by camera
    void Update(double pixelPerUnit);

    /// Add to sence
    void AddToSence();

    /// Notify extend
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);

public:
    /// Set circle visible
    void SetVisible(bool visible);

    /// Set circle center
    void SetCenter(double mercatorX, double mercatorY);

    /// Set circle radius, fill color, outline color
    void SetStyle(float radius, const NBGM_Color& fillColor, const NBGM_Color& outlineColor);

    /// Get circle id
    int ID() const;

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_CircleImpl);

private:
    NBGM_Context& mNBGMContext;
    NBRE_SceneManager*  mSceneManager;
    NBRE_SurfaceSubView* mSubView;
    NBRE_DefaultOverlayManager<DrawOrderStruct>& mOverlayManager;
    NBGM_TileMapBoundaryExtendType mExtendType;
    uint32              mOverlayID;
    NBRE_NodePtr        mParentNode;
    NBRE_NodePtr        mCircleNode;
    NBRE_NodePtr        mExtendCircleNode;
    NBRE_EntityPtr      mCircleEntity;
    NBRE_EntityPtr      mExtendCircleEntity;
    NBRE_ShaderPtr      mFillShader;
    NBRE_ShaderPtr      mOutlineShader;
    float               mModelScale;
    nb_boolean          mNeedShaderUpdate;
    NBRE_Vector2d       mCenter;
    float               mRadius;
    NBRE_Color          mFillClr;
    NBRE_Color          mOutlineClr;
    nb_boolean          mOutlineEnabled;
    int                 mZOrder;
    bool                mVisible;
    int                 mCircleId;
};
typedef shared_ptr<NBGM_CircleImpl> NBGM_CircleImplPtr;
/*! @} */
#endif

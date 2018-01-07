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

    @file nbgmrect2dimpl.h
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

#ifndef _NBGM_RECT2D_IMPL_H_
#define _NBGM_RECT2D_IMPL_H_
#include "nbgmcustomrect2d.h"
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
class NBGM_CustomObjectsContext;

/*! \addtogroup NBGM_Service
*  @{
*/

class NBGM_Rect2dImpl: public NBGM_CustomRect2d
{
public:
    NBGM_Rect2dImpl(NBGM_Context& nbgmContext, NBRE_SceneManager* sceneManager, NBRE_SurfaceSubView* subview, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, DrawOrderStruct drawOrder, NBGM_CustomObjectsContext& customContext, NBGM_Rect2dId rectId, const NBGM_Rect2dParameters &circlePara);
    virtual ~NBGM_Rect2dImpl();

public:
    /// Update circle by camera
    void Update(double pixelPerUnit);

    /// Add to sence
    void AddToSence();

    /// Notify extend
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);

public:
    /// Set Rect2D center and heading angle
    void Update(double mercatorX, double mercatorY, float heading);

    /// Set Rect2D visible
    void SetVisible(bool visible);

    /// Set Rect2d visible
    void SetSize(float width, float height);

    /// Set Rect2d texture
    void SetTexture(const NBGM_BinaryBuffer& buffer);

    /// Set Rect2d texture
    void SetTexture(NBGM_TextureId textureId);

    /// Get Rect2D id
    NBGM_Rect2dId ID() const;

private:
    void InitializeShader(const NBGM_Rect2dParameters &para);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_Rect2dImpl);

private:
    NBGM_Context& mNBGMContext;
    NBRE_SceneManager*  mSceneManager;
    NBRE_SurfaceSubView* mSubView;
    NBRE_DefaultOverlayManager<DrawOrderStruct>& mOverlayManager;
    NBGM_TileMapBoundaryExtendType mExtendType;
    NBGM_CustomObjectsContext&  mCustomContext;
    uint32              mOverlayID;
    NBRE_NodePtr        mParentNode;
    NBRE_NodePtr        mRectNode;
    NBRE_NodePtr        mExtendRectNode;
    NBRE_EntityPtr      mRectEntity;
    NBRE_EntityPtr      mExtendRectEntity;
    NBRE_ShaderPtr      mShader;
    nb_boolean          mNeedShaderUpdate;
    nb_boolean          mPosChanged;
    nb_boolean          mHeadingChanged;
    NBRE_Vector2d       mCenter;
    float               mWidth;
    float               mHeight;
    float               mHeading;
    bool                mVisible;
    NBGM_Rect2dId        mRectId;
};
typedef shared_ptr<NBGM_Rect2dImpl> NBGM_Rect2dImplPtr;
/*! @} */
#endif

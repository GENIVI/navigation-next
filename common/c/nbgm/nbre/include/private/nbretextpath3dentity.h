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

    @file nbreentity.h
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
#ifndef _NBRE_TEXT_PATH_3D_ENTITY_H_
#define _NBRE_TEXT_PATH_3D_ENTITY_H_
#include "nbretypes.h"
#include "nbremovableobject.h"
#include "nbrespatialobject.h"
#include "nbrecontext.h"
#include "nbretextentity.h"

class NBRE_Overlay;
class NBRE_TransformUtil;

/*! \addtogroup NBRE_Scene
*  @{
*/

/** A 3d version of label along a polyline path.
*/
class NBRE_TextPath3dEntity: public NBRE_IMovableObject, 
                   public NBRE_IRenderOperationProvider, 
                   public NBRE_ISpatialObject
{
public:
    NBRE_TextPath3dEntity(NBRE_Context& context, NBRE_TransformUtil* transUtil, int32 overlayId, const NBRE_WString& text, 
        const NBRE_FontMaterial& fontMaterial, float fontHeight, NBRE_Polyline2f* polyline, float offset);
    ~NBRE_TextPath3dEntity();

public:
    ///Derive From NBRE_IMovableObject
    virtual void NotifyAttached(NBRE_Node* node);
    virtual void NotifyDetatch();
    virtual void NotifyUpdatePosition();
    virtual NBRE_ISpatialObject* SpatialObject();
    virtual void Accept(NBRE_Visitor& visitor);

    ///From NBRE_IRenderOperationProvider
    virtual void NotifyCamera(const NBRE_Camera& camera);
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay);
    virtual int32 OverlayId() const;
    virtual nb_boolean Visible() const;    
    virtual void UpdateSurfaceList();
    virtual uint32 GetPriority() const { return 0; }

    ///From NBRE_ISpatialObject
    virtual const NBRE_AxisAlignedBox3d& AxisAlignedBox()const;

    void SetVisible(nb_boolean visable);
    void Rebuild();
    void Rebuild(float fontHeight, nb_boolean isRevert, NBRE_Polyline2f* polyline, float offset);
    void SetColor(const NBRE_Color& color);
    void SetFontMaterial(const NBRE_FontMaterial& fontMaterial);
    const NBRE_Vector2f* GetLayoutEndPoints();
    nb_boolean IsRevert();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_TextPath3dEntity);

private:
    NBRE_Context&           mContext;
    NBRE_TransformUtil*     mTransUtil;
    nb_boolean              mVisible;
    int32                   mOverlayId;
    /// entity world bounding box
    NBRE_AxisAlignedBox3d   mBoundingBox;
    /// attached node
    NBRE_Node*              mParentNode;
    NBRE_TextPathEntity*    mTextEntity;
    NBRE_Vector2d           mTextDirection;
    nb_boolean              mIsRevert;
    NBRE_Polyline2f*        mPolyline;
    float                   mFontHeight;
};

typedef shared_ptr<NBRE_TextPath3dEntity> NBRE_TextPath3dEntityPtr;
/*! @} */

#endif

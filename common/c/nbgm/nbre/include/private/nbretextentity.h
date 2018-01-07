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

    @file nbretextentity.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_TEXT_ENTITY_H_
#define _NBRE_TEXT_ENTITY_H_
#include "nbretypes.h"
#include "nbrevector2.h"
#include "nbrefontmaterial.h"
#include "nbrepolyline2.h"
#include "nbrepass.h"
#include "nbrevisitor.h"
#include "nbrefont.h"
#include "nbrecontext.h"
#include "nbrepolyline3.h"
#include "palfont.h"

class NBRE_Overlay;
class NBRE_VertexData;
class NBRE_IndexData;
/*! \addtogroup NBRE_Scene
*  @{
*/

struct TextEntityVertex
{
    NBRE_Vector3f position;
    NBRE_Vector2f texcoord;
    NBRE_Color color;
};

/** Text entity base class.
*/
class NBRE_TextEntity:
    public NBRE_IRenderOperationProvider
{   
    typedef NBRE_Vector<wchar> NBRE_CharList;
public:
    NBRE_TextEntity(NBRE_Context& context, const NBRE_WString& text, const NBRE_FontMaterial& fontMaterial, float fontHeight);
    virtual ~NBRE_TextEntity();

public:
    static float GetStringWidth(NBRE_Context& contex, const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_WString& text);

    /// Rebuild vertex buffer
    virtual void UpdateVertexBuffer() = 0;

    /// provider a overlay Id
    virtual int32 OverlayId() const;
    /// update global font render queue
    virtual void UpdateFontRenderQueue();
    /// update texture coordinates after font render queue update
    virtual void UpdateFontTexture();

    // NBRE_IRenderOperationProvider members
    virtual void NotifyCamera(const NBRE_Camera& camera);
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay);
    virtual nb_boolean Visible() const;
    virtual void UpdateSurfaceList(){}
    virtual uint32 GetPriority() const { return 0; }

    /// Set visible
    void SetVisible(nb_boolean value) { mVisible = value; }
    /// Set color
    void SetColor(const NBRE_Color& color);
    /// Get color
    const NBRE_Color& GetColor() const;
    /// Set transformation
    virtual void SetTransform(const NBRE_Matrix4x4d& transform);
    /// Get bounding box
    const NBRE_AxisAlignedBox3f& AxisAlignedBox() const;
    /// Get pass
    NBRE_PassPtr GetPass() { return mPassCache; }


private:
    DISABLE_COPY_AND_ASSIGN(NBRE_TextEntity);

protected:
    /// render engine context
    NBRE_Context& mContext;
    /// color
    NBRE_Color mColor;
    /// text
    NBRE_WString mText;
    /// font
    NBRE_FontMaterial mFontMaterial;
    /// font height
    float mFontHeight;
    /// vertex data
    NBRE_VertexData* mVertexData;
    /// index data
    NBRE_IndexData* mIndexData;
    /// pass
    NBRE_PassPtr mPassCache;
    /// is color changed
    nb_boolean mNeedUpdateColor;
    /// transformation
    NBRE_Matrix4x4d mTransform;
    /// bounding box
    NBRE_AxisAlignedBox3f mAABB;
    /// is text visible
    nb_boolean mVisible;
    /// Is vertexBufferChanged
    nb_boolean mNeedUpdateVertex;
};

/** A string displayed along a polyline.
*/
class NBRE_TextPathEntity:
    public NBRE_TextEntity
{
public:
    NBRE_TextPathEntity(NBRE_Context& context, const NBRE_WString& text, const NBRE_FontMaterial& fontMaterial, float fontHeight
        , NBRE_Polyline2f* polyline, float offset);
    ~NBRE_TextPathEntity();

public:
    /// Rebuild vertex buffer
    void Rebuild(float fontHeight, nb_boolean isRevert, NBRE_Polyline2f* polyline, float offset);
    /// Rebuild vertex buffer
    virtual void UpdateVertexBuffer();
    /// Set transformation
    virtual void SetTransform(const NBRE_Matrix4x4d& transform);
    /// Is text direction revert
    nb_boolean IsRevert() { return mIsRevert; }
    /// Set is text direction reverted
    void SetRevert(nb_boolean value) { mIsRevert = value; }
    /// Get text end points
    NBRE_Vector2f* GetLayoutEndPoints() { return mLayoutEndPoints; }
    /// Change font material
    void SetFontMaterial(const NBRE_FontMaterial& fontMaterial);
    /// Update rotate angle
    void UpdateTextRotate(const NBRE_Vector3d& eyePosition, const NBRE_Vector3d& cameraRight, float tilt);
private:
    /// Calculate text direction
    void UpdateTextDirection();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_TextPathEntity);

private:
    /// the polyline where text displayed
    NBRE_Polyline2f* mPolyline;
    /// offset along the polyine from the anchor point
    float mOffset;
    /// is polyline revert
    nb_boolean mIsRevert;
    /// text direction, for avoid dithering
    NBRE_Vector2f mLayoutEndPoints[2];
    /// is rotate enabled
    nb_boolean mRotateEnable;
    /// rotate center
    NBRE_Vector2f mRotateCenter;
    /// rotate axis
    NBRE_Vector2f mRotateAxis;
    /// local transform
    NBRE_Matrix4x4d mLocalTransform;
    /// world transform
    NBRE_Matrix4x4d mWorldTransform;
};
typedef shared_ptr<NBRE_TextPathEntity> NBRE_TextPathEntityPtr;

/** A string centered at a point.
*/
class NBRE_TextPointEntity:
    public NBRE_TextEntity
{
public:
    NBRE_TextPointEntity(NBRE_Context& context, const NBRE_WString& text, const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_Vector2f& position, PAL_TEXT_ALIGN align);
    virtual ~NBRE_TextPointEntity();

public:
    /// Rebuild vertex buffer
    virtual void UpdateVertexBuffer();
    /// Force rebuild
    void Rebuild();
    /// Change font material
    void SetFontMaterial(const NBRE_FontMaterial& fontMaterial);

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_TextPointEntity);

private:
    /// the position where text displayed
    NBRE_Vector2f mPosition;
    /// alignment
    PAL_TEXT_ALIGN mAlign;
};
typedef shared_ptr<NBRE_TextPointEntity> NBRE_TextPointEntityPtr;

/*! @} */
#endif

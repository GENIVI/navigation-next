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

    @file nbgmcustompin.h
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

#ifndef _NBGM_CUSTOM_PIN_H_
#define _NBGM_CUSTOM_PIN_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "nbrebillboardnode.h"
#include "nbgmlayoutmanager.h"
#include "nbgmcontext.h"
#include "nbreentity.h"
#include "nbgmnbmdata.h"

/*! \addtogroup NBGM_Service
*  @{
*/

/** Custom pin represent a clickable icon in custom layer.
*/
class NBGM_CustomPin:
    public NBGM_LayoutElement
{
public:
    NBGM_CustomPin(NBGM_Context& nbgmContext,
              NBRE_Node* parentNode,
              NBRE_SurfaceSubView* subView, 
              int32 overlayId,
              const NBRE_Vector2f& size,
              NBRE_ShaderPtr shader,
              NBRE_ShaderPtr selectedShader,
              NBRE_ShaderPtr unselectedShader,
              const NBRE_Vector3d& position,
              const NBRE_String& id,
              const NBRE_Vector2f& imageOffset,
              const NBRE_Vector2f& bubbleOffset,
              float distanceToPoi,
              const NBRE_String& layerId,
              nb_boolean enableCollisionDetection);
    virtual ~NBGM_CustomPin();

public:
    /// Is element visible
    virtual nb_boolean IsElementVisible();
    /// Update the region element takes
    virtual void UpdateRegion();
    /// Is element collide with buffer
    virtual nb_boolean IsElementRegionAvailable();
    /// Get position in world space
    virtual NBRE_Vector3d GetPosition();
    /// Set priority
    void SetPriority(int32 value) { mPriority = value; }
    /// Implement this to provide fade in/out effect
    virtual void SetColor(const NBRE_Color& color);
    /// Add to Scene for rendering
    virtual void AddToSence();
    /// Set element visibility
    virtual void SetVisible(nb_boolean value);
    /// Get clone element
    NBGM_LayoutElement* Clone(NBRE_Node* parentNode);
    /// Get icon id
    const NBRE_String& GetId() const { return mId; }
    /// Is icon selected
    nb_boolean GetSelected() const { return mSelected; }
    /// Set icon select state
    void SetSelected(nb_boolean val);
    /// Get icon screen position, if success, return TRUE and set position,
	/// else return FALSE
    nb_boolean GetScreenPosition(NBRE_Vector2f& position);
    /// Get icon bubble screen position for this icon, if success,
	/// return TRUE and set position, else return FALSE
    nb_boolean GetScreenBubblePosition(NBRE_Vector2f& position);
    /// return bubble offset from anchor points, in pixels
    NBRE_Vector2f GetScreenBubbleOffset();
    /// Is icon at point
    nb_boolean HitTest(const NBRE_Vector2f& screenPosition);
    /// Is element too close
    nb_boolean IsTooCloseTo(NBGM_LayoutElement* elem);
    /// Update icon scale
    virtual void OnUpdate();
    /// Get minimum distance to poi
    float GetDistanceToPoi() const { return mDistanceToPoi; }
    /// Get layer id
    const NBRE_String& GetLayerId() const { return mLayerId; }
    /// Enable or disable collision detection
    void EnableCollsionDetection(nb_boolean value);

private:
    /// Adjust text scale to font min/max range
    double UpdateScale(double pixelsPerUnit);
    /// Update screen rect
    void UpdateScreenRect(const NBRE_Vector2d& screenPosition, double pixelsPerUnit);
    

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_CustomPin);
    friend class NBGM_CustomPiniLabel;

private:
    /// surface view
    NBRE_SurfaceSubView* mSubView;
    /// overlay id
    int32 mOverlayId;
    /// Pixel size
    NBRE_Vector2f mSize;
    /// Text node
    NBRE_BillboardNodePtr mNode;
    /// World position
    NBRE_Vector3d mPosition;
    /// Is label visible
    nb_boolean mIsInFrustum;
    /// Scale for size limit
    double mScale;
    /// Icon id
    NBRE_String mId;
    /// Is selected
    nb_boolean mSelected;
    /// parent node
    NBRE_Node* mParentNode;
    /// Shader
    NBRE_ShaderPtr mShader;
    /// Shader when selected
    NBRE_ShaderPtr mSelectedShader;
    /// Shader when not selected
    NBRE_ShaderPtr mUnselectedShader;
    /// Base size
    double mBaseSize;
    /// Offset(%)
    NBRE_Vector2f mImageOffset;
    /// bubble offset(%)
    NBRE_Vector2f mBubbleOffset;
    /// space between elements
    float mDistanceToPoi;
    /// Layer id
    NBRE_String mLayerId;
    /// Enable collision detection
    nb_boolean mEnableCollisionDetection;
};
typedef shared_ptr<NBGM_CustomPin> NBGM_CustomPinPtr;

/*! @} */
#endif

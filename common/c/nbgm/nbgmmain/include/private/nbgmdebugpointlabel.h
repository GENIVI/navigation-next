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

    @file nbgmdebugpointlabel.h
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

#ifndef _NBGM_DEBUG_POINT_LABEL_H_
#define _NBGM_DEBUG_POINT_LABEL_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "nbrebillboardnode.h"
#include "nbgmlayoutmanager.h"

/*! \addtogroup NBGM_Service
*  @{
*/

/** A label displayed in the center of anchor point. For debug purpose only.
*/
class NBGM_DebugPointLabel:
    public NBGM_LayoutElement
{
public:
    NBGM_DebugPointLabel(NBGM_Context& nbgmContext, NBRE_Node* parentNode, NBRE_BillboardSet* billboardSet, const NBRE_WString& text, const NBRE_FontMaterial& material, float fontHeight, const NBRE_Vector3d& position);
    virtual ~NBGM_DebugPointLabel();

public:
    /// Is element visible
    virtual nb_boolean IsElementVisible();
    /// Update the region element takes
    virtual void UpdateRegion();
    /// Is element collide with buffer
    virtual nb_boolean IsElementRegionAvailable();
    /// Priority, less value has higher priority
    virtual int32 GetLayoutPriority() const;
    /// Get position in world space
    virtual NBRE_Vector3d GetPosition();
    /// Set priority
    void SetLayoutPriority(int32 value) { mPriority = value; }
    /// Add to Scene for rendering
    virtual void AddToScene();
    /// Set element visibility
    virtual void SetVisible(nb_boolean value);
    /// Refresh material
    virtual void RefreshFontMaterial();
    /// Append Text
    void AppendText(const NBRE_WString& text);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_DebugPointLabel);

private:
    /// Text node container
    NBRE_BillboardSet* mBillboardSet;
    /// Text node
    NBRE_BillboardNodePtr mNode;
    /// World position
    NBRE_Vector3d mPosition;
    /// Font height
    float mFontHeight;
    /// Font material
    NBRE_FontMaterial mFontMaterial;
    /// Priority
    int32 mPriority;
    /// Scale for size limit
    double mScale;
    /// parent node
    NBRE_Node* mParentNode;
    /// is visible
    nb_boolean mVisible;
    /// Text entity
    NBRE_TextAreaEntity* mTextAreaEntity;
    /// Need update text
    nb_boolean mNeedUpdate;
    /// Is init done
    nb_boolean mInitDone;
};

/*! @} */
#endif

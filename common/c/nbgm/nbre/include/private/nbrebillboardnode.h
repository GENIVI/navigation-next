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

    @file nbreentitynode.h
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
#ifndef _NBRE_BILLBOARD_NODE_H_
#define _NBRE_BILLBOARD_NODE_H_
#include "nbrenode.h"
#include "nbrevisitor.h"
#include "nbreaxisalignedbox2.h"
#include "nbretexture.h"
#include "nbrevector3.h"
#include "nbreshader.h"
#include "nbremath.h"
#include "nbrefontmaterial.h"
#include "nbretypes.h"
#include "nbrebillboardentity.h"
#include "nbrespatialobject.h"

/*! \addtogroup NBRE_Scene
*  @{
*/

/// Use this class to add billboard to scene
class NBRE_BillboardNode: public NBRE_Node
{
public:
    NBRE_BillboardNode():
        mSquaredDistanceToEye(0),
        mVisible(TRUE),
        mDrawIndex(0)
        {
        }
    virtual ~NBRE_BillboardNode() {}

public:
    // NBRE_Node members
    virtual void UpdatePosition();

    ///From NBRE_ISpatialObject
    virtual const NBRE_AxisAlignedBox3d& AxisAlignedBox()const;

    /// Update vertex cache and z order
    void Update(const NBRE_Camera& camera);
    /// Update render queue
    void UpdateRenderQueue(NBRE_Overlay& overlay);
    /// Add billboard entity
    void AddEntity(NBRE_BillboardEntityPtr entity);
    /// Remove billboard entity
    void RemoveEntity(NBRE_BillboardEntityPtr entity);
    /// Compare by z order and shader
    static int CompareBillboardsByZOrder(const void* v1, const void* v2);
    /// Get billboard elements
    NBRE_BillboardEntityList& GetEntities() { return mEntities;}
    /// Set visiblity
    void SetVisible(nb_boolean value) { mVisible = value; }
    /// Get visibility
    nb_boolean IsVisible() const { return mVisible; }
    /// Set color
    void SetColor(const NBRE_Color& color);
    /// Set drawIndex, lesser value draw firstly
    void SetDrawIndex(int32 value) { mDrawIndex = value; }
    /// Get drawIndex
    int32 GetDrawIndex() const { return mDrawIndex; }

private:
    /// node AABB
    NBRE_AxisAlignedBox3d mBoundingBox;
    /// sort key
    double mSquaredDistanceToEye;
    /// entity list
    NBRE_BillboardEntityList mEntities;
    /// model view matrix cancel rotation
    NBRE_Matrix4x4d mModelViewMatrix;
    /// is billboard visible
    nb_boolean mVisible;
    /// draw index
    int32 mDrawIndex;
};

typedef shared_ptr<NBRE_BillboardNode> NBRE_BillboardNodePtr;
typedef NBRE_Vector<NBRE_BillboardNodePtr> NBRE_BillboardNodeArray;

class NBRE_BillboardNodeLessThan
{
public:
    bool operator() (const NBRE_BillboardNodePtr& n1, const NBRE_BillboardNodePtr& n2) const
    {
        return (n1->GetDrawIndex() < n2->GetDrawIndex());
    }
};

/// Every billboard in a overlay will be collected into a single billboard set to render
class NBRE_BillboardSet: public NBRE_IVisitable, public NBRE_IRenderOperationProvider
{
public:
    explicit NBRE_BillboardSet(NBRE_Context& context, int32 overlayId);
    ~NBRE_BillboardSet();

public:
    // NBRE_IVisitable members
    virtual void Accept(NBRE_Visitor& visitor);

    // NBRE_IRenderOperationProvider members
    virtual void NotifyCamera(const NBRE_Camera& camera);
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay);
    virtual int32 OverlayId() const;
    virtual nb_boolean Visible() const;
    virtual void UpdateSurfaceList() {};
    virtual uint32 GetPriority() const { return 0xFFFFFFFF; }

    /// Add a billboard
    NBRE_BillboardNodePtr AddBillboard();
    /// Remove a billboard
    void RemoveBillboard(NBRE_BillboardNodePtr value);

    /// True for sorting by Z order, otherwise False.
    void EnableZOrder(nb_boolean enabled) { mEnableZOrder = enabled; }

private:
    void FindVisibleSet(const NBRE_Camera& camera);

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_BillboardSet);

private:
    NBRE_Context& mContext;
    uint32 mOverlayId;
    NBRE_BillboardNodeArray mBillboards;
    NBRE_BillboardNodeArray mVisibleSet;
    NBRE_AxisAlignedBox3f mAABB;
    nb_boolean mEnableZOrder;
};

/*! @} */
#endif

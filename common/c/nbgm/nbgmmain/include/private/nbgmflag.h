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

    @file nbgmflag.h
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

#ifndef _NBGM_FLAG_H_
#define _NBGM_FLAG_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "nbreentity.h"
#include "nbgmlayoutmanager.h"
#include "nbreshader.h"
#include "nbgmcontext.h"
#include "nbgmmaplayer.h"

class NBRE_SceneManager;

/*! \addtogroup NBGM_Service
*  @{
*/

/** A flag of 2D & 3D modes.
*/
class NBGM_Flag:
    public NBGM_LayoutElement
{
public:
    NBGM_Flag(NBGM_Context& nbgmContext, NBRE_SceneManager* sceneManager, int32 overlayId, nb_boolean isEndFlag);
    virtual ~NBGM_Flag();

public:
    /// Add to Scene for rendering
    virtual void AddToSence();
    /// Is element visible
    virtual nb_boolean IsElementVisible();
    /// Update the region element takes
    virtual void UpdateRegion();
    /// Is element collide with buffer
    virtual nb_boolean IsElementRegionAvailable();
    /// Get position in world space
    virtual NBRE_Vector3d GetPosition();

    /// Set flag unit
    void AdjustSize(float size, float minPixelSize, float maxPixelSize);
    /// Set flag location
    void SetPosition(const NBRE_Vector3d& position);
    /// Get screen polygon projected from flag
    void GetPolygon(Polygon2f polygons[]);

    NBRE_AxisAlignedBox3d AxisAlignedBox() const;
    NBRE_NoneAxisAlignedBox3d ObjectAlignedBox() const;

    nb_boolean IsScreenPointAvailable();

    /// Update flag by parameters
    void Update();

    void Hide();

    /// Notify flag extend
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);

private:
    nb_boolean IsExtendNodeInFrustum() const;

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_Flag);

private:
    NBRE_SceneManager*  mSceneManager;
    uint32              mOverlayID;

    NBRE_NodePtr        mParentNode;
    NBRE_NodePtr        mNode;
    NBRE_NodePtr        mExtendNode;
    NBRE_EntityPtr      m2DEntity;
    NBRE_EntityPtr      mExtend2DEntity;
    NBRE_EntityPtr      m3DEntity;
    NBRE_EntityPtr      mExtend3DEntity;
    NBRE_ShaderPtr      m2DStartShader;
    NBRE_ShaderPtr      m2DEndShader;
    NBRE_ShaderPtr      m3DStartShader;
    NBRE_ShaderPtr      m3DEndShader;

    NBRE_Vector3d       mPosition;
    nb_boolean          mIsEndFlag;
    float               mSize;
    float               mMinPixelSize;
    float               mMaxPixelSize;
    nb_boolean          mNeedUpdate;
    nb_boolean          mAvailableScreenPt;
    NBRE_Vector2d       mScreenPoints[8];
    NBGM_TileMapBoundaryExtendType mExtendType;
};
typedef shared_ptr<NBGM_Flag> NBGM_FlagPtr;
/*! @} */
#endif

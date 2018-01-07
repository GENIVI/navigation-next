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

    @file nbgmavatar.h
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

#ifndef _NBGM_AVATAR_H_
#define _NBGM_AVATAR_H_
#include "nbgmdevicelocation.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "nbreentity.h"
#include "nbgmlayoutmanager.h"
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
class NBGM_Avatar:
    public NBGM_LayoutElement, public NBGM_DeviceLocation
{
public:
    NBGM_Avatar(NBGM_Context& nbgmContext, NBRE_SceneManager* sceneManager, NBRE_SurfaceSubView* subview, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, DrawOrderStruct drawOrder, int32 glowOverlayId, NBGM_AvatarMode mode, float modelSize, float referenceHeight, const NBRE_Point3f& posOffset, const NBRE_String& id);
    virtual ~NBGM_Avatar();

public:
    /// Update avatar by parameters
    void Update(float cameraDistance);

public:
    // From NBGM_DeviceLocation
    /// Set model size scale, for Vector view, it is 1, for ECM view it is RADIUS_EARTH_METERS
    virtual void SetModelSale(float s);
    /// Enable halo
    virtual void EnableHalo(bool showHalo);
    /// Set avatar location
    virtual void SetLocation(const NBGM_Location64& location);
    /// Get avatar location
    virtual const NBGM_Location64& GetLocation();
    /// Set avatar scale
    virtual void SetScale(float scaleValue);
    /// Set avatar state
    virtual void SetState(NBGM_AvatarState state);
    /// Get screen position of avatar
    virtual bool GetScreenPosition(float& screenX, float& screenY) const;

    virtual bool SelectAndTrack(float x, float y);

    virtual const std::string& ID() const;
    /// Set halo style
    virtual PAL_Error SetHaloStyle(const NBGM_Color& edgeColor, const NBGM_Color& fillColor, uint8 edgeWidth, NBGM_HaloEdgeStyle style);
    /// Set avatar texture
    virtual PAL_Error SetDirectionalTexture(const std::string& textureFilePath, const NBGM_Point2d& offset);
    virtual PAL_Error SetDirectionalTexture(const NBGM_BinaryBuffer& textureBuffer, const NBGM_Point2d& offset);
    virtual PAL_Error SetDirectionlessTexture(const NBGM_BinaryBuffer& textureBuffer, const NBGM_Point2d& offset);
    virtual PAL_Error SetDirectionlessTexture(const std::string& textureFilePath, const NBGM_Point2d& offset);
    virtual void UpdateDPI();

public:
    //From NBGM_LayoutElement
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
    /// Set avatar mode
    virtual void SetMode(NBGM_AvatarMode mode);

public:
    /// Get screen polygon projected from avatar
    void GetPolygon(Polygon2f polygons[]);
    /// Notify avatar extend
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);
    /// Is avatar in screen at given position
    nb_boolean IsAvatarInScreen(const NBRE_Vector3d& position);

    /// Get avatar bounding box
    NBRE_AxisAlignedBox3d AxisAlignedBox() const;
    NBRE_NoneAxisAlignedBox3d ObjectAlignedBox() const;
    nb_boolean IsScreenPointAvailable();
    nb_boolean GetScreenPos(NBRE_Point2f& screenPos) const;
    nb_boolean IsExtendNodeInFrustum() const;

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_Avatar);

private:
    NBRE_SceneManager*  mSceneManager;
    NBRE_SurfaceSubView* mSubView;
    NBRE_DefaultOverlayManager<DrawOrderStruct>& mOverlayManager;
    uint32              mOverlayID;
    uint32              mOverlayHaloID;
    int32               mGlowOverlayId;
    NBGM_Location64     mLocation;
    NBRE_NodePtr        mParentNode;
    NBRE_NodePtr        mHaloNode;
    NBRE_NodePtr        mExtendHaloNode;
    NBRE_NodePtr        mAvatarNode;
    NBRE_NodePtr        mExtendAvatarNode;
    NBRE_NodePtr        mSubAvatarNode;
    NBRE_NodePtr        mExtendSubAvatarNode;
    NBRE_EntityPtr      mHaloEntity;
    NBRE_EntityPtr      mExtendHaloEntity;
    NBRE_EntityPtr      mCarEntity;
    NBRE_EntityPtr      mExtendCarEntity;
    NBRE_EntityPtr      mArrowEntity;
    NBRE_EntityPtr      mExtendArrowEntity;
    NBRE_EntityPtr      mMapEntity;
    NBRE_EntityPtr      mExtendMapEntity;
    float               mModelScale;
    float               mModelSize;
    float               mRefHeight;
    NBRE_Point3f        mPosOffset;
    NBGM_AvatarMode     mMode;
    NBGM_AvatarState    mState;
    nb_boolean          mNeedUpdate;
    nb_boolean          mUpdated;
    NBRE_ShaderPtr      mMapFollowMeShader;
    NBRE_ShaderPtr      mMapLocateMeShader;
    NBRE_ShaderPtr      mHaloOutlineShader;
    NBRE_ShaderPtr      mHaloInnerShader;
    nb_boolean          mAvailableScreenPt;
    NBRE_Vector2d       mScreenPoints[8];
    nb_boolean          mShowHalo;
    NBRE_Color          mHaloOutlineClr;
    NBRE_Color          mHaloInnerClr;
    Polygon2f           mProjectedPolygon;
    double              mMapEntityBaseSize;
    NBRE_Vector2d       mMapEntityFollowMePixelSize;
    NBRE_Vector2d       mMapEntityLocateMePixelSize;
    NBGM_TileMapBoundaryExtendType mExtendType;
    NBRE_Vector2f       mFollowMeOffset;
    NBRE_Vector2f       mLocateMeOffset;
    NBRE_String         mID;
    uint8               mHaloEdgeWidth;
};
typedef shared_ptr<NBGM_Avatar> NBGM_AvatarPtr;
/*! @} */
#endif

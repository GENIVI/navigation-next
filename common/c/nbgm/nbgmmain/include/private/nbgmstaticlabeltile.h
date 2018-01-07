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

    @file nbgmstaticlabeltile.h
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

#ifndef _NBGM_STATIC_LABEL_TILE_H_
#define _NBGM_STATIC_LABEL_TILE_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "nbgmlayoutmanager.h"
#include "nbrepolyline2.h"
#include "nbrepolyline3.h"
#include "nbretextpath3dentity.h"
#include "nbgmpolyline2entity.h"
#include "nbreentity.h"
#include "nbgmtaskqueue.h"

class NBGM_LabelLayer;
struct NBGM_PathArrowLayerData;
/*! \addtogroup NBGM_Service
*  @{
*/

/** static layout result base class.
*/
class NBGM_StaticLayoutResult
{
public:
    enum ResultType
    {
        RoadLabel,
        Shield,
        PathArrow
    };
public:
    NBGM_StaticLayoutResult(uint32 dataIndex, ResultType type): dataIndex(dataIndex), type(type) {}
    virtual ~NBGM_StaticLayoutResult() {}

public:
    /// Index of shared data in static label tile
    uint32 dataIndex;
    ResultType type;
};


/** tile base static layout data.
*/
class NBGM_StaticLabelTile
{
public:
    struct RoadTrackData
    {
    public:
        RoadTrackData(int32 overlayId, NBRE_Node* parentNode, const NBRE_WString& text, uint32 materialId, const NBRE_FontMaterial& material, const NBRE_Polyline2f& polyline, float width, int32 priority, float nearVisibility, float farVisibility);
    public:
        int32 overlayId;
        NBRE_Node* parentNode;
        NBRE_Vector3d refCenter;
        NBRE_WString text;
        uint32 materialId;
        NBRE_FontMaterial material;
        NBRE_Polyline2f polyline;
        float width;
        int32 priority;
        NBRE_Vector2f baseStringSize;
        float nearVisibility;
        float farVisibility;
    };
    typedef NBRE_Vector<RoadTrackData*> RoadTrackDataList;

    class RoadTrackLayoutResult:
        public NBGM_StaticLayoutResult
    {
    public:
        RoadTrackLayoutResult(uint32 dataIndex, const NBRE_Polyline2f& polyline, float fontHeight, float width);
        virtual ~RoadTrackLayoutResult() {}

    public:
        NBRE_Polyline2f polyline;
        float fontHeight;
        float width;
    };

    struct ShieldTrackData
    {
    public:
        ShieldTrackData(NBRE_Node* parentNode, NBRE_SurfaceSubView* subView, int32 overlayId, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, NBRE_ShaderPtr image, const NBRE_Vector2f& textureSize, const NBRE_Vector2f& offset, const NBRE_Polyline3d& polyline, int32 priority, float nearVisibility, float farVisibility);

    public:
        NBRE_Node* parentNode;
        NBRE_SurfaceSubView* subView;
        int32 overlayId;
        NBRE_WString text;
        NBRE_FontMaterial material;
        uint32 materialId;
        NBRE_ShaderPtr image;
        NBRE_Vector2f textureSize;
        NBRE_Vector2f offset;
        NBRE_Polyline3d polyline;
        int32 priority;
        float baseStringWidth;
        float nearVisibility;
        float farVisibility;
    };
    typedef NBRE_Vector<ShieldTrackData*> ShieldTrackDataList;


    class ShieldTrackLayoutResult:
        public NBGM_StaticLayoutResult
    {
    public:
        ShieldTrackLayoutResult(uint32 dataIndex, const NBRE_Vector3d& position, float fontHeight, const NBRE_Vector2f& textureSize, const NBRE_Vector2f& shieldOffset);
        virtual ~ShieldTrackLayoutResult() {}

    public:
        NBRE_Vector3d position;
        float fontHeight;
        NBRE_Vector2f textureSize;
        NBRE_Vector2f shieldOffset;
    };

    struct PathArrowTrackData
    {
    public:
        PathArrowTrackData(NBRE_Node* parentNode, NBGM_PathArrowLayerData* layerData, const NBRE_String& shaderName, int32 overlayId, float patLength, float patRepeat, float nearVisibility, float farVisibility);
        ~PathArrowTrackData();

    public:
        NBRE_Node* parentNode;
        NBGM_PathArrowLayerData* layerData;
        NBRE_String shaderName;
        int32 overlayId;
        NBRE_Vector3d refCenter;
        float patLength;
        float patRepeat;
        float nearVisibility;
        float farVisibility;
    };
    typedef NBRE_Vector<PathArrowTrackData*> PathArrowTrackDataList;


    class PathArrowTrackLayoutResult:
        public NBGM_StaticLayoutResult
    {
    public:
        PathArrowTrackLayoutResult(uint32 dataIndex, NBRE_Entity* entity);
        virtual ~PathArrowTrackLayoutResult();

    public:
        NBRE_Entity* entity;
    };


    typedef NBRE_Vector<NBRE_EntityPtr> EntityList;
    typedef NBRE_Map<int32, NBGM_LayoutElementList*> LayoutElementZoomLevelMap;
    typedef NBRE_Map<int32, EntityList> EntityListZoomLevelMap;
    typedef NBRE_Set<int32> LayoutLevelSet;

public:
    NBGM_StaticLabelTile(NBGM_Context& nbgmContext, NBRE_Node* parentNode);
    virtual ~NBGM_StaticLabelTile();

public:
    /// Add road label to tile
    void AddRoadLabel(int32 overlayId, NBRE_Node* parentNode, const NBRE_WString& text, uint32 materialId, const NBRE_FontMaterial& material, const NBRE_Polyline2f& polyline, float width, int32 priority, float nearVisibility, float farVisibility);
    /// Add shield track to tile
    void AddShieldTrack(NBRE_Node* parentNode, NBRE_SurfaceSubView* subView, int32 overlayId, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, NBRE_ShaderPtr image, const NBRE_Vector2f& textureSize, const NBRE_Vector2f& offset, const NBRE_Polyline3d& polyline, int32 priority, float nearVisibility, float farVisibility);
    /// Add path arrow track to tile
    void AddPathArrow(NBRE_Node* parentNode, NBGM_PathArrowLayerData* layerData, const NBRE_String& shaderName, int32 overlayId, float patLength, float patRepeat, float nearVisibility, float farVisibility);
    /// Get AABB
    const NBRE_AxisAlignedBox3d& GetAABB() { return mAABB; }
    /// Update layout list
    void UpdateLayoutList(NBGM_LayoutElementList& layoutElements, shared_ptr<WorkerTaskQueue> loadingThread, shared_ptr<NBGM_TaskQueue> renderThread, NBGM_LabelLayer* labelLayer);
    /// Add to scene
    void AddToScene();
    /// Get label layout result on level
    nb_boolean OnLayoutElementsDone(int32 level, NBRE_Vector<NBGM_StaticLayoutResult*>* layoutResult);
    /// Resolve conflict labels
    void ResolveStaticConflicts(int32 level, NBGM_StaticLabelTile* tile);
    /// Get font ids
    void GetFontMaterialIds(NBRE_Set<uint32>& materialIds);
    /// Update font material
    void RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials);
    /// Set tile Visible
    void SetVisible(nb_boolean visbile);
    /// Get id
    uint64 GetId() const { return mId; }
    
private:
    DISABLE_COPY_AND_ASSIGN(NBGM_StaticLabelTile);

private:
    /// context
    NBGM_Context& mNbgmContext;
    /// parent node
    NBRE_Node* mParentNode;
    /// road tracks
    RoadTrackDataList mRoadTracks;
    /// shield tracks
    ShieldTrackDataList mShieldTracks;
    /// path arrows
    PathArrowTrackDataList mPathArrows;
    /// parent node
    NBRE_NodePtr mPathArrowNode;
    /// bounding box
    NBRE_AxisAlignedBox3d mAABB;
    /// local aabb
    NBRE_AxisAlignedBox3d mLocalAABB;
    /// static layout result cache
    LayoutElementZoomLevelMap mLayoutZoomLevelCache;
    /// ongoing layout levels
    LayoutLevelSet mOngoingZoomLevels;
    /// path arrow result cache
    EntityListZoomLevelMap mPathArrowZoomLevelCache;
    /// max font size
    double mMaxFontSize;
    /// visible
    nb_boolean mVisible;
    /// id
    uint64 mId;
    /// uuid
    static uint32 staticId;
};

/*! @} */
#endif

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

    @file nbgmnavecmroutelayer.h
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
#ifndef _NBGM_NAV_ECM_ROUTE_LAYER_H_
#define _NBGM_NAV_ECM_ROUTE_LAYER_H_

#include "nbgmtypes.h"
#include "nbretypes.h"
#include "palerror.h"
#include "nbrecolor.h"
#include "nbgmcontext.h"
#include "nbrenode.h"
#include "nbrecontext.h"
#include "nbreshader.h"
#include "nbrevector3.h"
#include "nbgmnavdata.h"
#include "nbreentity.h"
#include "nbgmbuildmodelcontext.h"
#include "nbresurfacesubview.h"
#include "nbgmdefaultoverlaymanager.h"
#include "nbgmdraworder.h"

/*! \addtogroup NBGM_Service
*  @{
*/

/** This class is used for navigation ecm route.
*/
class NBGM_SnapRouteInfo
{
public:
    NBGM_SnapRouteInfo();
    ~NBGM_SnapRouteInfo();

public:
    void SetLastPositionIndex(uint32 index);
    void SetProjectPosition(float pos);
    void SetClosetDistance(float distance);

    uint32 GetLastPositionIndex() const;
    float GetProjectPosition() const;
    float GetClosetDistance() const;

    nb_boolean IsValid() const;

private:
    uint32 mLastPosIndex;
    float  mProjectPosition;
    float  mClosestDistance;
};

class NBGM_NavEcmRouteLayer
{
    class NBGM_Spline
    {
    public:
        NBGM_Spline(const NBGM_Context& context, const uint8* data, uint32 size);
        ~NBGM_Spline();

    public:
        uint32 GetVertexCount() const;
        const float* GetPositionArray() const;
        const float* GetOuttanArray() const;
        const float* GetIntanArray() const;

    private:
        DISABLE_COPY_AND_ASSIGN(NBGM_Spline);

    private:
        PAL_Error LoadSplineFromBinaryData(const NBGM_Context& context, const uint8* data, uint32 size);

    private:
        uint32 mVertexCount;
        uint32 mFromLinkPVID;
        uint32 mToLinkPVID;
        uint32 mFromLinkDirection;
        uint32 mToLinkDirection;
        float* mPositionArray;
        float* mOuttanArray;
        float* mIntanArray;
    };

    class Maneuver
    {
    friend class NBGM_NavEcmRouteLayer;

    public:
        Maneuver( const NBGM_Context& context, const NBRE_Point3f& offset, uint32 maneuverID);
        ~Maneuver();

    private:
        NBRE_Point3f mOffset;
        uint32 mID;
    };

public: 
    NBGM_NavEcmRouteLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, const DrawOrderStruct& routeDrawOrder, const DrawOrderStruct& arrowDrawOrder);
    ~NBGM_NavEcmRouteLayer();

public:
    PAL_Error AddManeuver(uint32 id, const NBRE_Point3f& position);
    PAL_Error SetCurrentManeuver(uint32 id);
    PAL_Error AppendSpline(const NBGM_SplineData* splineData);
    void RemoveAllSplines();

    PAL_Error SnapRoute(NBGM_Location64& location, NBGM_SnapRouteInfo& snapRouteInfo) const;
    PAL_Error SnapToRouteStart(NBGM_Location64& location, NBGM_SnapRouteInfo& pRouteInfo) const;
    PAL_Error CalculateForecastPoint(const NBGM_SnapRouteInfo& prevSnapInfo, float distance, NBRE_Point3f& resultPoint) const;
    PAL_Error CalculateDistanceFromRouteStartToSnapPosition(const NBGM_SnapRouteInfo& snapInfo, float& distance) const;

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_NavEcmRouteLayer);

private:
    NBRE_ShaderPtr CreateShader(NBRE_Context& nbreContext, const NBRE_Color& color, const NBRE_String& name);
    PAL_Error SplineModelConstruct(uint32 assistIndex, uint32 startIndex, uint32 count);
    PAL_Error FindPolylineBestSeg(const NBGM_Location64& location, nb_boolean reverse, NBGM_SnapRouteInfo& snapRouteInfo) const;
    void Line2dProject(const NBRE_Point2f& p1, const NBRE_Point2f& p2, const NBRE_Point2f& pt, float& distance, float& pos) const;
    nb_boolean CheckDirect(float degree1, float degree2) const;
    void ClipPolylineForManeuverDrawing(const NBGM_SnapRouteInfo& snapRouteInfo, float arrowLength);
    PAL_Error ManeuverModelConstruct(float turnArrowHeadHeight);
    NBRE_Entity* CreateEntity(NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer);
    uint32 CalcHermiteCurveStepCount(NBRE_Point3f v1, NBRE_Point3f v2, NBRE_Point3f t1, NBRE_Point3f t2);
    PAL_Error ConvertSplineToPolyline( const NBGM_Spline* spline, NBRE_Vector<NBRE_Point3f>& polyline );
    NBRE_Point3f CalcHermiteCurve( const NBRE_Point3f& p1, const NBRE_Point3f& p2, const NBRE_Point3f& t1, const NBRE_Point3f& t2, float s );

private:
    typedef NBRE_Vector<Maneuver> ManeuverArray;

private:
    NBGM_Context& mNBGMContext;
    NBGM_BuildModelContext* mBuildModelContext;
    NBRE_Node& mParentNode;

    NBRE_NodePtr mRouteNode;
    NBRE_NodePtr mArrowNode;

    ManeuverArray mManeuverArray;
    NBRE_Vector<NBRE_Point3f> mSplinesPolyline;
    NBRE_Vector<NBRE_Point3f> mArrowPolyline;

    uint32 mRouteLayerID;
    uint32 mArrowLayerID;

    float mRouteWidth;
    uint32 mCurrentManeuverID;

    NBRE_ShaderPtr mRouteShader;
    NBRE_ShaderPtr mTurnArrowShader;
};

/*! @} */
#endif
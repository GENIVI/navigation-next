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

    @file nbgmnavvectorroutelayer.h
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
#ifndef _NBGM_NAV_VECTOR_ROUTE_LAYER_H_
#define _NBGM_NAV_VECTOR_ROUTE_LAYER_H_

#include "nbgmtypes.h"
#include "nbretypes.h"
#include "nbrecommon.h"
#include "palerror.h"
#include "nbgmarray.h"
#include "nbrecolor.h"
#include "nbgmcontext.h"
#include "nbrenode.h"
#include "nbrecontext.h"
#include "nbreshader.h"
#include "nbrevector2.h"
#include "nbgmlayoutmanager.h"
#include "nbresurfacesubview.h"
#include "nbgmdefaultoverlaymanager.h"
#include "nbgmdraworder.h"

/*! \addtogroup NBGM_Service
*  @{
*/

/** This class is used for navigation highlight route.
*/
class NBRE_Entity;

class NBGM_NavVectorRouteLayer:
    public NBGM_LayoutElement, public NBGM_ILayoutProvider
{
    class Maneuver
    {
    friend class NBGM_NavVectorRouteLayer;

    public:
        Maneuver(NBGM_Context& context, NBGM_BuildModelContext* buildModelContext, const NBRE_Point2d& offset, const float* points, uint32 count, uint32 maneuverID);
        ~Maneuver();

    public:
        PAL_Error BuildRouteModel(float routeWidth, const NBRE_ShaderPtr& shader, int32* layerID, const NBRE_Point2f& foregroundTexcoord, const NBRE_Point2f& backgroundTexcoord);
        PAL_Error CreateTurnArrowPolyline(Maneuver* nextManeuver, float checkLength, float halfRouteWidth);
        void BuildTurnArrowModel(float routeWidth, const NBRE_ShaderPtr& shader, int32* layerID, const NBRE_Point2f& foregroundTexcoord, const NBRE_Point2f& backgroundTexcoord);
        uint32 ManeuverID() const { return mManeuverID; }
        void UpdateRegion();
        void SetOverlayId(int32* overlayId);

    private:
        DISABLE_COPY_AND_ASSIGN(Maneuver);

    private:
        NBRE_Array* CopyMercatorArray(NBRE_Array *polyline) const;
        void DestroyMercatorArray(NBRE_Array *polyline) const;
        void ManeuverBuildTurnArrowHeaderVertices(NBRE_Array* newArrowPolyline, float routeWidth, float arrowWidth, float texX, float texY);
        NBRE_Entity* BuildEntity();

    private:
        NBGM_Context& mNBGMContext;
        NBGM_BuildModelContext* mBuildModelContext;
        NBRE_NodePtr mNode;
        NBRE_Point2d mOffset;
        uint32 mManeuverID;
        NBRE_Array* mRoutePolyline;
        NBRE_Array* mArrowPolyline;
        float mArrowLength;
        NBRE_Polyline3d mWorldPolyline;
        NBRE_Polyline3d* mManeuverPolyline;
    };

public: 
    NBGM_NavVectorRouteLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint32 layerID);
    ~NBGM_NavVectorRouteLayer();

public:
    /// Is element visible
    virtual nb_boolean IsElementVisible() {return TRUE;}
    /// Update the region element takes
    virtual void UpdateRegion();
    /// Is element collide with buffer
    virtual nb_boolean IsElementRegionAvailable() {return TRUE;}
    /// Get position in world space
    virtual NBRE_Vector3d GetPosition() { return NBRE_Vector3d(0,0,0); }

    void Reset();
    PAL_Error AddManeuver(const NBRE_Point2d& offset, const float* points, uint32 count, uint32 maneuverID);
    void SetCurrentManeuver(uint32 maneuverId);
	uint32 CurrentManeuver();
    void SetOverlayId(int32 overlayId);
    virtual void UpdateLayoutList(NBGM_LayoutElementList& layoutElements);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_NavVectorRouteLayer);

private:
    NBRE_ShaderPtr CreateShader();
    void SetArrowVisible(Maneuver* man, nb_boolean visible);

private:
    typedef NBRE_Vector<Maneuver*> ManeuverArray;

private:
    NBGM_Context& mNBGMContext;
    NBRE_Node& mParentNode;
    NBGM_BuildModelContext* mBuildModelContext;

    ManeuverArray mManeuverArray;
    int32 mDrawOrder;
    int32 mLayerID[4];

    float mRouteWidth;
    uint32 mCurrentManeuverID;
    uint32 mClipManeuverID;

    Maneuver* mCurrentManeuver;

    NBRE_ShaderPtr mShader;
    NBRE_Point2f mRouteForegroundTexcoord;
    NBRE_Point2f mRouteBackgroundTexcoord;
    NBRE_Point2f mArrowForegroundTexcoord;
    NBRE_Point2f mArrowBackgroundTexcoord;

    NBRE_SurfaceSubView* mSubView;
    NBRE_DefaultOverlayManager<DrawOrderStruct>& mOverlayManager;
    nb_boolean mIsDrawOrderInitialized;
};

/*! @} */
#endif
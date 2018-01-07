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
 
 @file nbgmfakemapview.h
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

#ifndef _NBGM_FAKE_MAPVIEW_H_
#define _NBGM_FAKE_MAPVIEW_H_

#include "nbgmmapview.h"
#include "palerror.h"
#include "nbrevector3.h"
#include "nbretypes.h"
#include "nbgmvectortile.h"
#include "nbgmcamerasetting.h"
#include "pal.h"
#include "nbrecamera.h"
#include "nbgmavatarsetting.h"
#include "nbgmmapviewprofiler.h"
#include "nbgmcontext.h"
#include "nbgmmapviewconfig.h"
#include "nbgmnavdata.h"
#include "nbgmnavecmroutelayer.h"
#include "palgl.h"
#include "nbgmmapviewimpl.h"

/*! NBGM_MapViewImpl class
 
 This class can repespond UI event, load map file, decide which layer data can be show in certain zoom level.
 */
class NBGM_FakeMapView
{
public:
    NBGM_FakeMapView(const NBGM_MapViewImplConfig& config, NBRE_RenderEngine* renderEngine);
    ~NBGM_FakeMapView();
    
public:
    void InitializeEnvironment();
    nb_boolean RenderFrame(float secondsPassed);
    
    void ActiveSurface();
    
	NBRE_RenderSurface* Surface();
    
    ///Set a loction as map view center, camera move and rotate base on this center
    void SetViewCenter(double mercatorX, double mercatorY);
    ///Get a loction as map view center, its value will effect by SetViewCenter, OnPaning or Move operation
    void GetViewCenter(double& mercatorX, double& mercatorY);
    
    /*! Move map view.
     @param dx screen delta x
     @param dy screen delta y
     @return none
     */
    void Move(float dx, float dy);
    
    ///Touch Event
    void OnTouchEvent(float screenX, float screenY);
    void OnPaning(float screenX, float screenY);
    void OnLongPress();
    void OnSingleTapUp();
    
    /*! Project screen point to the map ground.
     @param screenX screen x coordinate point
     @param screenY screen y coordinate point
     @param mercatorX mercator x
     @param mercatorY mercator y
     @return none
     */
    void ScreenToMapPosition(float screenX, float screenY, double& mercatorX, double& mercatorY);
    
    /*! Retrieve current frustum position in world coordinate, to calculate visible tiles
     (Render thread function)
     @param frustumHeight Distance from frustum bottom to top. This value will be used to limit
     the frustum size when showing sky, unit is Mercator
     @param positions 4 points, which are map view corners projected to world, Mercator coordinate
     These 4 points will compose a trapezium:
     positions[0]------------- positions[1]
     \                       / |
     \                     /  |
     \                   / frustumHeight
     \                 /    |
     positions[3]-----positions[2]
     @return TURE for get a usable value, FALSE for got a singular value.
     */
    nb_boolean GetFrustumPositionInWorld(double frustumHeight, NBRE_Vector<NBGM_Point2d64>& positions);
    
    /*! Set the distance from screen bottom to the sky on the horizontal plane
     (Render thread function)
     @param horizonDistance which used to set the sky distance and limit the frustum height, unit is Mercator
     @return none
     */
    void SetHorizonDistance(double horizonDistance);
    
    /*! Set Rotate center.
     @param x screen x coordinate point
     @param y screen y coordinate point
     @return none
     */
    void SetRotateCenter(float screenX, float screenY);
    
    /*! Rotate Map base on screen point.
     @param angle it is a delta angle, unit is degree
     @return none
     */
    void Rotate(float angle);
    
    /*! Rotate Map base on screen center.
     @param angle it is a absolute angle, unit is degree
     @return none
     */
    void SetRotateAngle(float angle);
    
    /*! Get rotate angle, base on map view center and axis is verical with map ground.
     @return angle, unit is degree, north is zero, south is 180 degree, direction is counter clockwise
     */
    float RotateAngle();
    
    /*! Zoom Map
     @param deltaH Add a height base on current camera height, unit is Mercator
     @return none
     */
    void Zoom(float deltaH);
    /*! Set distance between camera and map view center
     @param height Set a absolute distance, unit is Mercator
     @return none
     */
    void SetViewPointDistance(float distance);
    
    /*! Get distance between camera and map view center
     */
    float ViewPointDistance();
    
    /*! Add delta tilt angle base on current angle
     @param angle it is a delta angle, unit is degree
     @return none
     */
    void Tilt(float angle);
    
    /*! Set tilt angle
     @param angle unit is degree
     @return none
     */
    void SetTiltAngle(float angle);
    
    /*! Get tilt angle between vertical line and veiw angle
     @return angle, unit is degree
     */
    float TiltAngle();
    
    /*! Set viewport and adjust frustum matrix
     @param x, start screen X position(left-bottom)
     @param y, start screen Y position(left-bottom)
     @param w, screen width used for rendering.
     @param h, screen height used for rendering.
     */
    void OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h);
    
    /*! Set up map view perspective projection.
     @param fov Specifies the field of view angle, in degrees, in the y(height) direction.
     @param aspect Specifies the aspect ratio that determines the field of view
     in the x direction. The aspect ratio is the ratio of x (width) to y (height).
     @return none
     */
    void SetPerspective(float fov, float aspect);
    
    void LoadCommonMaterial(const NBRE_String& materialName, NBRE_IOStream& istream);
    void SetCurrentCommonMaterial(const NBRE_String& materialName);
    
    /*! Load pin material file, which will be used for rendering poi/pin.
     @param pinMaterialName, Specifies material name, each material file should has different name
     @param filePath The full path of the material file to be loaded
     */
    void LoadPinMaterial(const NBRE_String& pinMaterialName, NBRE_IOStream& istream);
    
    nb_boolean IsNbmDataExist(const NBRE_String& nbmName);
    
    /*! Load nbm format tile file
     @param nbmName, nbm file unique id
     @param baseDrawOrder Base draw order of this tile
     @param labelDrawOrder Label draw order of this tile
     @param istream, input stream, used to read data
     @return none
     */
    void LoadNbmData(const NBRE_String& nbmName, uint8 baseDrawOrder, uint8 labelDrawOrder, NBRE_IOStream& istream);
    
    /*! Load bin format tile file, draw order is DO_MAIN
     @param binName, bin file unique id
     @param istream, input stream, used to read data
     @return none
     */
    void LoadBinData(const NBRE_String& binName, NBRE_IOStream& istream);
    
    /*! Unload a tile
     @param nbmName, nbm file unique id
     @return none
     */
    void UnLoadTile(const NBRE_String& nbmName);
    
    /*! Set avatar location
     @param location, avatar location
     @return none
     */
    void SetAvatarLocation(const NBGM_Location64& location);
    
    /*! Set avatar scale
     @param scaleValue, avatar scale
     @return none
     */
    void SetAvatarScale(float scaleValue);
    
    /*! Set avatar state
     @param state, avatar state
     @return none
     */
    void SetAvatarState(NBGM_AvatarState state);
    
    /*! Set avatar mode
     @param mode, avatar mode
     @return none
     */
    void SetAvatarMode(NBGM_AvatarMode mode);
    
    /*! Disable/Enable profiling
     @enable TRUE for enable, FALSE for disable
     @return none
     */
    void EnableProfiling(nb_boolean enabled);
    
    /*! Disable/Enable verbose profiling
     @enable TRUE for enable, FALSE for disable
     @return none
     */
    void EnableVerboseProfiling(nb_boolean enabled);
    
    /*! Notify the reandering thread sync data uploaded by other work threads
     @return none
     */
    void SyncMapData();
    
    /*! Get current poi/pin screen coordinate.
     @poiId, poi/pin's identifier defined in Pin NBM file.
     @coordinates, screen coordinate of current poi/pin.It is output.Screen left-bottom is(0,0).
     @return if current poi/pin is visible on screen then return TURE, otherwise return FALSE.
     */
    nb_boolean GetPoiPosition(const NBRE_String& poiId, NBRE_Point2f& coordinates);
    
    /*! Select a poi/pin which will change its state in next render cycle.
     @poiId, poi/pin's identifier defined in Pin NBM file.
     @return if current poi/pin is visible on screen then return TURE, otherwise return FALSE.
     */
    nb_boolean SelectAndTrackPoi(const NBRE_String& poiId);
    
    /*! Unselect a poi/pin which will change its state in next render cycle.
     @poiId, poi/pin's identifier defined in Pin NBM file.
     @return if current poi/pin is visible on screen then return TURE, otherwise return FALSE.
     */
    nb_boolean UnselectAndStopTrackingPoi(const NBRE_String& poiId);
    
    /*! Get all the pois/pins which are interacted
     @poiList, selected poi/pin's which are interacted.it is output.
     @screenPosition, screen coordinate. Screen left-bottom is(0,0).
     @return the total number of the interacted pois/pins.
     */
    uint32 GetInteractedPois(NBRE_Vector<NBRE_String>& pois, const NBRE_Point2f& screenPosition);
    
    float GetCameraHeight() const;
    
    /*! Set the size of sky wall for blocking
     @param width, the width of the sky in meters
     @param height, the height of the sky in meters
     @param distance, the distance form the camera position to the sky wall in meters
     @return none
     */
    void SetSkySize(float width, float height, float distance);
    
    /*! Set end flag location
     @param location End flag location
     @return none
     */
    void SetEndFlagLocation(const NBGM_Location64& location);
    
    /*! Set start flag location
     @param location Start flag location
     @return none
     */
    void SetStartFlagLocation(const NBGM_Location64& location);
    
    /*! Refresh navigation POI
     @param pois navigation pois
     @return none
     */
    void RefreshNavPois(const NBRE_Vector<NBGM_Poi*>& pois);
    
    /*! Load navigation route polyline.
     @par
     If there are more then one maneuvers, this func need be invoked multi-times to
     load route for each maneuver, or the maneuver turn arrow can not be rendered.
     @param routeData, Nav route data.
     @return PAL error code
     */
    PAL_Error AddNavVectorRoutePolyline(const NBGM_VectorRouteData* routeData);
    
    /*! Set current maneuver ID then draw the turn arrow.
     @param currentManuever, current manuever id;
     @return none
     */
    void SetNavVectorCurrentManeuver(uint32 currentManuever);
    
	uint32 NavVectorCurrentManeuver();
    
    /*! Reset vector route layer.clear all the data in vector route layer.
     @return none
     */
    void ResetNavVectorRouteLayer();
    
    /*! Set model size scale, for Vector view, it is 1, for ECM view it is RADIUS_EARTH_METERS */
    void SetAvatarModelSizeScale(float s);
    
    /*! Set camera setting in Ortho projection
     @param settings CameraSetting for calculating
     @return none
     */
    void SetOrthoCameraSetting(NBGM_CameraSetting setting);
    
    /*! Set camera setting in Perspective projection
     @param settings CameraSetting for calculating
     @return none
     */
    void SetPerspectiveCameraSetting(NBGM_CameraSetting setting);
    
    /*! Load navigation ecm route spline.
     @par
     If there are more then one spline, this func need be invoked multi-times to
     load spline data, or the maneuver turn arrow can not be rendered.
     @param data, spline data.
     @return PAL error code
     */
    PAL_Error AddNavEcmRouteSpline(const NBGM_SplineData* data);
    
    /*! Set current maneuver ID then draw the turn arrow.
     @param currentManuever, current manuever id;
     @return PAL error code
     */
    PAL_Error SetNavEcmCurrentManeuver(uint32 currentManueverID);
    
    /*! Reset ecm route layer.clear all the data in ecm route layer.
     @return none
     */
    void ResetNavEcmRouteLayer();
    
    /*! add new maneuver.
     @param maneuverID, manuever id;
     @param position, manuever offset;
     @return PAL error code
     */
    PAL_Error AddNavEcmManeuver(uint32 maneuverID, const NBGM_Point3d& position);
    
    /*! find the best position on route
     @param location, position,heading,speed info, input a not accurate location and get a accurate one on route.
     @param snapRouteInfo, the result of snapping.
     @return PAL error code
     */
    
    //TODO: This function would be changed in future
    PAL_Error NavEcmSnapRoute(NBGM_Location64& location, NBGM_SnapRouteInfo& snapRouteInfo) const;
    
    /*!Get avatar NBGM_SnapRouteInfo.
     @return avatar NBGM_SnapRouteInfo.
     */
    
    //TODO: This function would be changed in future
    NBGM_SnapRouteInfo& NavEcmGetAvatarSnapRouteInfo();
    
    /*! snap to the route start
     @param location, get location of route start.
     @param snapRouteInfo, the result of snapping.
     @return PAL error code
     */
    
    //TODO: This function would be changed in future
    PAL_Error NavEcmSnapToRouteStart(NBGM_Location64& location, NBGM_SnapRouteInfo& pRouteInfo) const;
    
    void SetIsNavMode(nb_boolean value) ;
    
    void SetRoadWidthFactor(float factor);
    
    void AdjustFlagSize(float size, float minPixelSize, float maxPixelSize);
    
    /*! Retrieve profile
     @return none
     */
    void DumpProfile(NBGM_MapViewProfile& profile);

    nb_boolean MapPositionToScreen(float& screenX, float& screenY, double mercatorX, double mercatorY);
    nb_boolean GetAvatarScreenPosition( NBRE_Point2f& pos );

private:
    class PackedTriangles
    {
        friend class NBGM_FakeMapView;
        
    public:
        PackedTriangles();
        ~PackedTriangles();
        
    private:
        void GenerateVBO();

    private:
        DISABLE_COPY_AND_ASSIGN(PackedTriangles);
        
    private:
        float* mVertex;
        GLuint mVertexVBOId;

        float* mNormal;
        GLuint mNormalVBOId;

        float* mTexcoord;
        GLuint mTexcoordVBOId;

        uint16* mIndex;
        
        uint16 mTrianglesNum;
        GLuint mTexid;

        uint32 mVertexNum;
    };
    
private:
    void CreateTriangles(uint16 trianglesCount);
    void CreateTriangleFan(uint16 trianglesCount);

private:
    NBGM_MapViewProfiler mProfiler;
    
    //NBRE_Vector<PackedTriangles*> mTriangleFanArray;
    NBRE_Vector<PackedTriangles*> mTrianglesArray;
    
    nb_boolean mInitialized;
};

/*! @} */


#endif

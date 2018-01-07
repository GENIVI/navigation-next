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

    @file nbgmmapviewimpl.h
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

#ifndef _NBGM_MAP_VIEW_IMPL_H_
#define _NBGM_MAP_VIEW_IMPL_H_
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
#include "nbgmpatharrowlayer.h"
#include "nbgmmapviewimpltask.h"
#include "paltimer.h"
#include "nbretransformutil.h"
#include "nbgmresourcemanager.h"
#include "nbgmdefaultoverlaymanager.h"
#include "nbgmdraworder.h"
#include "nbgmcustomobjectscontext.h"

class NBRE_IOStream;
class NBRE_RenderEngine;
class NBGM_TileMapLayer;
class NBGM_LabelLayer;
class NBGM_PoiLayer;
class NBGM_NavVectorRouteLayer;
class NBRE_IRenderEnginePlugin;
class NBRE_SurfaceSubView;
class NBRE_SceneManager;
class NBRE_Viewport;
class NBGM_LayoutManager;
class NBRE_BillboardSet;
class NBGM_Avatar;
class NBGM_MapViewDataCache;
class NBGM_Sky;
class NBGM_Flag;
class NBGM_NavIconLayer;
struct NBGM_Poi;
class NBGM_GridEntity;
class NBRE_Entity;
class NBGM_TransparentLayer;
class NBGM_GlowLayer;
class NBRE_RenderSurface;
class NBGM_ScreenShot;
class NBGM_Animation;
class NBGM_TileAnimationLayerImpl;
class NBGM_OptionalLayerImpl;
class NBGM_RenderTask;
class NBGM_Compass;
class NBGM_CircleImpl;
class NBGM_Rect2dImpl;
class NBGM_Icon;

/*! \addtogroup NBGM_Manager
*  @{
*/
struct NBGM_MapViewImplConfig
{
    NBGM_MapViewConfig                  viewConfig;
    nb_boolean                          drawVectorNavRoute;
    nb_boolean                          drawEcmNavRoute;
    nb_boolean                          drawNavPoi;
    float                               modelScaleFactor;
    float                               avatarSizeFactor;
    float                               referenceAvatarHeight;
    nb_boolean                          useNavRoadLabel;
    uint32                              maxLabels;
    uint32                              maxAreaBuildingLabels;
    nb_boolean                          allowRedraw;
    uint32                              renderInterval;
    NBGM_ResourceManager*               resourceManager;
    NBGM_MapViewImplConfig():
        drawVectorNavRoute(FALSE),
        drawEcmNavRoute(FALSE),
        drawNavPoi(FALSE),
        modelScaleFactor(1),
        avatarSizeFactor(1),
        referenceAvatarHeight(1),
        useNavRoadLabel(FALSE),
        maxLabels(1),
        maxAreaBuildingLabels(1),
        renderInterval(20),
        resourceManager(NULL)
    {
    }
};


/*! NBGM_MapViewImpl class

 This class can repespond UI event, load map file, decide which layer data can be show in certain zoom level.
 */
class NBGM_MapViewImpl
{
public:
    NBGM_MapViewImpl(const NBGM_MapViewImplConfig& config, NBRE_RenderEngine* renderEngine);
    virtual ~NBGM_MapViewImpl();

public:

    /*! Initialize environment. This function MUST be called before any
    other functions, or you will get a crash.
    */
    void InitializeEnvironment();

    /*! Finalize the mapviewimpl. This function MUST be called before delete mapviewimpl,
	or you will get a memory leak!
    */
    void Finalize();

    /*! Request render operation.
    It will add a render task into render thread if needed.
    */
    void RequestRenderFrame(NBGM_FrameListener* listener = NULL);

    /*! Set render interval.
     It will change the interval between two frames.
     */
    void SetRenderInterval(uint32 interval);

    /* Render a frame
     @return true/false. If it needs more render operation return true, else return false.
     */
    nb_boolean RenderFrame();

    /*! Notify map view is set from/to background.
     */
    void SetBackground(bool background);

    void ActiveSurface();

	NBRE_RenderSurface* Surface();

    ///Set a loction as map view center, camera move and rotate base on this center
    void SetViewCenter(double mercatorX, double mercatorY);

   /*! Set the distance from screen bottom to the sky on the horizontal plane
        (Render thread function)
     @param horizonDistance which used to set the sky distance and limit the frustum height, unit is Mercator
     @return none
    */
    void SetHorizonDistance(double horizonDistance);

    /*! Rotate Map base on screen center.
     @param angle it is a absolute angle, unit is degree
     @return none
     */
    void SetRotateAngle(float angle);

    /*! Set distance between camera and map view center
     @param height Set a absolute distance, unit is Mercator
     @return none
     */
    void SetViewPointDistance(float distance);

    /*! Set tilt angle
     @param angle unit is degree
     @return none
     */
    void SetTiltAngle(float angle);

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

    void LoadExternalMaterial(const NBRE_String& categoryName, const NBRE_String& materialName, shared_ptr<NBRE_IOStream> istream, const NBRE_Vector<NBRE_String>& dependsMaterial);
    void LoadExternalMaterial(const NBRE_String& categoryName, const NBRE_String& materialName, std::vector<NBGM_IMapMaterial*>& materialTable);
    void UnloadExternalMaterial(const NBRE_String& categoryName, const NBRE_String& materialName);
    void SetCurrentExternalMaterial(const NBRE_String& categoryName, const NBRE_String& materialName);
    void SetSkyDayNight(nb_boolean isDay);
    void SetIsSatelliteMode(nb_boolean isSatellite);

    nb_boolean IsNbmDataExist(const NBRE_String& nbmName);

    /*! Load nbm format tile file
     @param info, the information of a tile which want to be loaded
     @return none
     */
    void LoadNbmData(const NBGM_NBMDataLoadInfo& info);

    /*! Load nbm format tile files
     @param tiles, a container that includes the information of several tiles.
     @return none
     */
    void LoadNbmData(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tiles);

    /*! Load bin format tile file, draw order is DO_MAIN
     @param info, the information of a tile which want to be loaded
     @return none
     */
    void LoadBinData(const NBGM_NBMDataLoadInfo& info);

    /*! Unload a tile
     @param nbmName, nbm file unique id
     @return none
     */
    void UnLoadTile(const NBRE_String& nbmName);

    /*! Unload tiles
     @param nbmNames, a container that includes all the id of the tiles which will be unloaded.
     @return none
     */
    void UnLoadTile(const NBRE_Vector<NBRE_String>& nbmNames);

    /*! Reload nbm format tile
     @param info, the information of a tile which want to be loaded
     @param needUnloadedNbmName, tile need to be unloaded.
     @return PAL error code
     */
    PAL_Error ReloadNBMTile(const NBGM_NBMDataLoadInfo& info, const std::string& needUnloadedNbmName);

    /*! Unload all tiles from cache
     @return none
     */
    void UnLoadAllTiles();

    /*! Update one tile.
     @param info, the information of a tile which want to be loaded
     @return PAL error code
     */
    PAL_Error UpdateNBMTile(const NBGM_NBMDataLoadInfo& info);

    /*! Set avatar location
     @param location, avatar location
     @return none
     */
    void SetAvatarLocation(const NBGM_Location64& location);
    
    /*! Get font scale
     @return scaleValue
     */
    float GetFontScale(void);
    
    /*! Set font scale
     @param scaleValue, font scale
     @return none
     */
    void SetFontScale(float scaleValue);

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

    /*! Get current poi/pin screen coordinate.
    @poiId, poi/pin's identifier defined in Pin NBM file.
    @coordinates, screen coordinate of current poi/pin.It is output.Screen left-bottom is(0,0).
    @return if current poi/pin is visible on screen then return TURE, otherwise return FALSE.
    */
    nb_boolean GetPoiPosition(const NBRE_String& poiId, NBRE_Point2f& coordinates);
    nb_boolean GetPoiBubblePosition(const NBRE_String& poiId, NBRE_Point2f& coordinates);

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

    nb_boolean GetStaticPoiInfo(const NBRE_String& id, NBGM_StaticPoiInfo& info);
    void SelectStaticPoi(const NBRE_String& id, nb_boolean selected);
    void GetStaticPoiAt(NBRE_Vector<NBRE_String>& pois, const NBRE_Point2f& screenPosition);

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
     @deprecated.
     */
    void RefreshNavPois(const NBRE_Vector<NBGM_Poi*>& pois);

    /*! Load navigation route polyline.
     @par
     If there are more then one maneuvers, this func need be invoked multi-times to
     load route for each maneuver, or the maneuver turn arrow can not be rendered.
     @param routeData, Nav route data.
     @return PAL error code
     @deprecated.
     */
    PAL_Error AddNavVectorRoutePolyline(const NBGM_VectorRouteData* routeData);

     /*! Set current maneuver ID then draw the turn arrow.
     @param currentManuever, current manuever id;
     @return none
     @deprecated.
     */
    void SetNavVectorCurrentManeuver(uint32 currentManuever);

    //@deprecated.
	uint32 NavVectorCurrentManeuver();

     /*! Reset vector route layer.clear all the data in vector route layer.
     @return none
     @deprecated.
     */
    void ResetNavVectorRouteLayer();

    /*! Set model size scale, for Vector view, it is 1, for ECM view it is RADIUS_EARTH_METERS */
    void SetAvatarModelSizeScale(float s);

    /*! Set camera setting in Ortho projection
     @param settings CameraSetting for calculating
     @return none
     @deprecated.
     */
    void SetOrthoCameraSetting(NBGM_CameraSetting setting);

    /*! Set camera setting in Perspective projection
     @param settings CameraSetting for calculating
     @return none
     @deprecated.
     */
    void SetPerspectiveCameraSetting(NBGM_CameraSetting setting);

     /*! Load navigation ecm route spline.
     @par
     If there are more then one spline, this func need be invoked multi-times to
     load spline data, or the maneuver turn arrow can not be rendered.
     @param data, spline data.
     @return PAL error code
     @deprecated.
     */
    PAL_Error AddNavEcmRouteSpline(const NBGM_SplineData* data);

     /*! Set current maneuver ID then draw the turn arrow.
     @param currentManuever, current manuever id;
     @return PAL error code
     @deprecated.
     */
    PAL_Error SetNavEcmCurrentManeuver(uint32 currentManueverID);

     /*! Reset ecm route layer.clear all the data in ecm route layer.
     @return none
     @deprecated.
     */
    void ResetNavEcmRouteLayer();

     /*! add new maneuver.
     @param maneuverID, manuever id;
     @param position, manuever offset;
     @return PAL error code
     @deprecated.
     */
    PAL_Error AddNavEcmManeuver(uint32 maneuverID, const NBGM_Point3d& position);

    /*! find the best position on route
     @param location, position,heading,speed info, input a not accurate location and get a accurate one on route.
     @param snapRouteInfo, the result of snapping.
     @return PAL error code
     @deprecated.
     */

    //TODO: This function would be changed in future
    PAL_Error NavEcmSnapRoute(NBGM_Location64& location, NBGM_SnapRouteInfo& snapRouteInfo) const;

    /*!Get avatar NBGM_SnapRouteInfo.
    @return avatar NBGM_SnapRouteInfo.
    @deprecated.
    */

    //TODO: This function would be changed in future
    NBGM_SnapRouteInfo& NavEcmGetAvatarSnapRouteInfo();

    /*! snap to the route start
    @param location, get location of route start.
    @param snapRouteInfo, the result of snapping.
    @return PAL error code
    @deprecated
    */

    //TODO: This function would be changed in future
    PAL_Error NavEcmSnapToRouteStart(NBGM_Location64& location, NBGM_SnapRouteInfo& pRouteInfo) const;

    //@deprecated.
    void SetIsNavMode(nb_boolean value) ;

    void SetCurrentZoomLevel(int8 zoomlevel);

    void AdjustFlagSize(float size, float minPixelSize, float maxPixelSize);

    /*! Retrieve profile
     @return none
     */
    void DumpProfile(NBGM_MapViewProfile& profile);

    /*! Get avatar Screen Position
     @param screenPosition, avatar position on screen
     @return true/fase. If this avatar position is on the screen return true, else return false.
     */
    nb_boolean GetAvatarScreenPosition(NBRE_Point2f& screenPosition);

    /*! Is avatar in screen
     @param position, avatar position
     @return true/fase. If this avatar is in screen return true, else return false.
     */
    nb_boolean IsAvatarInScreen(const NBRE_Vector3d& position);

    /*! Synchronize labels with font changes.
    @param materialCategoryName, material category
    @param materials, materials
     @return none.
     */
    void RefreshFontMaterial(const NBRE_String& materialCategoryName, const NBRE_Map<uint32, NBRE_FontMaterial>& materials);

    /*! Get Camera fov
     @return camera fov
     */
    float GetCameraFov() const;

    /*! Re-layout all labels.
     @return none.
     @deprecated.
     */
    void RefreshLabelLayout();

    /*! Add a tile animation layer which manages animation data like weather.
      @return NBGM_TileAnimationLayer
     */
    NBGM_TileAnimationLayer* CreateTileAnimationLayer();

    /*! Remove a tile animation layer. All of tiles loaded by this layer will be removed too.
      @return NBGM_TileAnimationLayer
     */
    void RemoveTileAnimationLayer(NBGM_TileAnimationLayer* layer);

    /*! Add a optional layer.
      @return NBGM_OptionalLayer
     */
    NBGM_OptionalLayer* CreateOptionalLayer();

    /*! Remove a optional layer. All of tiles loaded by this layer will be removed too.
      @return NBGM_OptionalLayer
     */
    void RemoveOptionalLayer(NBGM_OptionalLayer* layer);

    /*! Get if given screen position hit avatar.
      @return is avatar hit
     */
    nb_boolean SelectAndTrackAvatar(float x, float y);

    /*! Adjust label sky wall draw order
      @return none
     */
    void AdjustLabelSkyDrawOrder(uint8 drawOrder);

   /*! Snap a screenshot of NBGM map view. This function MUST be called in the render thread.
    @param rgbaBuffer, an RGBA buffer. Please note that the first pixel of the buffer represents the top left of the image.
    @param x, y Specify the window coordinates of the first pixel that is read from the framebuffer.This location is the lower left corner of a rectangular block of pixels.
    @param width, height Specify the dimensions of the pixel rectangle.
    @return true/false.
    */
    PAL_Error SnapScreenshot(int x, int y, uint32 width, uint32 height, uint8* rgbaBuffer) const;

    PAL_Error AsyncSnapScreenshot(const shared_ptr<NBGM_AsyncSnapScreenshotCallback>& snapScreenshot);

    void AddTask(NBGM_Task* task);

    NBGM_DeviceLocation* CreateDeviceLocation(std::string ID);
    void ReleaseDeviceLocation(NBGM_DeviceLocation* device);

    void InvalidateLayout();
    nb_boolean UpdateLayout();

    void Render();

    void EnableRenderLog();

    void SnapScreenshot();

    void AddTimerCallback(PAL_TimerCallback *callback, void *userData);

    void GetElementsAt(const NBRE_Point2f& screenPosition, uint32 selectMask, NBRE_Set<NBGM_ElementId>& objectIds);

    void EnableCollisionDetection(const NBRE_String& layerId, nb_boolean enable);

    /*! To check if the compass is tapped.
    @param screenX, is screen x coordinate.
    @param screenY, is screen y coordinate.
    @return nb_boolean. Return ture when tapped.
    */
    nb_boolean TapCompass(float screenX, float screenY);

    /*! To enable compass.
    @param value. Set TRUE to enable compass
    @return none.
    */
    void EnableCompass(nb_boolean value);

    /*! Set compass screen position.
    @param screenX. x screen coordinate.
    @param screenY. y screen coordinate.
    @return none.
    */
    void SetCompassPosition(float screenX, float screenY);

    /*! Set Copmass Day or Night mode.
     @param isDay, it's a boolean value, true means day mode, false means night mode
     */
    void SetCompassDayNightMode(nb_boolean isDay);

    /*! Set compass icons.
    @param dayModeIconPath. Icon path of day mode.
    @param nightModeIconPath. Icon path of night mode.
    @return none.
    */
    void SetCompassIcons(const NBRE_String& dayModeIconPath, const NBRE_String& nightModeIconPath);

     /*! Get the bounding box of the compass.
     * @param leftBottomX  -  x screen coordinate of left bottom point, in pixels.
     * @param leftBottomY  - y screen coordinate of left bottom point, in pixels.
     * @param rightTopX - x screen coordinate of right top point, in pixels.
     * @param rightTopY - y screen coordinate of right top point, in pixels.
     * @return None.
     */
    void GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY)const;

    /*! Create a circle instance.
     * @param ID - an unique ID for this circle
     * @param circlePara - parameters for creating circle
     * @return a circle pointer
     */
    virtual NBGM_Circle* AddCircle(int circleId, const NBGM_CircleParameters &circlePara);

    /*! remove the circle
     * @param circle - pointer to this circle.
     * @return None.
     */
    virtual void RemoveCircle(NBGM_Circle* circle);

    /*!Remove all the circles.
     * @return None.
     */
    virtual void RemoveAllCircles();

    /*! Create a rect2d instance.
     * @param rectId - an unique ID for this rect2d
     * @param para - parameters for creating rect2d
     * @return a pointer to the rect2d
     */
    virtual NBGM_CustomRect2d* AddRect2d(NBGM_Rect2dId rectId, const NBGM_Rect2dParameters &para);

    /*!Remove the rect2d
     * @param rect2d - pointer to this rect2d.
     * @return None.
     */
    virtual void RemoveRect2d(NBGM_CustomRect2d* rect2d);

    /*!Remove all the rect2ds.
     * @return None.
     */
    virtual void RemoveAllRect2ds();

    /*! Create a texture.
     * @param textureId - an unique ID for this texture
     * @param textureData.
     * @return True if sccussed.
     */
    virtual bool AddTexture(NBGM_TextureId textureId, const NBGM_BinaryBuffer &textureData);

    /*!Remove the texture
     * @param textureId - an unique ID for this texture
     * @return None.
     */
    virtual void RemoveTexture(NBGM_TextureId textureId);

    /*!Remove all the textures.
     * @return None.
     */
    virtual void RemoveAllTextures();

    /*! Add the pins.
     * @param pinParameters - parameters for adding the pins
     * @return None
     */
    virtual void AddPins(const NBRE_Vector<NBGM_PinParameters>& pinParameters);

    /*! Remove the pins.
     * @param pinIDs - specified the pins to remove
     * @return None
     */
    virtual void RemovePins(const NBRE_Vector<shared_ptr<NBRE_String> >& pinIDs);

    /*! Remove all the pins.
     * @return None
     */
    virtual void RemoveAllPins();

    /*! Update the pin position.
     * @param pinID - specified the pin to update
     * @param locationX, pin x position.
     * @param locationY, pin y position.
     * @return None
     */
    virtual void UpdatePinPosition(shared_ptr<NBRE_String> pinID, double locationX, double locationY);

    /*! Set MapView HBAO parameters.
     *  @param params - specified parameters
     *  @return None
     */
    virtual void SetHBAOParameters(const NBGM_HBAOParameters& parameters);

    /*! Set MapView glow parameters.
     *  @param params - specified parameters
     *  @return None
     */
    virtual void SetGlowParameters(const NBGM_GlowParameters& parameters);

    /*! Set MapView dpi.
     *  @param dpi - specified dpi
     *  @return None
     */
    virtual void SetDPI(float dpi);

public:
    /// these functions are only used to debug.
    NBGM_Context& GetNBGMContext();
    NBRE_Node* GetRootNode() const;
    NBRE_SurfaceSubView* GetSurfaceSubView() const;
    NBRE_DefaultOverlayManager<DrawOrderStruct>& GetOverlayManager();
    const NBRE_Point2i& GetScreenSize() const{return mScreenSize;}
    void AddLayoutTaskRef();
    void ReleaseLayoutTaskRef();

public:
    void UnloadSyncTiles(const NBRE_Vector<NBGM_VectorTileID>& ids);
    void UnloadSyncAllTiles();

private:
    class ResourceCallBack : public NBGM_IResourceListener
    {
    public:
        ResourceCallBack(NBGM_MapViewImpl* impl);
        virtual ~ResourceCallBack();
    public:
        void OnResponseNBMData(const NBRE_Vector<NBGM_NBMDataLoadResult>& result);
        void OnUpdateNBMData(const NBRE_Vector<NBGM_NBMDataLoadResult>& result);
    public:
        NBGM_MapViewImpl* mMapViewImpl;
        NBRE_Vector<NBRE_String> mUnloadTileNames;
    };

private:

    //render thread tasks
    void CreateLayer();
    void InitDrawOrder();
    float ViewPointDistanceWithScale();
    uint32 ConvertToInnerLayerId(uint8 drawOrder);
    void CheckBoundary();
    float CalculateViewPointSkyHeight();
    void RefreshTiles();
    nb_boolean GetFrustumPositionInWorld(double frustumHeight, NBRE_Vector<NBGM_Point2d64>& positions);

    void EndRender(float secondsPassed);
    void UpdatePosition();
    void UpdateLabel();
    void PrepareRender();

    //Timer
    PAL_Error ScheduleTimer(int32 milliseconds, PAL_TimerCallback *callback, void *userData);
    PAL_Error CancelTimer(PAL_TimerCallback *callback, void *userData);
    static void RenderCallBack(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);

private:
    static const int32 LAYER_STRIDE = 5;
    static const uint8 NBM_OVERLAY_LAYER = 160;
    static const uint8 NBM_TOP_LAYER = 208;

    enum DrawOrder
    {
        /// Use this queue for objects which must be rendered first e.g. backgrounds
        DO_BACKGROUND = 0,
        /// First queue (after backgrounds), used for skyboxes if rendered first
        DO_SKIES = 5,
        /// The default render queue
        DO_MAIN = 50,
        /// Use this queue for objects which must be rendered last e.g. overlays
        DO_OVERLAY = 65535,
        /// Use this queue for blocking buildings
        DO_TRANSPARENT_WALL,
        /// Use this queue for transparent road background
        DO_TRANSPARENT_ROAD_BG_OVERLAY,
        /// Use this queue for buildings
        DO_BUILDINGOVERLAY,
        /// Use this queue for nav vector height route(background/foreground route, arrow)
        DO_NAV_VECTOR_ROUTE = DO_BUILDINGOVERLAY + 200,
        /// Use this queue for nav ecm height route
        DO_NAV_ECM_ROUTE = DO_NAV_VECTOR_ROUTE + 100,
        /// Use this queue for nav ecm arrow
        DO_NAV_ECM_ARROW,
        /// 3D building & landmark layer
        DO_TRANSPARENTOVERLAY,
        DO_LABEL_TRANSPARENT_WALL,
        /// Use this queue for lables
        DO_LABLE,
        /// Road label in nav mode
        DO_NAV_LABLE,
        /// Navigation elements, such as flag, poi
        DO_NAV_ELEMENTS,
        // Use for circle
        DO_CIRCLE = DO_NAV_ECM_ARROW + 400,
        DO_AVATAR = DO_CIRCLE + 100,
        // Use for compass
        DO_COMPASS = DO_AVATAR + 50,
        DO_GLOW_OVERLAY,
        /// Final possible render queue, don't exceed this
        DO_TOPSETOVERLAY,
    };

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_MapViewImpl);

private:
    typedef NBRE_Set<uint8> LayerOrderArray;
    

private:

    nb_boolean  mInitialized;
    NBGM_MapViewImplConfig mConfig;
    NBGM_Context mNBGMContext;
    NBRE_CameraPtr mCamera;
    NBRE_TransformUtil* mTransUtil;
    nb_boolean mIsLayoutInvalidate;
    float mPickingRadius;

    NBRE_RenderSurface* mSurface;
    NBRE_SurfaceSubView* mSubView;
    NBRE_SceneManager* mSceneManager;

    NBGM_MapLayerArray mMapLayers;
    NBGM_TileMapLayer* mTileMapLayer;
    NBGM_LabelLayer* mLabelLayer;
    NBGM_PoiLayer* mPoiLayer;
    NBRE_Overlay* mScreenOverlay;

    NBRE_BillboardSet* mBillboardSet;
    NBGM_Avatar* mAvatar;
    NBGM_Sky* mSky;
    NBGM_Sky* mLabelTransparentSky;
    NBRE_Vector3d mTouchedPosition;
    nb_boolean mIsTouched;
    NBRE_Vector3d mRotatePosition;

    NBRE_NodePtr mMapCenter;
    float mHorizonDistance;
    float mSkyHeightOnViewport;
    float mTiltAngle;
    float mRotateAngle;
    float mFontScale;

    nb_boolean mEnableProfiling;
    NBGM_MapViewProfiler mProfiler;

    NBGM_Flag*                   mStartFlag;
    NBGM_Flag*                   mEndFlag;
    NBGM_NavIconLayer*           mNavIconLayer;
    nb_boolean                   mEnableDebugDraw;
    NBGM_GridEntity*             mGridEntity;

    NBGM_NavVectorRouteLayer*    mNavVecRouteLayer;

    NBGM_NavEcmRouteLayer*       mNavEcmRouteLayer;
    NBGM_SnapRouteInfo           mAvatarSnapRouteInfo;

    NBGM_PathArrowLayer*         mPathArrowLayer;

    uint32                       mLastUpdateMs;
    float                        mSyncRenderDelay;

    double                       mCurrentViewPointDistance;

    NBGM_TransparentLayer*       mTransparentLayer;
    NBGM_GlowLayer*              mGlowLayer;

    nb_boolean                   mIsInvalidate;
    nb_boolean                   mBackground;
    nb_boolean                   mSizeValid;
    nb_boolean                   mIsRasterMap;
    float                        mLastSkyHeight;
    NBRE_Vector2i                mLastViewportSize;


    NBRE_Vector<NBGM_TileAnimationLayerImpl*> mTileAnimationLayers;
    NBRE_Vector<NBGM_OptionalLayerImpl*> mOptionalLayers;

    //saves loaded tiles
    NBGM_MapViewDataCache*       mTileIdCache;  //store loaded tile

    //used by render/syc task
    shared_ptr<NBGM_TaskQueue>        mRenderThread;

    //used by finalization of map view
    PAL_Event*                              mLoadingThreadFinalizeEvent;

    NBRE_CameraPtr               mScreenCamera;

    int8                         mCurrentZoomLevel;
    nb_boolean                   mChangeZoomLevel;

    uint32						mRenderedFrameTime;
    uint32						mRenderedFrameCount;
    NBRE_Vector<NBGM_Avatar*>   mCreatedAvatar;
    NBRE_Vector<NBGM_CircleImpl*> mCreatedCircles;
    NBRE_Vector<NBGM_Rect2dImpl*> mCreatedRect2ds;
    NBGM_CustomObjectsContext mCustomObjectesContext;

    uint32  mRenderExpectedTime;
    nb_boolean  mEnableRenderLog;

    NBRE_Point2i    mScreenSize;
    NBRE_List<NBGM_FrameListener*> mPendingRenderListeners;

    shared_ptr<NBGM_AsyncSnapScreenshotCallback> mSnapScreenshot;
    int32 mLayoutTaskCount;
    nb_boolean mWaitingLayoutFinished;
    PAL_QueueHandle mRenderThreadHandle;

    PAL_TimerCallback* mTimerCallback;
    void* mTimerUserData;

    NBGM_Compass* mCompass;

    NBRE_DefaultOverlayManager<DrawOrderStruct>* mOverlayManager;

    DrawOrderStruct mBackgoundDrawOrder;
    DrawOrderStruct mSkyDrawOrder;
    DrawOrderStruct mTransparentWallDrawOrder;
    DrawOrderStruct mBuildingDrawOrder;
    DrawOrderStruct mVectorRouteDrawOrder;
    DrawOrderStruct mECMRouteDrawOrder;
    DrawOrderStruct mECMArrowDrawOrder;
    DrawOrderStruct mTransparentOverlayDrawOrder;
    DrawOrderStruct mGlowOverlayDrawOrder;
    DrawOrderStruct mLabelTransparentWallDrawOrder;
    DrawOrderStruct mLabelTransparentSkyDrawOrder;
    DrawOrderStruct mLabelDrawOrder;
    DrawOrderStruct mNavLabelDrawOrder;
    DrawOrderStruct mNavElementDrawOrder;
    DrawOrderStruct mAvatarDrawOrder;
    DrawOrderStruct mCompassDrawOrder;
    DrawOrderStruct mCustomObj2DDrawOrder;

    NBRE_Map<NBRE_String, NBGM_Icon*> mCreatedIcons;


};

/*! @} */

#endif



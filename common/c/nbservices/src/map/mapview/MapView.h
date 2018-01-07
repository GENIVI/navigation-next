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
    @file     MapView.h
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef _MAP_VIEW_
#define _MAP_VIEW_

#include "pal.h"
#include "palevent.h"
#include "MapViewInterface.h"
#include "MapViewUIInterface.h"
#include "LayerManagerListener.h"
#include "MapNativeView.h"
#include "Frame.h"
#include "LayerManager.h"
#include "MapViewTileCallback.h"
#include "MapViewLayerCallback.h"
#include "workerqueuemanager.h"
#include "Pin.h"
#include "PinCushion.h"
#include "BubbleListenerInterface.h"
#include "NBGMViewController.h"
#include <vector>
#include "MapServicesConfigurationListener.h"
#include "palmath.h"
#include "nbuitaskqueue.h"
#include "PrefetchProcessor.h"
#include "MapViewCamera.h"
#include "MapViewCameraListener.h"
#include "MapViewGestureHandler.h"
#include "MapViewPinHandler.h"
#include "HybridStrategy.h"

namespace nbmap
{

class MapView;
class MapViewTask;

/*! Used for MapViewInitialize and MapViewDestroy */
enum EventType
{
    ET_CCC,     /*!< Event for CCC thread */
    ET_RENDER   /*!< Event for NBGM render thread */
};

/*! MapView Context, only accessed in UI thread.*/
class MapViewUIThreadContext
{
public:
    MapViewUIThreadContext():
    lastGpsLat(INVALID_LONGITUDE),
    lastGpsLon(INVALID_LATITUDE),
    lastGpsMode(NGM_INVALID),
    gestrueProcessingFlag(false){}

    double lastGpsLat;         /*! last gps latitude for calculating day&night mode */
    double lastGpsLon;         /*! last gps longitude for calculating day&night mode */
    NB_GpsMode lastGpsMode;    /*! last gps mode */
    bool gestrueProcessingFlag;
};

// MapViewInitialize .................................................................................................
/*
    Class to synchronize MapView initialization.
    This can be moved to its own header if desired.
*/
class MapViewInitialize
{
public:
    MapViewInitialize();
    virtual ~MapViewInitialize();

    /*! This function gets called from MapView::Initialize() and MapView::LayerRequestSuccess(). Only if both functions
        call it do we request the common materials and update the frame */
    void RequestCommonMaterialsIfNecessary(MapView* pMapView);
    /*! Check if common materials can be requested*/
    bool CommonMaterialsReadyToRequest() const;

    /*! Set/Check if rendering is allowed. It is only allowed after we have the common matterials requested */
    void SetRenderingAllowed(int status);
    bool IsRenderingAllowed() const;
    void ResetRenderingState();


    /*! Set/Check if initialization is done. It is done when CCC and NBGM are initialized */
    void SetDone();
    bool IsDone() const;

    /*! Set/Get shutting down state */
    void SetShuttingDown();
    bool IsShuttingDown() const;

    /*! We call this callback once MapView is fully initialized/destroyed */
    shared_ptr<AsyncCallback<void*> > m_initializeCallback;
    shared_ptr<AsyncCallback<void*> > m_destroyCallback;

    /*! Flag to indicate if MapView is valid. This flag gets set to true when the instance is created and set to 'false'
        just before the instance is destroyed. It is used in task callbacks */
    shared_ptr<bool> m_isValid;

private:

    /*! We count how many times RequestCommonMaterialsIfNecessary() gets called. Only if it gets called twice do we
        trigger the task */
    PAL_Lock* m_lock;
    int m_commonMaterialRequestCount;

    /*! Flag to indicate if rendering is allowed. Only after the layers have been successfully initialized do we allow
     any rendering calls */
    int m_renderingAllowed;

    /*! Flag to indicate if initialization is done (both CCC and NBGM are initialized)*/
    bool m_done;

    /*! Flag to indicate that we are shutting down */
    bool m_shuttingDown;
};

// AsyncGenerateMapImageCallback .................................................................................................

class AsyncGenerateMapImageCallback : public NBGM_AsyncSnapScreenshotCallback
{
public:
    AsyncGenerateMapImageCallback(MapView* pMapView,
                                  const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > > asyncGenerateMapImageCallback,
                                  shared_ptr<bool> screenshotInProgress);
    virtual ~AsyncGenerateMapImageCallback(){};

    /*! NBGM call this callback when screen short is finished.
        This function run in Render Thread.
    */
    virtual void OnScreenshotFinished(int width, int height, const uint8* buffer, uint32 bufferSize);

private:
    const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > > m_pAsyncGenerateMapImageCallback;
    shared_ptr<bool> m_pScreenshotInProgress;
    MapView* m_pMapView;
    shared_ptr<bool> m_isValid;         /*!< Flag to indicate if MapView instance is valid. */
};

// MapView ...........................................................................................................

class PinManager;
class CustomLayerManager;
class TrafficManager;
class DAMUtility;
class MapServicesConfigurationImpl;
class CombinedAtomicTask;

/*! Map view implementation class.

    This is the core class of the map engine. It connects to the LayerManager to retrieve tiles and forwards the
    tiles to nbgm. It also connects the native view in nbui.
*/
class MapView : public MapViewInterface,
                public MapViewUIInterface,
                public LayerManagerListener,
                public BubblelistenerInterface,
                public MapServicesConfigurationListener,
                public MapViewCameraListener
{
public:

    friend class MapViewInitialize;

    /*! MapView constructor */
    MapView(MapServicesConfigurationImpl* serviceConfiguration);

    NB_Error Initialize(NB_Context* nbContext,
                        shared_ptr<AsyncCallback<void*> > initializeCallback,
                        shared_ptr<AsyncCallback<void*> > destroyCallback,
                        const char* workFolder,
                        const char* resourceFolder,
                        double defaultLatitude,
                        double defaultLongitude,
                        double defaultTiltAngle,
                        shared_ptr<LayerManager> layerManager,
                        shared_ptr<PinCushion> pinCushion,
                        shared_ptr<PinManager> pinManager,
                        shared_ptr<CustomLayerManager> customLayerManager,
                        uint8 preferredLanguageCode,
                        int zorder_level,
                        void* mapViewContext,
                        shared_ptr<MetadataConfiguration> metadataConfig,
                        MapViewListener* pListener,
                        MapViewFontMagnifierLevel fontMagnifierLevel,
                        float scaleFactor);

    // MapViewInterface functions ......................................................................................

    /* See MapViewInterface.h for description */
    virtual NB_Error Destroy();
    virtual void InitializeRenderer();
    virtual void DestroyRenderer();
    virtual NB_Error ShowView();
    virtual NB_Error SetMapViewContext(void* context);
    virtual void SetLayerManager(LayerManagerPtr layerManager);
    virtual NB_Error ShowRouteLayer(bool isShown);
    virtual void* GetNativeContext();
    virtual void SetGpsMode(NB_GpsMode gpsMode);
    virtual NB_GpsMode GetGpsMode()const;
    virtual void SetAvatarState(bool headingValid);
    virtual void SetAvatarMode(MapViewAvatarMode mode);
    virtual void SetAvatarLocation(const NB_GpsLocation& gpsLocation);
    virtual void SetCustomAvatar(shared_ptr<nbcommon::DataStream> directionalAvatar, shared_ptr<nbcommon::DataStream> directionlessAvatar,
                                 int8 directionalCalloutOffsetX, int8 directionalCalloutOffsetY,
                                 int8 directionlessCalloutOffsetX, int8 directionlessCalloutOffsetY);
    virtual void SetCustomAvatar(shared_ptr<std::string> directionalAvatarPath, shared_ptr<std::string> directionlessAvatarPath,
                                 int8 directionalCalloutOffsetX, int8 directionalCalloutOffsetY,
                                 int8 directionlessCalloutOffsetX, int8 directionlessCalloutOffsetY);
    virtual void SetCustomAvatarHaloStyle(shared_ptr<GeographyMaterial> haloStyle);
    /*! Show / hide debug view */
    void ShowDebugView(bool enabled);

    virtual void SetBackground(bool background);
    virtual void SetBackgroundSynchronized(bool background);


    virtual NB_Error WindowToWorld(int dX, int dY, double* lat, double* lon);
    virtual NB_Error SetCamera(const MapViewCameraParameter& cameraParameter);
    virtual NB_Error GetCamera(MapViewCameraParameter& cameraParameter);
    virtual NB_Error SetCameraSettings(const MapViewCameraParameter& cameraParameter, bool animated = FALSE, uint32 duration = 0, MapViewCameraAnimationAccelertaionType type = MVCAAT_LINEAR);
    virtual PAL_Instance* GetPalInstance();
    virtual shared_ptr<list<string> > GetInteractedPois(float x, float y);
    virtual NB_Error WorldToWindow(double latitude, double longitude, int* dX, int* dY);
    virtual double GetCurrentZoomLevel();

    virtual NB_Error SetViewSettings(MapViewSettings viewSetting, bool flag);
    virtual NB_Error SetViewSettings(MapViewSettings viewSetting, bool flag, float portraitFOV, float landscapeFOV);
    virtual NB_Error SetDisplayScreen(int screenIndex);
    virtual void SetGpsFileName(const char* gpsFileName);
    virtual NB_Error SetTheme(MapViewTheme theme);
    virtual NB_Error SetAnimationLayerSettings(unsigned int totalAnimationTime, unsigned int loopPause, double minimumTilePercentage);
    virtual NB_Error SetAnimationLayerOpacity(unsigned int opacity);

    virtual void SetViewPort(int width, int height);
    virtual bool GetRotateByGestureSupported();
    virtual bool GetTiltByGestureSupported();
    virtual bool GetInlineTrafficSupported();

    virtual void ShowRouteLayer(uint32 layerID);
    virtual void HideRouteLayer(uint32 layerID);

    virtual NB_Error GenerateMapImage(shared_ptr<GenerateMapCallback> callback, int32 x = 0, int32 y = 0, uint32 width = 0, uint32 height = 0) const;
    virtual NB_Error GenerateMapImage(shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > >  callback);
    virtual void CancelGenerateMapImage();
    virtual void EnableGetTileWithFrame(bool enabled);
    virtual void SelectPin(shared_ptr<string> pinId);
    virtual void UnselectPin(shared_ptr<string> pinId);
    virtual void SetShowLabelDebugInfo(bool showLabelDebugInfo);
    virtual void RefreshMetadataAndLayers(shared_ptr<MetadataConfiguration> metadataConfig);

    virtual void GetElementsAt(float screenX, float screenY, set<std::string>& elements);
    virtual void EnableCustomLayerCollisionDetection(const std::string& layerId, bool enable);

    virtual void Prefetch(shared_ptr<vector<pair<double, double> > > polylineList,
                    double prefetchExtensionLengthMeters, double prefetchExtensionWidthMeters, float zoomLevel);

    virtual void Prefetch(double lat, double lon, float zoomLevel, float heading, float tilt);

    /*! Add a render task (if necessary) */
    virtual void AddRenderTask(RenderListener* listener = NULL);

    /*! Add a reset screen size task (if necessary) */
    virtual void AddResetScreenTask();


    virtual void SetNightMode(MapViewNightMode nightMode);
    virtual bool IsNightMode();

    virtual NB_Error SetCustomPinBubbleResolver(shared_ptr<PinBubbleResolver> bubbleResolver);

    /* Switch MapView from immediate mode to retained mode */
    virtual void BeginAtomicUpdate();
    /* Switch MapView from retained mode to immediate mode */
    virtual void EndAtomicUpdate();
    virtual void GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY)const;
    /* Stop the animation */
    virtual void StopAnimations(int type);
    /* Sets the scale value of the Text. */
    virtual void SetFontScale(float scale);
    /* Sets the scale value of the Avatar. */
    virtual void SetAvatarScale(float scale);
    virtual void AddPins(std::vector<PinParameters<BubbleInterface> >& parametersVector);
    virtual void RemovePins(const vector<shared_ptr<string> >& pinIDs);
    virtual void RemoveAllPins();
    virtual void UpdatePinPosition(shared_ptr<string> pinID, double lat, double lon);
    virtual void SetHBAOParameters(const HBAOParameters& parameters);
    virtual void SetGlowParameters(const GlowParameters& parameters);
	virtual void SetDPI(float dpi);
	
    // MapViewUIInterface functions ....................................................................................

    /* See MapViewUIInterface.h for description */
    virtual NB_Error UI_InitializeMapView();
    virtual NB_Error UI_Render(uint32 interval);
    virtual NB_Error UI_SetCurrentPosition(double lat, double lon);
    virtual NB_Error UI_SetViewPort(int width, int height);
    virtual NB_Error UI_SetScreenOrientation(bool bPortrait);
    virtual NB_Error UI_HandleViewGesture(const MapView_GestureParameters* parameters);
    virtual void UI_SetCamera(double lat, double lon, double zoomLevel, double heading, double tilt, bool animated = false,
                              uint32 duration = 0, MapViewUI_CameraAnimationAccelertaionType type = MVUICAAT_LINEAR);
    virtual float UI_GetCameraRotationAngle();
    virtual NB_Error UI_SetViewSize(int width, int height);
    virtual void UI_SetGpsMode(NB_GpsMode mode);
    virtual void UI_MapNativeViewCreated();
    virtual void UI_SetDopplerOpacity(uint32 opacity);
    virtual void UI_SetDopplerPlayState(bool play);
    virtual shared_ptr<BubbleInterface> UI_GetBubble(shared_ptr<std::string> pinID);
    virtual void UI_GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY) const;

    // CameraListener functions
    virtual void OnCameraAnimationDone(int animationId, MapViewAnimationStatusType animationStatus);
    virtual void OnCameraLimited(MapViewCameraLimitedType type);
    // LayerManagerListener functions ..................................................................................

    /* See LayerManagerListener.h for description */
    virtual void RefreshAllTiles();
    virtual void RefreshTilesOfLayer(LayerPtr layer);
    virtual void ReloadTilesOfLayer(LayerPtr layer);
    virtual void RefreshTiles(const std::vector<TileKeyPtr>& tileKeys, LayerPtr layer);
    virtual void LayersUpdated(const std::vector<LayerPtr>& layers);
    virtual void LayersAdded(const std::vector<LayerPtr>& layers);
    virtual void LayersRemoved(const std::vector<LayerPtr>& layers);
    virtual void AnimationLayerAdded(shared_ptr<Layer> layer);
    virtual void AnimationLayerUpdated(shared_ptr<Layer> layer, const vector<uint32>& frameList);
    virtual void AnimationLayerRemoved(shared_ptr<Layer> layer);
    virtual void PinsRemoved(shared_ptr<std::vector<PinPtr> > pins);
    virtual void EnableMapLegend(bool enabled, shared_ptr<MapLegendInfo> mapLegend);
    virtual void AllTilesDownloadFinished(const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > >  callback);

    virtual void MaterialAdded(const ExternalMaterialPtr& material);
    virtual void MaterialRemoved(const ExternalMaterialPtr& material);
    virtual void MaterialActived(const ExternalMaterialPtr& material);

    // BubblelistenerInterface functions ..................................................................................

    /* See BubblelistenerInterface.h for description */
    virtual void NotifyShowBubble(shared_ptr<std::string> pinId, float x, float y, bool orientation);
    virtual void NotifyHideBubble(shared_ptr<std::string> pinId);
    virtual void NotifyUpdateBubble(shared_ptr<std::string> pinId, float x, float y, bool orientation);
    virtual void NotifyShowLocationBubble(float x, float y, bool orientation,
                                          float latitude, float longitude);
    virtual void NotifyHideLocationBubble();
    virtual void NotifyUpdateLocationBubble(float x, float y, bool orientation,
                                            float latitude, float longitude);
    void NotifyShowStaticPoiBubble(float x, float y, bool orientation);
    void NotifyHideStaticPoiBubble();
    void NotifyUpdateStaticPoiBubble(float x, float y, bool orientation);

    /* See MapServicesConfigurationListener for description*/
    virtual void PreferredLanguageChanged(uint8 languageCode);
    virtual void MapThemeChanged(MapViewTheme theme);

    // Other public functions ..........................................................................................

    /*! Gets called when the layers have been successfully retrieved from the layer manager */
    void LayerRequestSuccess(const vector<LayerPtr>& layers);

    /*! Gets called when the layers were not received successfully */
    void LayerRequestError(NB_Error error);

    /*! Called when a tile is available.

        Called from TileCallback.
    */
    void TileRequestSuccess(TilePtr response     /*!< Returned tile */
                            );

    /*! Called when a tile request failed.

        Called from TileCallback.
     */
    void TileRequestError(NB_Error error        /*!< Error of request */
                          );

    /*! Called when Common materials are returned

        Called from CommonMaterialRequestSuccess
     */
    bool IsAllCommonMaterialDownloaded();

    /*! Called when Common materials are returned

        Called from CommonMaterialRequestCallback
     */
    void CommonMaterialRequestSuccess(TileKeyPtr request,  /*!< Tile key of requested tile */
                                      TilePtr response     /*!< Requested tile */
                                      );

    /*! Called when request to get common material fails

     Called from CommonMaterialRequestCallback
     */
    void CommonMaterialRequestError(TileKeyPtr request,  /*!< Tile key of requested tile */
                                    NB_Error error       /*!< Error of request */
                                    );
    
    /* Called when layers changed */
    void UpdateSatelliteMode();

    /*! Provide current zoom level to Frame class

     Used to request tiles for new zoom level

     */
    void SetFrameZoomAndCenter();

    /*! Get the version of NBGM library

        @return NBGM version
    */
    static uint32 GetNBGMVersion();

    /*! Return a flag to indicate if this mapview is still valid.

        @return a shared_ptr of flag, this shared pointer can be stored by MapViewTasks, so
                that they can keep track of the availability of MapView instance.
    */

    shared_ptr<bool> IsValid();

    bool IsFirstTilesLoaded(void);

    /*! Notify GenerateMapImage Callback*/
    void NotifyAsyncGenerateMapImageCallback(const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > > pAsyncGenerateMapImageCallback,
                                             shared_ptr<nbcommon::DataStream> dataStream,
                                             NB_Error                         error);

    /**
     * @name ShowCompass - Set true/false to show/dispear compass.
     * @return None.
     */
    void ShowCompass(bool enabled);

    /**
     * @name SetCompassPosition - Set compass's screen position.
     * @param screenX - x screen coordinate.
     * @param screenY - y screen coordinate.
     * @return None.
     */
    void SetCompassPosition(float screenX, float screenY);

    /*! Set compass icons.
    * @param dayModeIconPath. Icon path of day mode.
    * @param nightModeIconPath. Icon path of night mode.
    * @return none.
    */
    void SetCompassIcons(const std::string& dayModeIconPath, const std::string& nightModeIconPath);

    /*! update the rendering surface.
        @return none
     */
    void UI_UpdateSurface();

    /*! destroy the rendering surface.
        @return none
     */
    void UI_DestroySurface();

    /*! Play Doppler.
     * @return None.
     */
    void PlayDoppler();

    /*! Pause Doppler.
     * @return None.
     */
    void PauseDoppler();

    /*! SelectAvatarAndPin
     *  @param
     *  @param
     *  @return none
     */
    void SelectAvatarAndPin(float screenX, float screenY);

    /* Updata Doppler by gesture
     * @return none
     */
    void UpdateDopplerByGesture(bool isUpdate);

    /* Long pressed
     * @return nonoe
     */
    void OnLongPressed(double lat, double lon);

    /* Set GestureProcessing
     * @return nonoe
     */
    void SetGestureProcessing(bool isProcessing);

    /*! Add a circle to the map.
     *  @param circleId - indentity of this circle.
     *  @param lat, latitude of the center
     *  @param lon, longitude of the center
     *  @param radius, in pixels
     *  @param fillColor, in RGBA
     *  @param strokeColor, in RGBA
     *  @param zOrder, draw order
     *  @param visible
     *  @return None.
     */
    void AddCircle(int circleId,  double lat, double lon, float radius, int fillColor, int strokeColor, int zOrder, bool visible);

    /*! remove the circle
     *  @param circleId - indentity of this circle.
     *  @return None.
     */
    void RemoveCircle(int circleId);

    /*! Remove all the circls.
     *  @return None.
     */
    void RemoveAllCircles();

    /*! Set the center of the circle
     *  @param circleId - indentity of this circle.
     *  @param lat - latitude of this center.
     *  @param lon - longitude of this center.
     *  @return None.
     */
    virtual void SetCircleCenter(int circleId, double lat, double lon);

    /*! Set the visiblity of the circle
     *  @param circleId - indentity of this circle.
     *  @param visible.
     *  @return None.
     */
    virtual void SetCircleVisible(int circleId, bool visible);

    /*! Set the style of the circle
     *  @param circleId - indentity of this circle.
     *  @param radius - radius of this center, in meters
     *  @param fillColor - RGBA
     *  @param outlineColor - RGBA
     *  @return None.
     */
    virtual void SetCircleStyle(int circleId, float radius, int fillColor, int outlineColor);

    /*! set a relative center of map, in follow me mode avatar will be located in this position instead of screen center, and rotate and zoom will base on this position too.
     * @param x - X Screen Position in terms of pixel.
     * @param y - Y Screen Position in terms of pixel.
     * @return None.
     */
    void SetReferenceCenter(float x, float y);

    /*! enable or disable reference center.
     * @param enable - turn on or turn off ReferenceCenter
     * @return None.
     */
    void EnableReferenceCenter(nb_boolean enable);

    /*! Add a rect2d to the map.
     *  @param rectId - indentity of this rect2d.
     *  @param lat, latitude of the center
     *  @param lon, longitude of the center
     *  @param width, in pixels
     *  @param height, in pixels
     *  @param heading, in degree
     *  @param textureId
     *  @param textureBuffer
     *  @param visible
     *  @return None.
     */
    void AddRect2d(int rectId,  double lat, double lon, float heading, float width, float height, int textureId, shared_ptr<nbcommon::DataStream> textureBuffer, bool visible);

    /*! remove the rect2d
     *  @param rectId - indentity of this rect2d.
     *  @return None.
     */
    void RemoveRect2d(int rectId);

    /*! Update the rect2d
     *  @param rect2d - indentity of this rect2d.
     *  @param lat - latitude of this center.
     *  @param lon - longitude of this center.
     *  @param heading - heading angle of this rect2d, in degrees.
     *  @return None.
     */
    void UpdateRect2d(int rectId, double lat, double lon, float heading);

    /*! Set the visiblity of the rect2d
     *  @param rect2d - indentity of this rect2d.
     *  @param visible.
     *  @return None.
     */
    void SetRect2dVisible(int rectId, bool visible);

    /*! Set the size of the rect2d
     *  @param rect2d - indentity of this rect2d.
     *  @param width - in pixels.
     *  @param height - in pixels.
     *  @return None.
     */
    void SetRect2dSize(int rectId, float width, float height);

    /*! Remove all the rect2d.
     *  @return None.
     */
    void RemoveAllRect2ds();

    /*! Add a texture to the map.
     *  @param textureId, unique identity to the texture
     *  @param textureData
     *  @return None.
     */
    void AddTexture(int textureId, nbcommon::DataStreamPtr textureData);

    /*! Remove the texture.
     *  @param textureId - unique identity to the texture
     *  @return None.
     */
    void RemoveTexture(int textureId);

    /*! Remove all the rect2d.
     *  @return None.
     */
    void RemoveAllTextures();

    /*! Update the debug UI if it is enabled */
    void UpdateDebugUI();

private:

    /*! MapView destructor

        Avoid calling delete to destroy instances of map view. User should always call the function
        Destroy to destroy map view.
    */
    virtual ~MapView();


    // Callback functions from the CCC thread...........................................................................

    /*! Initialize MapView */
    NB_Error CCC_Initialize();

    /*! Initialize the layer manager.

        Has to be called before requesting any tiles.
     */
    NB_Error CCC_InitializeLayerManager();

    /*! Get the pin by a pin ID

        @return A shared pointer to the pin
    */
    shared_ptr<Pin> CCC_GetPin(shared_ptr<string> pinId);

    /*! Call the callback to the application level to indicate that the map is fully initialized */
    void CCC_InitializeComplete(const MapViewTask* pTask);

    /*! Shutdown CCC thread */
    void CCC_Shutdown(const MapViewTask* pTask);

    /*! Call the callback to the application level to indicate that the map was destroyed */
    void CCC_ShutdownComplete(const MapViewTask* pTask);

    /*! Update the frame in the CCC context */
    void CCC_UpdateFrame(const MapViewTask* pTask);

    /*! Download specified tiles using LayerManager. */
    void CCC_GetSpecifiedTiles(const MapViewTask* pTask);

    /*! Show or hide the route layer */
    void CCC_ShowRouteLayer(const MapViewTask* pTask);

    /*! Request metadata */
    void CCC_GetLayers(const MapViewTask* pTask);

    /*! Request common materials */
    void CCC_RequestCommonMaterials(const MapViewTask* pTask);

    /*! Set the theme for the map*/
    void CCC_SetTheme(const MapViewTask* pTask);

    /*! Show the pin bubble */
    void CCC_ShowPinBubble(const MapViewTask* pTask);

    /*! Hide the pin bubble */
    void CCC_HidePinBubble(const MapViewTask* pTask);

    /*! Update the pin bubble */
    void CCC_UpdatePinBubble(const MapViewTask* pTask);

    /*! Show the static poi bubble */
    void CCC_ShowStaticPoiBubble(const MapViewTask* pTask);

    /*! Hide the static poi bubble */
    void CCC_HideStaticPoiBubble(const MapViewTask* pTask);

    /*! Update the static poi bubble */
    void CCC_UpdateStaticPoiBubble(const MapViewTask* pTask);

    /*! Drop a pin */
    void CCC_DropPin(const MapViewTask* pTask);

    /*! change static poi bubble */
    void CCC_ChangeStaticPoiBubble(const MapViewTask* pTask);

    /*! Check and load tile. */
    void CCC_CheckAndLoadTile(const MapViewTask* pTask);

    /*! Refresh all tile. */
    void CCC_RefreshAllTiles(const MapViewTask* pTask);

    /*! Refresh raster tiles. */
    void CCC_RefreshRasterTiles(const MapViewTask* pTask);

    /*! Check traffic status, and update traffic tip if necessary. */
    void CCC_CheckTrafficStatus(const MapViewTask* pTask);

    /*! Remove the pin bubble */
    void CCC_RemovePinBubble(const MapViewTask* pTask);

    /*! Show location bubble */
    void CCC_ShowLocationBubble(const MapViewTask* pTask);

    void CCC_PrepareAsyncGenerateMapImage(const MapViewTask* pTask);

    /*! Notify GenerateMapImage Callback. */
    void CCC_NotifyAsyncGenerateMapImageCallback(const MapViewTask* pTask);

    /*! Set custom pin bubble resolver. */
    void CCC_SetCustomPinBubbleResolver(const MapViewTask* pTask);

    void CCC_SetFrameZoomAndCenter(const MapViewTask* pTask);

    void CCC_PrefetchForNAV(const MapViewTask* pTask);

    void CCC_Prefetch(const MapViewTask* pTask);

    void CCC_PinClickedListener(const MapViewTask* pTask);

    void CCC_UnselectAllPinsListener(const MapViewTask* pTask);

    void CCC_MarkerClickedListener(const MapViewTask* pTask);
    // Function called in NBGM render thread ...........................................................................

    /*! Initialize NBGM View controller */
    void RenderThread_Initialize(const MapViewTask* pTask);

    /*! Shutdown render thread */
    void RenderThread_Shutdown(const MapViewTask* pTask);

    /*! Initialize or destroy the renderer 'NBGMViewController' */
    void RenderThread_InitializeRenderer();
    void RenderThread_DestroyRenderer();
    void RenderThread_InitializeRendererWithTask(const MapViewTask* pTask);
    void RenderThread_DestroyRendererWithTask(const MapViewTask* pTask);

    /*! Render nbgm context in separate thread */
    void RenderThread_NBGMRender(const MapViewTask* pTask);

    /* Reset screen size in render thread*/
    void RenderThread_ResetScreen(const MapViewTask* pTask);

    /*! Remove the pin bubble */
    void RenderThread_RemovePinBubble(const MapViewTask* pTask);

    /*! Load tile to NBGM */
    void RenderThread_LoadTilesToNBGM(const MapViewTask* pTask);

    /*! Unload tiles from NBGM */
    void RenderThread_UnloadTilesToNBGM(const MapViewTask* pTask);

    /*! Unload all tiles from NBGM containing NBGM cache */
    void RenderThread_UnloadAllTilesFromNBGM(const MapViewTask* pTask);

    /*! Update Common Materials */
    void RenderThread_UpdateCommonMaterials(const MapViewTask* pTask);

    /*! Handle animation layer added/removed/updated calls */
    void RenderThread_HandleAnimationLayer(const MapViewTask* pTask);
    void RenderThread_UpdateAnimationLayerParameters(const MapViewTask* pTask);
    void RenderThread_UpdateAnimationLayerOpacity(const MapViewTask* pTask);

    /*! See corresponding function in MapViewInterface.h */
    void RenderThread_SetGpsMode(const MapViewTask* pTask);
    void RenderThread_SetAvatarState(const MapViewTask* pTask);
    void RenderThread_SetAvatarMode(const MapViewTask* pTask);
    void RenderThread_SetAvatarLocation(const MapViewTask* pTask);
    void RenderThread_SetCustomAvatar(const MapViewTask* pTask);
    void RenderThread_SetCustomAvatarHaloStyle(const MapViewTask* pTask);
    void RenderThread_SetBackground(const MapViewTask* pTask);
    void RenderThread_SetViewSettings(const MapViewTask* pTask);
    void RenderThread_SetCameraSettings(const MapViewTask* pTask);
    void RenderThread_AddPins(const MapViewTask* pTask);
    void RenderThread_RemovePins(const MapViewTask* pTask);
    void RenderThread_RemoveAllPins(const MapViewTask* pTask);
    void RenderThread_UpdataPinPosition(const MapViewTask* pTask);

    /*! Handle MapViewUIInterface functions */
    void RenderThread_SetCurrentPosition(const MapViewTask* pTask);
    void RenderThread_SetViewPort(const MapViewTask* pTask);
    void RenderThread_SetScreenOrientation(const MapViewTask* pTask);

    /*! Handle map view screenshot functions */
    void RenderThread_GenerateMapImage(const MapViewTask* pTask);

    void RenderThread_AsyncGenerateMapImage(const MapViewTask* pTask);

    void RenderThread_PrepareAsyncGenerateMapImage(const MapViewTask* pTask);
    void RenderThread_SetFrameZoomAndCenter(const MapViewTask* pTask);

    void RenderThread_ModifyExternalMaterial(const MapViewTask* pTask);


    void RenderThread_RunCombinedAtomicTask(const MapViewTask* pTask);
    void RenderThread_SelectAvatarAndPin(const MapViewTask* pTask);
    void RenderThread_SelectPin(const MapViewTask* pTask);
    void RenderThread_ReloadTiles(const MapViewTask* pTask);

    /*! Update Doppler playing or pausing state */
    void RenderThread_UpdateDopplerState(const MapViewTask* pTask);

    void RenderThread_UpdateDopplerStateByGesture(const MapViewTask* pTask);

    /*! Add circle in render thread */
    void RenderThread_AddCircle(const MapViewTask* pTask);

    /*! Remove circle in render thread */
    void RenderThread_RemoveCircle(const MapViewTask* pTask);

    /*! Remove all the circles in render thread */
    void RenderThread_RemoveAllCircles(const MapViewTask* pTask);

    /*! Set circle center in render thread */
    void RenderThread_SetCircleCenter(const MapViewTask* pTask);

    /*! Set circle visible in render thread */
    void RenderThread_SetCircleVisible(const MapViewTask* pTask);

    /*! Set circle style in render thread */
    void RenderThread_SetCircleStyle(const MapViewTask* pTask);

    /*! Detect if play Doppler;
     * @param nextFrame - if it is true, doppler plays next frame of the paused.
     * @param beginningFrame - if it is true, doppler plays the begining frame.
     * @return true for playing or false for pausing.
     */
    bool RenderThread_PlayDoppler(bool& nextFrame, bool& beginningFrame);

    /* Set Reference Center*/
    void RenderThread_SetReferenceCenter(const MapViewTask* pTask);

    /* Enable Reference Center*/
    void RenderThread_EnableReferenceCenter(const MapViewTask* pTask);

    /*! Add rect2d in render thread */
    void RenderThread_AddRect2d(const MapViewTask* pTask);

    /*! Remove rect2d in render thread */
    void RenderThread_RemoveRect2d(const MapViewTask* pTask);

    /*! Set the visiblity of the rect2d in render thread */
    void RenderThread_SetRect2dVisible(const MapViewTask* pTask);

    /*! Update rect2d in render thread */
    void RenderThread_UpdateRect2d(const MapViewTask* pTask);

    /*! Set the size of the rect2d in render thread */
    void RenderThread_SetRect2dSize(const MapViewTask* pTask);

    /*! Remove all the rect2d in render thread */
    void RenderThread_RemoveAllRect2ds(const MapViewTask* pTask);

    /*! Add texture in render thread */
    void RenderThread_AddTexture(const MapViewTask* pTask);

    /*! Remove texture in render thread */
    void RenderThread_RemoveTexture(const MapViewTask* pTask);

    /*! Remove all the textures in render thread */
    void RenderThread_RemoveAllTextures(const MapViewTask* pTask);
    
    /*! Set font scale in render thread */
    void RenderThread_SetFontScale(const MapViewTask* pTask);

    /*! Set avatar scale in render thread */
    void RenderThread_SetAvatarScale(const MapViewTask* pTask);

    /*! Set gesture processing in render thread */
    void RenderThread_RenderTaskSetGestureProcessing(const MapViewTask* pTask);

    /*! Clean all the materials in render thread */
    void RenderThread_CleanMaterilas(const MapViewTask* pTask);

    /*! Set HBAO parameters in render thread */
    void RenderThread_SetHBAOParameters(const MapViewTask* pTask);

    /*! Set glow parameters in render thread */
    void RenderThread_SetGlowParameters(const MapViewTask* pTask);

    /*! Set DPI in render thread */
    void RenderThread_SetDPI(const MapViewTask* pTask);
    // Function called in UI thread ...........................................................................

    /*! Initialize native view in nbui */
    void UI_InitializeNativeView(const MapViewTask* pTask);

    /*! Destroy the map native view */
    void UI_ShutdownNativeView(const MapViewTask* pTask);

    /*! Output debug UI */
    void UI_OutputDebugUI(const MapViewTask* pTask);

    /*! Handle UI part for adding/removing/updating animation layers */
    void UI_HandleAnimationLayer(const MapViewTask* pTask);

    /*! Show the map legend */
    void UI_ShowMapLegend(const MapViewTask* pTask);

    /*! Hide the map legend */
    void UI_HideMapLegend(const MapViewTask* pTask);

    /*! Update Traffic Tip */
    void UI_UpdateTrafficTip(const MapViewTask* pTask);


    /*! Show location bubble */
    void UI_UpdateLocationBubble(const MapViewTask* pTask);

    /*! Hide location bubble */
    void UI_HideLocationBubble(const MapViewTask* pTask);

    /*! Show the pin bubble */
    void UI_ShowPinBubble(const MapViewTask* pTask);

    /*! Hide the pin bubble */
    void UI_HidePinBubble(const MapViewTask* pTask);

    /*! Update the pin bubble */
    void UI_UpdatePinBubble(const MapViewTask* pTask);

    /*! Remove the pin bubble */
    void UI_RemovePinBubble(const MapViewTask* pTask);

    /*! Show the bubble */
    void UI_ShowBubble(const MapViewTask* pTask);

    /*! Hide the bubble */
    void UI_HideBubble(const MapViewTask* pTask);

    /*! Update the bubble */
    void UI_UpdateBubble(const MapViewTask* pTask);

    /*! Remove the bubble */
    void UI_RemoveBubble(const MapViewTask* pTask);

    /*! Calculate Night Mode */
    void UI_CalculateNightMode(const MapViewTask* pTask);

    void UI_OnMapInitialized(const MapViewTask* pTask);

    void UI_InvokeSingleTapListener(const MapViewTask* pTask);

    void UI_InvokeCameraUpdateListener(const MapViewTask* pTask);

    void UI_UpdateAnimationTip(const MapViewTask* pTask);

    void UI_AvatarClickedListener(const MapViewTask* pTask);

    void UI_OnMapReady(const MapViewTask* pTask);

    void UI_OnCameraAnimationDone(const MapViewTask* pTask);

    void UI_ChangeNightMode(const MapViewTask* pTask);

    void UI_StaticPOIClickedListener(const MapViewTask* pTask);

    void UI_TrafficIncidentPinClickedListener(const MapViewTask* pTask);

    void UI_OnTap(const MapViewTask* pTask);

    void UI_CameraLimited(const MapViewTask* pTask);

    void UI_InvokeMarkerClickListener(const MapViewTask* pTask);

    void UI_OnLongPressed(const MapViewTask* pTask);

    void UI_GenerateMapImage(const MapViewTask* pTask);

    /* Updates position of the frame */
    void RefreshFrame(double latitude, double longitude);

    /*! Update zoom range of raster tiles */
    void UpdateFrameAndNBGM();

    /*! Create a NBGM map view instance

        @return new NBGM map instance, or NULL on failure
    */
    NBGM_MapView* CreateNBGMMapView();

    /*! Keep only one updateFrame task in the event queue. */
    void AddUpdateFrameTask();

    /*! Calculate Night Mode*/
    bool CalculateNightMode();
    static void NightModeTimerCallback(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason);

    void AddRenderThreadTask(MapViewTask* task) const;
    void AddRenderThreadTask(MapViewTask* task, uint32 priority) const;

    static void OnFrameCheckTimerCallback(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason);
    static void OnRenderTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);

    void DoSetFrameZoomAndCenter();

    void RefreshAvatarMode();

    void UpdateAnimationUI(bool enabled, shared_ptr<MapLegendInfo> mapLegend);

    void CleanMaterials();
    // Private members .................................................................................................
private:
    struct DopplerState
    {
        DopplerState() :pauseByBar(false), pauseByGesture(false), checkPauseTimeInterval(false) ,playTime(0), pauseTime(0), isPlaying(false)
        {
        }

        bool    pauseByBar;
        bool    pauseByGesture;
        bool    checkPauseTimeInterval;
        uint32  playTime;
        uint32  pauseTime;
        bool    isPlaying;
    };

    class CombinedAtomicTaskExecuter
    {
    public:
        CombinedAtomicTaskExecuter(MapView* mapView, PAL_Instance* pal);
        ~CombinedAtomicTaskExecuter();

    public:
        void Enable(nb_boolean enable);
        nb_boolean AddTask(MapViewTask* task);
        void ClearAllTask();

    private:
        MapView* m_mapView;
        CombinedAtomicTask* m_CombinedAtomicTask;
        PAL_Lock* m_Lock;
    };

    /*! Class to handle all the initialization and shutdown */
    MapViewInitialize m_initialize;

    NB_Context* m_nbContext;
    PAL_Instance* m_palInstance;

    /*! Base full path of work folder */
    std::string m_workFolder;

    /*! Base full path of resource folder */
    std::string m_resourceFolder;

    /*! Default values used during initialization */
    double m_defaultLatitude;
    double m_defaultLongitude;
    double m_defaultTiltAngle;

    /*! Layer manager used to retrieve tiles */
    LayerManagerPtr m_layerManager;

    /*! Native view implemented in nbui */
    MapNativeView* m_nativeView;

    /*! Zoom level the last time we called UpdateFrame(). We need this to decide if we need to cancel any outstanding
        requests */
    uint32 m_lastZoom;

    /*! Frame class responsibe for all the tile calculation */
    Frame m_frame;

    /*! Route information */
    bool m_showRoute;

    /*! Flag to identify if the renderer should be initialized, it
        should only be used in the render thread. It is set in the
        functions RenderThread_InitializeRendererWithTask and
        RenderThread_DestroyRendererWithTask.
    */
    bool m_shouldInitRenderer;

    /*! NBGMViewController responsible for interaction with NBGM library */
    NBGMViewController* m_nbgmViewController;
    MapViewCamera* m_mapViewCamera;

    /*! Callback class to handle all tile requests */
    shared_ptr<MapViewTileCallback> m_pTileCallback;

    /*! Callback to handle request for common material update*/
    shared_ptr<MapViewCommonMaterialRequestCallback> m_pCommonMaterialCallback;

    /*! Callback class to handle layer callbacks. */
    shared_ptr<MapViewLayerCallback> m_pLayerCallback;

    /*! NBGM rendering worker task queue */
    shared_ptr<RenderTaskQueue> m_pRenderingQueue;

    /*! CCC worker thread */
    shared_ptr<EventTaskQueue> m_eventQueue;

    /*! UI task queue */
    shared_ptr<UiTaskQueue> m_UITaskQueue;

    shared_ptr<PinCushion> m_pinCushion;

    /*! Pin manager is used to get the pin by the pin ID, it must be used in the CCC thread. */
    shared_ptr<PinManager> m_pinManager;

    shared_ptr<CustomLayerManager> m_customLayerManager;

    /*! DAMUtility instance. */
    shared_ptr<DAMUtility> m_damUtility;

    /*! Max zoom level where we should display raster tiles */
    uint32 m_maxRasterZoomLevel;
    bool m_enabledDebugInfo;

    /*! Check Night mode */
    MapViewNightMode m_nightMode;
    bool m_isNight;
    /*! For night mode notify, UI thread */
    bool m_lastIsNight;

    /*! Check satellite mode */
    bool m_isSatellite;

    /*! Flag to indicate that we need to call the render function in NBGM. */
    bool m_needsRendering;

    /*! Flag to indicate background mode */
    bool m_background;

    /*! View size of the map control */
    Size m_viewSize;

    /*! Current GPS mode */
    NB_GpsMode m_gpsMode;
    MapViewAvatarMode m_avatarMode;
    NB_GpsLocation m_lastGpsLocation;

    /*! Layers provided by LayerManager. We need a copy of this so that when new layers
        added or removed, we can add/remove them into/from this vector, and reset draw order
        in NBGM.

        Important: Only access from CCC thread!
    */
    std::vector<LayerPtr> m_layers;

    /*! Listener used for follow-me notifications. Only one listener is allowed at a time (otherwise we can't decide
        if we should ignore the gesture or not */
    MapViewListener* m_pListener;

    /*! Used to display the map debug UI */
    DebugInfo m_info;

    std::string m_gpsFileName;
    NB_GpsMode m_lastGpsMode;             // Only used for debug output

    shared_ptr<string> m_droppedPinId;

    /*
        Data for animation layers.
        Access only in CCC thread!
     */
    map<LayerPtr, vector<uint32> > m_CCCAnimationFrameLists;  // Contains time stamps for each layer

    MapViewTheme     m_currentTheme;     /*!< Current adopted theme of MapView. */

    // Currently, there are only one traffic layer.
    bool           m_trafficLayerAvailable; /*!< Flag to indicate if traffic is available,
                                                 We use this boolean type across thread: it
                                                 get set in CCC thread, and get read in
                                                 Render thread. Just want to reduce task switches.
                                             */
    uint32         m_trafficLayerId;        /*!< Id of enabled traffic layer. */
    uint32         m_minZoomOfTrafficLayer; /*!< Min-Zoom Of enabled traffic layer */
    TrafficTipType m_lastTrafficTip;        /*!< Traffic tip of last time */

    /*! Only used during initialization */
    int m_viewSizeWidth;
    int m_viewSizeHeight;

    std::vector<TilePtr> m_commonMaterialTiles;
    bool m_firstTilesLoaded;
    int m_zorder;
    uint8 m_preferredLanguageCode;
    Point m_lastAvatarPosition;
    bool m_avatarRenderOneTimeOutofScreen;
    void* m_mapViewContext;
    bool m_showLabelDebugInfo;
    MapServicesConfigurationImpl* m_serviceConfiguration;

    /*! Currently whether has been a Screenshot Callback in the running. */
    shared_ptr<bool> m_pScreenshotInProgress;
    bool m_frameUpdated;  /*!< Flag to indicate that updateFrame task is available. */

    bool m_getTileWithFrame;

    shared_ptr<MetadataConfiguration> m_pMetadataConfig;
    map<TileKey, TilePtr> m_tilesToReload;
    /* To remember the render callback which from AppBB */
    RenderListener* m_defaultRenderListener;

    CombinedAtomicTaskExecuter* m_CombinedAtomicTaskExecuter;
    shared_ptr<PrefetchProcessor>   m_pPrefetchProcessor;

    shared_ptr<BubbleInterface> m_staticPoiBubble;
    bool m_staticPoiVisible;

    bool    m_isNeedFrameUpdate;

    DopplerState   m_dopplerState;

    shared_ptr<MapLegendInfo> m_mapLegendInfo;  /*!< Save the map legend info*/
    uint32 m_maxZoomOfAnimationLayer;           /*!< Max-Zoom Of enabled Animation layer */
    AnimationTipType m_lastAnimationTip;        /*!< Animation tip of last time */
    bool m_lastAnimationDisplay;                /*!< Animation status(show/hide) of last time */
    uint32 m_lastAnimationTimestamp;            /*!< Keep the last timestamp if no any animation data*/

    // Latest UI thread camera state
    double m_cameraPrevLat;
    double m_cameraPrevLon;
    float m_cameraPrevHeading;
    float m_cameraPrevTilt;
    float m_cameraPrevZoomLevel;

    uint32 m_lastDebugUiUpdateTime;

    bool m_cameraUpdateGuard;                   /*!< judge if only one camera update task*/
    MapViewUIThreadContext m_contextUI;

    bool m_isInAtomic;

    MapViewGestureHandler* m_gestureHandler;    /*! Gesture Handler*/

    uint32 m_cameraTimestamp;

    MapViewPinHandler* m_pinHandler;
    MapViewFontMagnifierLevel m_fontMagnifierLevel;
    bool m_refreshTilesAfterCommonMaterialUpdated;

    bool    m_updatingLayers;
    bool    m_renderPending;
};

}

#endif

/*! @} */

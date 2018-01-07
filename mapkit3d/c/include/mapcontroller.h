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
 @file     mapcontroller.h
 */
/*
 (C) Copyright 2013 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef MAPCONTROLER_H_
#define MAPCONTROLER_H_

#include "paltypes.h"
#include "pallock.h"
#include "MapViewInterface.h"
#include "PinManager.h"
#include "PinLayer.h"
#include "OptionalLayersAgent.h"
#include "GeoPolyline.h"
#include "Geographic.h"
#include "geoinfowindow.h"
#include "trafficlistener.h"
#include "trafficinformation.h"
#include "PinBubbleResolver.h"
#include "projection.h"
#include "CustomLayerManager.h"
#include "TileProvider.h"



namespace nbmap
{

class DefaultMapViewListener;
class MapPin;
class MapCircle;
class MapRect2d;
class MapTile;
class MapLayer;

/*! An enum type to categorize all the metedata types.
   Used for MapController::ConfigureMetadata and MapController::CCCThread_RefreshMetadata.
*/
enum MetaDataType
{
    MDT_NOTHING         = 0,
    MDT_LABLELAYERS     = 0x1,       /*!< label layers */
    MDT_OPTIONALLAYERS  = 0x1 << 1,  /*!< optional layers */
    MDT_WEATHERLAYERS   = 0x1 << 2,  /*!< weather layers */
    MDT_DAM             = 0x1 << 3,  /*!< DAM */
    MDT_LOC             = 0x1 << 4,  /*!< LOC */
    MDT_2DBUILDINGS     = 0x1 << 5,  /*!< 2D buildings */
    MDT_LOD             = 0x1 << 6,  /*!< LOD */
    MDT_SATELLITELAYERS = 0x1 << 7,  /*!< satellite layers*/
    MDT_ALL             = 0xFFFFFFFF /*!< all the above types */
};

enum CameraAnimationAccelerationType
{
    CAAT_LINEAR         = 0,         /*< linear acceleration */
    CAAT_DECELERATION   = 0x1,       /*< deceleration  */
    CAAT_ACCELERATION   = 0x2,       /*< acceleration  */
};

struct MapViewConfiguration
{
    double defaultLatitude;
    double defaultLongitude;
    double defaultTiltAngle;
    int maximumCachingTileCount;
    int maximumTileRequestCountPerLayer;
    std::string workFolder;
    std::string resourceFolder;
    std::string productClass;
    int zorderLevel;
    std::string languageCode;
    bool enableFullScreenAntiAliasing;
    bool enableAnisotropicFiltering;
    bool enableSSAO;
    bool enableGlow;
    MapViewFontMagnifierLevel fontMaginfierLevel;
    bool cleanMetadataFlag;
    float scaleFactor;

    MapViewConfiguration():defaultLatitude(0.0),
            defaultLongitude(0.0),
            defaultTiltAngle(0.0),
            maximumCachingTileCount(0),
            maximumTileRequestCountPerLayer(0),
            zorderLevel(0),
            enableFullScreenAntiAliasing(false),
            enableAnisotropicFiltering(false),
            enableSSAO(false),
            enableGlow(false),
            fontMaginfierLevel(MVFML_SMALL),
            cleanMetadataFlag(false),
            scaleFactor(1.0f)
    {}
};

struct MapPlace
{
    MapPlace(double lat, double lon):latitude(lat),
                                     longitude(lon)
    {}

    double latitude;
    double longitude;
};

class MapListener
{
public:
    virtual ~MapListener() {}

    /*! Callback when single tap occurred.

        @return true if caller finished handling this event and MapController will do
        nothing. Or return false then MapController will handle this event internally.
    */
    virtual bool
    OnSingleTap(float screenX,       /*!< tapped position. */
                float screenY,       /*!< tapped position. */
                std::set<nbmap::Geographic*>& graphics /*!< objects under the tap point */
                ) = 0;

    virtual void OnCameraUpdate(double lat, double lon, float zoomLevel, float heading, float tilt) = 0;

    virtual void OnOptionalLayerUpdated(const std::vector<nbmap::LayerAgentPtr>& layers) = 0;

    virtual void OnPinClicked(MapPin* pin) = 0;

    virtual void OnTrafficIncidentPinClicked(double latitude, double longitude) = 0;
    virtual void OnStaticPOIClicked(const string&id, const string&name, double latitude, double longitude) = 0;
    virtual void OnAvatarClicked(double lat, double lon) = 0;

    virtual void OnMapCreate() = 0;
    virtual void OnMapReady() = 0;

    virtual void OnCameraAnimationDone(int animationId, MapViewAnimationStatusType animationStatus) = 0;

    virtual void OnNightModeChanged(bool isNightMode) = 0;

    virtual void OnTap(double latitude, double longitude) = 0;

    virtual void OnCameraLimited(MapViewCameraLimitedType type) = 0;
    virtual void OnMarkerClicked(int markerId) = 0;

    virtual void OnLongPressed(double latitude, double longitude) = 0;
    virtual void OnGesture(MapViewGestureType type, MapViewGestureState state, int time) = 0;
};

class SnapShotCallback
{
public:
    virtual ~SnapShotCallback() {}

    virtual void OnSuccessed(nbcommon::DataStreamPtr dataStream, int width, int height) = 0;

    virtual void OnError(int code) = 0;
};

class MapController
{
public:
    MapController(NB_Context* nbContext, PAL_Instance* pal);
    virtual ~MapController();

public:
    void Create(const MapViewConfiguration& config, void* mapViewContext);

    void Destroy();

    /*! Switch MapView from immediate mode to retained mode
        @para
        By default, MapView works under immediate mode.
        In this mode, client calls directly cause MapView to
        update its status. Since all MapView updates
        happen on rendering thread, not the called thread,
        client can't make more than one updates synchronously.
        @para
        To make multi synchronized updates, client needs
        siwtch MapView to retained mode. In this mode, client
        calls do not directly cause actual map update, but instead
        update an internal update directive buffer, which is
        maintained within MapView's data space. This allows
        MapView to optimize when actual updates takes place
        along with the processing of related objects. Client can
        use retained mode to update multi MapView states synchronously.
        @para
        Note: all map view updates after BeginAtomicUpdate() will NOT
        be executed until EndAtomicUpdate() is called.

    @return void
    */
    virtual void BeginAtomicUpdate();

    /*! Switch MapView from immediate mode to retained mode, and execute
    all pending updates between BeginAtomicUpdate() and EndAtomicUpdate()

    @return NB_Error
    */
    virtual void EndAtomicUpdate();

    void SetCamera(double latitude, double longitude, float zoomLevel, float heading, float tilt, bool animated, unsigned int duration, CameraAnimationAccelerationType accelerationType, int animationId = 0);
    void SetPosition(double latitude, double longitude);

    /*! @name AddPin - add a pin to the map
     *  @param pinParameters - see the description of nbmap::PinParameters
     *  @return MapPin* - pointer to MapPin object
     */
    MapPin* AddPin(nbmap::PinParameters<nbmap::BubbleInterface> &pinParameters);
    void ShowTraffic(bool enable);
    void RemoveAllPins();
    void EnableDebugView(bool enable);
    void SetAvatarLocation(const NB_GpsLocation &location);
    void SetGpsMode(NB_GpsMode mode);
    NB_GpsMode GetGpsMode();
    void SetAvatarMode(MapViewAvatarMode mode);
    void SetAnimationLayerOpacity(unsigned int opacity);
    void SetNightMode(unsigned int mode);
    
    /** Is in night mode
     * @return true if in night mode, else false.
     */
    bool IsNightMode();

    std::vector<nbmap::LayerAgentPtr> GetOptionalLayers();

    /**
     * @name SetPinBubbleResolver - Set a custom pin bubble resolver
     * @return None.
     */
    void SetPinBubbleResolver(shared_ptr<nbmap::PinBubbleResolver> bubbleResolver);


    /**
     * @name ShowCompass - Set true/false to show/dispear compass.
     * @return None.
     */
    void ShowCompass(bool enable);

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

    /**
     * @name ConfigureMetadata
     * @param metaDataType - Bitwise OR of masks that indicate some meta data types to be enbaled/disabeled
     * @param enable - ture/false
     * @return None.
     */
    void ConfigureMetadata(int metaDataType, bool enable);

    /**
     * @name CCCThread_RefreshMetadata - This method Must be called in CCC thread.
     * @param metaDataType - Bitwise OR of masks that indicate some data types to be enbaled/disabeled
     * @param enable - ture/false
     * @return None.
     */
    void CCCThread_RefreshMetadata(int metaDataType, bool enable);

    /**
     * @name CCCThread_RemoveAllPins - This method Must be called in CCC thread.
     * @return None.
     */
    void CCCThread_RemoveAllPins();

    /**
     * @name AddPolyline - line
     * @return GeoPolyline*
     */
    nbmap::GeoPolyline* AddPolyline(const nbmap::GeoPolylineOption& option);

    void SetPolylineZorder(nbmap::GeoPolyline* polyline, int zorder);
    void SetPolylineSelected(nbmap::GeoPolyline* polyline, bool selected);
    void SetPolylineVisible(nbmap::GeoPolyline* polyline, bool visible);

    /**
     * @name AddGeoInfoWindow - Add an InfoWindow into Map.
     * @param window - a native window interface, if it's null, will use default.
     * @return GeoInfoWindow* - pointer of object ......
     */
    nbmap::GeoInfoWindow* AddGeoInfoWindow(nbmap::InfoWindow* window = NULL);


    /**
     * @name RemoveGeographic - remove an Geographic from Map.
     * @param graphic -  graphic to be removed.
     * @return None.
     */
    void RemoveGeographic(vector<nbmap::Geographic*>& graphic);

    /**
     * @name SetTrafficListener - Set traffic listener.
     * @param tl - traffic listener.
     * @return None.
     */
    void SetTrafficListener(TrafficListener* tl);

    /**
     * @name GetSelectedElements - Get Selected Elements.
     * @param graphics - return group of Geographic object.
     * @return None.
     */
    void GetSelectedElements(std::vector<nbmap::Geographic*>& graphics) const;

    /**
     * @name PickedUpElements - Get picked up Elements by screen position.
     * @param screenX - screen coordinate x.
     * @param screenY - screen coordinate y.
     * @param graphics - return group of Geographic object.
     * @return None.
     */
    void PickedUpElements(int screenX, int screenY, std::vector<nbmap::Geographic*>& graphics) const;

    /**
     * @name EnableCustomLayerCollisionDetection - Enable or disable the collision detection of custom layer.
     * @param layer - the custom layer.
     * @param enable - if true, enable the collision detection and overlapped items will be hide automatically, 
                       else allow layer items overlapping.
     * @return None.
     */
    void EnableCustomLayerCollisionDetection(MapLayer *layer, bool enable);

    /**
     * @name GetTrafficInformation - Get current traffic information.
     * @param traffics - return group of TrafficInformation object.
     * @return None.
     */
    void GetTrafficInformation(vector<TrafficInformation>& traffics) const;

    /**
     * @name SetMapListener - Set  map listener.
     * @param listener - listener object.
     * @return None.
     */
    void SetMapListener(MapListener* listener);

    void Prefetch(shared_ptr<vector<pair<double, double> > > polylineList,
            double prefetchExtensionLengthMeters, double prefetchExtensionWidthMeters, float zoomLevel);

    void Prefetch(double lat, double lon, float zoomLevel, float heading, float tilt);

    /*! Returns default Projection.
        @param none
        @return default projection pointer
     */
    const Projection* GetDefaultProjection() const;

    /**
     * @name RemovePin - remove the pin object.
     * @param pin - the pointer of this pin object.
     * @return None.
     */
    void RemovePin(MapPin *pin);

    /**
     * @name UpdatePinPosition - update the position of pin.
     * @param pin - the pointer of this pin object.
     * @param lat - latitude of the new position
     * @param lon - longitude of the new position
     * @return None.
     */
    void UpdatePinPosition(MapPin *pin, double lat, double lon);

    /**
     * @name SelectPin - select the pin object.
     * @param pin - the pointer of this pin object.
     * @param selected
     * @return None.
     */
    void SelectPin(MapPin *pin, bool selected);

    /**
     * @name GetPinSelected - Get state to show if the pin is selected.
     * @param pin - the pointer of this pin object.
     * @return true:selected and false:unselected.
     */
    bool GetPinSelected(MapPin* pin);

    /**
     * @name CCCThread_AddPin - This method Must be called in CCC thread.
     * @return None.
     */
    void CCCThread_AddPin(vector<nbmap::PinParameters<nbmap::BubbleInterface> > & pinData);

    /**
     * @name GetCamera - get the camera state.
     * @param lat Latitude
     * @param lon Longitude
     * @param altitude Camera distance
     * @param heading  Camera heading
     * @param tilt Camera angle
     * @return true if successed.
     */
    bool GetCamera(double &lat, double &lon, float &zoomLevel, float &heading, float &tilt);

    /*! Get the bounding box of the compass.
     * @param leftBottomX - x screen coordinate of left bottom point, in pixels.
     * @param leftBottomY - y screen coordinate of left bottom point, in pixels.
     * @param rightTopX - x screen coordinate of right top point, in pixels.
     * @param rightTopY - y screen coordinate of right top point, in pixels.
     * @return None.
     */
    void GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY)const;

    /*! Stop the animations.
     *  @param type - animation type, Bitwise OR of masks that indicate the animation to be stopped
     *  @return None.
    */
    void StopAnimations(int type);

   /*! Set to background.
     *  @return None.
    */
    void SetBackground(bool backGround);

    /*! Set to background.
     *  @return None.
     */
    void SetBackgroundSynchronized(bool backGround);

    /*! Play Doppler.
     * @return None.
     */
    void PlayDoppler();

    /*! Pause Doppler.
     * @return None.
     */
    void PauseDoppler();

    /*! set a relative center of map, in follow me mode avatar will be located in this position instead of screen center, and rotate and zoom will base on this position too.
     * @param x - X Screen Position in terms of pixel.
     * @param y - Y Screen Position in terms of pixel.
     * @return None.
     */
    void SetReferenceCenter(float x, float y);

    /*! Clear all data.
     * @return None.
     */
    void MasterClear();

    /*! enable reference center.
     * @
     * @return None.
     */
    void EnableReferenceCenter(nb_boolean enable);

    /*! Add a circle to the map.
     *  @param lat, latitude of the center
     *  @param lon, longitude of the center
     *  @param radius, in pixels
     *  @param fillColor, in RGBA
     *  @param strokeColor, in RGBA
     *  @param zOrder, draw order
     *  @param visible
     *  @return pointer to this circle
     */
    MapCircle* AddCircle(double lat, double lon, float radius, int fillColor, int strokeColor, int zOrder, bool visible);

    /*! Remove the circle from the map.
     *  @param circle - pointer to this circle.
     *  @return None.
     */
    void RemoveCircle(MapCircle* circle);

    /*! Remove all the circls.
     *  @return None.
     */
    void RemoveAllCircles();

    /*! Add a rect2d to the map.
     *  @param lat, latitude of the center
     *  @param lon, longitude of the center
     *  @param heading, in degrees
     *  @param width, in pixels
     *  @param height, in pixels
     *  @param texId
     *  @param texData
     *  @param visible
     *  @return pointer to this Rect2d. It must be removed by using RemoveRect2d.
     */
    MapRect2d* AddRect2d(double lat, double lon, float heading, float width, float height, int texId, nbcommon::DataStreamPtr texData, bool visible);

    /*! Remove the rect2d from the map.
     *  @param rect - pointer to this Rect2d.
     *  @return None.
     */
    void RemoveRect2d(MapRect2d* rect);

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

    /*! Remove all the textures.
     *  @return None.
     */
    void RemoveAllTextures();

    /*! Creates a custom tile.
     *  @return Pointer to this tile object.
     */
    MapTile* CreateCustomTile(int x, int y, int zoom, vector<nbmap::PinParameters<nbmap::BubbleInterface> >& parameters);

    /*! Destroys the custom tile.
     *  @param tile
     *  @return None.
     */
    void DestroyCustomTile(MapTile* tile);

    /*! Adds a custom Layer.
     *  @param  provider
     *  @return Pointer to this layer object.
     */
    MapLayer* AddCustomLayer(nbmap::TileProviderPtr provider, int mainOrder, int subOrder, int refZoom, int minZoom, int maxZoom, bool visible);

    /*! Removes a custom Layer.
     *  @param layer
     *  @return None
     */
    void RemoveCustomLayer(MapLayer* layer);

    /*! Removes all the custom Layers.
     *  @return None
     */
    void RemoveAllCustomLayers();

    /*! Removes all the custom tiles.
     *  @return None
     */
    void RemoveAllCustomTiles();
    
    /*! Sets the scale value of the Font.
     *  @param scale
     *  @return None
     */
    void SetFontScale(float scale);

    /*! Sets the scale value of the Avatar.
     *  @param scale
     *  @return None
     */
    void SetAvatarScale(float scale);

    /*! Snap Shot.
     *  @param callback
     *  @param x
     *  @param y
     *  @param width
     *  @param height
     *  @return None
     */
    void SnapShot(shared_ptr<SnapShotCallback> callback, int x, int y, int width, int height);


    /*! Set MapView HBAO parameters.
     *  @param params - specified parameters
     *  @return None
     */
    virtual void SetHBAOParameters(const HBAOParameters& parameters);

    /*! Set MapView glow parameters.
     *  @param params - specified parameters
     *  @return None
     */
    virtual void SetGlowParameters(const GlowParameters& parameters);

    /*! Set display screen.
        @param screenIndex screen index. If screen doesn't exist, do nothing.
        @return None
     */
    void SetDisplayScreen(int screenIndex);

    /*! Set map DPI.
        @param dpi display DPI.
        @return None
     */
    void SetDPI(float dpi);


private:
    static void ConfigureMetadataFunc(PAL_Instance* pal, void* userData);
    static void RemoveAllPinsFunc(PAL_Instance* pal, void* userData);
    static void AddPinFunc(PAL_Instance* pal, void* userData);
    static void RemovePinFunc(PAL_Instance* pal, void* userData);
    static void UpdatePinPositionFunc(PAL_Instance* pal, void* userData);
    static void PinSelectedCallBack(PAL_Instance* pal, void* userData);
    static void UnselectAllPinsCallBack(PAL_Instance* pal, void* userData);
    static void CreateTileFunc(PAL_Instance* pal, void* userData);
    static void DestroyTileFunc(PAL_Instance* pal, void* userData);
    static void AddLayerFunc(PAL_Instance* pal, void* userData);
    static void RemoveLayerFunc(PAL_Instance* pal, void* userData);
    static void EnableLayerCollisionFunc(PAL_Instance* pal, void* userData);

    bool OnSingleTap(float screenX, float screenY, const set<std::string>& ids);
    void OnCameraUpdate(double lat, double lon, float zoomLevel, float heading, float tilt);

    void OnLayerUpdate();
    void OnPinClicked(const std::string& id);
    void OnUnselectAllPins();

    void OnTrafficIncidentPinClicked(double lat, double lon);
    void OnStaticPOIClicked(const std::string& id, const std::string& name, double lat, double lon);
    static void CCC_Thread_CreateMap(PAL_Instance* pal, void* userData);
    void CreateMapViewInstance(const MapViewConfiguration& config, void* mapViewContext);

    static void CCC_Thread_ShowTraffic(PAL_Instance* pal, void* userData);
    void CCC_ShowTraffic(bool enabled);
    void OnAvatarClicked(double lat, double lon);
    void OnMapCreate();
    void OnMapReady();
    void OnCameraAnimationDone(int animationId, MapViewAnimationStatusType animationStatus);
    void OnNightModeChanged(bool isNightMode);
    void OnTap(double latitude, double longitude);
    void OnCameraLimited(MapViewCameraLimitedType type);
    void OnMarkerClicked(int id);
    void OnLongPressed(double lat, double lon);
    void OnGesture(MapViewGestureType type, MapViewGestureState state, int time);
    void CleanMetaData(const std::string& workPath);
    shared_ptr<nbmap::MetadataConfiguration> GetMetaDataConfiguration();
    static void CCC_MasterClear(PAL_Instance* pal, void* userData);

    friend class DefaultMapViewListener;
    friend class MapPin;
    friend class MapCircle;
    friend class MapRect2d;
    friend class MapLayer;

private:
    NB_Context* mNBContext;
    PAL_Instance* mPal;
    MapViewInterface* mMapView;
    shared_ptr<nbmap::PinManager> mPinManager;
    nbmap::PinLayerPtr mPinLayer;
    nbmap::OptionalLayersAgentPtr mOptionalLayersAgent;
    DefaultMapViewListener* mDefaultMapViewListener;
    MapListener* mMapListener;
    Projection* mProjection;
    // Only accessed by UI thread.
    set<MapPin*> mPinSet;
    set<MapTile*> mTileSet;
    set<MapLayer*> mLayerSet;
    // Only accessed by CCC thread.
    map<string, MapPin*> mPinMap;
    set<shared_ptr<MapRect2d> > mRect2dSet;
    set<shared_ptr<MapCircle> > mCircleSet;
    int mNextModelId;
    shared_ptr<nbmap::CustomLayerManager> mCustomLayerManager;
    shared_ptr<nbmap::MetadataConfiguration> mMetadataConfigPtr;
    bool mEnableSSAO;
    bool mEnableGlow;
};

}

#endif /* MAPCONTROLER_H_ */

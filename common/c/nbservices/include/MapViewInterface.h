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
    @file     MapViewInterface.h
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef _MAP_VIEW_INTERFACE_
#define _MAP_VIEW_INTERFACE_

#include "nberror.h"
#include <vector>
#include "navpublictypes.h" // For NB_LatitudeLongitude
#include "smartpointer.h"
#include "nbgpstypes.h"     // For NB_GpsLocation
#include "LayerManager.h"
#include <list>
#include "GeographyOperation.h"
#include "nbgm.h"
#include "MetadataConfiguration.h"
#include "PinBubbleResolver.h"
#include "PinParameters.h"
#include "BubbleInterface.h"
#include "nbgmtypes.h"

/* this value should be used as SetCameraSettings default argument if corresponding camera setting isn't changed */
#define INVALID_CAMERA_LOCATION  -999.0
#define INVALID_CAMERA_ZOOMLEVEL -999.0
#define INVALID_CAMERA_TILT      -999.0
#define INVALID_CAMERA_HEADING   -999.0
#define INVALID_LONGITUDE        -999.0
#define INVALID_LATITUDE         -999.0
namespace nbcommon
{
class DataStream;
}

namespace nbmap
{
class PinManager;
class CustomLayerManager;
class TrafficManager;
class OptionalLayersAgent;
}


/*! avatar mode */
enum MapViewAvatarMode
{
    MVAM_NONE,
    MVAM_MAP,
    MVAM_ARROW,
    MVAM_CAR

    // @todo: Add other modes for navigation.
};

/*! Settings of map view */
enum MapViewSettings
{
    MVS_SHOW_DEBUG_VIEW,      /*!< Show / hide debug info view */
    MVS_FAR_NEAR_VISIBILITY,  /*!< NBGM far near visibility property */
    MVS_NBGM_FOV_ANGLE        /*!< Change portrait and landscape FOV angles */
};

/*! Avatar state of map view */
enum MapViewAvatarState
{
    MVAS_INVALID,           /*!< Hide avatar on map */
    MVAS_STAND_BY,          /*!< Identify postion without heading */
    MVAS_FOLLOW_ME          /*!< Identify position with heading */
    // @todo: Add other avatar states.
};


/*! Gesture types used in MapViewListener */
enum MapViewGestureType
{
    MVGT_Invalid,

    MVGT_Move,
    MVGT_TiltAngle,
    MVGT_RotateAngle,
    MVGT_Scale,
    MVGT_Tap,
    MVGT_DoubleTap,
    MVGT_TwoFingerTap,
    MVGT_LongPress,
    MVGT_TwoFingerDoubleTap
};

enum MapViewGestureState
{
    MVGS_Possible,
    MVGS_Began,
    MVGS_Changed,
    MVGS_Ended,
    MVGS_Cancelled,
    MVGS_Failed,
    MVGS_Recognized
};

enum MapViewNightMode
{
    MVNM_AUTO,      /*!< Dynamically switch day/night mode */
    MVNM_DAY,       /*!< Day mode */
    MVNM_NIGHT      /*!< Night mode */
};

enum MapViewCameraAnimationAccelertaionType
{
    MVCAAT_LINEAR,
    MVCAAT_DECELERATION,
    MVCAAT_ACCELERATION
};

/*! All supported animation type, used in StopAnimation() */
enum MapViewAnimationType
{
    MVAT_NONE   = 0,
    MVAT_TILT   = 0x1,        /*!< Tilt animation */
    MVAT_ROTATE = 0x1<<1,     /*!< Rotate animation */
    MVAT_ZOOM   = 0x1<<2,     /*!< Zoom in/out animation */
    MVAT_MOVE   = 0x1<<3,     /*!< Move animation */
    MVAT_TOTAL  = 0xFFFFFFFF
 };

/*! Animation status type */
enum MapViewAnimationStatusType
{
    MVAST_FINISHED,         /*< Animation is finished. */
    MVAST_INTERRUPTED,      /*< Animation is interrupted. */
    MVAST_CANCELED          /*< Animation is canceld. */
};

enum MapViewCameraLimitedType
{
    MVCLT_LOCATION,        /*< Camera location limited. */
    MVCLT_ROTATION         /*< Camera rotation limited. */
};

enum MapViewFontMagnifierLevel
{
    MVFML_SMALL,        /*< Small level. */
    MVFML_MEDIUM,       /*< Medium level. */
    MVFML_LARGE         /*< Large level. */
};

/*! Listener for callbacks from render.

    call back when render begin or end.
 */
class RenderListener : public NBGM_FrameListener
{
public:
    virtual ~RenderListener(){};
    virtual void OnRenderFrameBegin() = 0;
    virtual void OnRenderFrameEnd() = 0;
    virtual void OnDiscard() = 0;
};

/*! Listener for callbacks from MapView.

    All callbacks are invokde in UI thread.
    To use this listener, derive from this class and call
    MapViewInterface::(Un)RegisterListener() to (un)register.
 */
class MapViewListener
{
public:

    /*! This callback gets called when the user performs a gesture while follow-me mode is enabled. It is up to the application
        to decide if follow-me should be disabled or not in response to this call.

        @return 'true' if the gesture should be allowed, 'false' to ignore the gesture
     */
    virtual bool GestureDuringFollowMeMode(MapViewGestureType gesture) = 0;

    /*! This callback gets called when the MapView Layers Request is Success.

        @return None
    */
    virtual void LayersUpdated() = 0;

    /*! Callback when single tap occurred.

        @return true if caller finished handling this event and MapView will do
        nothing. Or return false then MapView will handle this event internally.
    */
    virtual bool
    OnSingleTap(float /*screenX*/,       /*!< tapped position. */
                float /*screenY*/,       /*!< tapped position. */
                const set<std::string>& /*ids*/ /*!< Identifier of objects under the tap point */
                ) {return false;}

    /*! Callback when camera update.
    */
    virtual void OnCameraUpdate(double lat, double lon, float zoomLevel, float heading, float tilt) = 0;

    /*! Callback when pin is clicked.
    */
    virtual void OnPinClicked(const std::string& id) = 0;

    /*! Callback when traffic incident pin is clicked.
    */
    virtual void OnTrafficIncidentPinClicked(double latitude, double longitude) = 0;

    /*! Callback when static poi is clicked.
    */
    virtual void OnStaticPOIClicked(const std::string& id, const std::string& name, double latitude, double longitude) = 0;

    /*! Callback when unselected all pin.
     */
    virtual void OnUnselectAllPins() = 0;

    /*! Callback when avatar is clicked.
    */
    virtual void OnAvatarClicked(double lat, double lon) = 0;

    /*! Callback when a marker is clicked.
    */
    virtual void OnMarkerClicked(int id) = 0;

    /*! Callback when map is created.
    */
    virtual void OnMapCreate() = 0;

    /*! Callback when map is ready.
    */
    virtual void OnMapReady() = 0;

    /*! Callback when camera animation is done.
    */
    virtual void OnCameraAnimationDone(int animationId, MapViewAnimationStatusType animationStatus) = 0;

    /*! Callback when night mode changed
    */
    virtual void OnNightModeChanged(bool isNightMode) = 0;

    /*! Callback when tap nothing.
    */
    virtual void OnTap(double lat, double lon) = 0;

    /*! Callback when camera is limited.
     */
    virtual void OnCameraLimited(MapViewCameraLimitedType type) = 0;

    /*! Callback when map is long pressed.
     */
    virtual void OnLongPressed(double lat, double lon) = 0;

    /*! Callback when Gesture happened.
     */
    virtual void OnGesture(MapViewGestureType type, MapViewGestureState state, int time) = 0;
};


/*! Call back for map image generate.
 */
class GenerateMapCallback
{
public:

    /*! Callback snap shot successed.
    */
    virtual void OnSnapShotSuccessed(nbcommon::DataStreamPtr dataStream, uint32 width, uint32 height) = 0;

    /*! Callback snap shot failed.
    */
    virtual void OnSnapShotError(NB_Error error) = 0;
};

class MapViewCameraParameter
{
public:
    MapViewCameraParameter():
        latitude(INVALID_CAMERA_LOCATION),
        longitude(INVALID_CAMERA_LOCATION),
        heading(INVALID_CAMERA_HEADING),
        zoomLevel(INVALID_CAMERA_ZOOMLEVEL),
        tilt(INVALID_CAMERA_TILT),
        id(0) {}

    double latitude;
    double longitude;
    float  heading;
    float  zoomLevel;
    float  tilt;
    int    id;
};


/*! Configuration of MapView
 */
struct MapConfiguration
{
    MapConfiguration(shared_ptr<nbmap::PinManager>& pinManager,
                     shared_ptr<nbmap::CustomLayerManager>& customLayerManager,
                     shared_ptr<nbmap::OptionalLayersAgent>& optionalLayersAgent):
        defaultLatitude(0.0),
        defaultLongitude(0.0),
        defaultTiltAngle(0.0),
        maximumCachingTileCount(0),
        maximumTileRequestCountPerLayer(0),
        workFolder(NULL),
        resourceFolder(NULL),
        zorderLevel(-5),
        mapViewContext(NULL),
        mapViewListener(NULL),
        fontMagnifierLevel(MVFML_SMALL),
        scaleFactor(1.f),
        outPinManager(pinManager),
        outCustomLayerManager(customLayerManager),
        outOptionalLayersAgent(optionalLayersAgent) {}

    shared_ptr<nbmap::AsyncCallback<void* /* Not used */> > initializeCallback;
    shared_ptr<nbmap::AsyncCallback<void* /* Not used */> > destroyCallback;
    double defaultLatitude;
    double defaultLongitude;
    double defaultTiltAngle;
    uint32 maximumCachingTileCount;
    uint32 maximumTileRequestCountPerLayer;
    const char* workFolder;
    const char* resourceFolder;
    shared_ptr<nbmap::MetadataConfiguration> metadataConfig;
    int zorderLevel;
    void* mapViewContext;
    MapViewListener* mapViewListener;
    MapViewFontMagnifierLevel fontMagnifierLevel;
    float scaleFactor;
    shared_ptr<nbmap::PinManager>& outPinManager;
    shared_ptr<nbmap::CustomLayerManager>& outCustomLayerManager;
    shared_ptr<nbmap::OptionalLayersAgent>& outOptionalLayersAgent;
private:
    MapConfiguration(const MapConfiguration &);
    MapConfiguration &operator=(const MapConfiguration &);
};

typedef struct HBAOParameters
{
    HBAOParameters():
        enable(true),
        radius(40.0f),
        bias(0.0f),
        powerExponent(4.0f),
        detailAO(0.0f),
        coarseAO(1.0f),
        enableBlur(TRUE),
        blurRadius(4.f),
        blurSharpness(1.0f)
    {
    }

public:
    nb_boolean enable;         /*!< Enable or disable AO effect */
    float radius;              /*!< The AO radius in meters */
    float bias;                /*!< To hide low-tessellation artifacts // 0.0~1.0 */
    float powerExponent;       /*!< The final AO output is pow(AO, powerExponent) */
    float detailAO;            /*!< Scale factor for the detail AO, the greater the darker // 0.0~2.0 */
    float coarseAO;            /*!< Scale factor for the coarse AO, the greater the darker // 0.0~2.0 */
    nb_boolean enableBlur;     /*!< To blur the AO with an edge-preserving blur */
    int blurRadius;            /*!< Kernel radius pixels(should be 2, 4, 8) */
    float blurSharpness;       /*!< The higher, the more the blur preserves edges // 0.0~16.0 */
}HBAOParameters;


typedef struct GlowParameters
{
    GlowParameters():
        enable(true),
        blurRadius(4.0f),
        blurSamples(4)
    {
        clearColor[0] = 1.0f;
        clearColor[1] = 1.0f;
        clearColor[2] = 1.0f;

        scaleColor[0] = 1.0f;
        scaleColor[1] = 1.0f;
        scaleColor[2] = 1.0f;
        scaleColor[3] = 1.0f;
    }

public:
    nb_boolean enable;         /*!< Enable or disable glow effect */
    float      clearColor[3];  /*!< Glow layer clear color in RGB order, range from 0.0 to 1.0. */
    float      scaleColor[4];  /*!< Blurred color in RGBA order, range from 0.0 to 1.0. Blur result is multiplied by it */
    float      blurRadius;     /*!< The blur radius in pixels */
    int        blurSamples;    /*!< The number of nearby pixels used in blur */
}GlowParameters;

/*! MapViewInterface.

    This is the public interface to the MapView. The implementation details are hidden by this interface.
    Call MapServicesConfiguration::GetMap() to create a map view.

    @see MapServicesConfiguration
*/
class MapViewInterface
{
public:

    MapViewInterface(){};

    /*! Destroy the map view.

        Important:
        This function has to be called from the UI thread!

        The MapViewInterface and all other map view pointers returned by CreateMapView() will become invalid after this call.

        @return NB_Error
    */
    virtual NB_Error Destroy() = 0;

    /*! Initialize or destroy renderer of map view

        Both functions are only called in the render engine of bb10
        so far. There is a crash when the application exit. Because
        the renderer is destroyed after destroying the UI environment
        by system. Both functions are used to initialize or destroy
        the renderer explicitly. It is not necessary to call both
        functions if there is no above issue.
        Both functions should not affect the original processes of
        initializing and destroying map view. If the function Destroy
        has already been called, the function InitializeRenderer
        should do nothing and not initialize renderer. If the map
        view is initializing in progress, the renderer should be
        destroyed after map view initialization, or not be
        initialized. If the renderer has already been destroyed, the
        function DestroyRenderer destroys the renderer synchronously.
        Both functions could be called in any threads, but it is
        better to call them in the same thread.

        @return None
    */
    virtual void InitializeRenderer() = 0;
    virtual void DestroyRenderer() = 0;

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
    virtual void BeginAtomicUpdate() = 0;

    /*! Switch MapView from retained mode to immediate mode, and execute
    all pending updates between BeginAtomicUpdate() and EndAtomicUpdate()

    @return NB_Error
    */
    virtual void EndAtomicUpdate() = 0;

    /*! Show the map view on the screen.

        @return NB_Error
    */
    virtual NB_Error ShowView() = 0;

    virtual NB_Error SetMapViewContext(void* context) = 0;

    virtual void SetLayerManager(nbmap::LayerManagerPtr layerManager) = 0;

    /*! Configure Layer Manager to show/hide route layer.

     @return NB_Error
     */
    virtual NB_Error ShowRouteLayer(bool isShown) = 0;

    /*! Set a platform specific instance of MapNativeView.

        This function gets the platform specific instance of MapNativeView from NBUI and passes it to the application level to
        connect the map instance shown on the screen.
     */
    virtual void* GetNativeContext() = 0;

    /*! Set GPS mode

        @return None
    */
    virtual void SetGpsMode(NB_GpsMode gpsMode       /*!< GPS mode to set */
                            ) = 0;

    /*! Get GPS mode

        @return gpsMode
    */
    virtual NB_GpsMode GetGpsMode()const = 0;

    /*! Set the mode of avatar

        @return None
    */
    virtual void SetAvatarMode(MapViewAvatarMode mode     /*!< Avatar mode to set */
                                ) = 0;

    /*! Set the location of avatar

        @return None
    */
    virtual void SetAvatarState(bool headingValid     /*!< indicates gps heading valid or not */
                                ) = 0;

    /*! Set the location of avatar

     @return None
     */
    virtual void
    SetAvatarLocation(const NB_GpsLocation& gpsLocation) = 0;    /*!< GPS location to set  */

    /*! Set the custom avatar by file data stream.

     @return None
     */
    virtual void
    SetCustomAvatar(shared_ptr<nbcommon::DataStream> directionalAvatar,          /*!< The directional avatar */
                    shared_ptr<nbcommon::DataStream> directionlessAvatar,        /*!< The directionless avatar */
                    int8 directionalCalloutOffsetX,                              /*!< Offset x of the callout. The value is from
                                                                                      0 to 100. It is the percent of the directional
                                                                                      avatar image width. The top left point of the
                                                                                      callout is the orginal point (0, 0).*/
                    int8 directionalCalloutOffsetY,                              /*!< Offset y of the callout. The value is from
                                                                                       0 to 100. It is the percent of the directional
                                                                                       avatar image height. */
                    int8 directionlessCalloutOffsetX,                            /*!< Offset x of the callout. The value is from
                                                                                      0 to 100. It is the percent of the directionless
                                                                                      avatar image width. The top left point of the
                                                                                      callout is the orginal point (0, 0).*/
                    int8 directionlessCalloutOffsetY                             /*!< Offset y of the callout. The value is from
                                                                                       0 to 100. It is the percent of the directionless
                                                                                       avatar image height. */
                    ) = 0;

    /*! Set the custom avatar by file path.

     @return None
     */
    virtual void
    SetCustomAvatar(shared_ptr<std::string> directionalAvatarPath,    /*!< The directional avatar file path*/
                    shared_ptr<std::string> directionlessAvatarPath,  /*!< The directionless avatar file path*/
                    int8 directionalCalloutOffsetX,              /*!< Offset x of the callout. The value is from
                                                                      0 to 100. It is the percent of the directional
                                                                      avatar image width. The top left point of the
                                                                      callout is the orginal point (0, 0).*/
                    int8 directionalCalloutOffsetY,              /*!< Offset y of the callout. The value is from
                                                                      0 to 100. It is the percent of the directional
                                                                      avatar image height. */
                    int8 directionlessCalloutOffsetX,            /*!< Offset x of the callout. The value is from
                                                                      0 to 100. It is the percent of the directionless
                                                                      avatar image width. The top left point of the
                                                                      callout is the orginal point (0, 0).*/
                    int8 directionlessCalloutOffsetY              /*!< Offset y of the callout. The value is from
                                                                       0 to 100. It is the percent of the directionless
                                                                       avatar image height. */
                    ) = 0;

    /*! Set the custom avatar halo style

     @return None
     */
    virtual void
    SetCustomAvatarHaloStyle(shared_ptr<nbmap::GeographyMaterial> haloStyle          /*!< The avatar halo style */
                             ) = 0;

    /*! Show/Hide debug info view.

     @return none
     */
    virtual void ShowDebugView(bool enabled) = 0;

    /*! Set background mode.

        @return none
    */
    virtual void SetBackground(bool background) = 0;

    /*! Set background mode.

     @return none
     */
    virtual void SetBackgroundSynchronized(bool background) = 0;

    /*! Returns lat/lon for the corresponding delta x,y co-ordinates
        @param dX Delta on x-axis
        @param dY Delta on y-axis
        @param lat[out] Latitude corresponding to delta
        @param lon[out] Longitude corresponding to delta
        @return NB_Error
     */
    virtual NB_Error WindowToWorld(int dX, int dY, double* lat, double* lon) = 0;

    /*! Sets the NBGM camera.(not thread safe)
        @param cameraParameter MapViewCameraParameter
        @return NB_Error
     */
    virtual NB_Error SetCamera(const MapViewCameraParameter& cameraParameter) = 0;

    /*! Get the NBGM camera.
        @param cameraParameter MapViewCameraParameter
        @return NB_Error
     */
    virtual NB_Error GetCamera(MapViewCameraParameter& cameraParameter) = 0;

    /*! set camera position, height, heading and tilt.

        @return NB_Error
     */
    virtual NB_Error SetCameraSettings(const MapViewCameraParameter& cameraParameter, bool animated = FALSE,
                                       uint32 duration = 0, MapViewCameraAnimationAccelertaionType type = MVCAAT_LINEAR) = 0;

    virtual PAL_Instance* GetPalInstance() = 0;

    virtual shared_ptr<list<string> > GetInteractedPois(float x, float y) = 0;

    virtual NB_Error WorldToWindow(double latitude, double longitude, int* dX, int* dY) = 0;

    virtual double GetCurrentZoomLevel() = 0;

    /*! Set view settings.

        @return NB_Error
     */
    virtual NB_Error SetViewSettings(MapViewSettings viewSetting, bool flag) = 0;

    /*! Set view settings(Field of view angles).

        @return NB_Error
     */
    virtual NB_Error SetViewSettings(MapViewSettings viewSetting, bool flag, float portraitFOV, float landscapeFOV) = 0;

    /*! Set display screen.
        @param screenIndex screen index. If screen doesn't exist, do nothing.
        @return NB_Error
     */
    virtual NB_Error SetDisplayScreen(int screenIndex) = 0;

    virtual void SetGpsFileName(const char* gpsFileName) = 0;

    /*! Set the theme for the map.

        Default is day

        @return NB_Error
     */
    virtual NB_Error SetTheme(MapViewTheme theme) = 0;

    /*! Set parameters for the current animation layer, e.g. weather overlay animation layer */
    virtual NB_Error SetAnimationLayerSettings(unsigned int totalAnimationTime,  /*!< Total animation time, in milliseconds, for all frames to display (without the loop pause) */
                                               unsigned int loopPause,           /*!< Time (in milliseconds) of time to stop after the animation has finished and it re-starts from
                                                                                    the beginning again */
                                               double minimumTilePercentage      /*!< Percentage of frames needed for a tile before it is included into the animation.
                                                                                    E.g. if 8 frames are set and the value is set to 75.0%, then tiles which have a minimum
                                                                                    of 6 tiles received will be included in the animation */
                                               ) = 0;

    /*! Sets the opacity (transparency) of the current animation layer */
    virtual NB_Error SetAnimationLayerOpacity(unsigned int opacity               /*!< Opacity of the animation layer in percent. Value between 0 to 100 are valid. */
                                              ) = 0;

    virtual void SetViewPort(int width, int height) = 0;

    /*! Get the mapview if support rotate or not.
        Return true if support else not.
    */
    virtual bool GetRotateByGestureSupported() =0;

    /*! Get the mapview if support tilt or not.
        Return true if support else not.
    */
    virtual bool GetTiltByGestureSupported() = 0;

    /*! Get the mapview if support traffic layer.
        Return true if support else not.
    */
    virtual bool GetInlineTrafficSupported() = 0;

    /*! Show a route layer.

        A routes have to be first added/endabled to the LayerManager. The LayerManager will return a layerID for every route added.
        Use this layerID to show the route.
     */
    virtual void ShowRouteLayer(uint32 layerID          /*!< Layer ID returned from LayerManager */
                                ) = 0;

    /*! Hide a route layer.

        This does not unload the layer. The layer gets unloaded when the route layer(s) get removed/disabled in the LayerManager
     */
    virtual void HideRouteLayer(uint32 layerID) = 0;
    /*! Add a render task (if necessary)
     *  If a listener is passed as parameter, pass it to NBGM */
    virtual void AddRenderTask(RenderListener* listener = NULL) = 0;

    virtual void AddResetScreenTask() = 0;

    /*! Converts the current map into an image

        This function gets the screenshot of the current map view. It
        could be called in any threads.

        @return NB_Error
    */
    virtual NB_Error GenerateMapImage(shared_ptr<GenerateMapCallback> callback, int32 x = 0, int32 y = 0, uint32 width = 0, uint32 height = 0) const = 0;

    /*! Asynchronous Converts the current map into an image

        This function gets the screenshot of the current map view, it
        returns the image data asynchronous. It need to called in
        CCC threads.

        @return NE_BUSY when current having a callback is running, return NE_BADDATA when screenshot buffer size is 0,
                return NE_OK when invoke GenerateMapImage is success, return NE_INVAL when MapView is being shutdown,
                other will return NE_INVAL
    */
    virtual NB_Error GenerateMapImage(shared_ptr<nbmap::AsyncCallback<shared_ptr<nbcommon::DataStream> > >  callback) = 0;

    /*! Cancel Current Asynchronous GenerateMapImage Callback.

        This function need to called in CCC threads.

        @return None.
    */
    virtual void CancelGenerateMapImage() = 0;

    /*! Enable Show Label Debug Information

        This function will Enable/Disable label debug information for can show all the
        languages supported for each label. related the bug#163108

        This function only is temporary, please don't merge it for bb10 main branch.

    */
    virtual void SetShowLabelDebugInfo(bool showLabelDebugInfo) = 0;

    /*! Refresh metadata based on specified configuration.

        @note: Must be called in CCC thread.
        @return None.
    */
    virtual void RefreshMetadataAndLayers(shared_ptr<nbmap::MetadataConfiguration> metadataConfig) = 0;

    /*! Enable/Disable request tiles.

        @todo: Move Platform-specific APIs out of MapViewInterface.

        @return None.
    */
    virtual void EnableGetTileWithFrame(bool enabled) = 0;

    /*! Set Pin specified by pinID as selected.

       @Note: For RIM only!
              Pins should be handled internally in CoreSDK on other platforms(iOs, android
              ...), and user should not call this API on those platforms.

       @todo: Move Platform-specific APIs out of MapViewInterface.

       @return None.
    */
    virtual void SelectPin(shared_ptr<string> pinId) = 0;

    /*! Set Pin specified by pinID as unselected.
       @return None.
    */
    virtual void UnselectPin(shared_ptr<string> pinId) = 0;

    /**
     * @name ShowCompass - Set true/false to show/dispear compass.
     * @return None.
     */
    virtual void ShowCompass(bool enabled) = 0;

    /**
     * @name SetCompassPosition - Set compass's screen position.
     * @param screenX - x screen coordinate.
     * @param screenY - y screen coordinate.
     * @return None.
     */
    virtual void SetCompassPosition(float screenX, float screenY) = 0;

    /*! Set compass icons.
    * @param dayModeIconPath. Icon path of day mode.
    * @param nightModeIconPath. Icon path of night mode.
    * @return none.
    */
    virtual void SetCompassIcons(const std::string& dayModeIconPath, const std::string& nightModeIconPath) = 0;

    /*! Set Night Mode
        @return None.
     */
    virtual void SetNightMode(MapViewNightMode nightMode) = 0;

    /*! Get is night mode
        @return true if in night mode, else false.
     */
    virtual bool IsNightMode() = 0;

    /*! Return all elements at specified postion.
     *
     * Should be called in UI thread ONLY.
     *
     * @param screenX  - x screen coordinate.
     * @param screenY  - y screen coordinate.
     * @param elements - vector of elements.
     * @return vector of identifiers of elements.
     */
    virtual void GetElementsAt(float screenX, float screenY, set<std::string>& elements) = 0;

    /*! Enable or disable the collision detection of custom layer.
     *
     *
     * @param layerId - the ID of the custom layer.
     * @param enable - if true, enable the collision detection and overlapped items will be hide automatically,
                       else allow layer items overlapping.
     * @return None.
     */
    virtual void EnableCustomLayerCollisionDetection(const std::string& layerId, bool enable) = 0;

    /*! Set a custom pin bubble resolver to mapview.
        This interface is supposed to support custom pin bubble.
        It could be called in any thread.
       @return NB_Error.
    */
    virtual NB_Error SetCustomPinBubbleResolver(shared_ptr<nbmap::PinBubbleResolver> bubbleResolver) = 0;

    virtual void GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY)const = 0;

    /*! Calculation the tile prefetch for navigation.
        @param polylineList - prefetch tile base on the current polyline list.
        @param prefetchExtensionLengthMeters -
        @param prefetchExtensionWidthMeters -
        @param zoomLevel -
    */
    virtual void Prefetch(shared_ptr<vector<pair<double, double> > > polylineList,
                double prefetchExtensionLengthMeters, double prefetchExtensionWidthMeters, float zoomLevel) = 0;

    /*! Calculation the tile prefetch base on the current camera data.
        @param lat - current camera parameter for lat
        @param lon - current camera parameter for lon
        @param altitude - current camera parameter for altitude
        @param heading - current camera parameter for heading
        @param tilt - current camera parameter for tilt
    */
    virtual void Prefetch(double lat, double lon, float zoomLevel, float heading, float tilt) = 0;

    /*! Stop the animations.
        @param type - animation type, Bitwise OR of masks that indicate the animation to be stopped
        @return None.
    */
    virtual void StopAnimations(int type) = 0;

    /*! Play Doppler.
     * @return None.
     */
    virtual void PlayDoppler() = 0;

    /*! Pause Doppler.
     * @return None.
     */
    virtual void PauseDoppler() = 0;

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
    virtual void AddCircle(int circleId,  double lat, double lon, float radius, int fillColor, int strokeColor, int zOrder, bool visible) = 0;

    /*! remove the circle
     *  @param circleId - indentity of this circle.
     *  @return None.
     */
    virtual void RemoveCircle(int circleId) = 0;

    /*! Remove all the circls.
     *  @return None.
     */
    virtual void RemoveAllCircles() = 0;

    /*! Set the center of the circle
     *  @param circleId - indentity of this circle.
     *  @param lat - latitude of this center.
     *  @param lon - longitude of this center.
     *  @return None.
     */
    virtual void SetCircleCenter(int circleId, double lat, double lon) = 0;

    /*! Set the visiblity of the circle
     *  @param circleId - indentity of this circle.
     *  @param visible.
     *  @return None.
     */
    virtual void SetCircleVisible(int circleId, bool visible) = 0;

    /*! Set the style of the circle
     *  @param circleId - indentity of this circle.
     *  @param radius - radius of this center, in meters
     *  @param fillColor - RGBA
     *  @param outlineColor - RGBA
     *  @return None.
     */
    virtual void SetCircleStyle(int circleId, float radius, int fillColor, int outlineColor) = 0;

    /*! set a relative center of map, in follow me mode avatar will be located in this position instead of screen center, and rotate and zoom will base on this position too.
     * @param x - X Screen Position in terms of pixel.
     * @param y - Y Screen Position in terms of pixel.
     * @return None.
     */
    virtual void SetReferenceCenter(float x, float y) = 0;

    /*! enable or disable reference center.
     * @param enable - turn on or turn off ReferenceCenter
     * @return None.
     */
    virtual void EnableReferenceCenter(nb_boolean enable) = 0;

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
    virtual void AddRect2d(int rectId,  double lat, double lon, float heading, float width, float height, int textureId, shared_ptr<nbcommon::DataStream> textureBuffer, bool visible) = 0;

    /*! remove the rect2d
     *  @param rectId - indentity of this rect2d.
     *  @return None.
     */
    virtual void RemoveRect2d(int rectId) = 0;

    /*! Update the rect2d
     *  @param rect2d - indentity of this rect2d.
     *  @param lat - latitude of this center.
     *  @param lon - longitude of this center.
     *  @param heading - heading angle of this rect2d, in degrees.
     *  @return None.
     */
    virtual void UpdateRect2d(int rectId, double lat, double lon, float heading) = 0;

    /*! Set the visiblity of the rect2d
     *  @param rect2d - indentity of this rect2d.
     *  @param visible.
     *  @return None.
     */
    virtual void SetRect2dVisible(int rectId, bool visible) = 0;

    /*! Set the size of the rect2d
     *  @param rect2d - indentity of this rect2d.
     *  @param width - in pixels.
     *  @param height - in pixels.
     *  @return None.
     */
    virtual void SetRect2dSize(int rectId, float width, float height) = 0;

    /*! Remove all the rect2d.
     *  @return None.
     */
    virtual void RemoveAllRect2ds() = 0;

    /*! Add a texture to the map.
     *  @param textureId, unique identity to the texture
     *  @param textureData
     *  @return None.
     */
    virtual void AddTexture(int textureId, nbcommon::DataStreamPtr textureData) = 0;

    /*! Remove the texture.
     *  @param textureId - unique identity to the texture
     *  @return None.
     */
    virtual void RemoveTexture(int textureId) = 0;

    /*! Remove all the rect2d.
     *  @return None.
     */
    virtual void RemoveAllTextures() = 0;
    
    /*! Sets the scale value of the Text.
     *  @return None.
     */
    virtual void SetFontScale(float scale) = 0;

    /*! Sets the scale value of the Avatar.
     *  @return None.
     */
    virtual void SetAvatarScale(float scale) = 0;

    /*! Add the pins.
     *  @param pinParameters - parameters for adding the pins
     *  @return None
     */
    virtual void AddPins(std::vector<nbmap::PinParameters<nbmap::BubbleInterface> >& parametersVector) = 0;

    /*! Remove the pins.
     *  @param pinIDs - Specified the pins to remove
     *  @return None
     */
    virtual void RemovePins(const vector<shared_ptr<string> >& pinIDs) = 0;

    /*! Remove all the pins.
     *  @return None
     */
    virtual void RemoveAllPins() = 0;

    /*! Update the pin position.
     *  @param pinID - Specified the pin to update
     *  @param lat - The latitude of the new position.
     *  @param lon - The longitude of the new position.
     *  @return None
     */
    virtual void UpdatePinPosition(shared_ptr<string> pinID, double lat, double lon) = 0;


    /*! Set MapView HBAO parameters.
     *  @param params - specified parameters
     *  @return None
     */
    virtual void SetHBAOParameters(const HBAOParameters& parameters) = 0;

    /*! Set MapView glow parameters.
     *  @param params - specified parameters
     *  @return None
     */
    virtual void SetGlowParameters(const GlowParameters& parameters) = 0;

    /*! Set MapView DPI.
     *  @param dpi - specified DPI
     *  @return None
     */
	virtual void SetDPI(float dpi) = 0;
protected:

    /*! MapViewInterface destructor

        Avoid calling delete to destroy instances of map view. User should always call the function
        Destroy to destroy map view.
    */
    virtual ~MapViewInterface(){};
};

#endif

/*! @} */

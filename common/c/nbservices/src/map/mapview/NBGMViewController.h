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
 @file     NBGMViewController.h
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

#ifndef _NBGM_VIEW_CONTROLLER_H_
#define _NBGM_VIEW_CONTROLLER_H_

#include "nberror.h"
#include <string>
#include <list>
#include <set>
#include <vector>
#include "paltypes.h"
#include "TileKey.h"
#include "Tile.h"
#include "smartpointer.h"
#include "BubbleListenerInterface.h"
#include "MapViewProfiling.h"
#include "nbgm.h"
#include "nbgmtypes.h"
#include "MapViewAnimation.h"
#include "MapViewAnimationLayer.h"  // For ANIMATION_DATA
#include "ExternalMaterial.h"
#include "MapViewInterface.h"
#include "MapViewCamera.h"

#define DEFAULT_ANIMATION_DURATION 1000

#define INVALID_CAMERA_LOCATION -999.0
#define INVALID_CAMERA_ZOOMLEVEL -999.0
#define INVALID_CAMERA_TILT -999.0
#define INVALID_CAMERA_HEADING -999.0

static const uint32 MIN_RENDER_INTERVAL = 16;
static const uint32 MAX_RENDER_INTERVAL = 50;

using namespace std;

class NBGM_MapView;
class TileLoadingListener;
namespace nbmap
{
/* Class used for the debug output overlay on the map */
class DebugInfo
{
public:
    DebugInfo():
    actualZoomLevel(0.0),
    latitude(0.0),
    longitude(0.0),
    rotationAngle(0.0),
    tiltAngle(0.0),
    cameraHeight(0.0),
    renderTime(0),
    totalNumOfTriangles(0),
    numberOfLoadedTiles(0),
    lastRenderTickCount(0),
    lastLoggingTickCount(0){};

    double
    CalculateFrameRate(uint32 newRenderTime, vector<uint32>& lastRenderTimes) const
    {
        const size_t AVERAGE_COUNT = 20;

        double frameRate = 0.0;
        lastRenderTimes.push_back(newRenderTime);

        if (lastRenderTimes.size() > AVERAGE_COUNT)
        {
            lastRenderTimes.erase(lastRenderTimes.begin());
        }

        for (size_t i = 0; i < lastRenderTimes.size(); ++i)
        {
            frameRate += lastRenderTimes[i];
        }
        if (lastRenderTimes.size() > 0)
        {
            frameRate /= (double)(lastRenderTimes.size());
        }
        if (frameRate > 0)
        {
            frameRate = 1000.0 / frameRate;
        }

        return frameRate;
    }

    // Public members

    double actualZoomLevel;
    double latitude;
    double longitude;
    double rotationAngle;
    float  tiltAngle;
    float  cameraHeight;
    uint32 renderTime;
    uint32 totalNumOfTriangles;
    uint32 numberOfLoadedTiles;

    // Last tick count used to calculate total frame rate
    uint32 lastRenderTickCount;

    // Used for logging, we only want to log every second
    uint32 lastLoggingTickCount;

    // Last render times used to calculate average render time
    vector<uint32> lastRenderTimesNBGM;
    vector<uint32> lastRenderTimesTotal;
};


/*! Class to track avatar location */
class LocationBubbleInfo
{
public:
    LocationBubbleInfo() : m_x(0), m_y(0), m_latitude(0), m_longitude(0), m_enabled(false) {}
    virtual ~LocationBubbleInfo() {}

    float m_x;            /*!< The position of X, the whole length of X is 1 */
    float m_y;            /*!< The position of X, the whole length of X is 1 */
    float m_latitude;     /*!< Latitude of avatar  */
    float m_longitude;    /*!< Longitude of avatar */

    bool  m_enabled;
};

/*! A wrapper of TilePtr.

  It includes the original shared pointer of tile, and extra information which can be
  extended and used for debugging and logging.
 */
class TileWrapper
{
public:
    TileWrapper(TilePtr tile, bool logRenderTime=false)
            : m_tile(tile), m_logRenderTime(logRenderTime) {}
    virtual ~TileWrapper() {}

    TilePtr m_tile;                  /*!< Shared pointer of real tile. */
    shared_ptr<string> m_previousID; /*!< If present, this should use this tile to replace
                                          previously loaded tile specified by this id. */
    bool    m_logRenderTime;         /*!< Flag to indicate if we need to log render time
                                          of this tile. Required by BBMap */
};

typedef shared_ptr<TileWrapper> TileWrapperPtr;


class MaterialInfo
{
public:
    MaterialInfo(const string& categoryName,
                 const string& materialName):
    m_categoryName(categoryName),
    m_materialName(materialName)
    {}

    bool operator < (const MaterialInfo& info) const
    {
        return (m_categoryName < info.m_categoryName)
            ||((m_categoryName == info.m_categoryName) && (m_materialName < info.m_materialName));
    }

    string m_categoryName;/*!< Name of category */
    string m_materialName;/*!< Name of material */
};


class PinGroup
{

public:
    PinGroup():m_selectedPinId(NULL){}
    ~PinGroup()
    {
        removeAllPins();
    }
    shared_ptr<string>  m_selectedPinId;
    vector<shared_ptr<string>>  m_pinIds;
    void addPin(string pinId)
    {
        m_pinIds.push_back(shared_ptr<string>(new string(pinId.c_str())));
    }
    void removeAllPins()
    {
        m_pinIds.clear();
    }
};

/*! NBGMViewController class used in map view.

    This class is responsible for transformation map view settings (for example camera settings to NBGM format) and for
    communication between MapView and NBGM_Manager.
 */
class NBGMViewController
{
public:
    enum AnimationAccelerationType
    {
        AAT_LINEAR,
        AAT_DECELERATION,
        AAT_ACCELERATEDECELERATE,
    };


public:
    NBGMViewController(NBGM_MapView* nbgmMapView, MapViewCamera* mapViewCamera);
    virtual ~NBGMViewController();
    NB_Error Initiallize();
    NB_Error Finalize();

    void GenerateMapImage(uint8* buffer, uint32 bufferSize, int32 x, int32 y, uint32 width, uint32 height);

    /*! This function call nbgmmapview for asynchronous Generate a screenshot. */
    PAL_Error AsyncGenerateMapImage(const shared_ptr<NBGM_AsyncSnapScreenshotCallback>& snapScreenshot);

    bool LoadMapData(TileWrapperPtr tile);
    void RemoveMapData(const vector<TilePtr>& tiles);

    /*! Load and unload new tiles from NBGM.

        @return 'true' if tiles got loaded or unloaded, 'false' otherwise
     */
    bool LoadAndUnloadTiles();

    /*! Unload all tiles from NBGM containing NBGM cache

        @return None
    */
    void UnloadAllTiles();

    bool Render(NBGM_FrameListener* listener = NULL);

    /*! Notify map view is set to/from background*/
    void SetBackground(bool background);

    NB_Error UpdateCommonSettings(const vector<TilePtr>& tiles, bool isDay, bool isSatellite);
    void UseTileService() {m_useTileService = true;}


    /*! Return list of interacted pois for provided screen coordinates */
    shared_ptr<list<string> >
    GetInteractedPois(float x,      /*!< The position of X */
                      float y       /*!< The position of Y */
                      );

    /*! Return screen position for poi with provided poiId

        @return false is there is no poi with following poiId
    */
    bool
    GetPoiBubblePosition(const string& poiId,     /*!< The pin ID to get the position */
                         float* x,                /*!< The position of X, the whole length of
                                                 X is 1, so it is a percent. */
                         float* y                 /*!< The position of Y */
                         );

    /*! Get list of static pois for provided screen coordinates */
    void
    GetStaticPois(float x,                          /*!< The position of X */
                  float y,                          /*!< The position of Y */
                  std::vector<std::string>& poiList /*!< The result poi id list */
                  );

    /*! Return info for static poi with provided poiId
        @return false is there is no poi with following poiId
    */
    bool
    GetStaticPoiInfo(const string& poiId,        /*!< The poi ID to get the position */
                    float* bubbleOffsetX,   /*!< The position of X, the whole length of X is 1, so it is a percent. */
                    float* bubbleOffsetY,   /*!< The position of Y */
                    string& name,           /*!< The name of static poi */
                    double* achorLat,       /*!< The latitude of achor point */
                    double* anchorLon       /*!< The longtitude of achor point */
                    );

    /*! Return screen position for static poi with provided poiId

        @return false is there is no static poi with following poiId
    */
    bool
    GetStaticPoiBubblePosition(const string& poiId,     /*!< The pin ID to get the position */
                         float* x,                /*!< The position of X, the whole length of
                                                 X is 1, so it is a percent. */
                         float* y                 /*!< The position of Y */
                         );

    void SetBubblelistener(BubblelistenerInterface* listener);

    /*! Functions used to select, unselected and remove the pin.
        They must be called in the render thread.

        Both functions SelectPin and UnselectPin check if the pin is visible on the screen internal and
        notify to show or hide the bubble.

        TRICKY: Both functions SelectPin and UnselectPin notify the map view to show or hide the bubble
                by the pin listener, but the function RemovePin does not notify to hide the bubble, because
                the pin has been removed and cannot use the pin ID to find the bubble.
    */
    void SelectPin(shared_ptr<string> pinId, bool selected);
    void UnselectPin(shared_ptr<PinGroup> pinGroup);
    void DoUnselectPin(shared_ptr<PinGroup> group);
    void RemovePin();
    shared_ptr<string> GetSelectedPinId(std::string pinId);
    vector<shared_ptr<string>> GetSelectedPinIds();

    shared_ptr<string> GetSelectedStaticPoiId();
    void SetSelectedStaticPoiId(shared_ptr<string> id);


    /*! Set avatar mode

        @return None
    */
    void SetAvatarMode(NBGM_AvatarMode avatarMode        /*!< Avatar mode to set */
                       );

    /*! Set avatar state

        @return None
    */
    void SetAvatarState(NBGM_AvatarState avatarState     /*!< Avatar state to set */
                        );

    /*! Set avatar location

        @return None
    */
    void SetAvatarLocation(const NBGM_Location64& avatarLocation     /*!< Avatar location to set */
                           );

    /*! Set custom avatar

        @return None
    */
    void SetCustomAvatar(shared_ptr<nbcommon::DataStream> directionalAvatar,          /*!< The directional avatar */
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
                         );

    /*! Set custom avatar

        @return None
    */
    void SetCustomAvatar(shared_ptr<string> directionalAvatarPath,    /*!< The directional avatar file path*/
                         shared_ptr<string> directionlessAvatarPath,  /*!< The directionless avatar file path*/
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
                         );

    /*! Set the custom avatar halo style

     @return None
     */
    virtual void
    SetCustomAvatarHaloStyle(uint32 edgeColor,               /*!< Halo edge color */
                             uint32 fillColor,               /*!< Halo fill color */
                             uint8 edgeSize,                 /*!< Halo edge width */
                             NBGM_HaloEdgeStyle edgeStyle    /*!< Halo edge style */
                             );

    /*! Check if avatar is hit.

        @return true if avatar is hit, or false otherwise.
    */
    bool IsAvatarHit(float screenX, /*!< avatar screen x */
                     float screenY  /*!< avatar screen y */
                     );

    void EnableLocationBubble(bool enabled);

    /*! Fill debug info structure

        @return none
     */
    void GetDebugInfo(DebugInfo& info);

    /*! Enable / disable far near visibility property

        @return none
     */
    void EnableFarNearVisibility(bool flag);


    /*! Get supported NBM file version from NBGM library

        @return Supported NBM file version
    */
    static uint32 GetSupportedNBMFileVersion();


    // Animation overlay logic ...............................................................................................

    /* Add/Remove animation overlay layers. Removing an animation layer automatically removes any loaded tiles associated with that layer. */
    void AddAnimationLayer(uint32 layerID);
    void RemoveAnimationLayer(uint32 layerID);

    /*! Update frame data for all (or some) animation layers. */
    void SetAnimationFrameData(const map<uint32, ANIMATION_DATA>& data      /*!< The key into the map is a layer-ID. See ANIMATION_DATA for more info */
                               );

    /*! Unload animation tiles from NBGM. */
    void UnloadAnimationTiles(uint32 layerID,                               /*!< Layer-ID of tiles to unload */
                              const vector<shared_ptr<string> >& tiles      /*!< Content-IDs of tiles to unload */
                              );

    void SetAnimationLayerOpacity(uint32 opacity);

    bool IsAvatarInScreen(float mercatorX, float mercatorY);
    //void SetViewCenter(double mercatorX, double& mercatorY);


    /**
     * @name IsCompassTapped - Detect if compass is tapped.
     * @param screenX - x screen coordinate.
     * @param screenY - y screen coordinate.
     * @return nb_boolean - 'TRUE' if compass is tapped, 'FALSE' otherwise.
     */
    nb_boolean IsCompassTapped(float screenX, float screenY);

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

    /*! Some actions should be called when one frame rendering is over,
        such as UpdatePinInformation.If not,
        the calcultation of bubble position will be incorrected.
    */
    void OnRenderFrameEnd();
    void GetElementsAt(float screenX, float screenY, set<std::string>& elements);
    void EnableCustomLayerCollisionDetection(const std::string& layerId, bool enable);

    /*! Load, Unload or Activate external materials. */
    void LoadExternalMaterial(const ExternalMaterialPtr& material);
    void UnloadExternalMaterial(const ExternalMaterialPtr& material);
    void ActivateExternalMaterial(const ExternalMaterialPtr& material);
    void GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY)const;
    void UnloadCommonMaterials();

    /*! Reset static poi related state */
    void ResetPoiBubbleState();

    void ReloadTiles(const vector<TilePtr>& tiles);
    /*! Select the pending pin */
    void SelectPendingPin();
    /*! Cancel the pending pin */
    void CancelPendingPin();
    /*! Select the current pin */
    void SelectCurrentPin();

     /*! Add a circle to the map.
     * @param circleId - indentity of this circle.
     * @param circlePara -  circle parameters.
     * @return None.
     */
    void AddCircle(int circleId, const NBGM_CircleParameters &circlePara);

    /*! remove the circle
     * @param circleId - indentity of this circle.
     * @return None.
     */
    void RemoveCircle(int circleId);

    /*! Remove all the circls.
     * @return None.
     */
    void RemoveAllCircles();

     /*! set the center of the circle
     * @param circleId - indentity of this circle.
     * @param mercatorX - X of this center.
     * @param mercatorY - Y of this center.
     * @return None.
     */
    void SetCircleCenter(int circleId, double mercatorX, double mercatorY);

    /*! Set the visiblity of the circle
     *  @param circleId - indentity of this circle.
     *  @param visible.
     *  @return None.
     */
    void SetCircleVisible(int circleId, bool visible);

    /*! Set the style of the circle
     *  @param circleId - indentity of this circle.
     *  @param radius - radius of this center, in meters
     *  @param fillColor
     *  @param outlineColor
     *  @return None.
     */
    void SetCircleStyle(int circleId, float radius, const NBGM_Color& fillColor, const NBGM_Color& outlineColor);

     /*! Add a rect2d to the map.
     * @param rectId - indentity of this rect2d.
     * @param para -  rect2d parameters.
     * @return None.
     */
    void AddRect2d(NBGM_Rect2dId rectId, const NBGM_Rect2dParameters &para);

    /*! remove the rect2d
     * @param rectId - indentity of this rect2d.
     * @return None.
     */
    void RemoveRect2d(NBGM_Rect2dId rectId);

    /*! Remove all the rect2d.
     * @return None.
     */
    void RemoveAllRect2ds();

     /*! Update the rect2d
     *  @param rect2d - indentity of this rect2d.
     *  @param mercatorX - X of this center.
     *  @param mercatorY - Y of this center.
     *  @param heading - heading angle of this rect2d, in degrees.
     *  @return None.
     */
    void UpdateRect2d(NBGM_Rect2dId rectId, double mercatorX, double mercatorY, float heading);

    /*! Set the visiblity of the rect2d
     *  @param rect2d - indentity of this rect2d.
     *  @param visible.
     *  @return None.
     */
    void SetRect2dVisible(NBGM_Rect2dId rectId, bool visible);

    /*! Set the size of the rect2d
     *  @param rect2d - indentity of this rect2d.
     *  @param width - in pixels.
     *  @param height - in pixels.
     *  @return None.
     */
    void SetRect2dSize(NBGM_Rect2dId rectId, float width, float height);

    /*! Add a texture to the map.
     * @param textureId - indentity of this texture.
     * @param textureData.
     * @return true if success.
     */
    bool AddTexture(NBGM_TextureId textureId, const NBGM_BinaryBuffer &textureData);

    /*! remove the texture
     * @param textureId - indentity of this texture.
     * @return None.
     */
    void RemoveTexture(NBGM_TextureId textureId);

    /*! Remove all the textures.
     * @return None.
     */
    void RemoveAllTextures();
    
    /*! Sets the scale value of the Text.
     *  @return None.
     */
    void SetFontScale(float scale);

    /*! Sets the scale value of the Avatar.
     *  @return None.
     */
    void SetAvatarScale(float scale);

    void SetGestureProcessingFlag(bool gestureProcessing);

    /*! Add the pins.
     *  @param pinParameters - parameters for adding the pins
     *  @return None
     */
    void AddPins(const std::vector<NBGM_PinParameters>& pinParameters);

    /*! Remove the pins.
     *  @param pinIDs - specified the pins to remove
     *  @return None
     */
    void RemovePins(const vector<shared_ptr<string> >& pinIDs);

    /*! Remove all the pins.
     *  @return None
     */
    void RemoveAllPins();

    /*! Update the pin position.
     *  @param pinID - specified the pin to update
     *  @param locationX, pin x position.
     *  @param locationY, pin y position.
     *  @return None
     */
    void UpdatePinPosition(shared_ptr<string> pinID, double locationX, double locationY);

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

    /*! Set MapView glow parameters.
     *  @param params - specified parameters
     *  @return None
     */
    virtual void SetDPI(float dpi);

private:


    /*! Update the position of the selected static poi in the render thread */
    void UpdateStaticPoiInformation();

    /*! Update the position of the selected pin in the render thread */
    void UpdatePinInformation();

    /*! Update the position of the location bubble in the render thread */
    void UpdateLocationInformation();

    /*! Update location bubble internally.

        This function will collect necessary information which can be used to notify bubble
        listener to show or update location bubble.

        @return true if it is necessary to notify the listener(to show or update location
                bubble).
    */
    bool UpdateAvatarLocationInternally();



    void SetAvatar(const NBGM_Location64& location);

    /*! Load NBGM tile to NBGMManager */
    void LoadTile(TileWrapperPtr tile);

    /*! Check if any tiles need to be unloaded. If so unload them from NBGM

        @return 'true' if tiles were unloaded, 'false' otherwise
     */
    bool UnloadTilesIfNecessary();


    /*!
       Create an device location object for custom avatar.
     */
    NBGM_DeviceLocation* CreateCustomAvatar();

    /*!
       Use the avatar to default one.
     */
    void ResetDefaultAvatar();

    void ShowBubble(shared_ptr<string> pinId, bool visible, float x = 0.f, float y = 0.f);

    shared_ptr<PinGroup> getPinGroupByGroupId(string groupId);

    shared_ptr<PinGroup> getPinGroupByPinId(string pinId);

    // Private members ........................................................................................................

    NBGM_MapView*                       m_nbgmMapView;
    
    MapViewCamera*                      m_mapViewCamera;


    NBGM_Point3d                        m_avatarScale;

    double                              m_currentZoomLevel;
    bool                                m_materialsAreLoaded;
    BubblelistenerInterface*            m_bubblelistener;

    map<string, shared_ptr<PinGroup> >  m_pinGroups;

    /*! Selected pin ID, the variable is only used in the render thread. */
    //shared_ptr<string>                  m_selectedPinId;

    /*! Selected static poi ID, the variable is only used in the render thread. */
    shared_ptr<string>                  m_selectedStaticPoiId;

    /*! The flag if the selected pin is visible on the screen,
        the variable is only used in the render thread. */
    bool                                m_pinVisibleOnScreen;

    /*! The flag if the selected static poi is visible on the screen,
        the variable is only used in the render thread. */
    bool                                m_staticPoiVisibleOnScreen;

    /*! Avatar mode */
    NBGM_AvatarMode                     m_avatarMode;

    /*! Avatar state */
    NBGM_AvatarState                    m_avatarState;

    /*! Avatar location */
    NBGM_Location64                     m_avatarLocation;

    /*! Custom avatar object */
    NBGM_DeviceLocation*                m_customAvatar;

    /*! Custom avatar visibility. */
    bool                                m_customAvatarVisibility;

    /*! Tiles that need to be loaded to NBGM */
    vector<TileWrapperPtr>                     m_tilesToLoad;

    /*! Tiles that need to be unloaded from NBGM. */
    vector<TilePtr>                     m_tilesToUnload;

    /*! ONLY FOR DEBUGGING: Currently loaded tiles */
    set<string>                         m_debugLoadedTiles;

    /*! Overlay animation layers (e.g. weather radar overlay), get created/destroyed dynamically */
    typedef map<uint32, NBGM_TileAnimationLayer*> NBGMAnimationLayerMap;
    NBGMAnimationLayerMap m_animationLayers;

    /*! Profiling info. Only used if enabled */
    MapViewProfiling                    m_profilingRender;
    MapViewProfiling                    m_profilingLoad;

    /*! Layer far near visibility */
    bool                                m_layerVisibility;

    uint32                              m_animationLayerOpacity;

    /*!< LocationBubble information. */
    LocationBubbleInfo                  m_locationBubbleInfo;

    /*!< map view has been set to background or not. */
    bool                                m_background;

    /*!< use tile service or not. */
    bool                                m_useTileService;

    /*! avatar update or not. */
    bool                                m_AvatarUpdate;

    bool m_PinPendingFlag;
    shared_ptr<string> m_PendingPinId;
    shared_ptr<TileLoadingListener> m_TileLoadingListener;

    typedef map<int, NBGM_Circle*> NBGMCircleMap;
    NBGMCircleMap m_Circles;
    typedef map<NBGM_Rect2dId, NBGM_CustomRect2d*> NBGMRect2dMap;
    NBGMRect2dMap m_Rect2ds;
    float m_preBubblePositionX;
    float m_preBubblePositionY;
    string m_preSelectedPinId;
    bool m_showBubble;
    string m_prePinId;
    bool m_gestrueProcessing;
    typedef set<MaterialInfo> MaterialInfos;
    MaterialInfos m_loadedMaterials;
};

}
#endif //_NBGM_VIEW_CONTROLLER_H_

/*! @} */

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

    @file nbgmmapview.h
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

#ifndef _NBGM_MAP_VIEW_H
#define _NBGM_MAP_VIEW_H
#include "nbgmexp.h"
#include "palerror.h"
#include "paltypes.h"
#include "nbgmavatarsetting.h"
#include "nbgmmapviewprofile.h"
#include "nbgmtileanimationlayer.h"
#include "nbgmoptionallayer.h"
#include "nbgmdevicelocation.h"
#include "nbgmmapcamera.h"
#include "nbgmimapdata.h"
#include "nbgmstaticpoiinfo.h"
#include "nbgmcircle.h"
#include "nbgmcustomrect2d.h"

#include <string>
#include <vector>
#include <set>

#ifdef USE_NB_TILE_OBJECT
#include "Tile.h"
#endif


/*! This interface is used to inform ccc mapview screen shot complete.
*/
class NBGM_AsyncSnapScreenshotCallback
{
public:
    virtual ~NBGM_AsyncSnapScreenshotCallback(){};
    /*! Generate a screenshot bitmap data.
    @param width, the width of screenshot. 
    @param height, the height of screenshot. 
    @param buffer, an RGBA buffer. This memory is malloc, and nbgm will free it after call back.
    @param bufferSize, the size of the input buffer.
    */
    virtual void OnScreenshotFinished(int width, int height, const uint8* buffer, uint32 bufferSize) = 0;
};

/*! This interface is used to inform ccc mapview tile loading complete.
*/
class NBGM_TileLoadingListener
{
public:
    virtual ~NBGM_TileLoadingListener() {};
    /*! Will be called after tile loading finished.
    */
    virtual void OnLoadingFinished(const std::string& tileID) = 0;
};

class NBGM_FrameListener
{
public:
    virtual ~NBGM_FrameListener(){};
    virtual void OnRenderFrameBegin() = 0;
    virtual void OnRenderFrameEnd() = 0;
    virtual void OnDiscard() = 0;
};

/*! \addtogroup NBGM_Manager
*  @{
*/

enum NBGM_ProfilingFlags
{
    NBGM_PF_DISABLE         = 0,
    NBGM_PF_ENABLE          = 1,
    NBGM_PF_ENABLE_VERBOSE  = 2,
};

enum NBGM_NBMLoadFlags
{
    NBGM_NLF_NONE               = 0,        // Default flags.
    NBGM_NLF_ENABLE_LOG         = 0x1,      // The flag to enable the log in render function.
    NBGM_NLF_ENABLE_PICKING     = 0x1 << 1, // The flag to enable object picking function.
};

/*! NBGM material loading parameters
*/
struct NBGM_MaterialParameters
{
    std::string              categoryName;         /*!< material category*/
    std::string              materialName;         /*!< material name*/
    uint8*                   buffer;               /*!< material data block*/
    uint32                   bufferSize;           /*!< material data size in bytes*/
    std::string              filePath;             /*!< material file path*/
    std::vector<std::string> dependsMaterial;      /*!< dependent materials, can be empty*/
};

/*! NBGM material loading flags, reserved for future usage
*/
enum NBGM_MaterialLoadingFlags
{
    NBGM_MLF_NONE
};

/*! Provides low level map functions to make it easier for NBServices and UI layers.
    @remarks
        NBGM_MapView is supposed to be a map render, use this object, you can 
        render map tiles, control camera to show different content, as well as draw
        some other map elements such as POI/PIN and avatar etc.
    @par
        NBGM_MapView is not threading save, you must protect NBGM_MapView calls on
        different threads yourself. It is suggested that make NBGM_MapView work on 
        one thread: tile rendering thread.
    @par
        NBGM_MapView has a loading thread in it. You don't need to create a new load
        thread and call the loading functions in this new thread. When you call the
        loading functions such as LoadNBMTile, LoadNBMTileFromBuffer and UnLoadTile,
        NBGM_MapView will parse data and build models in the loadind thread of itself.
    @par
        NBGM_MapView only provides a limited set of camera control functions to let
        you control camera such as set tilt angle, set rotate angle, set view distance,
        etc. But NBGM_MapView doesn't provide camera animation, you need generate a
        sequence of individual camera setting and use them to update camera.
*/
class NBGM_MapView
{
public:
    virtual ~NBGM_MapView(){}

public: 

    /*! Initialize render environment. This function MUST be called before any
    other functions, or you will get a crash.        
    @return none
     */
    virtual void Initialize() = 0;

    /*! Finalize the map view. This function MUST be called before delete map view,
	or you will get a memory leak!
     @return none
     */
    virtual void Finalize() = 0;

    /*! Invalidate a frame.
     @return none
     */
    virtual void Invalidate() = 0;

    /*! Request to render a frame.
     @return none
     */
    virtual void RequestRender(NBGM_FrameListener* listener = NULL) = 0;

    /*! Set render interval.
     @return none
     */
    virtual void SetRenderInterval(uint32 interval) = 0;

    /*! Get out the map camera from map view, you should
     @return map camera, it is used to control map view.
     */
    virtual NBGM_MapCamera& GetMapCamera() = 0;

    /*! Notify map view is set from/to background.
     */
    virtual void SetBackground(bool background) = 0;

    /*! Load external material file, which will be used by rendering nbm map.
     @param params, loading parameters, see details in NBGM_MaterialParameters.
     @param flags, combinations of NBGM_MaterialLoadingFlags.
     @return PAL error code
     */
    virtual PAL_Error LoadExternalMaterial(const NBGM_MaterialParameters& params, uint32 flags) = 0;

    /*! Load external material file, which will be used by rendering nbm map.
     @param params, loading parameters, see details in NBGM_MaterialParameters.
     @param flags, combinations of NBGM_MaterialLoadingFlags.
     @return PAL error code
     */
    virtual PAL_Error LoadExternalMaterialFromBuffer(const NBGM_MaterialParameters& params, uint32 flags) = 0;

    /*! Load external material file, which will be used by rendering nbm map.
     @param categoryName, Specifies the category of this material.
     @param materialName, Specifies material name, each material file should has different name
     @param materialTable materials. The material id is same as its array index(begin with 0).
     @return PAL error code
     */
    virtual PAL_Error LoadExternalMaterialFromMemory(const std::string& categoryName, const std::string& materialName, std::vector<NBGM_IMapMaterial*>& materialTable) = 0;

    /*! Set current external material used by rendering nbm map. Eg, day & night model may has different common
        material file.
     @param categoryName, Specifies the category of this material.
     @param materialName, Material name, Specified by @LoadCommonMaterial
     @return PAL error code
     */
    virtual PAL_Error SetCurrentExternalMaterial(const std::string& categoryName, const std::string& materialName) = 0;

    /*! Unload external material used by rendering nbm map. Eg, day & night model may has different common
        material file.
     @param categoryName, Specifies the category of this material.
     @param materialName, Material name, Specified by @LoadCommonMaterial
     @return PAL error code
     */
    virtual PAL_Error UnloadExternalMaterial(const std::string& categoryName, const std::string& materialName) = 0;

    /*! Set Sky Day or Night mode.
     @param isDay, it's a boolean value, true means day mode, false means night mode
     */
    virtual void SetSkyDayNight(nb_boolean isDay) = 0;

    /*! Set is satellite mode.
        In satellite mode roads are rendered to a framebuffer, with a transparent opacity.
     @param isSatellite, it's a boolean value, true means satellite mode, false means non-satellite mode
     */
    virtual void SetSatelliteMode(nb_boolean isSatellite) = 0;

    /*! Check if a nbm tile has been loaded into map view or not.
     @return true for loaded, false for not loaded
     */
    virtual bool IsNbmTileLoaded(const std::string& nbmName) = 0;

    /*! Load nbm format tile file
     @param nbmName, nbm file unique id, NBM file name should be different from each other.
        Map view use this name to unload file.
     @param categoryName, Specifies the category of this material which is used by this nbm tile.
     @param groupName, logical group of nbm file. Currently used to manage custom overlapping test.
     @param baseDrawOrder Base draw order of this tile
     @param subDrawOrder Sub draw order of this tile
     @param labelDrawOrder Label draw order of this tile
     @param filePath The full path of the tile file to be loaded
     @param flags load options which are the combination of several NBGM_NBMLoadFlags.
     @param listener for loading finish.
     @return PAL error code
     */
    virtual PAL_Error LoadNBMTile(const std::string& nbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const std::string& filePath, uint32 flags, NBGM_TileLoadingListener* listener = NULL) = 0;

#ifdef USE_NB_TILE_OBJECT
    /*! Load nbm format tile files
     @param tiles, a vector that contains the information of all the tiles which are loaded.
     @return PAL error code
     */
    virtual PAL_Error LoadNBMTile(const std::vector<nbmap::TilePtr>& tiles) = 0;
#endif

    /*! Load nbm format tile from binary buffer.
     @param nbmName, nbm file unique id
     @param categoryName, Specifies the category of this material which is used by this nbm tile.
     @param groupName, logical group of nbm file. Currently used to manage custom overlapping test.
     @param baseDrawOrder Base draw order of this tile
     @param subDrawOrder Sub draw order of this tile
     @param labelDrawOrder Label draw order of this tile
     @param buffer Buffer address
     @param bufferSize Buffer size
     @param flags load options which are the combination of several NBGM_NBMLoadFlags.
     @param listener for loading finish.
     @return PAL error code
     */
    virtual PAL_Error LoadNBMTileFromBuffer(const std::string& nbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize, uint32 flags, NBGM_TileLoadingListener* listener = NULL) = 0;

    /*! Create standard color material.
     @param color color in BGRA format
     @return material data
     */
    virtual NBGM_IMapMaterial* CreateStandardColorMaterial(uint32 color) = 0;

    /*! Create memory nbm data.
     @param refCenter reference center of nbm data
     @return in memory nbm data
     */
    virtual NBGM_IMapData* CreateMapData(const NBGM_Point2d& refCenter) = 0;

    /*! Destroy memory nbm data. Do NOT call destroy if mapData was passed to LoadMapData().
     @param mapdata, in memory nbm data
     @return None
     */
    virtual void DestroyMapData(NBGM_IMapData* mapData) = 0;

    /*! Load memory nbm data.
     @remarks After calling load, NEVER perform any operation on mapdata object. 
              NBGM will destroy it automatically. Or there will be threading issues.
     @param nbmName, nbm file unique id
     @param categoryName, Specifies the category of this material which is used by this nbm tile.
     @param baseDrawOrder Base draw order of this tile
     @param subDrawOrder Sub draw order of this tile
     @param labelDrawOrder Label draw order of this tile
     @param flags load options which are the combination of several NBGM_NBMLoadFlags.
     @param mapdata, in memory nbm data
     @return PAL error code
     */
    virtual PAL_Error LoadMapData(const std::string& nbmName, const std::string& categoryName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, uint32 flags, NBGM_IMapData* mapdata) = 0;

    /*! Unload a tile.
     @param nbmName, nbm file unique id
     @return none
     */
    virtual void UnLoadTile(const std::string& nbmName) = 0;

    /*! Unload tiles.
     @param nbmNames, vector that contains all the nbm file unique id which is unloaded
     @return none
     */
    virtual void UnLoadTile(const std::vector<std::string>& nbmNames) = 0;

    /*! Unload all tiles from cache
     @return none
     */
    virtual void UnLoadAllTiles() = 0;

    /*! Reload nbm format tile from binary buffer.
     @param needUnloadedNbmName, tile need to be unloaded.
     @param needLoadedNbmName, nbm file unique id, it is should not be empty string.
     @param categoryName, Specifies the category of this material which is used by this nbm tile.
     @param groupName, logical group of nbm file. Currently used to manage custom overlapping test.
     @param baseDrawOrder Base draw order of this tile
     @param subDrawOrder Sub draw order of this tile
     @param labelDrawOrder Label draw order of this tile
     @param buffer Buffer address
     @param bufferSize Buffer size
     @param flags load options which are the combination of several NBGM_NBMLoadFlags.
     @param listener for loading finish.
     @return PAL error code
     */
    virtual PAL_Error ReloadNBMTileFromBuffer(const std::string& needUnloadedNbmName, const std::string& needLoadedNbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize, uint32 flags, NBGM_TileLoadingListener* listener = NULL) = 0;

    /*! Update nbm format tile from binary buffer, this function will replace old tile by new data.
     @param needLoadedNbmName, nbm file unique id, it is should not be empty string.
     @param categoryName, Specifies the category of this material which is used by this nbm tile.
     @param groupName, logical group of nbm file. Currently used to manage custom overlapping test.
     @param baseDrawOrder Base draw order of this tile
     @param labelDrawOrder Label draw order of this tile
     @param buffer Buffer address
     @param bufferSize Buffer size
     @param flags load options which are the combination of several NBGM_NBMLoadFlags.
     @param listener for loading finish.
     @return PAL error code
     */
    virtual PAL_Error UpdateNBMTileFromBuffer(const std::string& nbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize, uint32 flags) = 0;

    /*! Set avatar location.
     @param location, avatar location
     @return none
     */
    virtual void SetAvatarLocation(const NBGM_Location64& location) = 0;
    
    /*! Get font scale.
     @return scaleValue, font scale, 1.0 is original size
     */
    virtual float GetFontScale( void ) = 0;
    
    /*! Set font scale.
     @param scaleValue, font scale, 1.0 is original size
     @return none
     */
    virtual void SetFontScale(float scaleValue) = 0;

    /*! Set avatar scale.
     @param scaleValue, avatar scale, 1.0 is original size
     @return none
     */
    virtual void SetAvatarScale(float scaleValue) = 0;

    /*! Get avatar screen position.
     @param screenX, avatar screen x
     @param screenY, avatar screen y
     @return true/false. If this avatar is on the screen return true, else return false.
     */
    virtual bool GetAvatarScreenPosition(float& screenX, float& screenY) = 0;

    /*! Is avatar in screen
     @param mercatorX, avatar x position
     @param mercatorY, avatar y position
     @return true/fase. If this avatar is in screen return true, else return false.
     */
    virtual bool IsAvatarInScreen(float mercatorX, float mercatorY) = 0;

    /*! Get avatar screen polygons. 
        These polygons are projected from 5 faces(no bottom face) of avatar bounding box.
     @param screenPolygons, screen polygons
     @return none
     */
    virtual void GetAvatarScreenPolygons(std::vector<float>& screenPolygons) = 0;

    /*! Set avatar state.
     @param state, avatar state
     @return none
     */
    virtual void SetAvatarState(NBGM_AvatarState state) = 0;

    /*! Set avatar mode
     @param mode, avatar mode
     @return none
     */
    virtual void SetAvatarMode(NBGM_AvatarMode mode) = 0;

    /*! Get if given screen position hit avatar. 
      @param x, screen point x
      @param y, screen point y
      @return is avatar hit
     */
    virtual bool SelectAndTrackAvatar(float x, float y) = 0;

    /*! Disable/Enable profiling.
     @flags combination of NBGM_ProfilingFlags, default is 0 (no profiling)
     @return none
     */
    virtual void EnableProfiling(uint32 flags) = 0;

    /*! Get current poi/pin screen coordinate.
     @param poiId, poi/pin's identifier defined in Pin NBM file.
     @param coordinates, screen coordinate of current poi/pin.It is output.Screen left-top is(0,0).
     @return true/false.If current poi/pin is visible on screen then return true, otherwise return false.
     */
    virtual bool GetPoiPosition(const std::string& poiId, NBGM_Point2d& coordinates) = 0;

    /*! Get current poi/pin's bubble reference screen coordinate.
     @param poiId, poi/pin's identifier defined in Pin NBM file.
     @param coordinates, reference screen coordinate.It is output.Screen left-top is(0,0).
     @return true/false.If current reference coordinate is visible on screen then return true, otherwise return false.
     */
    virtual bool GetPoiBubblePosition(const std::string& poiId, NBGM_Point2d& coordinates) = 0;

    /*! Select a poi/pin which will change its state in next render cycle.
     @param poiId, poi/pin's identifier defined in Pin NBM file.
     @return true/false.If current poi/pin is visible on screen then return true, otherwise return false.
     */
    virtual bool SelectAndTrackPoi(const std::string& poiId) = 0;

    /*! Unselect a poi/pin which will change its state in next render cycle.
     @param poiId, poi/pin's identifier defined in Pin NBM file.
     @return true/false.If current poi/pin is visible on screen then return true, otherwise return false.
     */
    virtual bool UnselectAndStopTrackingPoi(const std::string& poiId) = 0;

    /*! Get all the pois/pins which are interacted
     @param poiList, selected poi/pin's which are interacted.it is output.
     @param x, screen x coordinate. Screen left-top is(0,0).
     @param y, screen y coordinate. Screen left-top is(0,0).
     @return the total number of the interacted pois/pins.
     */
    virtual uint32 GetInteractedPois(std::vector<std::string>& poiList, float x, float y) = 0;

    /*! Get static poi's information.
     @param poiId, poi's identifier defined in poi NBM file.
     @param name, poi info.It is output.
     @return true/false.If poiId exists return true, otherwise return false.
     */
    virtual bool GetStaticPoiInfo(const std::string& poiId, NBGM_StaticPoiInfo& info) = 0;

    /*! Get all the static pois at point
     @param poiList, poi list under the point.it is output.
     @param x, screen x coordinate. Screen left-top is(0,0).
     @param y, screen y coordinate. Screen left-top is(0,0).
     @return none.
     */
    virtual void GetStaticPois( std::vector<std::string>& poiList, float x, float y ) = 0;

    /*! Select or unselect one static poi;
     @param poiId, poi's identifier defined in poi NBM file
     @param selected, poi selected status.
     @return none.
     */
    virtual void SelectStaticPoi(const std::string& id, nb_boolean selected) = 0;

    /*! Get all elements at screen point. Only works on the tiles loaded with enablePicking=TRUE.
     @param x, screen x coordinate. Screen left-top is(0,0).
     @param y, screen y coordinate. Screen left-top is(0,0).
     @param ids, output selected elements id list.
     @return none
     */
    virtual void GetElementsAt(float x, float y, std::set<std::string>& ids) = 0;

    /*! Enable or disable collision detection for layer.
     @param layerId, layer ID.
     @param enable, if true NBGM will check the collision of layer elements and hide overlapped item, else no overlap checking.
     @return none
     */
    virtual void EnableCollisionDetection(const std::string& layerId, bool enable) = 0;

    /*! Retrieve profile
     @return none
     */
    virtual void DumpProfile(NBGM_MapViewProfile& profile) = 0;

    /*! Add a tile animation layer which manages animation data like weather. 
      @return NBGM_TileAnimationLayer
     */
    virtual NBGM_TileAnimationLayer* CreateTileAnimationLayer() = 0;

    /*! Remove a tile animation layer. All of tiles loaded by this layer will be removed too. 
      @return NBGM_TileAnimationLayer
     */
    virtual void RemoveTileAnimationLayer(NBGM_TileAnimationLayer* layer) = 0;

    /*! Add a optional layer. 
      @return NBGM_OptionalLayer
     */
    virtual NBGM_OptionalLayer* CreateOptionalLayer() = 0;

    /*! Remove a optional layer. All of tiles loaded by this layer will be removed too. 
      @return NBGM_OptionalLayer
     */
    virtual void RemoveOptionalLayer(NBGM_OptionalLayer* layer) = 0;

    /*! Create device location instance. 
      @param ID an unique ID for this device location
      @return a device location instance
     */
    virtual NBGM_DeviceLocation* CreateDeviceLocation(std::string ID) = 0;

    /*! Release a device location instance. 
     */
    virtual void ReleaseDeviceLocation(NBGM_DeviceLocation* device) = 0;

    /*! Snap a screenshot of NBGM map view. This function MUST be called in the render thread.
    @param buffer, an RGBA buffer. Please note that the first pixel of the buffer represents the top left of the image.
    @param bufferSize, the size of the input buffer, it should be larger than (width * height * 4) bytes.
    @param x, Left-top coordinate of screen shot.
    @param y, Left-top coordinate of screen shot.
    @param width, width of screen shot.
    @param height, height of screen shot.
    @return error code
    */
    virtual PAL_Error SnapScreenshot(uint8* buffer, uint32 bufferSize, int32 x, int32 y, uint32 width, uint32 height) const = 0;

    /*! Snap a screenshot of NBGM map view. This function is asynchronized version of SnapScreenshot.
    @param snapScreenshot, is a call back, when NBGM finish loading and rendering it will be called.
    @return error code
    */
    virtual PAL_Error AsyncSnapScreenshot(const shared_ptr<NBGM_AsyncSnapScreenshotCallback>& snapScreenshot) = 0;

    /*! To check if the compass is tapped.
    @param screenX, is screen x coordinate.
    @param screenY, is screen y coordinate.
    @return nb_boolean. Return ture when tapped.
    */
    virtual nb_boolean TapCompass(float screenX, float screenY) = 0;

    /*! To enable compass.
    @param value. Set TRUE to enable compass
    @return none.
    */
    virtual void EnableCompass(nb_boolean value) = 0;

    /*! Set compass screen position.
    @param screenX. x screen coordinate.
    @param screenY. y screen coordinate.
    @return none.
    */
    virtual void SetCompassPosition(float screenX, float screenY) = 0;

    /*! Set Copmass Day or Night mode.
     @param isDay, it's a boolean value, true means day mode, false means night mode
     */
    virtual void SetCompassDayNightMode(nb_boolean isDay) = 0;

    /*! Set compass icons.
    @param dayModeIconPath. Icon path of day mode.
    @param nightModeIconPath. Icon path of night mode.
    @return none.
    */
    virtual void SetCompassIcons(const std::string& dayModeIconPath, const std::string& nightModeIconPath) = 0;

     /*! Get the bounding box of the compass.
     * @param leftBottomX - x screen coordinate of left bottom point, in pixels.
     * @param leftBottomY - y screen coordinate of left bottom point, in pixels.
     * @param rightTopX - x screen coordinate of right top point, in pixels.
     * @param rightTopY - y screen coordinate of right top point, in pixels.
     * @return None.
     */
    virtual void GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY) const = 0;

    /*! Create a circle instance.
     * @param ID - an unique ID for this circle
     * @param circlePara - parameters for creating circle
     * @return a pointer to the circle
     */
    virtual NBGM_Circle* AddCircle(int circleId, const NBGM_CircleParameters &circlePara) = 0;

    /*!Remove the circle
     * @param circle - pointer to this circle.
     * @return None.
     */
    virtual void RemoveCircle(NBGM_Circle* circle) = 0;

    /*!Remove all the circles.
     * @return None.
     */
    virtual void RemoveAllCircles() = 0;

    /*! Create a rect2d instance.
     * @param rectId - an unique ID for this rect2d
     * @param para - parameters for creating rect2d
     * @return a pointer to the rect2d
     */
    virtual NBGM_CustomRect2d* AddRect2d(NBGM_Rect2dId rectId, const NBGM_Rect2dParameters &para) = 0;

    /*!Remove the rect2d
     * @param rect2d - pointer to this rect2d.
     * @return None.
     */
    virtual void RemoveRect2d(NBGM_CustomRect2d* rect2d) = 0;

    /*!Remove all the rect2ds.
     * @return None.
     */
    virtual void RemoveAllRect2ds() = 0;

    /*! Create a texture.
     * @param textureId - an unique ID for this texture
     * @param textureData.
     * @return True if sccussed.
     */
    virtual bool AddTexture(NBGM_TextureId textureId, const NBGM_BinaryBuffer &textureData) = 0;

    /*!Remove the texture
     * @param textureId - an unique ID for this texture
     * @return None.
     */
    virtual void RemoveTexture(NBGM_TextureId textureId) = 0;

    /*!Remove all the textures.
     * @return None.
     */
    virtual void RemoveAllTextures() = 0;

    /*! Add the pins.
     *  @param pinParameters - parameters for adding the pins
     *  @return None
     */
    virtual void AddPins(const std::vector<NBGM_PinParameters>& pinParameters) = 0;

    /*! Remove the pins.
     *  @param pinIDs - specified the pins to remove
     *  @return None
     */
    virtual void RemovePins(const vector<shared_ptr<string> >& pinIDs) = 0;

    /*! Remove all the pins.
     *  @return None
     */
    virtual void RemoveAllPins() = 0;

    /*! Update the pin position.
     *  @param pinID - specified the pin to update
     *  @param locationX, pin x position.
     *  @param locationY, pin y position.
     *  @return None
     */
    virtual void UpdatePinPosition(shared_ptr<string> pinID, double locationX, double locationY) = 0;

    /*! Set MapView HBAO parameters.
     *  @param params - specified parameters
     *  @return None
     */
    virtual void SetHBAOParameters(const NBGM_HBAOParameters& parameters) = 0;

    /*! Set MapView glow parameters.
     *  @param params - specified parameters
     *  @return None
     */
    virtual void SetGlowParameters(const NBGM_GlowParameters& parameters) = 0;

    /*! Set MapView dpi.
     *  @param dpi - specified dpi
     *  @return None
     */
    virtual void SetDPI(float dpi) = 0;
};
/*! @} */
#endif

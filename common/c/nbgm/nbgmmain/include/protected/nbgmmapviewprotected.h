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

    @file nbgmmapviewprotected.h
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

#ifndef _NBGM_MAP_VIEW_PROTECTED_H_
#define _NBGM_MAP_VIEW_PROTECTED_H_
#include "palerror.h"
#include "paltypes.h"
#include "nbrecommon.h"
#include "nbgmmapview.h"
#include "nbgmmapviewconfig.h"
#include "paltimer.h"

class NBGM_MapViewImpl;
class NBGM_FakeMapView;
class NBRE_RenderEngine;
class NBRE_IRenderPal;
class NBGM_ResourceManager;
class NBGM_MapCameraImpl;

/*! \addtogroup NBGM_Manager
*  @{
*/
#define GL_CAPABILITY_TEST 0

class NBGM_MapViewInternal : public NBGM_MapView
{
public:
    NBGM_MapViewInternal(const NBGM_MapViewConfig& config, PAL_Instance* pal);
    NBGM_MapViewInternal(const NBGM_MapViewConfig& config, PAL_Instance* pal, NBGM_ResourceManager* resourcManager);
    ~NBGM_MapViewInternal();

private:
    virtual void Initialize();
    virtual void Finalize();
    virtual void Invalidate();
    virtual void RequestRender(NBGM_FrameListener* listener);
    virtual void SetRenderInterval(uint32 interval);
    virtual NBGM_MapCamera& GetMapCamera();
    virtual void SetBackground(bool background);
    virtual PAL_Error LoadExternalMaterial(const NBGM_MaterialParameters& params, uint32 flags);
    virtual PAL_Error LoadExternalMaterialFromBuffer(const NBGM_MaterialParameters& params, uint32 flags);
    virtual PAL_Error LoadExternalMaterialFromMemory(const std::string& categoryName, const std::string& materialName, std::vector<NBGM_IMapMaterial*>& materialTable);
    virtual PAL_Error SetCurrentExternalMaterial(const std::string& categoryName, const std::string& materialName);
    virtual PAL_Error UnloadExternalMaterial(const std::string& categoryName, const std::string& materialName);
    virtual void SetSkyDayNight(nb_boolean isDay);
    virtual void SetSatelliteMode(nb_boolean isSatellite);
    virtual bool IsNbmTileLoaded( const std::string& nbmName );
    virtual PAL_Error LoadNBMTile(const std::string& nbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const std::string& filePath, uint32 flags, NBGM_TileLoadingListener* listener);
#ifdef USE_NB_TILE_OBJECT
    virtual PAL_Error LoadNBMTile(const std::vector<nbmap::TilePtr>& tiles);
#endif
    virtual PAL_Error LoadNBMTileFromBuffer(const std::string& nbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize, uint32 flags, NBGM_TileLoadingListener* listener);
    virtual NBGM_IMapMaterial* CreateStandardColorMaterial(uint32 color);
    virtual NBGM_IMapData* CreateMapData(const NBGM_Point2d& refCenter);
    virtual void DestroyMapData(NBGM_IMapData* mapData);
    virtual PAL_Error LoadMapData(const std::string& nbmName, const std::string& categoryName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, uint32 flags, NBGM_IMapData* mapdata);
    virtual void UnLoadTile( const std::string& nbmName );
    virtual void UnLoadTile(const std::vector<std::string>& nbmNames);
    virtual void UnLoadAllTiles();
    virtual PAL_Error ReloadNBMTileFromBuffer(const std::string& needUnloadedNbmName, const std::string& needLoadedNbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize, uint32 flags, NBGM_TileLoadingListener* listener);
    virtual PAL_Error UpdateNBMTileFromBuffer(const std::string& nbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize, uint32 flags);
    virtual void SetAvatarLocation( const NBGM_Location64& location );
    virtual float GetFontScale( void );
    virtual void SetFontScale( float scaleValue );
    virtual void SetAvatarScale( float scaleValue );
    virtual bool GetAvatarScreenPosition( float& screenX, float& screenY );
    virtual bool IsAvatarInScreen(float mercatorX, float mercatorY);
    virtual void GetAvatarScreenPolygons( std::vector<float>& screenPolygons );
    virtual void SetAvatarState( NBGM_AvatarState state );
    virtual void SetAvatarMode( NBGM_AvatarMode mode );
    virtual bool SelectAndTrackAvatar(float x, float y);
    virtual void EnableProfiling( uint32 flags );
    virtual bool GetPoiPosition( const std::string& poiId, NBGM_Point2d& coordinates );
    virtual bool GetPoiBubblePosition(const std::string& poiId, NBGM_Point2d& coordinates);
    virtual bool SelectAndTrackPoi( const std::string& poiId );
    virtual bool UnselectAndStopTrackingPoi( const std::string& poiId );
    virtual uint32 GetInteractedPois( std::vector<std::string>& poiList, float x, float y );
    virtual bool GetStaticPoiInfo(const std::string& poiId, NBGM_StaticPoiInfo& info);
    virtual void SelectStaticPoi(const std::string& id, nb_boolean selected);
    virtual void GetStaticPois( std::vector<std::string>& poiList, float x, float y );
    virtual void GetElementsAt(float x, float y, std::set<std::string>& ids);
    virtual void EnableCollisionDetection(const std::string& layerId, bool enable);
    virtual void DumpProfile(NBGM_MapViewProfile& profile);
    virtual NBGM_TileAnimationLayer* CreateTileAnimationLayer();
    virtual void RemoveTileAnimationLayer(NBGM_TileAnimationLayer* layer);
    virtual NBGM_OptionalLayer* CreateOptionalLayer();
    virtual void RemoveOptionalLayer(NBGM_OptionalLayer* layer);
    virtual void SetHBAOParameters(const NBGM_HBAOParameters& parameters);
    virtual void SetGlowParameters(const NBGM_GlowParameters& parameters);
    virtual void SetDPI(float dpi);
    //virtual void SetAllowRedraw(bool enable);
    //virtual void SetRasterMap(bool isRaster);

    /*! Disable/Enable a layer. If a layer is disabled, all loaded NBM files
     with in layer will not be rendered.
     (Render thread function)
     @remarks
     NBGM distinguish different layers based on "base draw order". That
     means if two NBM file have same base draw order, they will be treated
     as same layer.
     @par
     When enable/disable a layer, NBGM will enable/disable all loaded NBM
     files, whose base draw order are same as specified.
     @par
     This func doesn't unload tile.

     @param drawOrder Draw order of the layer.
     @enable true for enable, false for disable
     @return none
    */
    virtual void EnableLayer( uint8 drawOrder, bool enable );

    virtual NBGM_DeviceLocation* CreateDeviceLocation(std::string ID);
    virtual void ReleaseDeviceLocation(NBGM_DeviceLocation* device);

    /*! Snap a screenshot of NBGM map view. This function MUST be called in the render thread.
    @param rgbaBuffer, an RGBA buffer. Please note that the first pixel of the buffer represents the top left of the image.
    @param bufferSize, the size of the input buffer, it should be larger than (width * height * 4) bytes.
    @param x, Left-top coordinate of screen shot.
    @param y, Left-top coordinate of screen shot.
    @param width, width of screen shot.
    @param height, height of screen shot.
    @return error code.
    */
    virtual PAL_Error SnapScreenshot(uint8* rgbaBuffer, uint32 bufferSize, int32 x, int32 y, uint32 width, uint32 height) const;

    /*! Snap a screenshot of NBGM map view. This function is asynchronized version of SnapScreenshot.
    @param snapScreenshot, is a call back, when NBGM finish loading and rendering it will be called.
    @return error code
    */
    virtual PAL_Error AsyncSnapScreenshot(const shared_ptr<NBGM_AsyncSnapScreenshotCallback>& snapScreenshot);

    /*! To check if the compass is tapped.
    @param screenX, is screen x coordinate.
    @param screenY, is screen y coordinate.
    @return nb_boolean. Return ture when tapped.
    */
    virtual nb_boolean TapCompass(float screenX, float screenY);

    /*! To enable compass.
    @param value. Set TRUE to enable compass
    @return none.
    */
    virtual void EnableCompass(nb_boolean value);

    /*! Set compass screen position.
    @param screenX. x screen coordinate.
    @param screenY. y screen coordinate.
    @return none.
    */
    virtual void SetCompassPosition(float screenX, float screenY);

    /*! Set Copmass Day or Night mode.
     @param isDay, it's a boolean value, true means day mode, false means night mode
     */
    virtual void SetCompassDayNightMode(nb_boolean isDay);

    /*! Set compass icons.
    @param dayModeIconPath. Icon path of day mode.
    @param nightModeIconPath. Icon path of night mode.
    @return none.
    */
    virtual void SetCompassIcons(const std::string& dayModeIconPath, const std::string& nightModeIconPath);

    /*! Get the bounding box of the compass.
     * @param leftBottomX  -  x screen coordinate of left bottom point, in pixels.
     * @param leftBottomY  - y screen coordinate of left bottom point, in pixels.
     * @param rightTopX - x screen coordinate of right top point, in pixels.
     * @param rightTopY - y screen coordinate of right top point, in pixels.
     * @return None.
     */
    virtual void GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY)const;

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
    virtual NBGM_CustomRect2d* AddRect2d(int NBGM_Rect2dId, const NBGM_Rect2dParameters &para);

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
     *  @param pinParameters - parameters for adding the pins
     *  @return None
     */
    virtual void AddPins(const std::vector<NBGM_PinParameters>& pinParameters);

    /*! Remove the pins.
     *  @param pinIDs - specified the pins to remove
     *  @return None
     */
    virtual void RemovePins(const std::vector<shared_ptr<std::string> >& pinIDs);

    /*! Remove all the pins.
     *  @return None
     */
    virtual void RemoveAllPins();

    /*! Update the pin position.
     *  @param pinID - specified the pin to update
     *  @param locationX, pin x position.
     *  @param locationY, pin y position.
     *  @return None
     */
    virtual void UpdatePinPosition(shared_ptr<std::string> pinID, double locationX, double locationY);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_MapViewInternal);

public:
    void UpdateCamera();

private:
    void UpdateRoadWidthFactor();
    float CalculateViewPointSkyHeight();
    void RestrictViewCenter();
    static void CameraUpdateCallBack(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);

private:
    NBGM_MapViewConfig mConfig;
    PAL_Instance* mPal;

    NBRE_RenderEngine*  mRenderEngine;
#if GL_CAPABILITY_TEST
    NBGM_FakeMapView* mImpl; 
#else
    NBGM_MapViewImpl*   mImpl;
#endif
    int8 mPrevZoomLevel;
    uint32 mScreenHeight;
    uint32 mScreenWidth;

private:
    int32  mUpdateExpectedTime;
    NBGM_MapCameraImpl* mCameraImpl;
    nb_boolean mUseSharedResource;
    NBGM_ResourceManager* mResourceManager;
    nb_boolean mIsAvatarUpdate;
    NBGM_Location64 mAvatarLocation;
};

/*! @} */
#endif

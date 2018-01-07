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

    @file nbgmnavview.h
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
#ifndef _NBGM_NAV_VIEW_H
#define _NBGM_NAV_VIEW_H
#include "nbgmexp.h"
#include "palerror.h"
#include "paltypes.h"
#include "nbgmavatarsetting.h"
#include "nbgmnavdata.h"

#include <string>
#include <vector>
#include <map>

/*! \addtogroup NBGM_Manager
*  @{
*/

/*! Provides low level navigation functions to make it easier for NBServices and UI layers.
    @remarks
        NBGM_NavView is supposed to be a map & navigation elements renderer, use
        this object, you can render map tiles, control camera to show
        different content, as well as draw some other map elements such
        as POI/PIN and avatar etc.
    @par
        NBGM_NavView is based on NBGM_MapView, that means NBGM_NavView has
        same threading feature. For more information, please refer to NBGM_MapView.
    @par
        Except camera controlling APIs, NBGM_NavView also provides
        a interface to set camera configuration. NBGM_NavView can calculate camera status
        base on avatar position and camera configuration. You can use them to make different
        navigation view, such as navigation main view and look ahead view.
        NBGM_NavView doesn't provide camera animation, you need generate a
        sequence of individual camera setting and use them to update camera.
    @deprecated
*/
class NBGM_NavView
{
public:
    virtual ~NBGM_NavView(){}

public: 

    /*! Initialize render environment. This function MUST be called before any
    other functions, or you will get a crash.        
    @return none
     */
    virtual void InitializeEnvironment() = 0;

    /*! Invalidate a frame.
     @return none
     */
    virtual void Invalidate() = 0;

    /*! Load ECM tile before view shown up, to speedup nav view rendering
    @return none
    */
    virtual void LoadFirstECMTile() = 0;

    /*! Notify map view that view size has been changed;
     @param x Specifies view origin point x
     @param y Specifies view origin point y
     @param width Specifies view width
     @param height Specifies view height
     @return none
    */
    virtual void SetViewSize(int32 x, int32 y, uint32 width, uint32 height) = 0;

    /*! Load common material file, which will be used by rendering nbm map.
     @param categoryName, Specifies the category of this material which is used by this nbm tile.
     @param materialName, Specifies material name, each material file should has different name
     @param filePath The full path of the material file to be loaded
     @return PAL error code
     */
    virtual PAL_Error LoadCommonMaterial(const std::string& categoryName, const std::string& materialName, const std::string& filePath) = 0;

    /*! Load common material file, which will be used by rendering nbm map.
     @param categoryName, Specifies the category of this material which is used by this nbm tile.
     @param materialName, Specifies material name, each material file should has different name
     @param buffer Buffer address
     @param bufferSize Buffer size
     @return PAL error code
     */
    virtual PAL_Error LoadCommonMaterialFromBuffer(const std::string& categoryName, const std::string& materialName, const uint8* buffer, uint32 bufferSize) = 0;

    /*! Set current common material used by rendering nbm map. Eg, day & night model may has different common
        material file.
     @param categoryName, Specifies the category of this material.
     @param materialName, Material name, Specified by @LoadCommonMaterial
     @return PAL error code
     */
    virtual PAL_Error SetCurrentCommonMaterial(const std::string& categoryName, const std::string& materialName) = 0;

    /*! Set Sky Day or Night mode.
     @param isDay, it's a boolean value, true means day mode, false means night mode
     */
    virtual void SetSkyDayNight(nb_boolean isDay) = 0;

    /*! Load nbm format tile file
     @param nbmName, nbm file unique id, NBM file name should be different from each other.
        Map view use this name to unload file.
     @param categoryName, Specifies the category of this material which is used by this nbm tile.
     @param baseDrawOrder Base draw order of this tile
     @param labelDrawOrder Label draw order of this tile
     @param filePath The full path of the tile file to be loaded
     @return PAL error code
     */
    virtual PAL_Error LoadNBMTile(const std::string& nbmName, const std::string& categoryName, uint8 baseDrawOrder, uint8 labelDrawOrder, const std::string& filePath) = 0;

    /*! Load nbm format tile from binary buffer.
     @param nbmName, nbm file unique id
     @param categoryName, Specifies the category of this material which is used by this nbm tile.
     @param baseDrawOrder Base draw order of this tile
     @param labelDrawOrder Label draw order of this tile
     @param buffer Buffer address
     @param bufferSize Buffer size
     @return PAL error code
     */
    virtual PAL_Error LoadNBMTileFromBuffer(const std::string& nbmName, const std::string& categoryName, uint8 baseDrawOrder, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize) = 0;

    /*! Unload a tile.
     @param nbmName, nbm file unique id
     @return none
     */
    virtual void UnLoadTile(const std::string& nbmName) = 0;

    /*! Disable/Enable a layer. If a layer is disabled, all loaded NBM files
        with in layer will not be rendered.
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
    virtual void EnableLayer(uint8 drawOrder, bool enable) = 0;

    /*! Disable/Enable profiling.
     @flags combination of NBGM_ProfilingFlags, default is 0 (no profiling)
     @return none
     */
    virtual void EnableProfiling(uint32 flags) = 0;

    /*! Switch NavView work mode
     @param workMode Target work mode to switch.
     @return PAL error code
     */
    virtual PAL_Error SwitchToWorkMode(NBGM_NavViewWorkMode workMode) = 0;

    /*! Set palette to control navigation elements style.
     @param palette A NBGM_Palette instance to control navigation elements style.
     @return PAL error code
     */
    virtual PAL_Error SetPalette(const NBGM_Palette& palette) = 0;

    /*! Set view exclude rectangles, the exclude rectangles area can't be use to
        layout labels, POIS and shields.
     @param rects A NBGM_Rect2d array, contains the exclude rectangles area.
     @return PAL error code
     */
    virtual PAL_Error UpdateExcludeRect(std::vector<NBGM_Rect2d*> rects) = 0;

    /*! Set camera settings for nav view. Different camera setting is used by different nav state
        See NBGM_NavCameraUsage
     @param settings Camera setting used by this view
     @return PAL error code
     */
    virtual PAL_Error SetCameraSetting(const std::map<NBGM_NavCameraUsage, NBGM_NavCameraSetting>& settings) = 0;

    /*! Retrieve current working mode
     @return Current working mode
     */
    virtual NBGM_NavViewWorkMode GetWorkMode() const = 0;

    /*! Notify nav view navigation data changed, nav view will update its state base on this data
     @param data New nav data
     @return PAL error code
     */
    virtual PAL_Error UpdateNavData(NBGM_NavData& data, uint32 modifyFlag) = 0;

    /*! Show navigation main view
     @return non
     */
    virtual void ShowNavMainView() = 0;

    /*! Show the given maneuver on screen
     @param maneuverID Maneuver ID to show
     @return non
     */
    virtual void ShowManeuver(int32 maneuverID) = 0;

    /*! Switch nav mode to a specified mode
     @param mode New nav mode, see NBGM_NavViewMode
     @return non
     */
    virtual void SetNavMode(NBGM_NavViewMode mode) = 0;

    /*! Re-layout all labels. This method removes all displayed labels.
        Note that calling it repeatedly will cause label flashing.
     @return none.
     */
    virtual void RefreshLabelLayout() = 0;

    virtual void SetLandscape(bool is_Landscape) = 0;
};

/*! @} */
#endif

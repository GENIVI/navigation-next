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

    @file nbgmnavecmview.h
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
#ifndef _NBGM_NAV_ECM_VIEW_H_
#define _NBGM_NAV_ECM_VIEW_H_
#include "nbrecommon.h"
#include "nbrecolor.h"
#include "nbgmnavdata.h"
#include "nbgmtypes.h"
#include "nbgmavatarsetting.h"
#include "nbgmmapviewconfig.h"
#include "nbgmmapview.h"

class NBRE_RenderEngine;
class NBRE_IOStream;
class NBGM_MapViewImpl;
class NBGM_ResourceManager;

/*! \addtogroup NBGM_Manager
*  @{
*/

/*! Navigation view used by ECM/MJO view.
    @remark
    ECM view just used when navreply has ECM/MJO data, and nav state is on
    route. When off route, look ahead view, navigation will switch to 
    vector view.
    @par
    This is a internal class, UI layer need use NBGM_NavView instead of this
    class.
*/
class NBGM_NavEcmView
{
public:
    NBGM_NavEcmView(const NBGM_MapViewConfig& config, NBRE_RenderEngine* renderEngine, NBGM_ResourceManager* resourceManager);
    ~NBGM_NavEcmView();

public:
    /*! Initialize render environment. This function MUST be called before any
    other functions, or you will get a crash.        
        (Render thread function)
    @return none
     */
    void InitializeEnvironment() ;

    /*! Render a frame.
        (Render thread function)
     @return none
     */
    void RenderFrame() ;

    /* Invalidate a frame
     @return none
     */
    void RequestRenderFrame(NBGM_FrameListener* listener);

    /*! Notify map view that view size has been changed;
        (Render thread function)
     @param x Specifies view origin point x
     @param y Specifies view origin point y
     @param width Specifies view width
     @param height Specifies view height
     @return none
    */
    void SetViewSize(int32 x, int32 y, uint32 width, uint32 height) ;

    /*! Load BIN format tile file
     @param binName, BIN file unique id, BIN file name should be different from each other.
        Map view use this name to unload file.
        (Load thread function)
     @param filePath The full path of the tile file to be loaded
     @return PAL error code
     */
    PAL_Error LoadBINTile(const NBRE_String& binName, NBRE_IOStream& is) ;

    /*! Unload a tile.
        (Load thread function)
     @param binName, BIN file unique id
     @return none
     */
    void UnLoadTile(const NBRE_String& binName) ;

    /*! Set avatar location.
        (Render thread function)
     @param location, avatar location
     @return none
     */
    void SetAvatarLocation(const NBGM_Location64& location) ;

    /*! Set avatar scale.
        (Render thread function)
     @param scaleValue, avatar scale, 1.0 is original size
     @return none
     */
    void SetAvatarScale(float scaleValue) ;

    /*! Set avatar state.
        (Render thread function)
     @param state, avatar state
     @return none
     */
    void SetAvatarState(NBGM_AvatarState state) ;

    /*! Set avatar mode
        (Render thread function)
     @param mode, avatar mode
     @return none
     */
    void SetAvatarMode(NBGM_AvatarMode mode) ;

    /*! Set palette to control navigation elements style.
     @param palette A NBGM_Palette instance to control navigation elements style.
     @return PAL error code
     */
    PAL_Error SetPalette(const NBGM_Palette& palette) ;

    /*! Set end flag location
     @param location End flag location
     @return PAL error code
     */
    PAL_Error SetEndFlagLocation(const NBGM_Location& location) ;

    /*! Set start flag location
     @param location Start flag location
     @return PAL error code
     */
    PAL_Error SetStartFlagLocation(const NBGM_Location& location) ;

    /*! Load route spline
     @param data Binary spline data got from navreply.
     @param maneuverID Maneuver ID, used by draw maneuver turn arrow.
     @return PAL error code
     */
    PAL_Error AddRouteSpline(NBGM_SplineData* data) ;

    /*! Remove route spline which are loaded by @AddRouteSpline.
     @par
     When recalculate, you need remove old route and then load new again.
     */
    PAL_Error RemoveRouteSpline( ) ;

    /*! Set current maneuver.
     @par
     Usually, navigation just show one turn arrow. In navigation main view, it is the maneuver which user
     stay in. In look ahead view, it is the maneuver user select to see.
     @param maneuverIndex Current maneuver index.
     @return PAL error code
     */
    PAL_Error SetCurrentManeuver(int32 maneuverIndex) ;

    /*! Clip the route highlight, just show the part that is not been clipped out
     @remark
     Navigation requirement: navigation view should not show the route user already go through.
     @param avatarManeuverId Indicates the maneuver index which avatar is staying on.
     @location Route behind this location will be clipped out.
     @return PAL error code
     */
    PAL_Error ClipRoute(int32 avatarManeuverId, const NBGM_Location& location) ;

    /*! Set camera settings for nav view. Different camera setting is used by NBGM_NavCameraUsage
        See NBGM_NavCameraUsage
     @param setting Camera setting used by this view
     @return PAL error code
     */
    PAL_Error SetCameraSetting(NBGM_NavCameraSetting setting) ;

    /*! Notify nav view navigation data changed, nav view will update its state base on this data
     @param data New nav data
     @return PAL error code
     */
    PAL_Error UpdateNavData(const NBGM_NavData& data, uint32 modifyFlag) ;

     /*! Get if avatar is blocked by building at a given position
     @param tileNames Just check building in this list
     @param distance Distance from current position
     @param height Camera height from ground
     @return TURE for blocked, FALSE for not
     */
    nb_boolean IsAvatarBlocked(const NBRE_Vector<NBRE_String> tileNames, float distance, float height);

     /*! Set map background color, day/night, ECM/MJO may has different background color
     @param c Background color
     @return non
     */
    void SetBackgroundColor(const NBRE_Color& c);

     /*! Set viewport and adjust frustum matrix 
     @param x, start screen X position(left-bottom) 
     @param y, start screen Y position(left-bottom)
     @param w, screen width used for rendering.
     @param h, screen height used for rendering.
     */
    void OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h);

    void ActiveSurface();

    void SetSkyDayNight(nb_boolean isDay);

    //TODO: this interface is used for debug, it will be deleted in the future
    NBGM_MapViewImpl* GetMapViewImpl(){return mMapView;}

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_NavEcmView);

private:
    NBGM_MapViewImpl*   mMapView;
};
/*! @} */
#endif

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

    @file nbgmnavvectorview.h
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
#ifndef _NBGM_NAV_VECTOR_VIEW_H_
#define _NBGM_NAV_VECTOR_VIEW_H_
#include "nbrecommon.h"
#include "nbgmnavdata.h"
#include "nbgmtypes.h"
#include "nbgmavatarsetting.h"
#include "nbgmnavviewconfig.h"
#include "nbgmnavvectorroutelayer.h"
#include "nbgmmapview.h"

class NBRE_IOStream;
class NBGM_MapViewImpl;
class NBRE_RenderEngine;
class NBGM_Animation;
class NBRE_RenderSurface;
class NBGM_ResourceManager;

/*! \addtogroup NBGM_Manager
*  @{
*/

class NBGM_NavVectorView
{
public:
    NBGM_NavVectorView(const NBGM_NavViewConfig& config, NBRE_RenderEngine* re, NBGM_ResourceManager* resourceManager);
    ~NBGM_NavVectorView();

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

    /*! Load common material file, which will be used by rendering nbm map.
        (Render thread function)
     @param categoryName, Specifies the category of this material.
     @param materialName, Specifies material name, each material file should has different name
     @param filePath The full path of the material file to be loaded
     @return PAL error code
     */
    PAL_Error LoadCommonMaterial(const NBRE_String& categoryName, const NBRE_String& materialName, shared_ptr<NBRE_IOStream> is) ;

    /*! Set current common material used by rendering nbm map. Eg, day & night model may has different common
        material file.
        (Render thread function)
     @param categoryName, Specifies the category of this material.
     @param materialName, Material name, Specified by @LoadCommonMaterial
     @return PAL error code
     */
    PAL_Error SetCurrentCommonMaterial(const NBRE_String& categoryName, const NBRE_String& materialName) ;

    /*! Load nbm format tile file
     @param nbmName, nbm file unique id, NBM file name should be different from each other.
        Map view use this name to unload file.
        (Load thread function)
     @param categoryName, Specifies the category of this material which is used by this nbm tile.
     @param baseDrawOrder Base draw order of this tile
     @param labelDrawOrder Label draw order of this tile
     @param filePath The full path of the tile file to be loaded
     @return PAL error code
     */
    PAL_Error LoadNBMTile(const NBRE_String& nbmName, const NBRE_String& categoryName, uint8 baseDrawOrder, uint8 labelDrawOrder, NBRE_IOStream* is) ;

    /*! Unload a tile.
        (Load thread function)
     @param nbmName, nbm file unique id
     @return none
     */
    void UnLoadTile(const NBRE_String& nbmName) ;

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

    /*! Set view exclude rectangles, the exclude rectangles area can't be use to
        layout labels, POIS and shields.
     @param rects A NBGM_Rect2d array, contains the exclude rectangles area.
     @return PAL error code
     */
    PAL_Error UpdateExcludeRect(NBRE_Vector<NBGM_Rect2d*> rects) ;

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


    /*! Remove route polylines which are loaded by @AddRoutePolyline.
     @par
     When recalculate, you need remove old route and then load new again.
     */
    PAL_Error RemoveRoutePolyline( ) ;

    /*! Set current maneuver.
     @par
     Usually, navigation just show one turn arrow. In navigation main view, it is the maneuver which user
     stay in. In look ahead view, it is the maneuver user select to see.
     @param maneuverIndex Current maneuver index.
     @return PAL error code
     */
    PAL_Error SetCurrentManeuver(int32 maneuverIndex) ;

    /*! Clip the route highlight, just show the part that is not been clipped out
     @par
     Navigation requirement: navigation view should not show the route user already go through.
     @param avatarManeuverId Indicates the maneuver index which avatar is staying on.
     @location Route behind this location will be clipped out.
     @return PAL error code
     */
    PAL_Error ClipRoute(int32 avatarManeuverId, const NBGM_Location& location) ;

    /*! Set building transparency.
     @param alphaValue Building transparency value, 1 means solid, 0 means totally transparent.
     @return PAL error code
     */
    PAL_Error SetBuildingTransparency(float alphaValue) ;
    
     /*! Set camera settings for nav view. Different camera setting is used by NBGM_NavCameraUsage
        See NBGM_NavCameraUsage
     @param settings Camera setting used by this view
     @return PAL error code
     */
    PAL_Error SetCameraSetting(const NBGM_NavCameraSetting& setting) ;

    /*! Notify nav view navigation data changed, nav view will update its state base on this data
     @param data New nav data
     @param modifyFlag modified nav data field
     @return PAL error code
     */
    PAL_Error UpdateNavData(const NBGM_NavData& data, uint32 modifyFlag) ;

    void ShowNavMainView() ;

    void ShowManaeuver(int32 maneuverID) ;

    void SetSensorHeading();

    void SetNavMode(NBGM_NavViewMode mode) ;

     /*! Set viewport and adjust frustum matrix 
     @param x, start screen X position(left-bottom) 
     @param y, start screen Y position(left-bottom)
     @param w, screen width used for rendering.
     @param h, screen height used for rendering.
     */
    void OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h);

    void ActiveSurface();

    void SetSkyDayNight(nb_boolean isDay);

    /*! Re-layout all labels.
     @return none.
     */
    void RefreshLabelLayout();

    //TODO: this interface is used for debug, it will be deleted in the future
    NBGM_MapViewImpl* GetMapViewImpl(){return mMapView;}

    nb_boolean FindManeuver(int32 maneuverID);
    uint32 NavVectorCurrentManeuver();
	NBRE_RenderSurface* Surface();

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_NavVectorView);

private:
    /*! Load route polyline to NBGM_NavViw to show.
     @par
     If there are more then one maneuvers, this func need be invoked multi-times to
     load route for each maneuver, or the maneuver turn arrow can not be rendered.
     @param navViw The  instance
     @param points A float buffer, contains polyline position in (x, y) order.
        points[0] = p0.x, points[1] = p0.y, points[2] = p1.x, points[3] = p1.y
     @param count Buffer size.
     @param maneuverID Maneuver ID, used by draw maneuver turn arrow.
     @return PAL error code
     */
    PAL_Error AddVectorRoutePolyline(const NBGM_VectorRouteData* routeData);
 
    /*! Set current maneuver ID then draw the turn arrow.
     @param currentManuever, current manuever id;
     @return none
     */
    void SetVectorCurrentManeuver(uint32 murrentManeuver);

     /*! Reset vector route layer.clear all the data in vector route layer.
     @return none
     */
    void ResetVectorRouteLayer();

private:
    struct VecManeuver
    {
        NBRE_Point2d  position;
        float         heading;
    };

private:
    NBGM_MapViewImpl* mMapView;
    NBGM_Location64 mAvatarLocation;
    NBRE_Map<int32, VecManeuver> mVecManeuvers;
    nb_boolean mIsNavMainView;
    int32 mCurrentManeuverIndex;
	NBGM_Animation* mAnimation;
	nb_boolean mIsAnimating;
	nb_boolean mIsRight2Left;
	float mAnimationOffset;
	nb_boolean mIsUseAnimation;
};

/*! @} */
#endif

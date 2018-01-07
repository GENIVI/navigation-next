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
    @file     MapNativeView.h
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

#ifndef _MAP_NATIVE_VIEW_
#define _MAP_NATIVE_VIEW_

#include "nberror.h"
#include "smartpointer.h"
#include "pal.h"
#include <string>
#include <vector>
#include <map>
#include "nbgpstypes.h"
#ifdef RENDER_ENGINE_3D_BUILDING
#include "nbexp.h"
#endif
namespace nbmap
{
    class Pin;
    class MapLegendInfo;
}

#include "MapLegendInfo.h"
#include "nbgmrendersystemtype.h"

class MapViewUIInterface;
class MapWidgetGetter;
class NBGM_RenderContext;

/*! Type of Traffic Tip */
enum TrafficTipType
{
    TTT_None = 0, /*!< None, should hide tip */
    TTT_ZoomIn,   /*!< Need to zoom in to show traffic  */
    TTT_ZoomOut,  /*!< Need to zoom out to show traffic */
    TTT_NoData,   /*!< No traffic tile is available */
};


/*! Type of font */
enum MapFontType
{
    MFT_SANS,
    MFT_SAN_BLOD,
    MFT_SERIF,
    MFT_SERFI_BLOD
};

/*! Type of Animation Tip */
enum AnimationTipType
{
    ATT_None = 0, /*!< None, should hide tip */
    ATT_ZoomIn,   /*!< Need to zoom in to show Animation  */
    ATT_ZoomOut,  /*!< Need to zoom out to show Animation */
    ATT_NoData,   /*!< No Animation tile is available */
};

/*! Map native view.

    This is the public interface of the map native view exposed to nbservices. This interface should only be used
    from nbservices.

    This C++ class forwards all calls to the native layer (Objective-C in case of iPhone)
*/
#ifdef RENDER_ENGINE_3D_BUILDING
class NB_DEF MapNativeView
#else
class MapNativeView
#endif
{
public:
    MapNativeView(MapViewUIInterface* mapView, int zorder, bool thirdPartyUsage, void* threadId);
    virtual ~MapNativeView();

    /*! Initialize MapNativeViewContent

        !Important: This funciton should NOT call back into MapViewInterface!

        @return NB_Error
     */
    NB_Error Initialize(int& width,     /*!< On return the view size width */
                        int& height,    /*!< On return the view size height */
                        nb_threadId renderingThread,
                        PAL_Instance* pal
                        );

    /*! Finalize MapNativeView content - brief clean up MapNativeView content

        @return NB_Error
     */
    void Finallize();

    /*! Show the view on the screen

        @return NB_Error
    */
    NB_Error ShowView();

    /*! This function is used to transfer NBGMMapNativeView to client(in current iPhone realization NBGMMapNativeView is transfered to UIMapMainViewController). This mechanism of transfering NBGMView from MApKit3D library to client is specific for each of all platforms.
        On iPhone platform we use mechanism of protocols and delegates to transfer NBGMView to client.

     @return NB_Error
     */

    NB_Error SetMapViewProtocol(void* protocol);

    /*! Get a pointer to the map native instance.

        This function is called from MapView (nbservices) and the pointer is passed to the application level.
        This allows the application to receive a platform specific instance from nbui.

        @return Pointer to platform specific MapNativeView instance.
     */
    void* GetNativeContext();

    /*! Show, hide, update and remove the pin bubble in UI thread */
    void ShowPinBubble(shared_ptr<nbmap::Pin> pin, float x, float y, bool orientation);
    void HidePinBubble(shared_ptr<nbmap::Pin> pin);
    void UpdatePinBubble(shared_ptr<nbmap::Pin> pin, float x, float y, bool orientation);
    void RemovePinBubble(shared_ptr<nbmap::Pin> pin);

    /*! Show/hide map legend. */
    void ShowMapLegend(shared_ptr<nbmap::MapLegendInfo> mapLegend);
    void HideMapLegend();

    /*! Update traffic tip based on tip type */
    void UpdateTrafficTip(TrafficTipType type);

    /*! Update animation tip based on tip type */
    void UpdateAnimationTip(AnimationTipType type);

    /*! Show/hide/update the timestamp lable for the animation layer.

        Only displayed when we have an animation layer.
     */
    void CreateTimestampLabel();
    void UpdateTimestampLabel(uint32 gpsTime    /*!< Time in GPS time, has to be converted to a meaningful string for display.
                                                     Set to zero to display "no date" (or similar) */
                              );
    void RemoveTimestampLabel();
    /*! Update the state(play/pause) of button for the animation layer. */
    void UpdateButtonState(bool animationCanPlay);

    /*! Show, hide, update and remove the location bubble */
    void ShowLocationBubble(float x, float y, bool orientation, double latitude, double longitude);
    void HideLocationBubble();
    void UpdateLocationBubble(float x, float y, bool orientation, double latitude, double longitude);

    /*! Create a render context which will be used by NBGM rendering

    @return An NBGM_RenderContext instance created by native map view
    */
    shared_ptr<NBGM_RenderContext> GetRenderContext() const;

    /*! This function is used to add update host view task from MapView class (nbservices).

      @return NONE
     */
     void UpdateHostView();

    /*! This function is used to enable / disable displaying of debug information.

     @return NONE
     */
    void EnableDebugInfo(bool isEnabled);

    /*! This function is used to set text that should be displayed.

     @return NONE
     */
    void SetDebugInfo(const char* text);

    void setZorder(int zorder);
    void SetViewSize(int width, int height);
    void resetScreen();
    void CreateFontBuffer(PAL_Instance* pal, MapFontType fontType, uint8** buffer, uint32& bufferSize);
    void UpdateTilt(float tilt);
    void RenderingAllowedNotified();
    void OnMapInitialized();
    void UpdateSurface();
    void DestroySurface();
    void SetGpsMode(NB_GpsMode mode);
    void SetDisplayScreen(int screenIndex);

    void OnUIReady(void* glContext);

private:
    void CreateDebugView();

    /*! Interface to map view in nbservices */
    MapViewUIInterface* m_pMapView;

    /*! Pointer to MapNativeHostView for iPhone or platform specific object. This pointer is defined as (void*) to avoid compilation
        issues on iPhone platform only. Before using this pointer should be cast to (MapNativeHostView*) type */
    void* m_pNativeHostView;
    shared_ptr<NBGM_RenderContext> m_nbgmRenderContext;
    void* m_renderInterface;
    int m_width;
    int m_height;
    uint32 m_lastTimeval;

    /*
        I think all of these should be move to MapNativeHostView (or other implementation class on other platforms)
     */

    /*! Pointer on Debug View */
    void* m_pDebugView;

    /*! Flag indicates whether need display Debug View or not */
    bool m_enableDebugInformation;
    int m_zorder;

    void* m_pLocationBubble;    /*!< Pointer to location bubble. */
    void* m_pMapLegendView;     /*!< Pointer to Legend View */
    void* m_pTipView;           /*!< Pointer to a tip label */

    /*! Cached pin bubbles used by render thread to update a bubble of pin. It uses pinId as key. */
    map<string, shared_ptr<void> > m_pinBubbles;

    bool m_thirdPartyUsage;

    void* m_renderThreadId;
};

#endif

/*! @} */

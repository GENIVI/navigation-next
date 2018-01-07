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
 @file         MapPresenter.h
 @defgroup     nkui
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary to
 TeleCommunication Systems, Inc., and considered a trade secret as defined
 in section 499C of the penal code of the State of California. Use of this
 information by anyone other than authorized employees of TeleCommunication
 Systems is granted only under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 --------------------------------------------------------------------------*/

/*! @{ */

#ifndef MapPresenter__
#define MapPresenter__

#include <iostream>
#include <functional>
#include "PresenterBase.h"
#include "smartpointer.h"
#include "GPSInterpolator.h"

namespace nkui
{
class BreadCrumb;

class MapPresenter
    : public PresenterBase
    , public MapListener
    , public nbnav::SessionListener
    , private nbnav::TrafficListener
    , private nbnav::RoutePositionListener
    , private InterpolatedGPSListener
{
public:
    MapPresenter();
    virtual ~MapPresenter();

    void UpdateMapSetting(const MapSetting& mapSetting);

    // override PresenterBase
    virtual void NotifyEvent(NKEventID event, NKUIEventData data);
    virtual void OnCameraUpdate(const NKUICameraPosition& camera);
    virtual void OnUnlocked();
    virtual void OnPolylineClick(const std::vector<NKUIPolyline*>& polyline);
    virtual void OnPinSelected(NKUIPin* pin);
    virtual void OnTapAtCoordinate(const nkui::NKUICoordinate& coordinate);
    virtual void OnCameraAnimationDone(int animationId,
                                       AnimationStatusType status);
protected:
    virtual void OnActive();
    virtual void OnDeactivate();
    virtual void OnSetWidget();
    virtual void HandleEvent(NKEventID id, void* data);

    /*! please reference SessionListener */
    virtual void OffRoutePositionUpdate(double headingToRoute);

    // override nbnav::TrafficListener
    virtual void TrafficChanged(const nbnav::TrafficInformation& trafficInfo);

    /*! override nbnav::RoutePositionListener::UpdateManeuverList */
    virtual void UpdateManeuverList(const nbnav::ManeuverList& maneuvers);

    /*! override InterpolatedGPSListener */
    virtual void InterpolatedGPSUpdated(const nbnav::Coordinates& coordinate,
                                        double heading);

    void ClearPolylines();
    void ClearPins();

private:
    void CreateRoutes(const std::vector<nbnav::RouteInformation>& routes);
    bool CreateRoute(NKUIPolylineParameter& polylineParameter,
                     const nbnav::RouteInformation& route,
                     const std::vector<nbnav::TrafficEvent>& traffics);
    void AddManeuverArrow(const nbnav::ManeuverList& maneuvers);
    void RemoveManeuverArrow(bool isRemoveParameter = true);
    void AddBreadCrumb(const NKUILocation& currentLocation);
    void UpdateBreadCrumb();
    void ZoomToFitMutipleRoute(bool retryIfFailed = false);

    static NKUIPolylineParameter* CreatePolylineParameter();
    static void DestroyPolylineParameter(NKUIPolylineParameter* parameter);

    void SetAvatarLocationAndCamera(const NKUILocation& location, bool needsAnimation);
    void ZoomToFitBoundingBox(const nkui::NKUICoordinateBounds& boundingBox,
                              bool retryIfFailed = false);

    class AnimationCallbackParameter;
    typedef shared_ptr<AnimationCallbackParameter> AnimationCallbackParameterPtr;
    typedef mem_fun1_t<void, MapPresenter, AnimationCallbackParameterPtr> \
            AnimationCallbackFunctor;
    typedef mem_fun_t<void, MapPresenter> AnimationFailoverFunctor;

    typedef void (MapPresenter::*AnimationCallbackFunction)(AnimationCallbackParameterPtr param);
    typedef void (MapPresenter::*AnimationFailoverFunction)();
    typedef map<int, AnimationCallbackParameterPtr>  AnimationCallbackMap;

    class AnimationCallbackParameter
    {
    public:
        AnimationCallbackParameter(AnimationCallbackFunction function,
                                   AnimationFailoverFunction failover)
            : m_status(AST_FINISHED),
              m_callback(mem_fun1_t<void, MapPresenter, AnimationCallbackParameterPtr>(function)),
              m_failover(mem_fun_t<void, MapPresenter>(failover))
        {}
        virtual ~AnimationCallbackParameter(){}
        AnimationStatusType      m_status;
        AnimationCallbackFunctor m_callback;
        AnimationFailoverFunctor m_failover;
    };

    inline NKUIAnimationParameters
    PrepareAnimationParameter(AnimationCallbackFunction function,
                              AnimationFailoverFunction failover,
                              const char* from="Unknown");

    void AnimationCallbackLockCameraPosition(AnimationCallbackParameterPtr param);
    void AnimationCallback(AnimationCallbackParameterPtr param);
    void AnimationDefaultFailover();
    void AnimationFailoverZoomToAll();

    /*!
     *  Show or hide compass.
     *
     * @param isShow ture to show compass otherwise hide compass
     * @return none
     */
    void ShowCompass(bool isShow);

    std::vector<NKUIPolylineParameter*>    m_polylineParameters;
    std::vector<NKUIPolyline*>             m_polylines;
    size_t                                 m_currentRouteIndex;
    NKUIPin*                               m_pOriginPin;
    NKUIPin*                               m_pDestinationPin;
    BreadCrumb*                            m_pBreadCrumbs;
    NKUICameraPosition                     m_lastCameraPosition;
    double                                 m_heading; // why it is tored here ??
    double                                 m_headingToRoute;
    /*! pointer to the object of maneuver arrow. */
    shared_ptr<NKUIPolyline>               m_pManeuverArrow;
    /*! pointer to the object of parameter which mapping the current maneuver arrow. */
    shared_ptr<NKUIManeuverArrowParameter> m_pManeuverArrowParameter;
    bool                                   m_isShowManeuverArrow;
    bool                                   m_waitTransitionEnd;
    shared_ptr<GPSInterpolator>            m_gpsInterpolator;
    bool                                   m_isShowCompass;

    /*! Avatar location, if this is not NULL, we should always use it to draw avatar. */
    shared_ptr<nbnav::Coordinates>         m_avatarLocation;


    int32                m_animationId;
    AnimationCallbackMap m_animationCallbacks;
    map<int, string>     m_animationDebugInfo; //@todo: Just for debugging, should be removed.

    // @KLUDGE@: ZoomToAll and UpdateMapSetting are called multiple times during state
    //           transition, we should fix it.
    MapSetting m_currentMapSetting;
    uint32     m_animationCallbackCounter;
    shared_ptr<NKUICameraPosition> m_expectedCameraPostion;
};
}

#endif /* defined(MapPresenter__) */

/*! @} */

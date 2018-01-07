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
    @file     NavAnalytics.h
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __NANANALYTICS_H__
#define __NANANALYTICS_H__

#include <vector>
#include "smartpointer.h"

extern "C"
{
#include "nbcontext.h"
#include "nbrouteinformation.h"
#include "palclock.h"
}

namespace protocol
{
class AnalyticsEventMetadata;
class InvocationContext;
class AnalyticsEvent;
class AnalyticsEventPlace;
}
namespace nbnav
{

typedef enum
{
    TAT_OverAll = 0,
    TAT_Approaching_Congestion,
    TAT_Approaching_Incident,
    TAT_In_Congestion
} TrafficAnnouncementType;

typedef enum
{
    TC_Incident = 0,
    TC_Green,
    TC_Yellow,
    TC_Red,
} TrafficColor;
/*! NavAnalytics

   Internal implement of analytics, it will used in NavigationImpl.
 */
class NavAnalytics
{
public:
    NavAnalytics();

    /*! Initialize the NanAnlytics.

        it must be invoked before call any other function of this class.

        @param palInstance PAL instance, must be created by client and pass here.
        @param nbContext NB context, must be created by client and pass here.
        @param isValid shared bool value, make sure that navigation instance is valid.
        @return NB_Error.
    */
    NB_Error Initialize(PAL_Instance* palInstance, NB_Context* nbContext, shared_ptr<bool> isValid);

    /*! start the analytics session or end it.

        @param isStart - true to start the session otherwise to end it.
        @return none.
     */
    void StartAnalyticsSession(bool isStart);

    /*! Add route request event to analytics.

        @param reason Reason of route request, follow as "init, off-route, wrong-way, detour".
        @param navSessionId Navigation session Id.
        @param return NB_Error.
    */
    NB_Error AddRouteRequestEvent(const char* reason, uint32 navSessionId);


    /*! Add route reply event to analytics.

        @param navSessionId Navigation session Id.
        @param duration Trip remain time.
        @param distance Trip remain distance.
        @return NB_Error.
    */
    NB_Error AddRouteReplyEvent(uint32 navSessionId, uint32 duration, double distance);

    /*! Add route state event to analytics.

        @param navSessionId Navigation session Id.
        @param state State of navigation, follos as "new-route, recalc, pause-route, resume-route, stop".
        @param duration Trip remain time.
        @param distance Trip remain distance.
        @param distanceTraveled Traveled distance of current navigation session.
        @param location Current location
        @return NB_Error.
    */
    NB_Error AddRouteStateEvent(uint32 navSessionId, const char* state, uint32 duration,
                                double distance, double distanceTraveled, NB_GpsLocation* location);

    /*! Add arrival event to analytics.

        @param navSessionId Navigation session Id.
        @return NB_Error.
    */
    NB_Error AddArrivalEvent(uint32 navSessionId);

    /*! Add Gps probes event to analytics.

     @param navSessionId Navigation session Id.
     @param location     current location
     @return NB_Error.
     */
    NB_Error AddGpsProbesEvent(uint32 navSessionId, NB_GpsLocation& location);

    /*! Add Traffic announcement event to analytics.

        @param navSessionId Navigation session Id.
        @param announcement Announcement made to the user.
        @param distance Routing distance in meters away from destination.
        @param duration Travel time along route remaining (in seconds).
        @param type Traffic announcement type.
        @param color Traffic color
        @param location Current location
        @return NB_Error.
     */
    NB_Error AddAnnouncementEvent(uint32 navSessionId, string announcement, float distance,
                                  uint32 duration, TrafficAnnouncementType type, TrafficColor color,
                                  NB_GpsLocation* location);
private:

    /*!------------------private function-------------------------*/
    NB_Error AddEvent(const char* name, shared_ptr<protocol::AnalyticsEvent> event, const char* routeState);
    void ResetAnalyticsEventPlace();
    /*! if the gps location is too soon or not

        @param location current location
        @return ture if too soon, else return false.
     */
    nb_boolean NB_AnalyticsIsGpsProbeTooSoon(const NB_GpsLocation& location);

    /*! create analytics event metadata.

        @param name     Event name.

        @return shared pointer to analyticsEventmetadata object.
     */
    shared_ptr<protocol::AnalyticsEventMetadata> CreateMetaDate(const char* name, const char* routeState);

    /*! create analytics event invocation.

        @param sourceModule Module in which the event was triggered .
        @param screenId     Screen in which the query was triggered.
        @param inputSource  Data source used for query parameters.
        @param invocationMethod  Specifies how the request was initiated.

        @return shared pointer to InvocationContext object.
     */
    shared_ptr<protocol::InvocationContext> CreateInvocation(const char* sourceModule,
                                                             const char* screenId,
                                                             const char* inputSource,
                                                             const char* invocationMethod);

    /*!------------------private member--------------------------*/
    PAL_Instance*           m_palInstance;
    NB_Context*             m_context;
    uint32                  m_requestRouteID;
    uint32                  m_replyRouteID;
    nb_boolean              m_isLastGpsProbeTimeValid;
    nb_gpsTime              m_lastGpsProbeTime;
    shared_ptr<protocol::AnalyticsEventPlace>     m_defaultPlace;
};
}
#endif

/*! @} */
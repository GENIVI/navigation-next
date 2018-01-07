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
    @file     Detour.h
*/
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __DETOUR_H__
#define __DETOUR_H__

#include "NavApiRouteInformation.h"
extern "C"
{
#include "nbcontext.h"
#include "nbnavigation.h"
#include "nbroutehandler.h"
#include "pallock.h"
}
#include "smartpointer.h"
#include <list>

namespace nbnav
{

using namespace std;

/*! Internal implementation of navigation session.

    It is used to hide all internal specific from the public header.
 */
class NavigationImpl;

/*! Detour.*/
class Detour
{
public:
    Detour(NavigationImpl* _impl);
    ~Detour();

    /*! Apply detour

        @param route RouteInformation
        @return true if apply else not
    */
    bool ApplyDetour(const RouteInformation& route);

    /*! Get if detour is requesting

        @return true if requesting else not
     */
    bool IsRequesting();

    static const int detourOffset = 100;

private:
    static void Static_CallbackDetourRoute(void* handler, NB_RequestStatus status,
                                           NB_Error err, uint8 up, int percent, void* pUserData);
    NB_Error DetourRouteParameters();
    NB_Error RequestDetour(NB_DetourParameters* detourParameters);
    NB_Error UpdateDetour();
    NB_Error ClearDetour(bool noDestroy);
    NB_Error AcceptDetour(const RouteInformation& route);
    void NotifyError(NavigateRouteError error);

    Detour(const Detour& info) {}
    void operator=( const Detour& info) {}

    NavigationImpl*        m_impl;
    bool                   m_requestingDetour;
    NB_RouteHandler*       m_routeHandler;
    NB_RouteInformation*   m_route;
    NB_TrafficInformation* m_trafficInfo;
    NB_CameraInformation*  m_cameraInfo;
    bool                   m_destorying;
};

}

#endif

/*! @} */

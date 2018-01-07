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
    @file navigationimpl.cpp
    @date 10/08/2014
    @addtogroup navigationkit
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
---------------------------------------------------------------------------*/

#include "navigationimpl.h"
#include "NavApiTypes.h"
#include "ltkcontextimpl.h"
#include "navigationlocation.h"
namespace locationtoolkit
{

Navigation* Navigation::CreateNavigation( LTKContext& ltkContext,
                                    const Place& destination,
                                    const RouteOptions& routeOptions,
                                    const Preferences& preferences)
{
    NavigationImpl* navigationImpl = new NavigationImpl();
    QVector<WayPoint> wayPoints;
    if( !navigationImpl->InitializeNavigation(ltkContext,destination,routeOptions,preferences, wayPoints) )
    {
        //init NavigationImpl failed
        delete navigationImpl;
        return NULL;
    }
    return navigationImpl;
}

Navigation* Navigation::CreateNavigation(LTKContext &ltkContext,
                                         const Place &destination,
                                         const RouteOptions &routeOptions,
                                         const Preferences &preferences,
                                         const QVector<WayPoint>& wayPoints)
{
    NavigationImpl* navigationImpl = new NavigationImpl();
    if( !navigationImpl->InitializeNavigation(ltkContext,destination,routeOptions,preferences, wayPoints) )
    {
        //init NavigationImpl failed
        delete navigationImpl;
        return NULL;
    }
    return navigationImpl;
}

Navigation* Navigation::CreateNavigationWithWorkFoler(LTKContext& ltkContext,
                               const Place& destination,
                               const RouteOptions& routeOptions,
                               const Preferences& preferences,
                               const QString& workFolder)
{
    NavigationImpl* navigationImpl = new NavigationImpl();
    QVector<WayPoint> wayPoints;
    if( !navigationImpl->InitializeNavigation(ltkContext,destination,routeOptions,preferences,wayPoints,workFolder) )
    {
        //init NavigationImpl failed
        delete navigationImpl;
        return NULL;
    }
    return navigationImpl;
}

NavigationImpl::NavigationImpl()
    : mRouteOptions(RT_Fastest,TM_Car,RA_Unpaved),
      mbNavigaitonStart(false),
      mbNavigaitonPaused(false)
{
    mNbNavNavigation = NULL;
}

NavigationImpl::~NavigationImpl()
{
    RemoveAllListeners();
    delete mNbNavNavigation;
}

bool NavigationImpl::InitializeNavigation( LTKContext& ltkContext, const Place& destination,
                                           const RouteOptions& routeOptions, const Preferences& preferences,
                                           const QVector<WayPoint>& wayPoints, const QString& workFolder)
{
    mDestPlace = destination;
    mRouteOptions = routeOptions;
    mPreferences = preferences;
    mbNavigaitonPaused = false;
    LTKContextImpl& contextImpl = static_cast<LTKContextImpl &>( ltkContext );
    NB_Context* pNbContext = contextImpl.GetNBContext();
    nbnav::Place nbDest;
    nbnav::RouteOptions nbRouteOptions(nbnav::Fastest,nbnav::Car,nbnav::AVD_Unpaved) ;
    nbnav::Preferences nbPreferences;
    TypeTranslater::Translate(destination,nbDest);
    TypeTranslater::Translate(routeOptions,nbRouteOptions);
    TypeTranslater::Translate(preferences,nbPreferences);
    if(workFolder.isEmpty())
    {
        mNbNavNavigation = nbnav::Navigation::GetNavigation(pNbContext, nbDest,nbRouteOptions, nbPreferences);
    }
    else
    {
        std::vector<nbnav::WayPoint> nbWayPoints;
        for(int i = 0; i < wayPoints.size(); i++)
        {
            nbnav::WayPoint nbWayPoint;
            nbWayPoint.isStopPoint = wayPoints[i].isStopPoint;
            TypeTranslater::Translate(wayPoints[i].place, nbWayPoint.location);
            nbWayPoints.push_back(nbWayPoint);
        }
        mNbNavNavigation = nbnav::Navigation::GetNavigationWithWorkFoler(pNbContext, nbDest,nbRouteOptions, nbPreferences,workFolder.toStdString(), nbWayPoints);
    }
    if (mNbNavNavigation)
    {
        mNbNavNavigation->AddSessionListener(&mSessionHelperListener);
        mNbNavNavigation->AddRoutePositionListener(&mRoutePositionHelperListener);
        mNbNavNavigation->AddTrafficListener(&mTrafficHelperListener);
        mNbNavNavigation->AddAnnouncementListener(&mAnnouncementHelperListener);
        mNbNavNavigation->AddSpecialRegionListener(&mSpecialRegionHelperListener);
        mNbNavNavigation->AddEnhancedNavigationStartupListener(&mEnhancedNavigationStartupListener);
    }
    else
    {
        return false;
    }
    mbNavigaitonStart = true;
    return true;
}


void NavigationImpl::UpdatePosition(const Location& location)
 {
    NavigationLocation navLocation(location);
    mNbNavNavigation->UpdatePosition(navLocation);
 }

void NavigationImpl::PauseSession()
{
    //protect ,already paused
    if( mbNavigaitonPaused || !mbNavigaitonStart )
    {
        return;
    }
    mNbNavNavigation->PauseSession();
    mbNavigaitonPaused = true;
}

void NavigationImpl::ResumeSession()
{
    //protect ,only paused  can resume
    if( mbNavigaitonPaused && mbNavigaitonStart )
    {
        mNbNavNavigation->ResumeSession();
        mbNavigaitonPaused = false;
    }
}

bool NavigationImpl::IsPaused()const
{
    return mbNavigaitonPaused;
}

void NavigationImpl::StopSession()
{
    //protect, no double stop
    if( mbNavigaitonStart )
    {
        RemoveAllListeners();
        mNbNavNavigation->StopSession();
        mbNavigaitonPaused = false;
        mbNavigaitonStart = false;
    }
}

void NavigationImpl::Announce()
{
    mNbNavNavigation->Announce();
}


void NavigationImpl::Announce(qint32 manueverNumber)
{
    mNbNavNavigation->Announce(manueverNumber);
}

void NavigationImpl::Announce(qint32 manueverNumber, const RouteInformation& route)
{
    std::string routeID = route.GetRouteID().toStdString();
    const nbnav::RouteInformation* routeInfo = mSessionHelperListener.GetNbNavRouteInfomation(routeID);
    if( routeInfo != NULL)
    {
        mNbNavNavigation->Announce( (int)manueverNumber, *routeInfo );
    }
}

void NavigationImpl::DoDetour()
{
    mNbNavNavigation->DoDetour();
}

void NavigationImpl::CancelDetour()
{
    mNbNavNavigation->CancelDetour();
}

bool NavigationImpl::SetActiveRoute(const RouteInformation& route)const
{
    std::string routeID = route.GetRouteID().toStdString();
    const nbnav::RouteInformation* routeInfo = mSessionHelperListener.GetNbNavRouteInfomation(routeID);
    bool bRet = false;
    if( routeInfo != NULL )
    {
        bRet = mNbNavNavigation->SetActiveRoute( *routeInfo );
    }

    return bRet;

}

void NavigationImpl::Recalculate()
{
    mNbNavNavigation->Recalculate();
}

void NavigationImpl::Recalculate(const RouteOptions& routeOptions,const Preferences& preference, bool wantAlternateRoute)
{
    nbnav::RouteOptions nbRouteOption(nbnav::Fastest,nbnav::Car,nbnav::AVD_Unpaved);
    TypeTranslater::Translate(routeOptions,nbRouteOption);
    nbnav::Preferences nbPreferences;
    TypeTranslater::Translate(preference,nbPreferences);
    mNbNavNavigation->Recalculate(nbRouteOption,nbPreferences,(bool)wantAlternateRoute);
}

const NavigationUpdateSignals& NavigationImpl::GetNavigationUpdateSignals() const
{
    return mRoutePositionHelperListener.GetNavigationUpdateSignals();
}


const SessionSignals& NavigationImpl::GetSessionSignals() const
{
    return mSessionHelperListener.GetSignals();
}

const TrafficSignals& NavigationImpl::GetTrafficSignals() const
{
    return mTrafficHelperListener.GetSignals();
}

const SpecialRegionSignals& NavigationImpl::GetSpecialRegionSignals() const
{
    return mSpecialRegionHelperListener.GetSignals();
}

const AnnouncementSignals& NavigationImpl::GetAnnouncementSignals() const
{
    return mAnnouncementHelperListener.GetSignals();
}

const EnhancedNavigationStartupSignals& NavigationImpl::GetEnhancedNavigationStartupSignals() const
{
    return mEnhancedNavigationStartupListener.GetSignals();
}

void NavigationImpl::RemoveAllListeners()
{
    if (mNbNavNavigation)
    {
        mNbNavNavigation->RemoveSessionListener(&mSessionHelperListener);
        mNbNavNavigation->RemoveRoutePositionListener(&mRoutePositionHelperListener);
        mNbNavNavigation->RemoveTrafficListener(&mTrafficHelperListener);
        mNbNavNavigation->RemoveAnnouncementListener(&mAnnouncementHelperListener);
        mNbNavNavigation->RemoveSpecialRegionListener(&mSpecialRegionHelperListener);
        mNbNavNavigation->RemoveEnhancedNavigationStartupListener(&mEnhancedNavigationStartupListener);
    }
}

}

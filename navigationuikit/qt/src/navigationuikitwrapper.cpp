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
    @file navigationuikitwrapper.cpp
    @date 10/15/2014
    @addtogroup navigationuikit
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

#include "private/navigationuikitwrapper.h"
#include "navigationerrormessagebox.h"
#include <QFontDatabase>

namespace locationtoolkit
{
NavigationUIError NkuiErrorToNavigationError( NKUI_Error errorCode )
{
    NavigationUIError result = EC_None;
    if( errorCode <= NUE_PedRouteTooLong )
    {
        result = (NavigationUIError)( errorCode - NUE_None );
    }
    else
    {
        switch(errorCode)
        {
        case NUE_OriginCountryUnsuppoted:
            result = EC_OriginCountryUnsuppoted;
            break;
        case NUE_DestinationCountryUnsupported:
            result = EC_DestinationCountryUnsupported;
            break;
        case NUE_Unauthorized:
            result = EC_Unauthorized;
            break;
        case NUE_BicycleRouteTooLong:
            result = EC_BicycleRouteTooLong;
            break;
        case NUE_RouteTooLong:
            result = EC_RouteTooLong;
            break;
        case NUE_GPSTimeout:
            result = EC_GPSTimeout;
            break;
        case NUE_Cancelled:
            result = EC_Cancelled;
            break;
        case NUE_Internal:
            result = EC_Internal;
            break;
        case NUE_ParseLayoutConfigFileError:
            result = EC_ParseLayoutConfigFileError;
            break;
        }
    }

    return result;
}

QString GetErrorDescription( NKUI_Error errorCode )
{
    QString result;

    switch (errorCode) {
    case NUE_TimeOut:
    case NUE_ServerError:
    case NUE_NetError:
    case NUE_UnknownError:
        result = QObject::tr( "We are having trouble communicating with its servers. Please try again." );
        break;
    case NUE_BadDestination:
        result = QObject::tr( "Your chosen destination is not on or near a road and cannot provide directions to that location. Please change the destination and try again." );
        break;
    case NUE_BadOrigin:
        result = QObject::tr( "The beginning of your route is too far away from a road. Please proceed to the nearest road and try again." );
        break;
    case NUE_CannotRoute:
        result = QObject::tr( "We are having trouble routing you to your destination. Please try a different start or end point." );
        break;
    case NUE_EmptyRoute:
        result = QObject::tr( "The destination is too close to create a route. Please select another destination." );
        break;
    case NUE_NoMatch:
        result = QObject::tr( "The beginning of your route is too far away from a road. Please proceed to the nearest road and try again." );
        break;
    case NUE_NoDetour:
        result = QObject::tr( "We are unable to find a detour at this time." );
        break;
    case NUE_PedRouteTooLong:
        result = QObject::tr( "Pedestrian route is too long. Please choose a closer destination." );
        break;
    case NUE_Internal:
        result = QObject::tr( "Internal Error." );
        break;
    case NUE_Unauthorized:
        result = QObject::tr( "Your subscription has expired." );
        break;
    case NUE_BicycleRouteTooLong:
        result = QObject::tr( "Bicycle route is too long. Please choose a closer destination." );
        break;
    case NUE_RouteTooLong:
        result = QObject::tr( "Route is too long. Please choose a closer destination." );
        break;
    case NUE_GPSTimeout:
        result = QObject::tr( "We're having trouble finding your exact location..." );
        break;
    case NUE_ParseLayoutConfigFileError:
        result = QObject::tr( "Parse layout config file error" );
        break;
    default:
        break;
    }

    return result;
}

 NavigaitonUIKitWrapper* NavigaitonUIKitWrapper::uikitWrapperInstance = NULL;

NavigaitonUIKitWrapper::NavigaitonUIKitWrapper(const UserPreferences& prefs, const Place& destination, const Place& origin)
    :mNavigationUISignals(NULL)
    ,mParentView(prefs.parentView)
{
    mResourcePath = prefs.resourcePath.c_str();
    //1.1 mNkuiController preparation:get widgetManager,stringHelper object
    mLayoutConfig = new WidgetLayoutConfig(prefs.configFilePath, prefs.deviceType);
    NKUIWidgetManagerPtr widgetManager(new WidgetManager(prefs.parentView,mLayoutConfig));
    NKUIStringHelperPtr stringHelper( new LocalizedStringHelperImpl() );

    //1.2 mNkuiController preparation:get routeOption,preferences object
    nbnav::RouteOptions  routeOption(nbnav::Fastest,nbnav::Car,nbnav::HOV);
    TypeTranslater::Translate(prefs.navRouteOptions,routeOption);
    nbnav::Preferences  preferences;
    TypeTranslater::Translate(prefs.navPreferences,preferences);

    //1.3 mNkuiController preparation:get destination,origin object
    TypeTranslater::Translate(destination,mDestination);
    TypeTranslater::Translate(origin,mOrigin);
    mNavDestination.reset(&mDestination);
    mNavOrigin.reset(&mOrigin);

    //1.4 mNkuiController preparation:get mMapInterfaceWrapper object
    mMapInterfaceWrapper = new MapInterfaceWrapper();
    mMapInterfaceWrapper->SetMapInterface(prefs.mapInterface);

    //1.5 Finally, get mNkuiController object.
    LTKContextImpl* pImpl = dynamic_cast<LTKContextImpl *>( prefs.ltkContext );
    NB_Context* context = pImpl->GetNBContext();
    mNkuiController = new NKUIController(  widgetManager,
                                           mNavDestination,
                                           mNavOrigin,
                                           routeOption,
                                           preferences,
                                           context,
                                           mMapInterfaceWrapper,
                                           stringHelper,
                                           prefs.enableTurnByTurnNavigation,
                                           prefs.isPlanTrip);
    mNkuiController->AddNKUIListener(this);
    mNkuiController->SetWorkFolder(mResourcePath.toStdString());

    //2 Get mLocationListener object.
    mLocationListener = new LocationProviderListener(mNkuiController, prefs.ltkLocationProvider);

    //3 register events to navui
    mNkuiController->RegisterEvent(EVT_ERROR_OCCURRED, this);

    LoadNexGenFont();
}

NavigaitonUIKitWrapper::NavigaitonUIKitWrapper()
    :mNavigationUISignals(NULL)
    ,mParentView(NULL)
{

}

NavigaitonUIKitWrapper::~NavigaitonUIKitWrapper()
{
    delete mMapInterfaceWrapper;
    delete mNkuiController;
    delete mLocationListener;
    delete mLayoutConfig;
}

NavigaitonUIKitWrapper* NavigaitonUIKitWrapper::GetInstance(const UserPreferences& prefs, const Place& destination, const Place& origin)
{
    //uikitWrapperInstance will delete by caller.
    uikitWrapperInstance =  new NavigaitonUIKitWrapper(prefs, destination, origin);
    return uikitWrapperInstance;
}

QString& NavigaitonUIKitWrapper::GetResourcePath()
{
    return uikitWrapperInstance->mResourcePath;
}

const NavigationUISignals& NavigaitonUIKitWrapper::GetNavigationUISignals() const
{
    return mNavigationUISignals;
}

void NavigaitonUIKitWrapper::Recalculate(const NavUIRouteOptions& routeOption)
{
    nbnav::RouteOptions  nbRouteOption(nbnav::Fastest,nbnav::Car,nbnav::HOV);
    TypeTranslater::Translate(routeOption,nbRouteOption);
    mNkuiController->Recalculate(&nbRouteOption);
}

ShareInformation NavigaitonUIKitWrapper::GetNavShareInformation()const
{
    nkui::NKUISharedInformation sharedInfo = mNkuiController->GetCurrentNavInformation();
    ShareInformation shareInformation;
    shareInformation.currentRouteName = QString::fromStdString(sharedInfo.m_currentRoadName);
    shareInformation.tripRemainingTime = sharedInfo.m_tripRemainingTime;
    shareInformation.inTrafficArea = sharedInfo.m_inTrafficArea;
    return shareInformation;
}

void NavigaitonUIKitWrapper::Start()
{
    mLocationListener->StartTracking();
    mNkuiController->Start();
}

void NavigaitonUIKitWrapper::Pause()
{
    mNkuiController->Pause();
}

void NavigaitonUIKitWrapper::Resume()
{
    mNkuiController->Resume();
}

void NavigaitonUIKitWrapper::Delete()
{
    mLocationListener->StopTracking();
    mNkuiController->Delete();
}

void NavigaitonUIKitWrapper::AddErrorHandler(NavigationUIErrorHandler* errorHandler)
{
    if (errorHandler)
    {
        mUIErrorHandlerSet.insert(errorHandler);
    }
}

void NavigaitonUIKitWrapper::RemoveErrorHandler(NavigationUIErrorHandler* errorHandler)
{
    if (errorHandler)
    {
        mUIErrorHandlerSet.erase(errorHandler);
    }
}

void NavigaitonUIKitWrapper::NotifyEvent(NKEventID event, NKUIEventData data)
{
    //For some special handling, we can register events in CTOR, then these events will be received in this function.
    NKUI_Error* pErrorCode = (NKUI_Error*)data.eventData;

    switch (event) {

    case EVT_ERROR_OCCURRED:
        {
            NavigationErrorMessageBox* msgbox =
                    new NavigationErrorMessageBox( mParentView, GetErrorDescription(*pErrorCode) );
            if( mParentView )
            {
                int posX = ( mParentView->width() - msgbox->width() ) / 2;
                int posY = ( mParentView->height() - msgbox->height() ) / 2;
                msgbox->move( posX, posY );
            }
            msgbox->show();
        }
        break;

    default:
        break;
    }
}

void NavigaitonUIKitWrapper::OnNavigationStart()
{
    emit mNavigationUISignals.OnNavigationStart();
}

void NavigaitonUIKitWrapper::OnRouteOptionsChanged()
{
    emit mNavigationUISignals.OnRouteOptionsChanged();
}

void NavigaitonUIKitWrapper::OnArrived()
{
    emit mNavigationUISignals.OnArrived();
}

void NavigaitonUIKitWrapper::OnNavigationEnd()
{
    emit mNavigationUISignals.OnNavigationEnd();
}
void NavigaitonUIKitWrapper::OnNavigationCancel()
{
    emit mNavigationUISignals.OnNavigationCancel();
}
bool NavigaitonUIKitWrapper::OnConfirmAction(NavigationUIAction action)
{
    return false;
}

void NavigaitonUIKitWrapper::OnTurnByTurnNavigationStart()
{
     emit mNavigationUISignals.OnTurnByTurnNavigationStart();
}

void NavigaitonUIKitWrapper::ShowOverflowMenu(NKUIPublicState state, std::vector<MenuOption>& items, bool show)
{
    //@todo add menuitem defination
}

void NavigaitonUIKitWrapper::OnNavigationStateChanged(NKUIPublicState state)
{
    PublicState publicState = (PublicState)state;
    emit mNavigationUISignals.OnNavigationStateChanged(publicState);
}

bool NavigaitonUIKitWrapper::OnError(NKUI_Error errorCode)
{
    NavigationUIError error = NkuiErrorToNavigationError( errorCode );
    std::set<NavigationUIErrorHandler*>::const_iterator iter = mUIErrorHandlerSet.begin();
    std::set<NavigationUIErrorHandler*>::const_iterator end  = mUIErrorHandlerSet.end();
    bool isHandled(false);
    bool  ret = false;
    for (; iter != end; ++iter)
    {
        isHandled = (*iter)->OnError(error);
        if( isHandled )
        {
            ret = true;
        }
    }
    return ret;
}

void NavigaitonUIKitWrapper::SetBackground(bool isBackground)
{

}

void NavigaitonUIKitWrapper::SetCallingState(bool isInCalling)
{

}

void NavigaitonUIKitWrapper::CancelEndTripTimer()
{

}

void NavigaitonUIKitWrapper::EndTrip()
{

}

bool NavigaitonUIKitWrapper::IsInCallingState()
{
    return false;
}

bool NavigaitonUIKitWrapper::LoadNexGenFont()
{
    QDir dir = QFileInfo(mResourcePath).dir();
    QString root = dir.absolutePath();
    QString fontFilePath = root + "/font/LG_NexGen.ttf";

    return LoadNexGenFont(fontFilePath);
}

bool NavigaitonUIKitWrapper::LoadNexGenFont(QString fontFilePath)
{
    QString fontName(fontFilePath.toLatin1().data());
    int appFontId = QFontDatabase::addApplicationFont(fontName);
    if(appFontId < 0)
    {
        return false;
    }

    return true;
}
}



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
    @file typetranslater.cpp
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
#include "typetranslater.h"

namespace locationtoolkit
{
void TypeTranslater::Translate(const MapLocation& ltkMapLocation, nbnav::MapLocation& nbMapLocation)
{
    nbMapLocation.airport = ltkMapLocation.airport.toStdString();
    nbMapLocation.freeform = ltkMapLocation.freeform.toStdString();
    nbMapLocation.number = ltkMapLocation.number.toStdString();
    nbMapLocation.street = ltkMapLocation.street.toStdString();
    nbMapLocation.street2 = ltkMapLocation.street2.toStdString();
    nbMapLocation.city = ltkMapLocation.city.toStdString();
    nbMapLocation.county = ltkMapLocation.county.toStdString();
    nbMapLocation.state = ltkMapLocation.state.toStdString();
    nbMapLocation.postal = ltkMapLocation.postal.toStdString();
    nbMapLocation.country = ltkMapLocation.country.toStdString();
    nbMapLocation.areaname = ltkMapLocation.areaname.toStdString();
    nbMapLocation.center.latitude = ltkMapLocation.center.latitude;
    nbMapLocation.center.longitude = ltkMapLocation.center.longitude;

}

void TypeTranslater::Translate(const Category& ltkCategory, nbnav::Category& nbCategory)
{
    nbCategory.code = ltkCategory.code.toStdString();
    nbCategory.name = ltkCategory.name.toStdString();
}

void TypeTranslater::Translate(const Phone& ltkPhone, nbnav::Phone& nbnavPhone)
{
    nbnavPhone.type = (nbnav::PhoneType)ltkPhone.type;
    nbnavPhone.country = ltkPhone.country.toStdString();
    nbnavPhone.area = ltkPhone.area.toStdString();
    nbnavPhone.number = ltkPhone.number.toStdString();
}

void TypeTranslater::Translate(const Place& ltkPlace, nbnav::Place& nbPlace)
{
    nbPlace.SetName(ltkPlace.GetName().toStdString());
    nbnav::MapLocation nbMapLocation;
    Translate(ltkPlace.GetLocation(),nbMapLocation);
    nbPlace.SetLocation(nbMapLocation);
    int nCategroyCount = ltkPlace.GetCategoriesCount();
    nbPlace.ClearCategories();
    for(int index =0 ; index < nCategroyCount; index++ )
    {
        Category ltkCat ;
        nbnav::Category nbCat;
        if( !ltkPlace.GetCategory(index,ltkCat) )
        {
            break;
        }
        Translate(ltkCat,nbCat);
        nbPlace.AddCategory(nbCat);
    }
    int nPhoneNumCount = ltkPlace.GetPhoneNumberCount();
    nbPlace.ClearPhoneNumbers();
    for(int index =0 ; index < nPhoneNumCount; index++ )
    {
        Phone ltkPhone ;
        nbnav::Phone nbnavPhone;
        if( !ltkPlace.GetPhoneNumber(index,ltkPhone) )
        {
            break;
        }
        Translate(ltkPhone,nbnavPhone);
        nbPlace.AddPhoneNumber(nbnavPhone);
    }

}

void TypeTranslater::Translate(const RouteOptions& ltkRouteOptions,nbnav::RouteOptions& nbRouteOptions)
{
    nbRouteOptions.SetAvoidFeatures(ltkRouteOptions.GetAvoidFeatures());
    nbRouteOptions.SetRouteType((nbnav::RouteType)ltkRouteOptions.GetRouteType());
    nbRouteOptions.SetTransportationMode((nbnav::TransportationMode)ltkRouteOptions.GetTransportationMode());
    nbRouteOptions.SetPronunciationStyle(ltkRouteOptions.GetPronunciationStyle().toStdString());
}
void TypeTranslater::Translate(const Preferences& ltkPreferences,nbnav::Preferences& nbPreferences)
{
    nbPreferences.SetRecalcCriteria( (int)ltkPreferences.GetRecalcCriteria() );
    nbPreferences.SetOffRouteIgnoreCount( (int)ltkPreferences.GetOffRouteIgnoreCount() );
    nbPreferences.SetMultipleRoutes( (bool)ltkPreferences.GetMultipleRoutesEnabled() );
    nbPreferences.SetRealisticSign( (bool)ltkPreferences.GetRealisticSignEnabled() );
    nbPreferences.SetNaturalGuidance( (bool)ltkPreferences.GetNaturalGuidanceEnabled() );
    nbPreferences.SetLaneGuidance( (bool)ltkPreferences.GetLaneGuidanceEnabled() );
    nbPreferences.SetDownloadableAudioEnabled( (bool)ltkPreferences.GetDownloadableAudioEnabled() );
    nbPreferences.SetPositionUpdateRate( ltkPreferences.GetPositionUpdateRate() );
    nbPreferences.SetSchoolZoneAlertEnabled( (bool)ltkPreferences.IsSchoolZoneAlertEnabled() );
    nbPreferences.SetSpeedLimitAlertEnabled( (bool)ltkPreferences.IsSpeedLimitAlertEnabled() );
    nbPreferences.SetSpeedLimitSignEnabled( (bool)ltkPreferences.IsSpeedLimitSignEnabled() );
    nbPreferences.SetMeasurement( (nbnav::Preferences::Measurement)ltkPreferences.GetMeasurement() );
    nbPreferences.SetEnableBridgeManeuvers( (bool)ltkPreferences.IsBridgeManeuversEnabled() );
    nbPreferences.SetTrafficAnnouncements( (bool)ltkPreferences.IsTrafficAnnouncements() );
    nbPreferences.SetNeedNCManeuver( (bool)ltkPreferences.IsNeedNCManeuver() );
    nbPreferences.SetSpeedingWarningLevel( (double)ltkPreferences.GetSpeedingWarningLevel() );
    nbPreferences.SetOriginManeuverTurnAngleTolerance( (double)ltkPreferences.GetOriginManeuverTurnAngleTolerance() );
    nbPreferences.SetSecondaryNameForExitsEnabled( (bool)ltkPreferences.IsSecondaryNameForExits() );
    nbPreferences.SetCheckUnsupportedCountriesEnabled( (bool)ltkPreferences.IsCheckUnsupportedCountries() );
    nbPreferences.SetEnableEnhancedNavigationStartup( (bool)ltkPreferences.GetEnableEnhancedNavigationStartup() );
    nbPreferences.SetSpeedingDisplayAlertEnabled( (bool)ltkPreferences.GetSpeedingDisplayAlertEnabled() );
}

void TypeTranslater::Translate(const nbnav::Category& nbCategory, Category& ltkCategory)
{
    ltkCategory.code = QString::fromStdString(nbCategory.code);
    ltkCategory.name = QString::fromStdString(nbCategory.name);
}

void TypeTranslater::Translate(const nbnav::Phone& nbnavPhone, Phone& ltkPhone)
{
    ltkPhone.type = (PhoneType)nbnavPhone.type;
    ltkPhone.country = QString::fromStdString(nbnavPhone.country);
    ltkPhone.area = QString::fromStdString(nbnavPhone.area);
    ltkPhone.number = QString::fromStdString(nbnavPhone.number);
}

void TypeTranslater::Translate(const nbnav::MapLocation& nbMapLocation, MapLocation& ltkMapLocation)
{

    ltkMapLocation.airport = QString::fromStdString(nbMapLocation.airport);
    ltkMapLocation.freeform = QString::fromStdString(nbMapLocation.freeform);
    ltkMapLocation.number = QString::fromStdString(nbMapLocation.number);
    ltkMapLocation.street = QString::fromStdString(nbMapLocation.street);
    ltkMapLocation.street2 = QString::fromStdString(nbMapLocation.street2);
    ltkMapLocation.city = QString::fromStdString(nbMapLocation.city);
    ltkMapLocation.county = QString::fromStdString(nbMapLocation.county);
    ltkMapLocation.state = QString::fromStdString(nbMapLocation.state);
    ltkMapLocation.postal = QString::fromStdString(nbMapLocation.postal);
    ltkMapLocation.country = QString::fromStdString(nbMapLocation.country);
    ltkMapLocation.areaname = QString::fromStdString(nbMapLocation.areaname);
    ltkMapLocation.center.latitude = nbMapLocation.center.latitude;
    ltkMapLocation.center.longitude = nbMapLocation.center.longitude;
}

void TypeTranslater::Translate(const nbnav::Place& nbPlace, Place& ltkPlace)
{
    ltkPlace.SetName( QString::fromStdString(nbPlace.GetName()) );
    MapLocation ltkMapLocation;
    Translate(nbPlace.GetLocation(),ltkMapLocation);
    ltkPlace.SetLocation(ltkMapLocation);
    int nCategroyCount = nbPlace.GetCategoriesCount();
    ltkPlace.ClearCategories();
    for(int index =0 ; index < nCategroyCount; index++ )
    {
        Category ltkCat ;
        nbnav::Category nbCat;
        if( !nbPlace.GetCategory(index,nbCat) )
        {
            break;
        }
        Translate(nbCat,ltkCat);
        ltkPlace.AddCategory(ltkCat);
    }
    int nPhoneNumCount = nbPlace.GetPhoneNumberCount();
    ltkPlace.ClearPhoneNumbers();
    for(int index =0 ; index < nPhoneNumCount; index++ )
    {
        Phone ltkPhone ;
        nbnav::Phone nbnavPhone;
        if( !nbPlace.GetPhoneNumber(index,nbnavPhone) )
        {
            break;
        }
        Translate(nbnavPhone,ltkPhone);
        ltkPlace.AddPhoneNumber(ltkPhone);
    }
}

void TypeTranslater::Translate(nbnav::NavigateRouteError nbError, LTKError &ltkError)
{
    int errCode;
    if(nbError < nbnav::NRE_Unauthorized)
    {
        errCode = nbError;
    }
    else
    {
        switch(nbError)
        {
        case nbnav::NRE_Unauthorized:
            errCode = LTKError::LTK_NavigationErrorUnauthorized;
            break;
        case nbnav::NRE_BicycleRouteTooLong:
            errCode = LTKError::LTK_NavigationErrorBicycleRouteTooLong;
            break;
        case nbnav::NRE_RouteTooLong:
            errCode = LTKError::LTK_NavigationErrorRouteTooLong;
            break;
        case nbnav::NRE_GPSTimeout:
            errCode = LTKError::LTK_NavigationErrorGPSTimeout;
            break;
        case nbnav::NRE_Cancelled:
            errCode = LTKError::LTK_NavigationErrorCancelled;
            break;
        case nbnav::NRE_Internal:
            errCode = LTKError::LTK_NavigationErrorInternal;
            break;
        default:
            errCode = LTKError::LTK_NavigationErrorUnknownError;
        }
    }

    ltkError.set(errCode);
}
}

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
    @date 10/27/2014
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

void TypeTranslater::Translate(const NavUIRouteOptions& ltkRouteOptions,nbnav::RouteOptions& nbRouteOptions)
{
    nbRouteOptions.SetAvoidFeatures(ltkRouteOptions.GetAvoidFeatures());
    nbRouteOptions.SetRouteType((nbnav::RouteType)ltkRouteOptions.GetRouteType());
    nbRouteOptions.SetTransportationMode((nbnav::TransportationMode)ltkRouteOptions.GetTransportationMode());
    nbRouteOptions.SetPronunciationStyle(ltkRouteOptions.GetPronunciationStyle().toStdString());
}
void TypeTranslater::Translate(const NavUIPreferences& ltkPreferences,nbnav::Preferences& nbPreferences)
{
    nbPreferences.SetRecalcCriteria( (int)ltkPreferences.GetRecalcCriteria() );
    nbPreferences.SetOffRouteIgnoreCount( (int)ltkPreferences.GetOffRouteIgnoreCount() );
    nbPreferences.SetMultipleRoutes( (bool)ltkPreferences.GetMultipleRoutesEnabled() );
    nbPreferences.SetRealisticSign( (bool)ltkPreferences.GetRealisticSignEnabled() );
    nbPreferences.SetNaturalGuidance( (bool)ltkPreferences.GetNaturalGuidanceEnabled() );
    nbPreferences.SetLaneGuidance( (bool)ltkPreferences.GetLaneGuidanceEnabled() );
    nbPreferences.SetDownloadableAudioEnabled( (bool)ltkPreferences.GetDownloadableAudioEnabled() );
    nbPreferences.SetPositionUpdateRate( (int)ltkPreferences.GetPositionUpdateRate() );
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
    nbPreferences.SetMockGpsEnabled(ltkPreferences.GetMockGpsEnabled() == true);
    nbPreferences.SetMockGpsSpeed(ltkPreferences.GetMockGpsSpeed());
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

void TypeTranslater::Translate(const nkui::NKUICameraPosition& nkuiCameraPosition, LTKNKUICameraPosition& ltkCameraPosition)
{
    ltkCameraPosition.coordinate.latitude = nkuiCameraPosition.m_coordinate.m_latitude;
    ltkCameraPosition.coordinate.longitude = nkuiCameraPosition.m_coordinate.m_longitude;
    ltkCameraPosition.zoomLevel = nkuiCameraPosition.m_zoomLevel;
    ltkCameraPosition.headingAngle = nkuiCameraPosition.m_headingAngle;
    ltkCameraPosition.tiltAngle = nkuiCameraPosition.m_tiltAngle;
}

void TypeTranslater::Translate(const LTKNKUICameraPosition& ltkCameraPosition,nkui::NKUICameraPosition& nkuiCameraPosition)
{
    nkuiCameraPosition.m_coordinate.m_latitude = ltkCameraPosition.coordinate.latitude;
    nkuiCameraPosition.m_coordinate.m_longitude = ltkCameraPosition.coordinate.longitude;
    nkuiCameraPosition.m_zoomLevel = ltkCameraPosition.zoomLevel;
    nkuiCameraPosition.m_headingAngle = ltkCameraPosition.headingAngle;
    nkuiCameraPosition.m_tiltAngle = ltkCameraPosition.tiltAngle;
}

void TypeTranslater::Translate(const nkui::NKUIAnimationParameters& nkuiAnimation, LTKNKUIAnimationParameters& ltkAnimation)
{
    ltkAnimation.accelerationType = (AnimationAccelerationType)nkuiAnimation.m_accelerationType;
    ltkAnimation.duration = nkuiAnimation.m_durationTime;
    ltkAnimation.animationId = nkuiAnimation.m_id;
}

void TypeTranslater::Translate(const nkui::NKUICoordinateBounds& nkuiCoordinateBounds, LTKNKUICoordinateBounds& ltkCoordinateBounds)
{
    ltkCoordinateBounds.northEast.latitude = nkuiCoordinateBounds.northEast.m_latitude;
    ltkCoordinateBounds.northEast.longitude = nkuiCoordinateBounds.northEast.m_longitude;
    ltkCoordinateBounds.southWest.latitude = nkuiCoordinateBounds.southWest.m_latitude;
    ltkCoordinateBounds.southWest.longitude = nkuiCoordinateBounds.southWest.m_longitude;
}

void TypeTranslater::Translate(const nkui::NKUILocation& nkuiLocation, Location& ltkLocation)
{
    ltkLocation.status = 0;
    ltkLocation.gpsTime = nkuiLocation.GpsTime();
    ltkLocation.valid = nkuiLocation.Valid();
    ltkLocation.latitude = nkuiLocation.m_latitude;
    ltkLocation.longitude = nkuiLocation.m_longitude;
    ltkLocation.heading = nkuiLocation.m_heading;
    ltkLocation.horizontalVelocity = nkuiLocation.m_horizontalVelocity;
    ltkLocation.altitude = nkuiLocation.Altitude();
    ltkLocation.verticalVelocity = nkuiLocation.VerticalVelocity();
    ltkLocation.horizontalUncertaintyAngleOfAxis = nkuiLocation.HorizontalUncertaintyAngleOfAxis();
    ltkLocation.horizontalUncertaintyAlongAxis = nkuiLocation.HorizontalUncertaintyAlongAxis();
    ltkLocation.horizontalUncertaintyAlongPerpendicular = nkuiLocation.HorizontalUncertaintyAlongPerpendicular();
    ltkLocation.verticalUncertainty = nkuiLocation.VerticalUncertainty();
    ltkLocation.utcOffset = nkuiLocation.UtcOffset();
    ltkLocation.numberOfSatellites = nkuiLocation.NumberOfSatellites();
}

void TypeTranslater::Translate(const nkui::NKUIPolylineParameter& nkuiPolylineParameter, LTKNKUIPolylineParameter& ltkPolyLineParameter)
{
    std::vector<nkui::NKUICoordinate>::iterator iterCoordinates = nkuiPolylineParameter.points->begin();
    nkui::NKUICoordinate nkuiCoordinate;
    Coordinates ltkCoordinate;
    while (iterCoordinates != nkuiPolylineParameter.points->end())
    {
        nkuiCoordinate = *iterCoordinates;
        ltkCoordinate.latitude = nkuiCoordinate.m_latitude;
        ltkCoordinate.longitude = nkuiCoordinate.m_longitude;
        ltkPolyLineParameter.points.push_back(ltkCoordinate);
        iterCoordinates++;
    }

    std::vector<nkui::NKUIColorSegmentAttribute>::iterator iterColorSegments = nkuiPolylineParameter.colorSegments->begin();
    nkui::NKUIColorSegmentAttribute nkuiColorSegment(0,nkui::TC_Transparent);
    NavTrafficColor color;
    LTKNKUIColorSegmentAttribute ltkColorSegment = {0, TC_Transparent};
    while (iterColorSegments != nkuiPolylineParameter.colorSegments->end())
    {
        nkuiColorSegment = *iterColorSegments;
        switch( nkuiColorSegment.m_trafficColor )
        {
        case nkui::TC_Transparent :
            color = TC_Transparent;
        break;
        case nkui::TC_Red:
            color = TC_Red;
        break;
        case nkui::TC_Yellow:
            color = TC_Yellow;
        break;
        case nkui::TC_Green:
            color = TC_Green;
        break;
        default:
        break;
        }

        ltkColorSegment.trafficColor = color;
        ltkColorSegment.endIndex = nkuiColorSegment.m_endIndex;
        ltkPolyLineParameter.colorSegments.push_back(ltkColorSegment);
        iterColorSegments++;
    }

    ltkPolyLineParameter.width = nkuiPolylineParameter.width;

}

void TypeTranslater::Translate(const nkui::NKUIManeuverArrowParameter& nkuiManeuverArrowParameter, LTKNKUIManeuverArrowParameter& ltkManeuverArrowParameter)
{
    std::vector<nkui::NKUICoordinate>::const_iterator iterCoordinates = nkuiManeuverArrowParameter.points.begin();
    nkui::NKUICoordinate nkuiCoordinate;
    Coordinates ltkCoordinate;
    while (iterCoordinates != nkuiManeuverArrowParameter.points.end())
    {
        nkuiCoordinate = *iterCoordinates;
        TypeTranslater::Translate( nkuiCoordinate,ltkCoordinate );
        ltkManeuverArrowParameter.points.push_back(ltkCoordinate);
        iterCoordinates++;
    }
    ltkManeuverArrowParameter.maneuverwidth = (qint32)nkuiManeuverArrowParameter.maneuverwidth;
    ltkManeuverArrowParameter.arrowWidth    = (qint32)nkuiManeuverArrowParameter.arrowWidth;
    ltkManeuverArrowParameter.arrowLength   = (qint32)nkuiManeuverArrowParameter.arrowLength;
}

void TypeTranslater::Translate(const nkui::NKUIPinParameter& nkuiPinParameters, LTKNKUIPinParameter& ltkPinParameters)
{
    TypeTranslater::Translate( nkuiPinParameters.coordinate,ltkPinParameters.coordinate );
    ltkPinParameters.pinType = (PinType)nkuiPinParameters.pinType;
}

void TypeTranslater::Translate(const nkui::NKUIBreadCrumbParameter& nkuiBreadCrumbParameters, LTKNKUIBreadCrumbParameter& ltkBreadCrumbParameter)
{
    TypeTranslater::Translate(nkuiBreadCrumbParameters.m_center,ltkBreadCrumbParameter.center);
    ltkBreadCrumbParameter.heading = nkuiBreadCrumbParameters.m_heading;
    ltkBreadCrumbParameter.visible  = nkuiBreadCrumbParameters.m_visible;
    ltkBreadCrumbParameter.type     = (BreadCrumbType)nkuiBreadCrumbParameters.m_type;
}

void TypeTranslater::Translate(const nkui::NKUICoordinate& nkuiCoordinates, Coordinates& ltkCoordinates)
{
    ltkCoordinates.latitude = nkuiCoordinates.m_latitude;
    ltkCoordinates.longitude = nkuiCoordinates.m_longitude;
}

void TypeTranslater::Translate(const Coordinates& ltkCoordinates, nkui::NKUICoordinate& nkuiCoordinates)
{
    nkuiCoordinates.m_latitude = (double)ltkCoordinates.latitude;
    nkuiCoordinates.m_longitude = (double)ltkCoordinates.longitude;
}

}

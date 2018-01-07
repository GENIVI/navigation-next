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
    @file typetranslater.h
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
/*! @{ */
#ifndef LOCATIONTOOLKIT_LTK_UTILES_H
#define LOCATIONTOOLKIT_LTK_UTILES_H
#include "place.h"
#include "navuirouteoptions.h"
#include "navuipreferences.h"
#include "NavApiNavigation.h"
#include "NKUIMapInterface.h"
#include "navigationuimapinterface.h"

namespace locationtoolkit
{
class TypeTranslater
{
public:
    /**
     * Translate ltk Category to nb Category.
     * @param ltkCategory ltk Category type
     * @param nbCategory  nbnav::Category type
     * @return .
     */
    static void Translate(const Category& ltkCategory, nbnav::Category& nbCategory);
    /**
     * Translate ltk Phone to nb Phone.
     * @param ltkPhone ltk Phone type
     * @param nbPhone  nbnav::Phone type
     * @return .
     */
    static void Translate(const Phone& ltkPhone, nbnav::Phone& nbPhone);
    /**
     * Translate ltk MapLocation to nb MapLocation.
     * @param ltkMapLocation ltk MapLocation type
     * @param nbMapLocation  nbnav::MapLocation type
     * @return .
     */
    static void Translate(const MapLocation& ltkMapLocation, nbnav::MapLocation& nbMapLocation);
    /**
     * Translate ltkPlace to nb Place.
     * @param ltkPlace ltk Place type
     * @param nbPlace  nbnav::Place type
     * @return .
     */
    static void Translate(const Place& ltkPlace, nbnav::Place& nbPlace);
    /**
     * Translate ltk RouteOptions to nb RouteOptions.
     * @param ltkRouteOptions ltk RouteOptions type
     * @param nbRouteOptions  nbnav::RouteOptions type
     * @return .
     */
    static void Translate(const NavUIRouteOptions& ltkRouteOptions,nbnav::RouteOptions& nbRouteOptions);
    /**
     * Translate ltk Preferences to nb Preferences.
     * @param ltkPreferences ltk Preferences type
     * @param nbPreferences  nbnav::Preferences type
     * @return .
     */
    static void Translate(const NavUIPreferences& ltkPreferences,nbnav::Preferences& nbPreferences);

     /**
     * Translate nb Category to ltk Category.
     * @param nbCategory nb Category type
     * @param ltkCategory  ltk::Category type
     * @return .
     */
    static void Translate(const nbnav::Category& nbCategory, Category& ltkCategory);
    /**
     * Translate nb Phone to ltk Phone.
     * @param nbPhone nb Phone type
     * @param ltkPhone  ltk Phone type
     * @return .
     */
    static void Translate(const nbnav::Phone& nbPhone, Phone& ltkPhone);
    /**
     * Translate nb MapLocation to ltk MapLocation.
     * @param nbMapLocation nb MapLocation type
     * @param ltkMapLocation  MapLocation type
     * @return .
     */
    static void Translate(const nbnav::MapLocation& nbMapLocation, MapLocation& ltkMapLocation);

    /**
     * Translate nb Place to ltk Place.
     * @param nbPlace nb Place type
     * @param ltkPlace ltk Place type
     * @return .
     */
    static void Translate(const nbnav::Place& nbPlace, Place& ltkPlace);

    /**
    * Translate nkui CameraPosition to ltk CameraPosition.
    * @param nkuiCameraPosition nkui CameraPosition type
    * @param ltkCameraPosition ltk CameraPosition type
    * @return .
    */
    static void Translate(const nkui::NKUICameraPosition& nkuiCameraPosition, LTKNKUICameraPosition& ltkCameraPosition);

    /**
    * Translate ltk CameraPosition to nkui CameraPosition.
    * @param nkuiCameraPosition nkui CameraPosition type
    * @param ltkCameraPosition ltk CameraPosition type
    * @return .
    */
    static void Translate(const LTKNKUICameraPosition& ltkCameraPosition,nkui::NKUICameraPosition& nkuiCameraPosition);

    /**
    * Translate nkui AnimationParameter to ltk AnimationParameter.
    * @param nkuiAnimation nkui AnimationParameter type
    * @param ltkAnimation ltk AnimationParameter type
    * @return .
    */
    static void Translate(const nkui::NKUIAnimationParameters& nkuiAnimation, LTKNKUIAnimationParameters& ltkAnimation);

    /**
    * Translate nkui CoordinateBounds to ltk CoordinateBounds.
    * @param nkuiCoordinateBounds nkui CoordinateBounds type
    * @param ltkCoordinateBounds ltk CoordinateBounds type
    * @return .
    */
    static void Translate(const nkui::NKUICoordinateBounds& nkuiCoordinateBounds, LTKNKUICoordinateBounds& ltkCoordinateBounds);

    /**
    * Translate nkui location to ltk location.
    * @param nkuiLocation nkui location type
    * @param ltkLocation ltk location type
    * @return .
    */
    static void Translate(const nkui::NKUILocation& nkuiLocation, Location& ltkLocation);

    /**
    * Translate nkui PolylineParameter to ltk PolylineParameter.
    * @param nkuiPolylineParameter nkui PolylineParameter type
    * @param ltkPolyLineParameter ltk PolylineParameter type
    * @return .
    */
    static void Translate(const nkui::NKUIPolylineParameter& nkuiPolylineParameter, LTKNKUIPolylineParameter& ltkPolyLineParameter);

    /**
    * Translate nkui NKUIManeuverArrowParameter to ltk ManeuverArrowParameter.
    * @param maneuverArrowParameter nkui ManeuverArrowParameter type
    * @param ltkManeuverArrowParameter ltk ManeuverArrowParameter type
    * @return .
    */
    static void Translate(const nkui::NKUIManeuverArrowParameter& nkuiManeuverArrowParameter, LTKNKUIManeuverArrowParameter& ltkManeuverArrowParameter);

    /**
    * Translate nkui NKUIPinParameter to ltk PinParameter.
    * @param nkuiPinParameters nkui pinParameters type
    * @param ltkPinParameters ltk pinParameters type
    * @return .
    */
    static void Translate(const nkui::NKUIPinParameter& nkuiPinParameters, LTKNKUIPinParameter& ltkPinParameters);

    /**
    * Translate nkui NKUIBreadCrumbParameter to ltk BreadCrumbParameter.
    * @param nkuiBreadCrumbParameters nkui breadCrumbParameters type
    * @param ltkBreadCrumbParameter ltk breadCrumbParameters type
    * @return .
    */
    static void Translate(const nkui::NKUIBreadCrumbParameter& nkuiBreadCrumbParameters, LTKNKUIBreadCrumbParameter& ltkBreadCrumbParameter);

    /**
    * Translate nkui NKUICoordinate to ltk Coordinates.
    * @param nkuiCoordinates nkui coordinates type
    * @param ltkCoordinates ltk coordinates type
    * @return .
    */
    static void Translate(const nkui::NKUICoordinate& nkuiCoordinates, Coordinates& ltkCoordinates);

    /**
    * Translate ltk Coordinates to nkui NKUICoordinate.
    * @param nkuiCoordinates ltk coordinates type
    * @param coordinates nkui coordinates type
    * @return .
    */
    static void Translate(const Coordinates& ltkCoordinates, nkui::NKUICoordinate& nkuiCoordinates);
};
}
#endif //LOCATIONTOOLKIT_LTK_UTILES_H

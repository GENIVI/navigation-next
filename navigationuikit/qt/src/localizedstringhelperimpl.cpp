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
    @file localizedstringhelperimpl.cpp
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
#include "localizedstringhelperimpl.h"
#include <QDateTime>

namespace locationtoolkit
{
LocalizedStringHelperImpl::LocalizedStringHelperImpl()
{
    // @todo initialize here temporarily,
    //       but need to support intialization from resource files and multilanguage
    mLocalizedStringMap["IDS_DONE"] = "Done";

    mLocalizedStringMap["IDS_CANCEL"] = "Cancel";

    mLocalizedStringMap["IDS_MI"] = "mi";

    mLocalizedStringMap["IDS_FT"] = "ft";

    mLocalizedStringMap["IDS_KM"] = "km";

    mLocalizedStringMap["IDS_METER_SMALL"] = "m";

    mLocalizedStringMap["IDS_MPH"] = "mph";

    mLocalizedStringMap["IDS_KPH"] = "kph";

    mLocalizedStringMap["IDS_HRS"] = "hr";

    mLocalizedStringMap["IDS_H"] = "h";

    mLocalizedStringMap["IDS_MIN_SMALL"] = "m";

    mLocalizedStringMap["IDS_MIN"] = "min";

    mLocalizedStringMap["IDS_SYM_NONE"] = "--";

    mLocalizedStringMap["IDS_S"] = "S";

    mLocalizedStringMap["IDS_N"] = "N";

    mLocalizedStringMap["IDS_W"] = "W";

    mLocalizedStringMap["IDS_E"] = "E";

    mLocalizedStringMap["IDS_FAHRENHEIT"] = "°F";

    mLocalizedStringMap["IDS_C"] = "°C";

    mLocalizedStringMap["IDS_AT"] = "at";

    mLocalizedStringMap["IDS_ARRIVING_AT"] = "ARRIVING AT";

    mLocalizedStringMap["IDS_GETTING_ROUTE"] = "Getting Route...";

    mLocalizedStringMap["IDS_THIS_ROUTE_INC"] = "This route includes";

    mLocalizedStringMap["IDS_HIGHWAYS"] = "Highways";

    mLocalizedStringMap["IDS_TOLL_ROADS"] = "Toll Roads";

    mLocalizedStringMap["IDS_UNPAVED_ROADS"] = "Unpaved Roads";

    mLocalizedStringMap["IDS_FERRIES"] = "Ferries";

    mLocalizedStringMap["IDS_START"] = "Start";

    mLocalizedStringMap["IDS_STARTING_ON"] = "Starting on";

    mLocalizedStringMap["IDS_DRIVING_ON"] = "Driving on";

    mLocalizedStringMap["IDS_WALKING_ON"] = "Walking on";

    mLocalizedStringMap["IDS_OF"] = "of";

    mLocalizedStringMap["IDS_DELAY"] = "Delay";

    mLocalizedStringMap["IDS_TRAFFIC_CONGESTION_REPORTED"] = "Traffic congestion reported";

    mLocalizedStringMap["IDS_NAV_ETA"] = "ETA";

    mLocalizedStringMap["IDS_EXIT"] = "Exit";

    mLocalizedStringMap["IDS_RECALC"] = "Recalculating route...";

    mLocalizedStringMap["IDS_THEN"] = "THEN";

    mLocalizedStringMap["IDS_DIR_SEC_STARTUP"] = "Go to";

    mLocalizedStringMap["IDS_END_TRIP"] = "End Trip";

    mLocalizedStringMap["IDS_ARRIVAL_IN"] = "ARRIVING IN";

    mLocalizedStringMap["IDS_DESTINATION"] = "Destination";

    mLocalizedStringMap["IDS_IS_AHEAD"] = "is ahead";

    mLocalizedStringMap["IDS_IS_ON_THE_LEFT"] = "is on the left";

    mLocalizedStringMap["IDS_IS_ON_THE_RIGHT"] = "is on the right";

    mLocalizedStringMap["IDS_ROUTE_ERROR"] = "Failed to get route";

    mLocalizedStringMap["IDS_RETRY_IN_SECONDS"] = "Automatically retry in %1 seconds.";

    mLocalizedStringMap["IDS_RETRY"] = "Retry";

    mLocalizedStringMap["IDS_DETOUR"] = "Detour";

    mLocalizedStringMap["IDS_GETTING_DETOUR"] = "Getting detours...";

    mLocalizedStringMap["IDS_CURRENT"] = "Current";

    mLocalizedStringMap["IDS_HEAD_TOWARD"] = "Head Toward";

    mLocalizedStringMap["IDS_STARTING_NAVIGATION"] = "Starting navigation...";

    mLocalizedStringMap["IDS_VIA"] = "Via";

    mLocalizedStringMap["IDS_ROUTE"] = "Route";

    mLocalizedStringMap["IDS_NO_GPS"] = "No GPS";

    mLocalizedStringMap["IDS_ROUTE_OPTIONS"] = "Route Options";

    mLocalizedStringMap["IDS_CLOSE"] = "Close";

    mLocalizedStringMap["IDS_INTERNAL_ERROR"] = "Internal Error:";

    mLocalizedStringMap["IDS_GETTING_GPS"] = "Getting GPS...";

    mLocalizedStringMap["IDS_THE_HIGHLIGHTED_ROUTE"] = "The Highlighted Route";

    mLocalizedStringMap["IDS_PEDESTRIAN_ROUTE_IS_TOO_LONG"] = "Pedestrian route is too long. Please choose a closer destination.";

    mLocalizedStringMap["IDS_RECALCULATE_ROUTE"] = "Recalculate Route";

    mLocalizedStringMap["IDS_ARE_YOU_STILL_DRIVING"] = "Are you still driving?";

    mLocalizedStringMap["IDS_ARE_YOU_STILL_DRIVING_YOU_HAVE_NOT_MOVED"] = "Are you still driving? You haven't moved for 5 minutes. To help conserve battery life we will end your navigation session if you continue to stay still for the next 5 minutes.";

    mLocalizedStringMap["IDS_GETTING_LOCATION"] = "Getting location…";

    mLocalizedStringMap["IDS_ETA"] = "ETA";

    mLocalizedStringMap["IDS_ARRIVING_IN"] = "ARRIVING IN";

    mLocalizedStringMap["IDS_AND"] = "and";

    mLocalizedStringMap["IDS_RECALCULATING_ROUTE"] = "Recalculating route…";

    mLocalizedStringMap["IDS_UPDATING_ROUTE"] = "Updating route…";

    mLocalizedStringMap["IDS_GETTING_DETOURS"] = "Getting detours…";

    mLocalizedStringMap["IDS_RESUME"] = "Resume";

    mLocalizedStringMap["IDS_STARTING_ROUTE"] = "Starting route…";

    mLocalizedStringMap["IDS_THIS_ROUTE_INCLUDES"] = "This route includes";

    mLocalizedStringMap["IDS_HOVS"] = "HOVs";

    mLocalizedStringMap["IDS_INCIDENTS_REPORTED"] = "Incidents reported";

    mLocalizedStringMap["IDS_TOTAL_DELAY"] = "TOTAL DELAY";

    mLocalizedStringMap["IDS_WAITING_TO_UPDATE_YOUR_ROUTE"] = "Waiting to update your route.";

    mLocalizedStringMap["IDS_THERE_WAS_A_PROBLEM_GETTING"] = "There's a problem getting detours. Tap to try again.";

    mLocalizedStringMap["IDS_ERROR_FINDING_YOUR_LOCATION"] = "We're having trouble finding your exact location...";

    mLocalizedStringMap["IDS_YOU_ARE_CURRENTLY_OFF_ROUTE"] = "You are currently off-route.";

    mLocalizedStringMap["IDS_TAP_TO_RECALCULATE"] = "Tap to Recalculate";

    mLocalizedStringMap["IDS_M"] = "m";

    mLocalizedStringMap["IDS_YD"] = "yd";

    mLocalizedStringMap["IDS_SUNDAY"] = "Sunday";

    mLocalizedStringMap["IDS_MONDAY"] = "Monday";

    mLocalizedStringMap["IDS_TUESDAY"] = "Tuesday";

    mLocalizedStringMap["IDS_WEDNESDAY"] = "Wednesday";

    mLocalizedStringMap["IDS_THURSDAY"] = "Thursday";

    mLocalizedStringMap["IDS_FRIDAY"] = "Friday";

    mLocalizedStringMap["IDS_SATURDAY"] = "Saturday";

    mLocalizedStringMap["IDS_MON"] = "Mon";

    mLocalizedStringMap["IDS_TUE"] = "Tue";

    mLocalizedStringMap["IDS_WED"] = "Wed";

    mLocalizedStringMap["IDS_THU"] = "Thu";

    mLocalizedStringMap["IDS_FRI"] = "Fri";

    mLocalizedStringMap["IDS_SAT"] = "Sat";

    mLocalizedStringMap["IDS_SUN"] = "Sun";

    mLocalizedStringMap["IDS_MON_SMALL"] = "M";

    mLocalizedStringMap["IDS_TUE_SMALL"] = "Tu";

    mLocalizedStringMap["IDS_WED_SMALL"] = "W";

    mLocalizedStringMap["IDS_THU_SMALL"] = "Th";

    mLocalizedStringMap["IDS_SUN_SMALL"] = "Su";

    mLocalizedStringMap["IDS_SAT_SMALL"] = "Sa";

    mLocalizedStringMap["IDS_FRI_SMALL"] = "F";

    mLocalizedStringMap["IDS_JAN"] = "Jan";

    mLocalizedStringMap["IDS_FEB"] = "Feb";

    mLocalizedStringMap["IDS_MAR"] = "Mar";

    mLocalizedStringMap["IDS_APR"] = "Apr";

    mLocalizedStringMap["IDS_MAY"] = "May";

    mLocalizedStringMap["IDS_JUN"] = "Jun";

    mLocalizedStringMap["IDS_JUL"] = "Jul";

    mLocalizedStringMap["IDS_AUG"] = "Aug";

    mLocalizedStringMap["IDS_SEPT"] = "Sept";

    mLocalizedStringMap["IDS_OCT"] = "Oct";

    mLocalizedStringMap["IDS_NOV"] = "Nov";

    mLocalizedStringMap["IDS_DEC"] = "Dec";

    mLocalizedStringMap["IDS_JANUARY"] = "January";

    mLocalizedStringMap["IDS_FEBRUARY"] = "February";

    mLocalizedStringMap["IDS_MARCH"] = "March";

    mLocalizedStringMap["IDS_APRIL"] = "April";

    mLocalizedStringMap["IDS_JUNE"] = "June";

    mLocalizedStringMap["IDS_JULY"] = "July";

    mLocalizedStringMap["IDS_AUGUST"] = "August";

    mLocalizedStringMap["IDS_SEPTEMBER"] = "September";

    mLocalizedStringMap["IDS_OCTOBER"] = "October";

    mLocalizedStringMap["IDS_NOVEMBER"] = "November";

    mLocalizedStringMap["IDS_DECEMBER"] = "December";

    mLocalizedStringMap["IDS_ROUTE_IS_TOO_LONG"] = "Route is too long. Please choose a closer destination.";

    mLocalizedStringMap["IDS_BICYCLE_ROUTE_IS_TOO_LONG"] = "Bicycle route is too long. Please choose a closer destination.";

    mLocalizedStringMap["IDS_PROBLEM_GETTING_ROUTE"] = "Problem getting route. Please check your route options.";

    mLocalizedStringMap["IDS_ERROR"] = "Error";

    mLocalizedStringMap["IDS_NAVIGATION_ERROR"] = "Navigation Error";

    mLocalizedStringMap["IDS_LOCATION_ERROR"] = "Location Error";

    mLocalizedStringMap["IDS_PLEASE_RESTART_NAVIGATION"] = "Please restart navigation.";

    mLocalizedStringMap["IDS_REQUEST_GPS_TIME_OUT"] = "Request GPS time out. Please make sure your GPS setting is turned on.";

    mLocalizedStringMap["IDS_PLEASE_TURN_ON_YOUR_GPS"] = "Please turn on your GPS setting.";

    mLocalizedStringMap["IDS_PLEASE_TURN_ON_YOUR_LOCATION"] = "Please turn on your Location setting.";

    mLocalizedStringMap["IDS_NO_LOCATION_AVAILABLE"] = "No location available.";

    mLocalizedStringMap["IDS_BACK"] = "Back";

    mLocalizedStringMap["IDS_STOP_NAVIGATION"] = "Stop Navigation?";

    mLocalizedStringMap["IDS_YES"] = "Yes";

    mLocalizedStringMap["IDS_NO"] = "No";

    mLocalizedStringMap["IDS_ERROR_SERVER_COMMUNICATION"] = "We are having trouble communicating with its servers. Please try again.";

    mLocalizedStringMap["IDS_ERROR_DESTINATION_NOT_NEAR_A_ROAD"] = "Your chosen destination is not on or near a road and  cannot provide directions to that location. Please change the destination and try again.";

    mLocalizedStringMap["IDS_ERROR_INVALID_DESTINATION"] = "We are having trouble routing you to your destination. Please try a different start or end point.";

    mLocalizedStringMap["IDS_ERROR_EXPIRED_SUBSCRIPTION"] = "Your subscription has expired.";

    mLocalizedStringMap["IDS_ERROR_START_NOT_NEAR_A_ROAD"] = "The beginning of your route is too far away from a road. Please proceed to the nearest road and try again.";

    mLocalizedStringMap["IDS_HIGHLIGHTED_ROUTE"] = "Highlighted route";

    mLocalizedStringMap["IDS_REQUEST_CANCELLED"] = "Request cancelled";

    mLocalizedStringMap["IDS_PARSE_LAYOUT_CONFIG_FILE_ERROR"] = "Parse layout config file error";

    mLocalizedStringMap["IDS_INVALID"] = "Invalid";

    mLocalizedStringMap["IDS_ERROR_DESTINATION_TOO_CLOSE"] = "The destination is too close to create a route. Please select another destination.";

    mLocalizedStringMap["IDS_ERROR_NO_DETOUR"] = "We are unable to find a detour at this time.";

    mLocalizedStringMap["IDS_THIS_FEATURE_REQUIRES_AN_UPGRADE"] = "This feature requires an upgrade. Would you like to upgrade?";

    mLocalizedStringMap["IDS_CURRENT_LOCATION"] = "Current location";

    mLocalizedStringMap["IDS_CLICK_TO_RECALCULATE"] = "Click to recalculate";

    mLocalizedStringMap["IDS_FASTER"] = "Faster";

    mLocalizedStringMap["IDS_SLOWER"] = "Slower";

    mLocalizedStringMap["IDS_OVERVIEW"] = "Overview";

    mLocalizedStringMap["IDS_OPTIONS"] = "Options";

    mLocalizedStringMap["IDS_DO_YOU_STILL_NEED_NAVIGATION"] = "Do you still need Navigation?";

    mLocalizedStringMap["IDS_DO_YOU_STILL_NEED_NAVIGATION_YOU_HAVE_NOT_MOVED"] = "Do you still need Navigation? You haven't moved for 5 minutes. To help conserve battery life we will end your navigation session if you continue to stay still for the next 5 minutes.";

    mLocalizedStringMap["IDS_ORIGIN"] = "Origin";

    mLocalizedStringMap["IDS_PRESS_START_TO_NAVIGATE"] = "press start trip to navigate";
}

LocalizedStringHelperImpl::~LocalizedStringHelperImpl()
{

}

std::string LocalizedStringHelperImpl::GetLocalizedString(const std::string& stringID)
{
    std::map<std::string, std::string>::const_iterator it = mLocalizedStringMap.find(stringID);
    if(mLocalizedStringMap.end() != it)
    {
        return it->second;
    }

    return stringID;
}

std::string LocalizedStringHelperImpl::GetLocalizedTimeString(int time)
{
    QDateTime t;
    t.setTime_t(time);
    QString strTime = t.toString( "hh:mm AP" );
    return strTime.toStdString();
}

}  // namespace locationtoolkit

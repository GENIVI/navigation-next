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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "ManeuverImpl.h"

namespace nbnav
{
static void GuidanceInfoTextCb(NB_Font font, nb_color color, const char* txt, nb_boolean newline, void* pUser);
// class GuidanceTextData
class GuidanceTextData
{
public:
    GuidanceTextData(bool _isMetric, FormattedTextBlock* _textBlock);
    bool isMetric;
    FormattedTextBlock* textBlock;
};
    
    void GuidanceInfoTextCb(NB_Font font, nb_color color, const char* txt, nb_boolean newline, void* pUser)
    {
        if (pUser)
        {
            GuidanceTextData* pThis = static_cast<GuidanceTextData*>(pUser);
            FormattedText text;
            text.color = color;
            switch (font)
            {
                case NB_Font_Normal:
                {
                    text.font = Normal;
                    break;
                }
                case NB_Font_Bold:
                {
                    text.font = Bold;
                    break;
                }
                case NB_Font_Large:
                {
                    text.font = Large;
                    break;
                }
                case NB_Font_Large_Bold:
                {
                    text.font = LargeBold;
                    break;
                }
                case NB_Font_Custom1:
                case NB_Font_Custom2:
                case NB_Font_Custom3:
                case NB_Font_Custom4:
                default:
                    text.font = Normal;
                    break;
            }
            text.newline = (newline == TRUE);
            string str(txt);
            text.text = str;
            pThis->textBlock->AddFormattedText(text);
        }
    }

GuidanceTextData::GuidanceTextData(bool _isMetric, FormattedTextBlock* _textBlock)
: isMetric(_isMetric),
textBlock(_textBlock)
{
};

ManeuverImpl::ManeuverImpl(NavigationImpl* impl, unsigned int startIndex, unsigned int index,unsigned int routeIndex,
                           double redundantDistance, bool collapsed)
    : m_id(0),
    m_time(0.0),
    m_trafficDelay(0.0),
    m_distance(0.0),
    m_impl(impl),
    m_routeIndex(routeIndex),
    m_speed(0.0)
{
    NB_RouteInformation* routeInfo = impl->GetNbRoute(routeIndex);
    
    m_id = index;
    m_speed = 0.0;
    char primaryNameBuffer[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    char secondaryNameBuffer[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    char currentNameBuffer[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    char secondaryCurrentNameBuffer[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    char commandBuffer[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    char exitNumberBuffer[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    
    NB_LatitudeLongitude turnPoint;
    memset(&turnPoint, 0, sizeof(turnPoint));
    nb_boolean nbStackAdvise = FALSE;
    (void)NB_RouteInformationGetTurnInformation2(routeInfo, index, &turnPoint,
                                                 primaryNameBuffer, NB_NAVIGATION_STREET_NAME_MAX_LEN,
                                                 secondaryNameBuffer, NB_NAVIGATION_STREET_NAME_MAX_LEN,
                                                 currentNameBuffer, NB_NAVIGATION_STREET_NAME_MAX_LEN,
                                                 secondaryCurrentNameBuffer, NB_NAVIGATION_STREET_NAME_MAX_LEN,
                                                 &m_distance, &m_speed, &nbStackAdvise,
                                                 commandBuffer, NB_NAVIGATION_STREET_NAME_MAX_LEN,
                                                 collapsed);
    m_stackAdvise = (nbStackAdvise == TRUE);
    
    m_distance -= redundantDistance;
    if (m_speed != 0)
    {
        m_time = m_distance / m_speed;
    }
    
    m_point.latitude = turnPoint.latitude;
    m_point.longitude = turnPoint.longitude;
    
    m_primaryStreet.assign(primaryNameBuffer);
    m_secondaryStreet.assign(secondaryNameBuffer);
    m_command.assign(commandBuffer);
    
    // Get exit number
    NB_RouteInformationGetExitNumber(routeInfo, index, NULL, exitNumberBuffer, NB_NAVIGATION_STREET_NAME_MAX_LEN);
    m_exitNumber.assign(exitNumberBuffer);
    
    // Get the traffic delay
    uint32 delay = 0;
    NB_RouteInformationGetTrafficDelay(routeInfo, m_id, &delay);
    m_trafficDelay = delay;
    
    // Get polylines
    NB_MercatorPolyline* polylines = NULL;
    if (NE_OK == NB_RouteInformationGetRoutePolyline(routeInfo, startIndex, index+1, &polylines))
    {
        for (uint32 segIdx = 0; segIdx < (uint32)polylines->count; ++segIdx)
        {
            Coordinates coord = {0};
            NB_SpatialConvertMercatorToLatLong(polylines->points[segIdx].mx, polylines->points[segIdx].my,
                                               &coord.latitude, &coord.longitude);
            m_polyline.push_back(coord);
        }
        NB_MercatorPolylineDestroy(polylines);
    }
    
    // Get TTF char
    NB_CommandFont nextTurnCharacter = {0};
    NB_RouteInformationGetTurnCommandCharacter(routeInfo, impl->GetNbGuidanceInfo(), index,
                                               NB_NMP_Turn, NB_NAU_Miles, &nextTurnCharacter);
    m_ttfChar = (char*)(&nextTurnCharacter.character);
    NB_OnRouteInformation onRouteInformation;
    if(NB_RouteInformationGetManeuverPropertys(routeInfo, index, &onRouteInformation) == NE_OK)
    {
        if (onRouteInformation.isHovLanesOnRoute)
        {
            m_maneuverProperties.push_back(HOV);
        }
        if (onRouteInformation.isHighWayOnRoute)
        {
            m_maneuverProperties.push_back(HIGHWAY);
        }
        if (onRouteInformation.isTollsOnRoute)
        {
            m_maneuverProperties.push_back(TOLL_ROAD);
        }
        if (onRouteInformation.isFerryOnRoute)
        {
            m_maneuverProperties.push_back(FERRY);
        }
        if (onRouteInformation.isUnpavedOnRoute)
        {
            m_maneuverProperties.push_back(UNPAVED_ROAD);
        }
        if (onRouteInformation.isGatedAccessOnRoute)
        {
            m_maneuverProperties.push_back(GATED_ACCESS_ROAD);
        }
        if (onRouteInformation.isPrivateRoadOnRoute)
        {
            m_maneuverProperties.push_back(PRIVATE_ROAD);
        }
        if (onRouteInformation.isEnterCoutryOnRoute)
        {
            m_maneuverProperties.push_back(ENTER_COUNTRY);
        }
    }
}

FormattedTextBlock ManeuverImpl::GetManeuverText(bool isMetric) const
{
    NB_RouteInformation* routeInfo = m_impl->GetNbRoute(m_routeIndex);
    
    FormattedTextBlock maneuverText;
    
    GuidanceTextData* guidanceTextData = new GuidanceTextData(isMetric, &maneuverText);
    if (guidanceTextData)
    {
        (void)NB_RouteInformationEnumerateManeuverText(routeInfo, m_impl->GetNbGuidanceInfoForUI(),
                                                       NB_NTT_Primary, isMetric ? NB_NAU_Kilometers : NB_NAU_Miles,
                                                       m_id, GuidanceInfoTextCb, guidanceTextData);
        delete guidanceTextData;
    }
    
    return maneuverText;
}

string ManeuverImpl::GetDescription(bool isMetric, bool isFormatted) const
{
    string maneuverDescription = "";
    
    NB_RouteInformation* routeInfo = m_impl->GetNbRoute(m_routeIndex);
    const NB_GpsLocation* currentLocation = m_impl->GetLastLocation();
    if (!currentLocation)
    {
        return "";
    }
    
    NB_GuidanceMessage* pMessage = NULL;
    NB_RouteInformationGetTurnAnnouncementExt(routeInfo, m_impl->GetNbGuidanceInfoForUI(), m_id,
                                              NB_NMP_Turn, NB_NAS_Lookahead, NB_NAT_Street,
                                              isMetric ? NB_NAU_Kilometers : NB_NAU_Miles, &pMessage,
                                              currentLocation->latitude, currentLocation->longitude);
    m_impl->ConvertGuidanceMessageToString(pMessage, maneuverDescription);
    if (pMessage)
    {
        NB_GuidanceMessageDestroy(pMessage);
    }
    
    if(!isFormatted)
    {
        maneuverDescription = NavUtils::ConvertFormatStringToPlainString(maneuverDescription);
    }
    
    return maneuverDescription;
}
}

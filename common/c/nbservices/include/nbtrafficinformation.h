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

    @file     nbtrafficinformation.h

*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef TRAFFICINFORMATION_H
#define TRAFFICINFORMATION_H

#include "nbrouteinformation.h"

/*!
    @addtogroup nbtrafficinformation
    @{
*/

/*! Type of traffic event */
typedef enum
{
    NB_TET_None = 0,        /*!< No traffic event */
    NB_TET_Incident,        /*!< Incident traffic event */
    NB_TET_Congestion       /*!< Congestion traffic event */

} NB_TrafficEventType;

/*! Traffic congestion information */
typedef struct
{
    NB_NavigateTrafficCongestionType    severity;                               /*!< Severity level of this congestion */
    double                              speed;                                  /*!< Average speed within this congestion */
    double                              length;                                 /*!< Length of this congestion */
    nb_boolean                          inCongestion;                           /*!< Indicates if currently in congestion */
    char                                road[NB_LOCATION_MAX_STREET_LEN+1];     /*!< Road where this congestion exists */
    uint32                              updateGpsTimeSeconds;                   /*!< Time updated (seconds since GPS epoch) */
    uint32                              delaySeconds;                           /*!< Delay due to congestion, in seconds*/    

}  NB_TrafficCongestion;

/*! Generic traffic event object containing an incident or congestion */
struct NB_TrafficEvent
{
    NB_TrafficEventType                 type;                       /*!< Traffic event type contained */
    uint32                              maneuverIndex;              /*!< Index of maneuver traffic event is within */
    double                              distanceToEvent;            /*!< Current distance to traffic event */
    double                              routeRemainAfterEvent;      /*!< Route remaining after traffic event */
    NB_Place                            place;                      /*!< Incident place; valid if type is NB_TET_Incident */
    union
    {
        NB_TrafficIncident              incident;                   /*! Traffic incident; valid if type is NB_TET_Incident */
        NB_TrafficCongestion            congestion;                 /*! Traffic congestion; valid if type is NB_TET_Congestion */
    } detail;
};

/*! Get traffic information summary

@param information A NB_TrafficInformation object
@param route information A NB_RouteInformation object
@param startManeuver The starting maneuver
@param distanceOffset Distance along the starting maneuver
@param endManeuver The ending maneuver (non-inclusive)
@param meter Optional.  On success, the traffic intensity scale.  This parameter can be set to NULL
@param incidents Optional.  On success, total number of incidents on route.  This parameter can be set to NULL
@param time Optional.  On success, the total travel time required for route.  This parameter can be set to NULL
@param delay Optional.  On success, the delay time caused by incidents.  This parameter can be set to NULL
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficInformationGetSummary(NB_TrafficInformation* information, NB_RouteInformation* route, uint32 startManeuver, double distanceOffset, uint32 endManeuver, double* meter, uint32* incidents, uint32* time, uint32* delay);

/*! Destroy a previously created TrafficInformation object

@param information A NB_TrafficInformation object
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficInformationDestroy(NB_TrafficInformation* information);

/*! Get information on a traffic incident

@param information A NB_TrafficInformation object
@param index the index of the traffic incident to retrieve
@param place A pointer to an NB_Place to receive basic location information
@param incident A pointer to an NB_TrafficIncident to receive more details on the incident
@param distance A pointer to a double to receive the distance to the incident from the start of the route
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficInformationGetIncident(NB_TrafficInformation* information, uint32 index, NB_Place* place, NB_TrafficIncident* incident, double* distance);

/*! Get the number of traffic incidents 

@param information A NB_TrafficInformation object
@returns Number of incidents
*/
NB_DEC uint32 NB_TrafficInformationGetIncidentCount(NB_TrafficInformation* information);

/*! Get information on a traffic incident for a maneuver

@param information A NB_TrafficInformation object
@param maneuverIndex The maneuver containing the incident
@param incidentIndex The index of the incident on the maneuver
@param place A pointer to an NB_Place to receive basic location information
@param incident A pointer to an NB_TrafficIncident to receive more details on the incident
@param distance A pointer to a double to receive the distance to the incident from the start of the route
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficInformationGetRouteManeuverIncident(NB_TrafficInformation* information, NB_RouteInformation* route, uint32 maneuverIndex, uint32 incidentIndex, NB_Place* place, NB_TrafficIncident* incident, double* distance);

/*! Compare traffic record ID

@param information A NB_TrafficInformation object
@param id a traffic record identifier string
@returns nb_boolean. TRUE if the traffic record identifier of the NB_TrafficInformation object matches id, FALSE otherwise.
 */
NB_DEC nb_boolean NB_TrafficInformationCompareTrafficRecordId(NB_TrafficInformation* information, const char* id);

/*! Get a pronun information

@param route NB_TrafficInformation previously created traffic object
@param key name of base voice or street voice
@param textBuffer plain text buffer
@param textBufferSize size of plain text buffer (return 0 if empty)
@param phoneticsData Phonetic representation of the text (if available), empty otherwise. The text is encoded using UTF-8 format
@param phoneticsDataSize Size of Phonetics data (return 0 if empty)
@param duration time of playback duration in ms
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficInformationGetPronunInformation(NB_TrafficInformation* trafficInformation, const char* key,
                                                          char* textBuffer, uint32* textBufferSize,
                                                          byte* phoneticsData, uint32* phoneticsDataSize,
                                                          double* duration);

/*! @} */

#endif

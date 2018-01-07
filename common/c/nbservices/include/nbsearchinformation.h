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

    @file     nbsearchinformation.h
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

#ifndef NBSEARCHINFORMATION_H
#define NBSEARCHINFORMATION_H

#include "nbexp.h"
#include "nbplace.h"
#include "nbsearchtypes.h"
#include "nbextappcontent.h"

/*!
    @addtogroup nbsearchinformation
    @{
*/

struct NB_SearchInformation;
typedef struct NB_SearchInformation NB_SearchInformation;

/*! Callback function used for NB_SearchInformationGetFormattedContentText(). */
typedef NB_Error (*NB_SearchFormattedTextCallback)(
                                            NB_Font font,       /*!< Font used for returned text */ 
                                            nb_color color,     /*!< Font color for returned text */ 
                                            const char* text,   /*!< Returned text */ 
                                            nb_boolean newline, /*!< TRUE if a new line should be added. */ 
                                            void* userData      /*!< User data passed in NB_SearchInformationGetFormattedContentText() */ 
                                            );

/*! This function gets information about the results of the current POI search operation.
 
    This function has to be called after NB_SearchHandlerStartRequest() to check what type of information
    was returned by the POI search. Multiple types can be returned. See NB_SearchResultType on
    which function to call to retrieve the information.

    The returned type is based on the current slice.

    @return NB_Error

    @see NB_SearchHandler
*/
NB_DEC NB_Error 
NB_SearchInformationGetResultType(
                        NB_SearchInformation* information,                  /*!< A pointer to a NB_SearchInformation instance */
                        NB_SearchResultType* type                           /*!< On return the type of result information that was gathered. 
                                                                                 Can be a combination of values. */
                     );

/*! This function gets the count of results of the current POI search operation.
 
    This function has to be called after NB_SearchHandlerStartRequest()

    @return NB_Error
    @see NB_SearchHandler
*/
NB_DEC NB_Error 
NB_SearchInformationGetResultCount(
                        NB_SearchInformation* information,                  /*!< A pointer to a NB_SearchInformation instance */
                        int* count                                          /*!< On return the number of results that are present in the current slice (may be less than requested number) */
                     );

/*! This function returns whether or not more results are available for the current POI search operation.
 
    This function has to be called after NB_SearchHandlerStartRequest()

    @return NB_Error
    @see NB_SearchHandler
*/
NB_DEC NB_Error 
NB_SearchInformationHasMoreResults(
                        NB_SearchInformation* information,                  /*!< A pointer to a NB_SearchInformation instance */
                        nb_boolean* moreResults                             /*!< On return, non-zero if more result are available; zero otherwise */
                     );

/*! This function retrieves the place details of a specific result of a search operation.

    Call this function if NB_SearchInformationGetResultInformation() returns the type 'NB_SRT_Place'. 
    
    The extended place structure (if requested) is allocated by this function and NB_SearchInformationFreeResultData() 
    has to be called to free the data.

    @return \a NE_OK If successful <br>
            \a NE_INVAL If the specified index is invalid 

    @see NB_SearchInformationGetResultInformation
    @see NB_SearchInformationFreeResultData
*/
NB_DEC NB_Error 
NB_SearchInformationGetPlace(
                        NB_SearchInformation* information,                  /*!< A pointer to a NB_SearchInformation instance */
                        int resultIndex,                                    /*!< The index of the result for which details will be returned */
                        NB_Place* place,                                    /*!< On return place information of POI search. Required. */
                        double* distance,                                   /*!< On return optional distance from the center of the search to the location of this POI.
                                                                                 Pass in NULL if not needed. */
                        NB_ExtendedPlace** extendedPlace                    /*!< On return optional extended place information. Call NB_SearchInformationFreeResultData() to free.
                                                                                 Pass in NULL if not needed. */
                      );

/*! Retrieve event data from a search

    Call this function if NB_SearchInformationGetResultInformation() returns the type 'NB_SRT_Events'.
    Event information can be returned in addition to place information.

    To retrieve detailed event information (formatted text) call NB_SearchInformationGetFormattedContentText()
    and pass in the 'contentHandle' from the NB_Event structure.

    The event data is allocated by this function and NB_SearchInformationFreeResultData() has to be called to
    free the data

    @return NB_Error

    @see NB_SearchInformationGetResultInformation
    @see NB_SearchInformationFreeResultData
    @see NB_SearchInformationGetFormattedContentText
*/
NB_DEC NB_Error 
NB_SearchInformationGetEvents(
                        NB_SearchInformation* information,                 /*!< A pointer to a NB_SearchInformation instance */
                        int resultIndex,                                   /*!< The index of the result for which details will be returned */
                        int* numberOfEvents,                               /*!< On return number of events returned in the events array */
                        NB_Event** events                                  /*!< On return array of events for this search */
                        );

/*! This function retrieves event summary information for the current POI search operation.
 
    Call this function if NB_SearchInformationGetResultInformation() returns the type 'NB_SRT_EventSummary'. 

    @return \a NE_OK If successful <br>
            \a NE_INVAL If the specified NB_SearchInformation instance is invalid 

    @see NB_SearchInformationGetResultInformation
    @see NB_SearchInformationGetFormattedContentText
*/
NB_DEC NB_Error 
NB_SearchInformationGetEventSummary(
                        NB_SearchInformation* information,                  /*!< A pointer to a NB_SearchInformation instance */
                        int resultIndex,                                    /*!< The index of the result for which details will be returned */
                        NB_SummaryResult* summaryResult                     /*!< On return, this will contain the summary information for 
                                                                                 the current search operation. */
                        );


/*! Retrieve optional fuel price information from a search

    Call this function if NB_SearchInformationGetResultInformation() returns the type 'NB_SRT_FuelDetails'.

    @return NB_Error

    @see NB_SearchInformationGetResultInformation
*/
NB_DEC NB_Error
NB_SearchInformationGetFuelDetails(
                        NB_SearchInformation* information,                  /*!< A pointer to a NB_SearchInformation instance */
                        int resultIndex,                                    /*!< The index of the result for which details will be returned */
                        NB_FuelDetails* details                             /*!< Returned fuel details. */
                        );

/*! This function retrieves fuel price information for the current search operation.
 
    Call this function if NB_SearchInformationGetResultInformation() returns the type 'NB_SRT_FuelSummary'.

    The fuel price summary information is alway based on one downloaded slice.

    @return \a NE_OK If successful <br>
            \a NE_INVAL If pFuelPriceSummary is invalid 

    @see NB_SearchInformationGetResultInformation
*/
NB_DEC NB_Error 
NB_SearchInformationGetFuelSummary(
                        NB_SearchInformation* information,                  /*!< A pointer to a NB_SearchInformation instance */
                        NB_FuelSummary* fuelSummary                         /*!< On return, this will contain information about fuel prices. */
                        );

/*! This function retrieves weather information for the current POI search operation.

    Call this function if NB_SearchInformationGetResultInformation() returns the type 'NB_SRT_Weather'.

    This function doesn't take an index like the other search result functions since there is only 
    one weather result per weather search. Call NB_SearchInformationGetPlace() with index zero to retrieve the 
    place for which the weather information applies.

    @return \a NE_OK If successful <br>
            \a NE_INVAL If the specified NB_SearchInformation instance is invalid 

    @see NB_SearchInformationGetPlace
    @see NB_SearchInformationGetResultInformation
*/
NB_DEC NB_Error 
NB_SearchInformationGetWeather(
                        NB_SearchInformation* information,                  /*!< A pointer to a NB_SearchInformation instance */
                        NB_WeatherConditions* weatherConditions,            /*!< On return, this will contain information about the weather conditions */
                        NB_WeatherForecasts* weatherForecasts               /*!< On return, this will contain information about the current weather forecast */
                        );


/*! This function retrieves the traffic incident of a search operation.

    Call this function if NB_SearchInformationGetResultInformation() returns the type 'NB_SRT_TrafficIncident'. 

    @return \a NE_OK If successful <br>
            \a NE_INVAL If the specified index is invalid 

    @see NB_SearchInformationGetResultInformation
*/
NB_DEC NB_Error 
NB_SearchInformationGetTrafficIncident(
                        NB_SearchInformation* information,                  /*!< A pointer to a NB_SearchInformation instance */
                        int resultIndex,                                    /*!< The index of the result for which details will be returned */
                        NB_TrafficIncident* incident,                       /*!< On return traffic incident information. Required. */
                        double* distance                                    /*!< On return optional distance from the center of the search to the location of this POI.
                                                                                 Pass in NULL if not needed. */
                      );


/*! Free all or partial search data.

    This function is used to free partial or all search data associated with the POI instance.
    It can be called after each download/slice or after the search data is no longer needed.
    If the search filters are freed then no sub-search can be performed anymore.

    All data will be freed automatically once the NB_SearchInformation instance is destroyed.

    @return NE_OK on success.
*/
NB_DEC NB_Error
NB_SearchInformationFreeResultData(
                        NB_SearchInformation* information,                  /*!< A pointer to a NB_SearchInformation instance */
                        nb_boolean freeExtendedPlaces,                      /*!< Set to TRUE to free all extended place data return by NB_SearchInformationGetPlace() */
                        nb_boolean freeEvents                               /*!< Set to TRUE to free all event data returned by NB_SearchInformationGetEvents() */
                        );


/*! Retrieve formatted event content.

    This function is used to retrieve formatted event content text. Pass in a pointer to a 
    callback function to retrieve the formatted event content. The function blocks until
    all event content is retrieved by the callback function.

    @return NB_Error

    @see NB_SearchInformationGetEvents
*/
NB_DEC NB_Error
NB_SearchInformationGetFormattedContentText(
                        NB_SearchFormattedTextContentHandle* contentHandle, /*!< Content handle from the NB_Event structure. */
                        NB_SearchFormattedTextCallback callback,            /*!< Callback which will be called with the formatted content text. */
                        void* userData                                      /*!< User data passed to the callback function. */
                        );


/*! Destroy a previously created SearchParameters object

@ingroup searchparameters
@returns NB_Error
*/
NB_DEC NB_Error NB_SearchInformationDestroy(
                        NB_SearchInformation* information           /*!< A pointer to a NB_SearchInformation instance returned from a previous search */
                        );


/*! Returns the analytics event id associated with the current search event

You must call NB_SearchInformationHasAnalyticsEventId first to ensure that
information object contains a valid event id.

@param information  The search information containing the place
@param eventId      On success, Event id (0 - max(uint32)
@return NB_Error
@see NB_SearchInformationHasAnalyticsEventId
*/
NB_DEC NB_Error NB_SearchInformationGetAnalyticsEventId(
                        NB_SearchInformation* information,
                        uint32 *eventId
                        );


/*! Checks if there is a valid event id associated with information parameter object

If the information object contains a valid event id, call
NB_SearchInformationGetAnalyticsEventId method to retrieve it.

@param information The search information containing the place
@param hasEventId   TRUE if information object contains a valid event id
@return NB_Error
@see NB_SearchInformationGetAnalyticsEventId
*/
NB_DEC NB_Error NB_SearchInformationHasAnalyticsEventId(
                        NB_SearchInformation* information,
                        nb_boolean *hasEventId
                        );


/*! Returns the count of spelling suggestions returned for the current search

Returns the number of spelling suggestions returned from this query.  The search parameter
create function must specify NB_EXT_WantSpellingSuggestions to receive spelling suggestions.

@param information Pointer to a search information object
@param count On return, the count of spelling suggestion returned from this query
@return NB_Error
@see NB_SearchParametersCreatePOI
@see NB_SearchInformationGetSpellingSuggestion
*/
NB_DEC NB_Error NB_SearchInformationGetSpellingSuggestionCount(NB_SearchInformation* information, uint32* count);


/*! Returns a spelling suggestion from the current search

Returns the spelling suggestions returned from this query by index.  The search parameter
create function must specify NB_EXT_WantSpellingSuggestions to receive spelling suggestions.

@param information Pointer to a search information object
@param suggestionIndex Index of spelling suggestion to return (must be less than value returned by NB_SearchInformationGetSpellingSuggestionCount)
@param suggestionSize Size of suggestion buffer specified to receive spelling suggestion
@param suggestion On return, will contain requested spelling suggestion
@return NB_Error
@see NB_SearchInformationGetSpellingSuggestionCount
*/
NB_DEC NB_Error NB_SearchInformationGetSpellingSuggestion(
                        NB_SearchInformation* information,
                        uint32 suggestionIndex,
                        uint32 suggestionSize,
                        char* suggestion
                        );


/*! Indicates if specified search result item is mappable

Returns boolean indicating whether the returned search item (referenced by index) is mappable.

@param information Pointer to a search information object
@param resultIndex Index of result to check if mappable
@param isMappable On return, will indicate if specified search result is mappable
@return NB_Error
*/
NB_DEC NB_Error NB_SearchInformationIsMappable(NB_SearchInformation* information, int resultIndex, nb_boolean* isMappable);

NB_DEC NB_Error NB_SearchInformationGetPublicTransitPlace(NB_SearchInformation* pThis, int resultIndex, NB_PublicTransitPlaceInfo* pPublicTransitPlace);
NB_DEC NB_Error NB_SearchInformationReleasePublicTransitPlace(NB_PublicTransitPlaceInfo* pPublicTransitPlace);


/*! This function returns the place ext app content.

    @return NB_Error
    @see NB_SearchInformationGetResultInformation
    @see NB_SearchInformationFreeResultData
*/
NB_DEC NB_Error
NB_SearchInformationGetPlaceExtAppContent(
                       NB_SearchInformation* information,                  /*!< A pointer to a NB_SearchInformation instance */
                       int resultIndex,                                    /*!< The index of the result for which details will be returned */
                       NB_ExtAppContent* extAppContent                     /*!< The address of a NB_ExtAppContent object to be filled in with the result */
                       );

/*! @} */

#endif //NBSEARCHINFORMATION_H


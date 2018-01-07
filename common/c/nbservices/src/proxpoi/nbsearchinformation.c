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

    @file     nbsearchinfomation.h
    @defgroup nbinformation Information

    Information object contain the information downloaded from the 
    NAVBuilder server.
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

#include "nbsearchinformation.h"
#include "nbsearchinformationprivate.h"
#include "data_proxpoi_reply.h"
#include "nbcontextprotected.h"

#include "nbplace.h"
#include "nbutility.h"
#include "palmath.h"


// Local Structures ..............................................................................

/*! Event array used to store all event data. */
typedef struct
{
    NB_Event*                  eventArray;             /*!< Dynamic allocated array of events */
    int                         count;                  /*!< Number in event array */

} EventArray;

struct NB_SearchInformation
{
    NB_Context*                 context;

    int                         startIndex;
    data_proxpoi_query          query;
	data_proxpoi_reply	        reply;

    NB_SearchResultType         resultType;

    uint32                      analyticsEventId;
    nb_boolean                  hasValidAnalyticsEventId;

    /*! Internal vectors to keep all dynamically allocated memory.
    */
    CSL_Vector*                 eventArrayVector;       /*!< vector of EventArray items */
    CSL_Vector*                 extendedPlaceVector;    /*!< vector of NB_ExtendedPlace pointers */
};


// Local Functions ...............................................................................

static NB_SearchInformation* AllocateSearchInformation(NB_Context* context);
static int ReplyProxmatchCount(NB_SearchResultType resultType, data_proxpoi_reply* reply);

// Vector management
static NB_Error AddToEventArrayVector(NB_SearchInformation* information, NB_Event* eventArray, int eventCount);
static NB_Error AddToExtendedPlaceVector(NB_SearchInformation* information, NB_ExtendedPlace* extendedPlace);
static void FreeEventArrayVectorData(NB_SearchInformation* information);
static void FreeExtendedPlaceVectorData(NB_SearchInformation* information);


// Public Functions ..............................................................................

NB_DEF NB_Error 
NB_SearchInformationGetResultType(NB_SearchInformation* information, NB_SearchResultType* type)
{
    if (!information || !type)
    {
        return NE_INVAL;
    }
    *type = information->resultType;

	return NE_OK;
}

NB_DEF NB_Error 
NB_SearchInformationGetResultCount(NB_SearchInformation* information, int* count)
{
    if (!information || !count)
    {
        return NE_INVAL;
    }
    *count = ReplyProxmatchCount(information->resultType, &information->reply);

    return NE_OK;
}

NB_DEF NB_Error 
NB_SearchInformationHasMoreResults(NB_SearchInformation* information, nb_boolean* moreResults)
{
    if (!information || !moreResults)
    {
        return NE_INVAL;
    }
    *moreResults = !information->reply.iter_result.exhausted;

    return NE_OK;
}


// See header file for description
NB_DEF NB_Error 
NB_SearchInformationGetPlace(NB_SearchInformation* information, 
                             int resultIndex, 
                             NB_Place* place, 
                             double* distance, 
                             NB_ExtendedPlace** extendedPlace)
{
	NB_Error error = NE_OK;
	data_proxmatch* pproxmatch = NULL;
	int resultCount = CSL_VectorGetLength(information->reply.vec_proxmatch);

    // Validate index
	if ((resultIndex < information->startIndex) || (resultIndex >= (information->startIndex + resultCount)))
    {
		return NE_INVAL;
    }

	nsl_memset(place, 0, sizeof(*place));
	
	pproxmatch = CSL_VectorGetPointer(information->reply.vec_proxmatch, resultIndex - information->startIndex);

    // Set distance. Distance is an optional parameter
    if (distance)
    {
        *distance = pproxmatch->distance;
    }

    // Set place
	error = SetNIMPlaceFromPlace(place, NB_ContextGetDataState(information->context), &pproxmatch->place);
    if (error != NE_OK)
    {
        return error;
    }

    // If the extended place pointer was passed in then we also fill in the extended place information.
    // We allocate a dynamic structure here and pass it back to the caller. We save the pointer in a 
    // vector so we can free the data in NB_SearchInformationDestroy().
    if (extendedPlace)
    {
        error = CreateNIMExtendedPlace(NB_ContextGetDataState(information->context), pproxmatch, extendedPlace);
        if (error != NE_OK)
        {
            return error;
        }

        // Add to internal vector so that we can free it later
        error = AddToExtendedPlaceVector(information, *extendedPlace);
        if (error != NE_OK)
        {
            FreeNIMExtendedPlace(NB_ContextGetDataState(information->context), *extendedPlace);
            return error;
        }
    }

	return error;
}

// See header file for description
NB_DEF NB_Error
NB_SearchInformationGetEvents(NB_SearchInformation* information, int resultIndex,
                              int* numberOfEvents, NB_Event** events)
{
    // The events can either be contained in the event vector of in the place reply or one
    // single event can be contained in the content result.

	NB_Error error = NE_NOENT;
	int placeCount = CSL_VectorGetLength(information->reply.vec_proxmatch);
    int contentCount = CSL_VectorGetLength(information->reply.vec_proxmatch_content);

    // Reset return parameters
    *events = NULL;
    *numberOfEvents = 0;

    // Do we have place content
    if (placeCount > 0)
    {
        struct CSL_Vector* eventVector = NULL;
	    data_proxmatch* pproxmatch = NULL;

        // Validate index
	    if ((resultIndex < information->startIndex) || (resultIndex >= (information->startIndex + placeCount)))
        {
		    return NE_INVAL;
        }

        // Get event vector
	    pproxmatch = CSL_VectorGetPointer(information->reply.vec_proxmatch, resultIndex - information->startIndex);
        eventVector = pproxmatch->place.vec_event;

        if ((!eventVector) || (CSL_VectorGetLength(eventVector) == 0))
        {
            // No data
            return NE_INVAL;
        }

        // The event array and the performance array inside each event will be allocated
        // dynamically here. The pointers are stored in a vector so that we can free everything
        // once NB_SearchInformationDestroy() gets called.

        error = CreateNIMEventsFromVector(events, numberOfEvents, NB_ContextGetDataState(information->context), eventVector);
        if (error != NE_OK)
        {
            return error;
        }

        // save the event array pointer in a vector so that we can later free it.
        return AddToEventArrayVector(information, *events, *numberOfEvents);
    }

    // Check if we have content 
    if (contentCount > 0)
    {
	    data_proxmatch_content* proxmatchContent = NULL;

        // Validate index
	    if ((resultIndex < information->startIndex) || (resultIndex >= (information->startIndex + contentCount)))
        {
		    return NE_INVAL;
        }

        // Get content 
        proxmatchContent = CSL_VectorGetPointer(information->reply.vec_proxmatch_content, resultIndex - information->startIndex);
        
        // Create an dynamic allocated event. The pointers are stored in a vector so that we can 
        // free everything once NB_SearchInformationDestroy() gets called.
        error = CreateNIMEventFromContent(events, NB_ContextGetDataState(information->context), &(proxmatchContent->event_content));
        if (error != NE_OK)
        {
            return error;
        }

        // We have exactly one event in the content information. 
        *numberOfEvents = 1;

        // save the event array pointer in a vector so that we can later free it.
        return AddToEventArrayVector(information, *events, *numberOfEvents);
    }

    // We didn't have any events
    return NE_INVAL;   
}

// See header file for description
NB_DEF NB_Error 
NB_SearchInformationGetEventSummary(NB_SearchInformation* information, int resultIndex, NB_SummaryResult* summaryResult)
{
    data_proxmatch_summary* pproxmatch = NULL;
    int summaryCount = CSL_VectorGetLength(information->reply.vec_proxmatch_summary);

    // Validate parameter and result index
    if ((! summaryResult) || (resultIndex < information->startIndex) || (resultIndex >= (information->startIndex + summaryCount)))
    {
        return NE_INVAL;
    }

    nsl_memset(summaryResult, 0, sizeof(*summaryResult));

    pproxmatch = CSL_VectorGetPointer(information->reply.vec_proxmatch_summary, resultIndex - information->startIndex);

    return SetNIMSummaryResultFromSummaryResult(summaryResult, NB_ContextGetDataState(information->context), pproxmatch);
}

// See header file for description
NB_DEF NB_Error
NB_SearchInformationGetFuelDetails(NB_SearchInformation* information, int resultIndex, NB_FuelDetails* details)
{
	NB_Error err = NE_OK;
	data_proxmatch* pproxmatch = NULL;
	int resultCount = CSL_VectorGetLength(information->reply.vec_proxmatch);

    // Validate parameter and result index
    if ((! details) || (resultIndex < information->startIndex) || (resultIndex >= (information->startIndex + resultCount)))
    {
        return NE_INVAL;
    }

	pproxmatch = CSL_VectorGetPointer(information->reply.vec_proxmatch, resultIndex - information->startIndex);

	if (pproxmatch->place.vec_fuel_product)
	{
        int i = 0;
		int fuelCount = CSL_VectorGetLength(pproxmatch->place.vec_fuel_product);
		details->num_fuelproducts = fuelCount;

		for (i = 0; i < fuelCount && !err; i++)
        {
			err = err ? err : SetNIMFuelProdFromFuelProd(&details->fuel_product[i], 
                                                         NB_ContextGetDataState(information->context), 
                                                         (data_fuelproduct*) CSL_VectorGetPointer(pproxmatch->place.vec_fuel_product, i));
        }
	}

    return err;
}

// See header file for description
NB_DEF NB_Error 
NB_SearchInformationGetFuelSummary(NB_SearchInformation* information, NB_FuelSummary* fuelSummary)
{
	NB_Error err = NE_OK;
	data_fuelproduct* fuelProduct = NULL;
	int fuelIndex = 0;
    int fuelCount = 0;

	if (fuelSummary == NULL)
    {
		return NE_INVAL;
    }

	nsl_memset(fuelSummary, 0, sizeof(*fuelSummary));

	fuelCount = CSL_VectorGetLength(information->reply.fuel_pricesummary.vec_fuelproducts);

	for (fuelIndex = 0; fuelIndex < fuelCount; ++fuelIndex) 
    {
		fuelProduct = (data_fuelproduct*) CSL_VectorGetPointer(information->reply.fuel_pricesummary.vec_fuelproducts, fuelIndex);

		if (fuelProduct->average) 
        {
			err = err ? err : SetNIMFuelProdFromFuelProd(&fuelSummary->average, NB_ContextGetDataState(information->context), fuelProduct);
		}
		else if (fuelProduct->low) 
        {
			err = err ? err : SetNIMFuelProdFromFuelProd(&fuelSummary->low, NB_ContextGetDataState(information->context), fuelProduct);
		}
	}

    return err;
}

// See header file for description
NB_DEF NB_Error 
NB_SearchInformationGetWeather(NB_SearchInformation* information, 
                               NB_WeatherConditions* weatherConditions, 
                               NB_WeatherForecasts* weatherForecasts)
{
	NB_Error err = NE_OK;
	data_proxmatch* pproxmatch = NULL;

    // We should have at least (exactly) one result
    if (CSL_VectorGetLength(information->reply.vec_proxmatch) == 0)
    {
        return NE_INVAL;
    }

    // Get first result. It should contain the weather information
	pproxmatch = CSL_VectorGetPointer(information->reply.vec_proxmatch, 0);

	// weather-condition
    err	= err ? err : SetNIMWeatherConditionsFromWeatherConditions(weatherConditions,
                                                                   NB_ContextGetDataState(information->context),
                                                                   &pproxmatch->place.weather_conditions);
	// weather-forecast details
	if (err == NE_OK && pproxmatch->place.vec_weather_forecast && weatherForecasts)
	{
        int i = 0;
	    int forcastCount = CSL_VectorGetLength(pproxmatch->place.vec_weather_forecast);
		weatherForecasts->num_weather_forecasts = forcastCount;

		for (i = 0; i < forcastCount && !err; i++)
		{
			err = err ? err : SetNIMWeatherForecastFromWeatherForecast(
                                    &weatherForecasts->weather_forecasts[i], 
                                    NB_ContextGetDataState(information->context), 
				                    (data_weather_forecast*) CSL_VectorGetPointer(pproxmatch->place.vec_weather_forecast, i));
		}
	}

    return err;
}

// See header file for description
NB_DEF NB_Error 
NB_SearchInformationGetTrafficIncident(
                        NB_SearchInformation* information,
                        int resultIndex,
                        NB_TrafficIncident* incident,
                        double* distance)
{
	data_proxmatch* pproxmatch = NULL;
	int resultCount = CSL_VectorGetLength(information->reply.vec_proxmatch);

    // Validate index
	if ((resultIndex < information->startIndex) || (resultIndex >= (information->startIndex + resultCount)))
    {
		return NE_INVAL;
    }

	nsl_memset(incident, 0, sizeof(*incident));
	
	pproxmatch = CSL_VectorGetPointer(information->reply.vec_proxmatch, resultIndex - information->startIndex);

    // Set distance. Distance is an optional parameter
    if (distance)
    {
        *distance = pproxmatch->distance;
    }

    // Set traffic incident from the proxpoi result
	return SetNIMTrafficIncidentDetailsFromTrafficIncident(incident, 
                                                           NB_ContextGetDataState(information->context), 
                                                           &pproxmatch->place.traffic_incident);
}

// See header file for description
NB_DEF NB_Error
NB_SearchInformationFreeResultData(NB_SearchInformation* information, nb_boolean freeExtendedPlaces, nb_boolean freeEvents)
{
    if (freeExtendedPlaces)
    {
        FreeExtendedPlaceVectorData(information);
    }

    if (freeEvents)
    {
        FreeEventArrayVectorData(information);
    }

    return NE_OK;
}

// See header file for description
NB_DEF NB_Error
NB_SearchInformationGetFormattedContentText(
    NB_SearchFormattedTextContentHandle* contentHandle, 
    NB_SearchFormattedTextCallback callback, 
    void* userData)
{
    NB_Error err = NE_OK;

    // The content handle points to a data_formatted_text structure
    data_formatted_text* data = (data_formatted_text *) contentHandle;
    data_format_element* elem = NULL;
    NB_Font font = NB_Font_Normal;
    nb_color color = MAKE_NB_COLOR(0,0,0); 
    char empty[2] = {' ', 0 };
    int i = 0;

    if (contentHandle == NULL)
        return NE_INVAL;

    for (i=0; i < CSL_VectorGetLength(data->vec_format_elements); i++) 
    {
        elem = (data_format_element*) CSL_VectorGetPointer(data->vec_format_elements, i);

        if (elem != NULL) 
        {
            switch (elem->type)
            {
                case NB_FormatElement_Font:
                    err = data_format_element_get_font( elem, &font );
                    break;
                case NB_FormatElement_Color:
                    color = elem->color;
                    break;
                case NB_FormatElement_Text:
                    if (elem->text != NULL) 
                    {
                        // Call the user supplied callback function.
                        callback(font, color, elem->text, FALSE, userData);
                    }
                    break;
                case NB_FormatElement_NewLine:
                    // Call the user supplied callback function.
                    callback(font, color, empty, TRUE, userData);
                    break;
                default:
                    break;
            }
        }
    }

    return err;
}

// See header file for description
NB_DEF NB_Error 
NB_SearchInformationDestroy(NB_SearchInformation* information)
{
    NB_Error result = NE_OK;

    if (information)
    {
        data_proxpoi_query_free(NB_ContextGetDataState(information->context), &information->query);
        // Free reply
	    data_proxpoi_reply_free(NB_ContextGetDataState(information->context), &information->reply);

        // Free all our internal vectors
        result = NB_SearchInformationFreeResultData(information, TRUE, TRUE);

        if (information->extendedPlaceVector)
        {
            CSL_VectorDealloc(information->extendedPlaceVector);
        }

        if (information->eventArrayVector)
        {
            CSL_VectorDealloc(information->eventArrayVector);
        }

        // Free the information structure itself
        nsl_free(information);
    }

    return result;
}


// See header file for description
NB_DEF NB_Error NB_SearchInformationGetAnalyticsEventId(
                        NB_SearchInformation* information,
                        uint32 *eventId
                        )
{
    if (information == NULL ||
        eventId == NULL)
    {
        return NE_INVAL;
    }

    *eventId = information->analyticsEventId;

    return NE_OK;
}

// See header file for description
NB_DEF NB_Error NB_SearchInformationHasAnalyticsEventId(
                        NB_SearchInformation* information,
                        nb_boolean *hasEventId
                        )
{
    if (information == NULL ||
        hasEventId == NULL)
    {
        return NE_INVAL;
    }

    *hasEventId = information->hasValidAnalyticsEventId;

    return NE_OK;
}


// See header file for description
NB_DEF NB_Error NB_SearchInformationGetSpellingSuggestionCount(NB_SearchInformation* pThis, uint32* count)
{
    if (pThis == NULL || count == NULL)
    {
        return NE_INVAL;
    }

    *count = CSL_VectorGetLength(pThis->reply.spelling_suggestion.search_filter.vec_pairs);

    return NE_OK;
}


// See header file for description
NB_DEF NB_Error NB_SearchInformationGetSpellingSuggestion(
                    NB_SearchInformation* pThis,
                    uint32 suggestionIndex,
                    uint32 suggestionSize,
                    char* suggestion)
{
    data_pair* suggestionPair = NULL;
    
    if (pThis == NULL || suggestion == NULL)
    {
        return NE_INVAL;
    }

    if (suggestionIndex >= (uint32)CSL_VectorGetLength(pThis->reply.spelling_suggestion.search_filter.vec_pairs))
    {
        return NE_NOENT;
    }

    suggestionPair = (data_pair*)CSL_VectorGetPointer(pThis->reply.spelling_suggestion.search_filter.vec_pairs, suggestionIndex);

    data_string_get_copy(NB_ContextGetDataState(pThis->context), &suggestionPair->value, suggestion, (size_t)suggestionSize);

    return NE_OK;
}


// See header file for description
NB_DEF NB_Error NB_SearchInformationIsMappable(NB_SearchInformation* pThis, int resultIndex, nb_boolean* isMappable)
{
    data_proxmatch* pproxmatch = NULL;
    int resultCount = 0;
    
    if (pThis == NULL || isMappable == NULL)
    {
        return NE_INVAL;
    }

    resultCount = CSL_VectorGetLength(pThis->reply.vec_proxmatch);

    if ((resultIndex < pThis->startIndex) || (resultIndex >= (pThis->startIndex + resultCount)))
    {
        return NE_NOENT;
    }

    pproxmatch = CSL_VectorGetPointer(pThis->reply.vec_proxmatch, resultIndex - pThis->startIndex);
    if (pproxmatch == NULL)
    {
        return NE_NOENT;
    }

    *isMappable = (pproxmatch->unmappable == 0);

    return NE_OK;
}


// Functions defined in nbsearchinformationprivate.h .............................................

// See nbsearchinformationprivate.h for description
NB_Error 
NB_SearchInformationCreateFromTPSReply(tpselt reply, 
                                       data_proxpoi_query* query,
                                       NB_Context* context,                 
                                       NB_SearchInformation** information)
{
    NB_Error error = NE_OK;
    NB_SearchInformation* newInformation = AllocateSearchInformation(context);

    if (newInformation == NULL) 
    {
        return NE_NOMEM;
    }

    if (query)
    {
        error = error ? error : data_proxpoi_query_copy(NB_ContextGetDataState(newInformation->context), &newInformation->query, query);
	    error = error ? error : data_proxpoi_reply_from_tps(NB_ContextGetDataState(newInformation->context), &newInformation->reply, reply);
    }
    // @todo: (BUG 55802) better way to accomplish this?
    // if no data_proxpoi_query supplied, assume the given tps reply is from any query that
    // contains a proxmatch element in its reply and from that create a data_proxpoi_reply
    else
    {
	    error = error ? error : data_proxpoi_reply_from_any_tps_with_proxmatch(NB_ContextGetDataState(newInformation->context), &newInformation->reply, reply);
    }
   
    if (error == NE_OK)
    {
        // Set the return type based on the retrieved information. The result can contain multiple types.
        int resultCount = CSL_VectorGetLength(newInformation->reply.vec_proxmatch);
        newInformation->resultType = NB_SRT_None;

        // Check if proxpoi-error element was returned; if so, return code value to the client as error
        if (newInformation->reply.proxpoi_error.code != 0)
        {
            error = newInformation->reply.proxpoi_error.code;
        }

        // Check if search-cookie was returned; if so, store with context for future queries
        if (newInformation->reply.search_cookie.provider_id)
        {
            (void)NB_ContextSetSearchCookie(newInformation->context, &newInformation->reply.search_cookie);
        }

        // Do we have a place result
        if (resultCount > 0)
        {
            newInformation->resultType |= NB_SRT_Place;

            // Check if we have a search filter. We have to look at all results.
            {
                int resultIndex = 0; 
                data_proxmatch* pproxmatch = NULL;

                // For all results in this slice
                for (resultIndex = 0; resultIndex < resultCount; ++resultIndex)
                {
                    pproxmatch = CSL_VectorGetPointer(newInformation->reply.vec_proxmatch, resultIndex);

                    // Do we have a search filter
                    if (CSL_VectorGetLength(pproxmatch->search_filter.vec_pairs))
                    {
                        // We found a search filter. Exit for loop and set flag.
                        newInformation->resultType |= NB_SRT_SubSearch;
                        break;
                    }
                }
            }
        }

        // Do we have any summary information
        if (CSL_VectorGetLength(newInformation->reply.vec_proxmatch_summary))
        {
            // If we have an event summary then we also have a search builder.
            newInformation->resultType |= NB_SRT_EventSummary;
            newInformation->resultType |= NB_SRT_SubSearch;
        }

        // Do we have any fuel information
    	if (CSL_VectorGetLength(newInformation->reply.fuel_pricesummary.vec_fuelproducts))
        {
            newInformation->resultType |= NB_SRT_FuelSummary;
        }

        // In order to see if we have any fuel details we have to walk the entire slice to see if
        // there is any fuel information. 
        {
            int resultIndex = 0; 
            data_proxmatch* pproxmatch = NULL;

            // For all results in this slice
            for (resultIndex = 0; resultIndex < resultCount; ++resultIndex)
            {
                pproxmatch = CSL_VectorGetPointer(newInformation->reply.vec_proxmatch, resultIndex);

                // Do we have any fuel results
                if ((pproxmatch->place.vec_fuel_product) && CSL_VectorGetLength(pproxmatch->place.vec_fuel_product))
                {
                    // If we have a fuel product then we set the flag and exit the for loop
                    newInformation->resultType |= NB_SRT_FuelDetails;
                    break;
                }
            }
        }

        // To see if we have any events we have to walk the entire slice to see if
        // there are any events contained. Don't check if it is an event search since it
        // could be a subsearch.
        {
            int resultIndex = 0; 
            data_proxmatch* pproxmatch = NULL;

            // For all results in this slice
            for (resultIndex = 0; resultIndex < resultCount; ++resultIndex)
            {
                pproxmatch = CSL_VectorGetPointer(newInformation->reply.vec_proxmatch, resultIndex);

                // Do we have any event results
                if ((pproxmatch->place.vec_event) && (CSL_VectorGetLength(pproxmatch->place.vec_event)))
                {
                    // Set the flag and exit the loop
                    newInformation->resultType |= NB_SRT_Events;
                    break;
                }
            }
        }

        // If we have content then we also set the event flag. It contains exactly one event.
        if (CSL_VectorGetLength(newInformation->reply.vec_proxmatch_content))
        {
            // If we have content then we also have a search filter
            newInformation->resultType |= NB_SRT_Events;
            newInformation->resultType |= NB_SRT_SubSearch;
        }

        // Check for weather results
        {
            int resultIndex = 0; 
            data_proxmatch* pproxmatch = NULL;

            // For all results in this slice
            for (resultIndex = 0; resultIndex < resultCount; ++resultIndex)
            {
                pproxmatch = CSL_VectorGetPointer(newInformation->reply.vec_proxmatch, resultIndex);

                // Do we have any weather results
                if (CSL_VectorGetLength(pproxmatch->place.vec_weather_forecast) || (pproxmatch->place.weather_conditions.update_time != 0))
                {
                    // Set the flag and exit the loop
                    newInformation->resultType |= NB_SRT_Weather;
                    break;
                }
            }
        }

        // Check for traffic incidents
        {
            int resultIndex = 0; 
            data_proxmatch* pproxmatch = NULL;

            // For all results in this slice
            for (resultIndex = 0; resultIndex < resultCount; ++resultIndex)
            {
                pproxmatch = CSL_VectorGetPointer(newInformation->reply.vec_proxmatch, resultIndex);

                // Do we have any traffic incidents. Is there a more elegant way to check if we have a traffic incident?
                if (pproxmatch->place.traffic_incident.entry_time > 0)
                {
                    // Set the flag and exit the loop
                    newInformation->resultType |= NB_SRT_TrafficIncident;
                    break;
                }
            }
        }

        // Check for speed cameras
        {
            int resultIndex = 0; 
            data_proxmatch* pproxmatch = NULL;
            const char* status = 0;

            // For all results in this slice
            for (resultIndex = 0; resultIndex < resultCount; ++resultIndex)
            {
                pproxmatch = CSL_VectorGetPointer(newInformation->reply.vec_proxmatch, resultIndex);
                status = data_string_get(NB_ContextGetDataState(newInformation->context), &pproxmatch->place.camera.status);

                // Do we have any traffic incidents. Is there a more elegant way to check if we have a traffic incident?
                if (status && nsl_strlen(status) > 0)
                {
                    // Set the flag and exit the loop
                    newInformation->resultType |= NB_SRT_SpeedCamera;
                    break;
                }
            }
        }

        *information = newInformation;
	}
    else 
    {
        NB_SearchInformationDestroy(newInformation);

        *information = NULL;
    }

    return error;
}

// See nbsearchinformationprivate.h for description
data_blob* 
NB_SearchInformationGetIterationState(NB_SearchInformation* information)
{
    return &information->reply.iter_result.state;
}

// See nbsearchinformationprivate.h for description
data_proxpoi_query* 
NB_SearchInformationGetQuery(NB_SearchInformation* information)
{
    return &information->query;
}

// See nbsearchinformationprivate.h for description
NB_Error
NB_SearchInformationCloneSearchFilter(NB_SearchInformation* information, 
                                      int resultIndex,
                                      data_search_filter* searchFilter)
{
    data_search_filter* existingFilter = NULL;

    // The search filter can be either in the summary, content or regular data proxi result.
    // Check all three here.

    // Check the regular result
    if (information->reply.vec_proxmatch && CSL_VectorGetLength(information->reply.vec_proxmatch))
    {
        data_proxmatch* pproxmatch = NULL;
        int count = CSL_VectorGetLength(information->reply.vec_proxmatch);

        if (count > 0)
        {
            // Validate result index
            if ((resultIndex < information->startIndex) || (resultIndex >= (information->startIndex + count)))
            {
                return NE_INVAL;
            }

            // Get the search filter from the regular result
            pproxmatch = CSL_VectorGetPointer(information->reply.vec_proxmatch, resultIndex - information->startIndex);
            existingFilter = &pproxmatch->search_filter;
        }
    }
    // Check summary result
    else if (information->reply.vec_proxmatch_summary && CSL_VectorGetLength(information->reply.vec_proxmatch_summary))
    {
        data_proxmatch_summary* pproxmatch = NULL;
        int count = CSL_VectorGetLength(information->reply.vec_proxmatch_summary);

        if (count > 0)
        {
            // Validate result index
            if ((resultIndex < information->startIndex) || (resultIndex >= (information->startIndex + count)))
            {
                return NE_INVAL;
            }

            // Get the search filter from the summary result
            pproxmatch = CSL_VectorGetPointer(information->reply.vec_proxmatch_summary, resultIndex - information->startIndex);
            existingFilter = &pproxmatch->search_filter;
        }
    }
    // Check content result
    else if (information->reply.vec_proxmatch_content && CSL_VectorGetLength(information->reply.vec_proxmatch_content))
    {
        data_proxmatch_content* pproxmatch = NULL;
        int count = CSL_VectorGetLength(information->reply.vec_proxmatch_content);

        if (count > 0)
        {
            // Validate result index
            if ((resultIndex < information->startIndex) || (resultIndex >= (information->startIndex + count)))
            {
                return NE_INVAL;
            }

            // Get the search filter from the summary result
            pproxmatch = CSL_VectorGetPointer(information->reply.vec_proxmatch_content, resultIndex - information->startIndex);
            existingFilter = &pproxmatch->search_filter;
        }
    }

    // Did we find a search filter
    if (existingFilter)
    {
        // Make a copy/clone of the existing search filter and return it
        return data_search_filter_copy(NB_ContextGetDataState(information->context), searchFilter, existingFilter);
    }

    // We didn't find a search filter
    return NE_INVAL;
}

// Local Functions ...............................................................................

NB_SearchInformation* 
AllocateSearchInformation(NB_Context* context)
{
    NB_SearchInformation* pThis = NULL;

	if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
		return NULL;
    }

	nsl_memset(pThis, 0, sizeof(*pThis));

	pThis->context = context;

    // Initialize query and reply
    data_proxpoi_query_init(NB_ContextGetDataState(pThis->context), &pThis->query);
	data_proxpoi_reply_init(NB_ContextGetDataState(pThis->context), &pThis->reply);

    // Initialize our internal vectors
    pThis->eventArrayVector = CSL_VectorAlloc(sizeof(EventArray*));
    pThis->extendedPlaceVector = CSL_VectorAlloc(sizeof(NB_ExtendedPlace*));

    if (!(pThis->eventArrayVector) || !(pThis->extendedPlaceVector))
    {
        if (pThis->eventArrayVector)
        {
            CSL_VectorDealloc(pThis->eventArrayVector);
        }
        if (pThis->extendedPlaceVector)
        {
            CSL_VectorDealloc(pThis->extendedPlaceVector);
        }
        nsl_free(pThis);
        return NULL;
    }

	return pThis;
}

/*! Get the total search result count.

The total count is dependend on the result type

@return total count
*/
static int
ReplyProxmatchCount(NB_SearchResultType resultType, data_proxpoi_reply* reply)
{
    // The fuel summary is independent from a result index. Just return reply count.
    if ((resultType & NB_SRT_Place) || 
        (resultType & NB_SRT_FuelDetails) ||
        (resultType & NB_SRT_TrafficIncident) ||
        (resultType & NB_SRT_FuelSummary))
    {
        return CSL_VectorGetLength(reply->vec_proxmatch);
    }

    if (resultType & NB_SRT_Events)
    {
        // Events can be contained in either vec_proxmatch or vec_proxmatch_content. Take the larger of the two.
        // See NB_SearchInformationGetEvents() for more details.
	    int placeCount = CSL_VectorGetLength(reply->vec_proxmatch);
        int contentCount = CSL_VectorGetLength(reply->vec_proxmatch_content);

        return MAX(placeCount, contentCount);
    }

    if (resultType & NB_SRT_EventSummary)
    {
        return CSL_VectorGetLength(reply->vec_proxmatch_summary);
    }

    if (resultType & NB_SRT_Weather)
    {
        // Weather always returns one entry
        return 1;
    }

    return 0;
}

/*! Add an element to the event array vector.
    
    @return NB_Error

    @see FreeEventArrayVectorData
*/
static NB_Error 
AddToEventArrayVector(NB_SearchInformation* information,    /*!< NB_SearchInformation instance */
                      NB_Event* eventArray,                 /*!< Event array to add to the vector */
                      int eventCount                        /*!< Number in event array */
                     )
{
    // In order to free all events and all child elements of the events we can't just
    // store the event array pointer in the array. We need to create a wrapper structure
    // which stores the event count also.

    // Create new event array structure
    EventArray* array = nsl_malloc(sizeof(EventArray));
    if (!array)
    {
        return NE_NOMEM;
    }

    array->eventArray = eventArray;
    array->count = eventCount;

    // Add it to the vector
    if (!CSL_VectorAppend(information->eventArrayVector, &array))
    {
        nsl_free(array);
        return NE_NOMEM;
    }

    return NE_OK;
}


/*! Add an element to the extended place vector.

    @return NB_Error

    @see FreeExtendedPlaceVectorData
*/
NB_Error 
AddToExtendedPlaceVector(NB_SearchInformation* information,    /*!< NB_SearchInformation instance */
                         NB_ExtendedPlace* extendedPlace   /*!< The extended place pointer to add */
                        )
{
    // Add pointer to internal vector
    if (!CSL_VectorAppend(information->extendedPlaceVector, &extendedPlace))
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

/*! Free all the data in the event array vector.

    This also frees all dynamically allocated child elements. This function does
    not free the vector itself so it can be reused.

    @return None

    @see AddToEventArrayVector
*/
void
FreeEventArrayVectorData(NB_SearchInformation* information)
{
    int length = CSL_VectorGetLength(information->eventArrayVector);
    int i = 0;

    // For all the event array pointers
    for (i = 0; i < length; i++)
    {
        // Get event array pointer
        EventArray* array = NULL;
        CSL_VectorGet(information->eventArrayVector, i, &array);

        // Free all the events including all child elements
        FreeNIMEvents(array->eventArray, array->count, NB_ContextGetDataState(information->context));

        // Now free the EventArray structure itself
        nsl_free(array);
    }

    // Remove all items from the vector but don't destroy the vector itself,
    // we might reuse it for an additional search. It gets destroyed in NB_SearchInformationDestroy().
    CSL_VectorRemoveAll(information->eventArrayVector);
}


/*! Free all the extended place structures.

    This also frees all child elements. This function does
    not free the vector itself so it can be reused.

    @return None

    @see AddToExtendedPlaceVector
*/
void
FreeExtendedPlaceVectorData(NB_SearchInformation* information)
{
    int length = CSL_VectorGetLength(information->extendedPlaceVector);
    int i = 0;

    // For all the extended places
    for (i = 0; i < length; i++)
    {
        // Get pointer to extended place from vector and free it.
        NB_ExtendedPlace* extendedPlace = NULL;
        CSL_VectorGet(information->extendedPlaceVector, i, &extendedPlace);
        FreeNIMExtendedPlace(NB_ContextGetDataState(information->context), extendedPlace);
    }

    // Remove all items from the vector but don't destroy the vector itself,
    // we might reuse it for an additional search. It gets destroyed in NB_SearchInformationDestroy().
    CSL_VectorRemoveAll(information->extendedPlaceVector);
}

data_place*
NB_SearchInformationGetCameraPlace(NB_SearchInformation* pThis, int index, double* distance)
{
	data_proxmatch* pproxmatch = NULL;
	int count = CSL_VectorGetLength(pThis->reply.vec_proxmatch);
    if (distance)
    {
        *distance = 0.0;
    }

	if (index < pThis->startIndex || index >= (pThis->startIndex + count))
    {
		return 0;
    }

	pproxmatch = CSL_VectorGetPointer(pThis->reply.vec_proxmatch, index - pThis->startIndex);
    if (pproxmatch->place.elements & PlaceElement_SpeedCamera)
    {
        if (distance)
        {
            *distance = pproxmatch->distance;
        }
        return &pproxmatch->place;
    }

    return 0;
}


/*! Sets the event id associated with information object parameter

@param information The search information containing the place
@param eventId The value to set for the analytics event id
*/
void NB_SearchInformationSetAnalyticsEventId(NB_SearchInformation* information, uint32 eventId)
{
    information->analyticsEventId = eventId;
    information->hasValidAnalyticsEventId = TRUE;
}


// See header file for description
NB_DEF NB_Error NB_SearchInformationGetSearchEventCookie(NB_SearchInformation* pThis, NB_SearchEventCookie* searchEventCookie)
{
    if (pThis == NULL || searchEventCookie == NULL)
    {
        return NE_INVAL;
    }

    searchEventCookie->providerId = pThis->reply.search_event_cookie.provider_id;
    searchEventCookie->stateSize = (int)pThis->reply.search_event_cookie.state.size;
    searchEventCookie->state = pThis->reply.search_event_cookie.state.data;

    return NE_OK;
}

NB_DEF NB_Error NB_SearchInformationGetPublicTransitPlace(NB_SearchInformation* pThis, int resultIndex, NB_PublicTransitPlaceInfo* pPublicTransitPlace)
{
    NB_Error error = NE_OK;
    data_proxmatch* pProxmatch = NULL;
    NB_PublicTransitStopInfo* publicTransitStops = NULL;
    int count = 0;
    int resultCount = 0;

    if (pThis == NULL || pPublicTransitPlace == NULL)
    {
        return NE_INVAL;
    }

    resultCount = CSL_VectorGetLength(pThis->reply.vec_proxmatch);

    if ((resultIndex < 0) || (resultIndex >= resultCount))
    {
        return NE_NOENT;
    }

    pProxmatch = CSL_VectorGetPointer(pThis->reply.vec_proxmatch, resultIndex);

    if (!pProxmatch)
    {
        return NE_NOENT;
    }

    error = CreatePublicTransitStopsFromVector(&publicTransitStops, &count, NB_ContextGetDataState(pThis->context), pProxmatch->vec_transit_poi);
    pPublicTransitPlace->transitStops = publicTransitStops;
    pPublicTransitPlace->transitStopCount = count;

    return error;
}

NB_DEF NB_Error NB_SearchInformationReleasePublicTransitPlace(NB_PublicTransitPlaceInfo* pPublicTransitPlace)
{
    NB_PublicTransitStopInfo* publicTransitStop = NULL;
    int i = 0;

    if (pPublicTransitPlace == NULL)
    {
        return NE_INVAL;
    }

    for (i = 0; i < pPublicTransitPlace->transitStopCount; i++)
    {
        publicTransitStop = pPublicTransitPlace->transitStops + i;
        if(publicTransitStop->hoursOfOperation)
        {
            nsl_free(publicTransitStop->hoursOfOperation);
        }
    }
    nsl_free(pPublicTransitPlace->transitStops); //all stops are allocates as one chank
    return NE_OK;
}

NB_DEF NB_Error
NB_SearchInformationGetPlaceExtAppContent(NB_SearchInformation* information,
                                          int resultIndex,
                                          NB_ExtAppContent* extAppContent)
{
    NB_Error error = NE_OK;
    data_proxmatch* pproxmatch = NULL;
    int resultCount = 0;

    if (!information || !extAppContent)
    {
        return NE_INVAL;
    }

    resultCount = CSL_VectorGetLength(information->reply.vec_proxmatch);

    if ((resultIndex < information->startIndex) || (resultIndex >= (information->startIndex + resultCount)))
    {
        return NE_INVAL;
    }

    pproxmatch = CSL_VectorGetPointer(information->reply.vec_proxmatch, resultIndex - information->startIndex);
    if (!pproxmatch)
    {
        return NE_NOENT;
    }

    nsl_memset(extAppContent, 0, sizeof(*extAppContent));

    error = SetNIMExtAppContentFromExtAppContentVector(extAppContent, NB_ContextGetDataState(information->context), pproxmatch->vec_extapp_content);

    return error;
}

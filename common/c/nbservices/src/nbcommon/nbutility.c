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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * nimnbutil.c: created 2004/12/31 by Mark Goddard.
 */

#include "nbutility.h"
#include "nbutilityprotected.h"
#include "palmath.h"
#include "navtypes.h"
#include "csltypes.h"

// Local Functions ...............................................................................

NB_Error
CreateNIMPerformancesFromVector(NB_EventPerformance** performanceArray,
                                int* count,
                                data_util_state* pDUS,
                                struct CSL_Vector* performanceVector);

void FreeNIMPerformances(NB_EventPerformance* performanceArray);
static NB_Error SetNIMHoursFromHoursOfOperation(NB_Hours* pHours, data_util_state* pDus, data_hours_of_operation* pHoursOfOperation);


// Public/exported functions for internal use ....................................................

NB_DEF NB_NetworkRequestStatus
TranslateNetStatus(NB_NetworkResponseStatus status)
{
    NB_NetworkRequestStatus reqstatus = NB_NetworkRequestStatus_Failed;

    switch (status) {

        case COMPLETE:
            reqstatus = NB_NetworkRequestStatus_Success;
            break;
        case CANCELLED:
            reqstatus = NB_NetworkRequestStatus_Canceled;
            break;
        case FAILED:
            reqstatus = NB_NetworkRequestStatus_Failed;
            break;
        case TIMEDOUT:
            reqstatus = NB_NetworkRequestStatus_TimedOut;
            break;
        case RESP_ERROR:
            reqstatus = NB_NetworkRequestStatus_Failed;
            break;
        default:
            break;
    }

    return reqstatus;
}

NB_DEF uint32
GetReplyErrorCode(NB_NetworkResponse* resp)
{
    const char* code = NULL;
    if (resp->status == RESP_ERROR && resp->reply != NULL) {
        code = te_getattrc(resp->reply, "code");
        return (uint32)nsl_strtoul((code != NULL) ? code : "0", NULL, 10);
    }
    return 0;
}

NB_DEF NB_Error
SetNIMPlaceFromPlace(NB_Place* pPlace, data_util_state* pDUS, data_place* pDataPlace)
{
    NB_Error err = NE_OK;
    int n,l;

    data_string_get_copy(pDUS, &pDataPlace->name, pPlace->name, sizeof(pPlace->name));

    err = err ? err : SetNIMLocationFromLocation(&pPlace->location, pDUS, &pDataPlace->location);

    l = CSL_VectorGetLength(pDataPlace->vec_phone);

    for (n=0,pPlace->numphone=0; n<l && n < NB_PLACE_NUM_PHONE && !err;n++,pPlace->numphone++)
        err = err ? err : SetNIMPhoneFromPhone(&pPlace->phone[n], pDUS, (data_phone*) CSL_VectorGetPointer(pDataPlace->vec_phone, n));

    l = CSL_VectorGetLength(pDataPlace->vec_category);

    for (n=0,pPlace->numcategory=0; n<l && n < NB_PLACE_NUM_CAT && !err;n++,pPlace->numcategory++)
        err = err ? err : SetNIMCategoryFromCategory(&pPlace->category[n], pDUS, (data_category*) CSL_VectorGetPointer(pDataPlace->vec_category, n));

    return err;
}

NB_DEF NB_Error
SetNIMHoursFromHoursOfOperation(NB_Hours* pHours, data_util_state* pDus, data_hours_of_operation* pHoursOfOperation)
{
    if (pHours == NULL || pDus == NULL || pHoursOfOperation == NULL)
    {
        return NE_INVAL;
    }
    data_string_get_copy(pDus, &pHoursOfOperation->operating_days, pHours->operatingDays, sizeof(pHours->operatingDays));
    data_string_get_copy(pDus, &pHoursOfOperation->operating_hours.starttime, pHours->startTime, sizeof(pHours->startTime));
    data_string_get_copy(pDus, &pHoursOfOperation->operating_hours.endtime, pHours->endTime, sizeof(pHours->endTime));
    data_string_get_copy(pDus, &pHoursOfOperation->operating_hours.headway_sec, pHours->headwaySeconds, sizeof(pHours->headwaySeconds));

    return NE_OK;
}

NB_DEF NB_Error
SetNIMLocationFromLocMatch(NB_Location* pLocation, data_util_state* pDUS, data_locmatch* pLocMatch)
{
    NB_Error err = NE_OK;

    nsl_memset(pLocation, 0, sizeof(*pLocation));

    /* Address */
    err = err ? err : SetNIMLocationFromLocation(pLocation, pDUS, &pLocMatch->location);

    return err;
}

NB_DEF NB_Error
SetNIMLocationFromLocation(NB_Location* pLocation, data_util_state* pDUS, data_location* pLoc)
{
    if ((!pLocation) || (!pDUS) || (!pLoc))
    {
        return NE_INVAL;
    }

    nsl_memset(pLocation, 0, sizeof(NB_Location));
    return data_location_to_nimlocation(pDUS, pLoc, pLocation);
}

NB_DEF NB_Error
SetNIMPhoneFromPhone(NB_Phone* pPhone, data_util_state* pDUS, data_phone* pPh)
{
    if (data_string_compare_cstr(&pPh->kind, "primary"))
        pPhone->type = NB_Phone_Primary;
    else if (data_string_compare_cstr(&pPh->kind, "secondary"))
        pPhone->type = NB_Phone_Secondary;
    else if (data_string_compare_cstr(&pPh->kind, "national"))
        pPhone->type = NB_Phone_National;
    else if (data_string_compare_cstr(&pPh->kind, "mobile"))
        pPhone->type = NB_Phone_Mobile;
    else if (data_string_compare_cstr(&pPh->kind, "fax"))
        pPhone->type = NB_Phone_Fax;
    else
        pPhone->type = NB_Phone_Primary;

    data_string_get_copy(pDUS, &pPh->country, pPhone->country, sizeof(pPhone->country));
    data_string_get_copy(pDUS, &pPh->area, pPhone->area, sizeof(pPhone->area));
    data_string_get_copy(pDUS, &pPh->number, pPhone->number, sizeof(pPhone->number));
    data_string_get_copy(pDUS, &pPh->formatted_phone.text, pPhone->formattedText, sizeof(pPhone->formattedText));

    return NE_OK;
}

NB_DEF NB_Error
SetNIMCategoryFromCategory(NB_Category* pCategory, data_util_state* pDUS, data_category* pCat)
{
    NB_Error err = NE_OK;

    data_string_get_copy(pDUS, &pCat->code, pCategory->code, sizeof(pCategory->code));
    data_string_get_copy(pDUS, &pCat->name, pCategory->name, sizeof(pCategory->name));

    return err;
}

NB_DEF NB_Error
SetNIMExtAppContentFromExtAppContentVector(NB_ExtAppContent* pExtAppContent, data_util_state* pDUS, CSL_Vector* pVecExtAppContent)
{
    NB_Error err = NE_OK;
    int n = 0;
    int l = 0;

    if (!pExtAppContent || !pDUS || !pVecExtAppContent)
    {
        return NE_INVAL;
    }

    l = CSL_VectorGetLength(pVecExtAppContent);

    for (n = 0; n < l; n++)
    {
        data_extapp_content* eac = CSL_VectorGetPointer(pVecExtAppContent, n);

        if (eac && eac->vec_pairs)
        {
            CSL_Vector* vecPairs = eac->vec_pairs;
            const char* appName = data_string_get(pDUS, &eac->name);

            if (nsl_strcmp(appName, "HSAPP") == 0)
            {
                pExtAppContent->tpa |= NB_TPA_HopStop;
                // TODO: to be defined
            }
            else if (nsl_strcmp(appName, "ASAPP") == 0)
            {
                int n2 = 0;
                int l2 = 0;

                l2 = CSL_VectorGetLength(vecPairs);

                for (n2 = 0; n2 < l2; n2++)
                {
                    data_pair* pair = CSL_VectorGetPointer(vecPairs, n2);

                    const char* key = data_string_get(pDUS, &pair->key);
                    if (nsl_strcmp(key, "retailerstoreid") == 0)
                    {
                        const char* value = data_string_get(pDUS, &pair->value);
                        pExtAppContent->tpa |= NB_TPA_Aisle411;
                        pExtAppContent->aisle411Content.retailerStoreID = nsl_atoi(value);
                        break;
                    }
                }
            }
            else if (nsl_strcmp(appName, "FDAPP") == 0)
            {
                pExtAppContent->tpa |= NB_TPA_Fandango;
                // TODO: to be defined
            }
            else if (nsl_strcmp(appName, "OTAPP") == 0)
            {
                pExtAppContent->tpa |= NB_TPA_OpenTable;
                // TODO: to be defined
            }
        }
    }

    return err;
}

NB_DEF NB_Error
SetExtAppContentVectorFromNIMExtAppContent(CSL_Vector* pVecExtAppContent, data_util_state* pDUS, NB_ExtAppContent* pExtAppContent)
{
    NB_Error err = NE_OK;

    if (!pVecExtAppContent || !pDUS || !pExtAppContent)
    {
        return NE_BADDATA;
    }

    if (pExtAppContent->tpa & NB_TPA_Aisle411)
    {
        data_extapp_content deac;
        data_pair pair; // pair for retailer store id
        char retailerStoreIdString[16] = {0}; // buffer to hold string converted from integer

        DATA_MEM_ZERO(&deac, data_extapp_content);
        DATA_INIT(pDUS, err, &deac, data_extapp_content);

        DATA_MEM_ZERO(&pair, data_pair);
        DATA_INIT(pDUS, err, &pair, data_pair);

        nsl_memset(retailerStoreIdString, 0, sizeof(retailerStoreIdString));
        nsl_snprintf(retailerStoreIdString, sizeof(retailerStoreIdString), "%d", pExtAppContent->aisle411Content.retailerStoreID);

        err = err ? err : data_pair_set(pDUS, &pair, "retailerstoreid", retailerStoreIdString);

        if (err == NE_OK)
        {
            if (!CSL_VectorAppend(deac.vec_pairs, &pair))
            {
                err = NE_NOMEM;
            }
        }

        err = err ? err : data_string_set(pDUS, &deac.name, "ASAPP");

        if (err == NE_OK)
        {
            if (!CSL_VectorAppend(pVecExtAppContent, &deac))
            {
                err = NE_NOMEM;
            }
        }

        if (err != NE_OK)
        {
            DATA_FREE(pDUS, &deac, data_extapp_content);
            DATA_FREE(pDUS, &pair, data_pair);
        }
    }

/* TODO: to be defined
    if (pExtAppContent->tpa & NB_TPA_HopStop)
    {
    }

    if (pExtAppContent->tpa & NB_TPA_Fandango)
    {
    }

    if (pExtAppContent->tpa & NB_TPA_OpenTable)
    {
    }
*/

    return err;
}

NB_DEF void
ClearNIMPlace(NB_Place* pPlace)
{
    nsl_memset(pPlace, 0, sizeof(*pPlace));
    ClearNIMLocation(&pPlace->location);
}


// Public functions ..............................................................................

boolean
NIMLocationValid(NB_Location* pLocation)
{
    return (boolean) (pLocation->latitude != INVALID_LATLON && pLocation->longitude != INVALID_LATLON &&
        (nsl_strlen(pLocation->areaname) > 0 || nsl_strlen(pLocation->streetnum) > 0 ||
         nsl_strlen(pLocation->street1) > 0 || nsl_strlen(pLocation->street2) > 0 ||
         nsl_strlen(pLocation->city) > 0 || nsl_strlen(pLocation->county) > 0 ||
         nsl_strlen(pLocation->state) > 0 || nsl_strlen(pLocation->postal) > 0 ||
         nsl_strlen(pLocation->country) > 0 || nsl_strlen(pLocation->airport) > 0 ||
         nsl_strlen(pLocation->country_name) > 0));
}

void
ClearNIMLocation(NB_Location* pLocation)
{
    nsl_memset(pLocation, 0, sizeof(*pLocation));

    pLocation->latitude = INVALID_LATLON;
    pLocation->longitude = INVALID_LATLON;
}

void
ClearLocationIfGPSPlace(NB_Place* pPlace)
{
    if (pPlace->location.type == NB_Location_MyGPS) {

        ClearNIMPlace(pPlace);
        pPlace->location.type = NB_Location_MyGPS;
    }
}


void
ClearNIMTrafficIncidentDetails(NB_TrafficIncident* pIncident)
{
    nsl_memset(pIncident, 0, sizeof(*pIncident));
}

NB_Error
SetNIMFuelProdFromFuelProd(NB_FuelProduct* pFuelProduct, data_util_state* pDUS, data_fuelproduct* pDataFuelProd)
{
    NB_Error err = NE_OK;

    data_string_get_copy(pDUS, &pDataFuelProd->units, pFuelProduct->units, sizeof(pFuelProduct->units));
    data_string_get_copy(pDUS, &pDataFuelProd->fuel_type.code, pFuelProduct->fuel_type.code, sizeof(pFuelProduct->fuel_type.code));

    pFuelProduct->price.value = pDataFuelProd->price.value;
    pFuelProduct->price.modtime = pDataFuelProd->price.modtime;

    data_string_get_copy(pDUS, &pDataFuelProd->price.currency, pFuelProduct->price.currency, sizeof(pFuelProduct->price.currency));
    data_string_get_copy(pDUS, &pDataFuelProd->fuel_type.product_name, pFuelProduct->fuel_type.product_name, sizeof(pFuelProduct->fuel_type.product_name));
    data_string_get_copy(pDUS, &pDataFuelProd->fuel_type.type_name, pFuelProduct->fuel_type.type_name, sizeof(pFuelProduct->fuel_type.type_name));

    return err;
}

NB_Error
SetNIMTrafficIncidentDetailsFromTrafficIncident(NB_TrafficIncident* pIncident, data_util_state* pDUS, data_traffic_incident* pDataTrafficIncident)
{
    NB_Error err = NE_OK;

    pIncident->type            =    pDataTrafficIncident->type;
    pIncident->severity        =    pDataTrafficIncident->criticality;
    pIncident->entry_time    =    pDataTrafficIncident->entry_time;
    pIncident->start_time    =    pDataTrafficIncident->start_time;
    pIncident->end_time        =    pDataTrafficIncident->end_time;
    pIncident->utc_offset   =   pDataTrafficIncident->utc_offset;

    data_string_get_copy(pDUS, &pDataTrafficIncident->road, pIncident->road, sizeof(pIncident->road));
    data_string_get_copy(pDUS, &pDataTrafficIncident->description, pIncident->description, sizeof(pIncident->description));

    return err;
}

NB_Error
SetNIMWeatherConditionsFromWeatherConditions(NB_WeatherConditions* pWeatherConditions, data_util_state* pDUS, data_weather_conditions* pDataWeatherConditions)
{
    NB_Error err = NE_OK;

    pWeatherConditions->temp                    =    pDataWeatherConditions->temp;
    pWeatherConditions->dewpt                    =    pDataWeatherConditions->dewpt;
    pWeatherConditions->rel_humidity            =    pDataWeatherConditions->rel_humidity;
    pWeatherConditions->wind_speed                =    pDataWeatherConditions->wind_speed;
    pWeatherConditions->wind_dir                =    pDataWeatherConditions->wind_dir;
    pWeatherConditions->wind_gust                =    pDataWeatherConditions->wind_gust;
    pWeatherConditions->pressure                =    pDataWeatherConditions->pressure;
    pWeatherConditions->condition_code            =    pDataWeatherConditions->condition_code;
    data_string_get_copy(pDUS, &pDataWeatherConditions->condition, pWeatherConditions->condition, sizeof(pWeatherConditions->condition));
    data_string_get_copy(pDUS, &pDataWeatherConditions->sky, pWeatherConditions->condition, sizeof(pWeatherConditions->sky));
    pWeatherConditions->ceil                    =    pDataWeatherConditions->ceil;
    pWeatherConditions->visibility                =    pDataWeatherConditions->visibility;
    pWeatherConditions->heat_index                =    pDataWeatherConditions->heat_index;
    pWeatherConditions->wind_chill                =    pDataWeatherConditions->wind_chill;
    pWeatherConditions->snow_depth                =    pDataWeatherConditions->snow_depth;
    pWeatherConditions->max_temp_24hr            =    pDataWeatherConditions->max_temp_24hr;
    pWeatherConditions->min_temp_6hr            =    pDataWeatherConditions->min_temp_6hr;
    pWeatherConditions->min_temp_24hr            =    pDataWeatherConditions->min_temp_24hr;
    pWeatherConditions->precipitation_3hr        =    pDataWeatherConditions->precipitation_3hr;
    pWeatherConditions->precipitation_6hr        =    pDataWeatherConditions->precipitation_6hr;
    pWeatherConditions->precipitation_24hr        =    pDataWeatherConditions->precipitation_24hr;
    pWeatherConditions->update_time                =    pDataWeatherConditions->update_time;
    pWeatherConditions->utc_offset              =   pDataWeatherConditions->utc_offset;

    return err;
}

NB_Error
SetNIMWeatherForecastFromWeatherForecast(NB_WeatherForecast* pWeatherforecast, data_util_state* pDUS, data_weather_forecast* pDataWeatherForecast)
{
    NB_Error err = NE_OK;

    pWeatherforecast->date                            =    pDataWeatherForecast->date;
    pWeatherforecast->high_temp                        =    pDataWeatherForecast->high_temp;
    pWeatherforecast->low_temp                        =    pDataWeatherForecast->low_temp;
    pWeatherforecast->wind_dir                        =    pDataWeatherForecast->wind_dir;
    pWeatherforecast->wind_speed                    =    pDataWeatherForecast->wind_speed    ;
    pWeatherforecast->precipitation_probability        =    pDataWeatherForecast->precipitation_probability;
    pWeatherforecast->rel_humidity                    =    pDataWeatherForecast->rel_humidity;
    pWeatherforecast->condition_code                =    pDataWeatherForecast->condition_code;
    data_string_get_copy(pDUS, &pDataWeatherForecast->condition, pWeatherforecast->condition, sizeof(pWeatherforecast->condition));
    pWeatherforecast->uv_index                        =    pDataWeatherForecast->uv_index;
    data_string_get_copy(pDUS, &pDataWeatherForecast->uv_desc, pWeatherforecast->uv_desc, sizeof(pWeatherforecast->uv_desc));
    data_string_get_copy(pDUS, &pDataWeatherForecast->sunrise, pWeatherforecast->sunrise, sizeof(pWeatherforecast->sunrise));
    data_string_get_copy(pDUS, &pDataWeatherForecast->sunset, pWeatherforecast->sunset, sizeof(pWeatherforecast->sunset));
    data_string_get_copy(pDUS, &pDataWeatherForecast->moonrise, pWeatherforecast->moonrise, sizeof(pWeatherforecast->moonrise));
    data_string_get_copy(pDUS, &pDataWeatherForecast->moonset, pWeatherforecast->moonset, sizeof(pWeatherforecast->moonset));
    pWeatherforecast->moon_phase                    =    pDataWeatherForecast->moon_phase;
    pWeatherforecast->update_time                    =    pDataWeatherForecast->update_time;
    pWeatherforecast->utc_offset                    =   pDataWeatherForecast->utc_offset;

    return err;
}

int
PlaceEqual(NB_Place* aplace, NB_Place* bplace)
{
    return    nsl_strcmp(aplace->name, bplace->name) == 0 &&
            nsl_strcmp(aplace->location.areaname, bplace->location.areaname) == 0 &&
            nsl_strcmp(aplace->location.streetnum, bplace->location.streetnum) == 0 &&
            nsl_strcmp(aplace->location.street1, bplace->location.street1) == 0 &&
            nsl_strcmp(aplace->location.street2, bplace->location.street2) == 0 &&
            nsl_strcmp(aplace->location.city, bplace->location.city) == 0 &&
            nsl_strcmp(aplace->location.state, bplace->location.state) == 0 &&
            nsl_strcmp(aplace->location.postal, bplace->location.postal) == 0 &&
            nsl_strcmp(aplace->location.airport, bplace->location.airport) == 0 &&
            nsl_strcmp(aplace->location.country_name, bplace->location.country_name) == 0;
}

NB_Error
SetOriginFromPlace(data_util_state* pds, data_origin* pod, NB_Place* pPlace)
{
    NB_Error err = NE_OK;

    data_origin_free(pds, pod);
    err = err ? err : data_origin_init(pds, pod);

    if (err == NE_OK)
    {
        pod->point.lat = pPlace->location.latitude;
        pod->point.lon = pPlace->location.longitude;
    }

    err = err ? err : data_string_set(pds, &pod->sa, pPlace->location.streetnum);
    err = err ? err : data_string_set(pds, &pod->str, pPlace->location.street1);

    return err;
}

NB_Error
SetDestinationFromPlace(data_util_state* pds, data_destination* pod, NB_Place* pPlace)
{
    NB_Error err = NE_OK;

    data_destination_free(pds, pod);
    err = err ? err : data_destination_init(pds, pod);

    if (err == NE_OK)
    {
        pod->point.lat = pPlace->location.latitude;
        pod->point.lon = pPlace->location.longitude;
    }

    err = err ? err : data_address_from_nimlocation(pds, &pod->address, &pPlace->location);

    err = err ? err : data_string_set(pds, &pod->sa, pPlace->location.streetnum);
    err = err ? err : data_string_set(pds, &pod->str, pPlace->location.street1);

    return err;
}

void
CopyPlace(NB_Place* pDest, NB_Place* pSource, boolean replaceemptyonly)
{
    if (!replaceemptyonly)
        nsl_memcpy(pDest, pSource, sizeof(*pDest));
    else {

        if (nsl_strlen(pDest->name) == 0)
            nsl_strcpy(pDest->name, pSource->name);

        nsl_memcpy(&pDest->location, &pSource->location, sizeof(pDest->location));

        if (pDest->numphone == 0) {
            pDest->numphone = pSource->numphone;
            nsl_memcpy(pDest->phone, pSource->phone, sizeof(pDest->phone));
        }

        if (pDest->numcategory == 0) {
            pDest->numcategory = pSource->numcategory;
            nsl_memcpy(pDest->category, pSource->category, sizeof(pDest->category));
        }
    }
}

NB_Error
SetNIMSummaryResultFromSummaryResult(NB_SummaryResult* pSummaryResult, data_util_state* pDUS, data_proxmatch_summary* pDataSummary)
{
    pSummaryResult->count = pDataSummary->count;
    pSummaryResult->time = pDataSummary->time; // @todo: (BUG 55797) account for time offset

    return SetNIMCategoryFromCategory(&pSummaryResult->category, pDUS, &pDataSummary->category);
}

/*! Create events based on a given event vector.

    This function creates a dynamic event array based on the given event vector. It then allocates a
    dymanic performances array for each event and fills these performances with the data from the
    given vector. It also allocates a dynamic event content. To free all this information call
    FreeNIMEvents().

    This function is similar to CreateNIMEventFromContent

    @return NB_Error

    @see CreateNIMEventFromContent
    @see FreeNIMEvents
*/
NB_Error
CreateNIMEventsFromVector(NB_Event** eventArray,   /*!< Returned array of events */
                          int* eventCount,          /*!< Returned number of events in event array */
                          data_util_state* pDUS,
                          struct CSL_Vector* eventVector   /*!< Input vector containing all event information */
                         )
{
    NB_Error err = NE_OK;
    int i = 0;

    // Allocate event array
    int vectorSize = CSL_VectorGetLength(eventVector);
    int allocationSize = vectorSize * sizeof(NB_Event);
    NB_Event* events = nsl_malloc(allocationSize);
    if (! events)
    {
        return NE_NOMEM;
    }

    nsl_memset(events, 0, allocationSize);

    // For all events in the input vector
    for (i = 0; i < vectorSize; ++i)
    {
        data_formatted_text* eventContentText = NULL;

        // Get event from vector
        data_event* event = CSL_VectorGetPointer(eventVector, i);

        // Create a dynamic performance array and fill in the data
        err = CreateNIMPerformancesFromVector(&(events[i].performances),
                                              &(events[i].num_performance),
                                              pDUS,
                                              event->vec_performances);
        if (err != NE_OK)
        {
            break;
        }

        // Allocate memory for the event context text
        eventContentText = nsl_malloc(sizeof(data_formatted_text));
        if (!eventContentText)
        {
            err = NE_NOMEM;
            break;
        }

        // Copy the formatted text
        err = err ? err : data_formatted_text_init(pDUS, eventContentText);
        err = err ? err : data_formatted_text_copy(pDUS, eventContentText, &(event->content.formated_text));
        if (err != NE_OK)
        {
            nsl_free(eventContentText);
            break;
        }

        // The content handle in the NBI_Event structure points to a data_formatted_text pointer.
        // Set it to the new created pointer.
        events[i].contentHandle = (NB_SearchFormattedTextContentHandle*)eventContentText;

        // Copy remaining data from content
        data_string_get_copy(pDUS, &(event->content.name), events[i].name, sizeof(events[i].name));
        data_string_get_copy(pDUS, &(event->content.rating.mpaa), events[i].mpaa, sizeof(events[i].mpaa));
        events[i].star = event->content.rating.star;

        // What about the remaining content? Don't we need that? E.g. url
    }

    if (err == NE_OK)
    {
        // Set return parameters
        *eventArray = events;
        *eventCount = vectorSize;
    }
    else
    {
        // If an error occured then we clean up everything we've created so far
        FreeNIMEvents(events, vectorSize, pDUS);
    }

    return err;
}


/*! Create event based on a given event content.

    This function is similar to CreateNIMEventsFromVector(). It differs that it only creates
    one single event without any performances. It is currently used for movie searches.

    This function creates a dynamic event based on the given event content. It also allocates
    a dynamic event content. To free the event and content call FreeNIMEvents().

    @return NB_Error

    @see CreateNIMEventsFromVector
    @see FreeNIMEvents
*/
NB_Error
CreateNIMEventFromContent(NB_Event** event,            /*!< Returned event */
                          data_util_state* pDUS,
                          data_event_content* content   /*!< Input content to fill event data */
                         )
{
    NB_Error err = NE_OK;
    data_formatted_text* eventContentText = NULL;

    // Allocate event
    NB_Event* newEvent = nsl_malloc(sizeof(NB_Event));
    if (! newEvent)
    {
        return NE_NOMEM;
    }

    nsl_memset(newEvent, 0, sizeof(*newEvent));

    // Allocate memory for the event context text
    eventContentText = nsl_malloc(sizeof(data_formatted_text));
    if (!eventContentText)
    {
        nsl_free(newEvent);
        return NE_NOMEM;
    }

    // Copy the formatted text
    err = err ? err : data_formatted_text_init(pDUS, eventContentText);
    err = err ? err : data_formatted_text_copy(pDUS, eventContentText, &(content->formated_text));

    // The content handle in the NBI_Event structure points to a data_formatted_text pointer.
    // Set it to the new created pointer.
    newEvent->contentHandle = (NB_SearchFormattedTextContentHandle*)eventContentText;

    // Copy remaining data from content
    data_string_get_copy(pDUS, &(content->name), newEvent->name, sizeof(newEvent->name));
    data_string_get_copy(pDUS, &(content->rating.mpaa), newEvent->mpaa, sizeof(newEvent->mpaa));
    newEvent->star = content->rating.star;

    // What about the remaining content? Don't we need that? E.g. url

    if (err == NE_OK)
    {
        // Set return parameter
        *event = newEvent;
    }
    else
    {
        // Free event and content handle
        nsl_free(newEvent);
        nsl_free(eventContentText);
    }

    return err;
}


/*! Free all event data and all its associated data.

    Call this function to free the event array created by CreateNIMEventsFromVector().
    This function also frees all the dynamic performance arrays and all the event content.

    @return None

    @see CreateNIMEventsFromVector
*/
void
FreeNIMEvents(NB_Event* eventArray,    /*!< Event array to free */
              int eventCount,           /*!< Number in event array */
              data_util_state* pDUS
              )
{
    int i = 0;
    for (i = 0; i < eventCount; i++)
    {
        // Free the content if set
        if (eventArray[i].contentHandle)
        {
            // The content handle in the event structure points to a data_formatted_text object. Free it.
            data_formatted_text_free(pDUS, (data_formatted_text*)(eventArray[i].contentHandle));
            nsl_free(eventArray[i].contentHandle);
        }

        // Free the performance array
        if (eventArray[i].performances)
        {
            FreeNIMPerformances(eventArray[i].performances);
        }
    }

    // Now free the event array itself
    nsl_free(eventArray);
}

/*! Create a dynamically allocated extended place structure.

    Call FreeNIMExtendedPlace() to free the structure

    @return NB_Error

    @see FreeNIMExtendedPlace
*/
NB_Error
CreateNIMExtendedPlace(
    data_util_state* pDUS,
    data_proxmatch* pproxmatch,
    NB_ExtendedPlace** extendedPlace    /*!< On return the new allocated structure */
    )
{
    NB_Error err = NE_OK;
    NB_ExtendedPlace* newExtendedPlace = NULL;
    data_formatted_text* formattedText = NULL;
    int i = 0;

    if (!pproxmatch)
    {
        return NE_INVAL;
    }

    newExtendedPlace = nsl_malloc(sizeof(NB_ExtendedPlace));
    if (!newExtendedPlace)
    {
        return NE_NOMEM;
    }

    nsl_memset(newExtendedPlace, 0, sizeof(NB_ExtendedPlace));

    // Premium placement
    if (pproxmatch->premium_placement)
    {
        newExtendedPlace->extendedInformation |= NB_EPI_PremiumPlacement | NB_EPI_POIContent;
    }

    // Enhanced POI
    if (pproxmatch->enhanced_poi)
    {
        newExtendedPlace->extendedInformation |= NB_EPI_EnhancedPOI | NB_EPI_POIContent;
    }

    // POI content
    if (pproxmatch->has_poi_content)
    {
        // content id
        if (pproxmatch->poi_content.id)
        {
            newExtendedPlace->poiContentId = nsl_malloc(nsl_strlen(pproxmatch->poi_content.id) + 1);
            if (!newExtendedPlace->poiContentId)
            {
                err = NE_NOMEM;
                goto error;
            }
            data_string_get_copy(pDUS, &pproxmatch->poi_content.id, newExtendedPlace->poiContentId, nsl_strlen(pproxmatch->poi_content.id) + 1);
        }

        // tagline
        if (pproxmatch->poi_content.tagline.text)
        {
            newExtendedPlace->tagline = nsl_malloc(nsl_strlen(pproxmatch->poi_content.tagline.text) + 1);
            if (!newExtendedPlace->tagline)
            {
                err = NE_NOMEM;
                goto error;
            }
            data_string_get_copy(pDUS, &pproxmatch->poi_content.tagline.text, newExtendedPlace->tagline, nsl_strlen(pproxmatch->poi_content.tagline.text) + 1);
        }

        // overall rating
        newExtendedPlace->overallRating.averageRating = pproxmatch->poi_content.overall_rating.average_rating;
        newExtendedPlace->overallRating.ratingCount = pproxmatch->poi_content.overall_rating.rating_count;

        // Golden cookie
        if (pproxmatch->poi_content.golden_cookie.provider_id)
        {
            newExtendedPlace->goldenCookie.providerId = nsl_malloc(nsl_strlen(pproxmatch->poi_content.golden_cookie.provider_id) + 1);
            if (!newExtendedPlace->goldenCookie.providerId)
            {
                err = NE_NOMEM;
                goto error;
            }
            data_string_get_copy(pDUS, &pproxmatch->poi_content.golden_cookie.provider_id, newExtendedPlace->goldenCookie.providerId, nsl_strlen(pproxmatch->poi_content.golden_cookie.provider_id) + 1);
        }

        newExtendedPlace->goldenCookie.stateSize = (int)pproxmatch->poi_content.golden_cookie.state.size;
        if (newExtendedPlace->goldenCookie.stateSize != 0)
        {
            newExtendedPlace->goldenCookie.state = nsl_malloc(newExtendedPlace->goldenCookie.stateSize);
            if (!newExtendedPlace->goldenCookie.state)
            {
                err = NE_NOMEM;
                goto error;
            }
            nsl_memcpy(newExtendedPlace->goldenCookie.state, pproxmatch->poi_content.golden_cookie.state.data, newExtendedPlace->goldenCookie.stateSize);
        }

        // Place Event Cookie
        if (pproxmatch->poi_content.place_event_cookie.provider_id)
        {
            newExtendedPlace->placeEventCookie.providerId = nsl_malloc(nsl_strlen(pproxmatch->poi_content.place_event_cookie.provider_id) + 1);
            if (!newExtendedPlace->placeEventCookie.providerId)
            {
                err = NE_NOMEM;
                goto error;
            }
            data_string_get_copy(pDUS, &pproxmatch->poi_content.place_event_cookie.provider_id, newExtendedPlace->placeEventCookie.providerId, nsl_strlen(pproxmatch->poi_content.place_event_cookie.provider_id) + 1);
        }

        newExtendedPlace->placeEventCookie.stateSize = (int)pproxmatch->poi_content.place_event_cookie.state.size;
        if (newExtendedPlace->placeEventCookie.stateSize != 0)
        {
            newExtendedPlace->placeEventCookie.state = nsl_malloc(newExtendedPlace->placeEventCookie.stateSize);
            if (!newExtendedPlace->placeEventCookie.state)
            {
                err = NE_NOMEM;
                goto error;
            }
            nsl_memcpy(newExtendedPlace->placeEventCookie.state, pproxmatch->poi_content.place_event_cookie.state.data, newExtendedPlace->placeEventCookie.stateSize);
        }

        // Formatted text
        formattedText = nsl_malloc(sizeof(data_formatted_text));
        if (!formattedText)
        {
            err = NE_NOMEM;
            goto error;
        }
        err = err ? err : data_formatted_text_init(pDUS, formattedText);
        err = err ? err : data_formatted_text_copy(pDUS, formattedText, &(pproxmatch->poi_content.formatted_text));
        if (err != NE_OK)
        {
            nsl_free(formattedText);
            formattedText = NULL;
            goto error;
        }
        newExtendedPlace->formattedContentHandle = (NB_SearchFormattedTextContentHandle*)formattedText;

        // Vendor content
        if (pproxmatch->poi_content.has_vendor_content)
        {
            NB_VendorContent* vendorContent = NULL;
            int vendorContentCount = CSL_VectorGetLength(pproxmatch->poi_content.vec_vendor_content);
            nsl_assert(vendorContentCount > 0);

            vendorContent = (NB_VendorContent*) nsl_malloc(sizeof(NB_VendorContent) * vendorContentCount);
            if (vendorContent == NULL)
            {
                err = NE_NOMEM;
                goto error;
            }
            nsl_memset(vendorContent, 0, sizeof(NB_VendorContent) * vendorContentCount);

            for (i = 0; i < vendorContentCount; ++i)
            {
                int ii = 0;

                data_vendor_content* dvc = (data_vendor_content*) CSL_VectorGetPointer(pproxmatch->poi_content.vec_vendor_content, i);
                nsl_assert(dvc != NULL);

                vendorContent[i].contentCount = CSL_VectorGetLength(dvc->vec_pairs);
                nsl_assert(vendorContent[i].contentCount > 0);

                vendorContent[i].content = (NB_POIContent*) nsl_malloc(sizeof(NB_POIContent) * vendorContent[i].contentCount);
                if (vendorContent[i].content == NULL)
                {
                    err = NE_NOMEM;
                    goto error;
                }
                nsl_memset(vendorContent[i].content, 0, sizeof(NB_POIContent) * vendorContent[i].contentCount);

                for (ii = 0; ii < vendorContent[i].contentCount; ++ii)
                {
                    data_pair* pdp = CSL_VectorGetPointer(dvc->vec_pairs, ii);

                    char* key = (char*) nsl_malloc(nsl_strlen(data_string_get(pDUS, &pdp->key)) + 1);
                    char* value = NULL;
                    if (key == NULL)
                    {
                        err = NE_NOMEM;
                        goto error;
                    }
                    nsl_strcpy(key, data_string_get(pDUS, &pdp->key));
                    vendorContent[i].content[ii].key = key;

                    value = (char*) nsl_malloc(nsl_strlen(data_string_get(pDUS, &pdp->value)) + 1);
                    if (value == NULL)
                    {
                        err = NE_NOMEM;
                        goto error;
                    }
                    nsl_strcpy(value, data_string_get(pDUS, &pdp->value));
                    vendorContent[i].content[ii].value = value;
                }

                vendorContent[i].overallRating.averageRating = dvc->overall_rating.average_rating;
                vendorContent[i].overallRating.ratingCount = dvc->overall_rating.rating_count;

                vendorContent[i].name = nsl_strdup(dvc->name);
                if (vendorContent[i].name == NULL)
                {
                    err = NE_NOMEM;
                    goto error;
                }
            }

            newExtendedPlace->vendorContent = vendorContent;
            newExtendedPlace->vendorContentCount = vendorContentCount;
        }

    } // POI content

    // Additional POI content as value pairs.  Present only if NB_EXT_WantFormattedText was not specified in the request
    if (pproxmatch->poi_content.vec_pairs)
    {
        newExtendedPlace->poiContentCount = CSL_VectorGetLength(pproxmatch->poi_content.vec_pairs);
        if (newExtendedPlace->poiContentCount != 0)
        {
            newExtendedPlace->poiContent = nsl_malloc(sizeof(NB_POIContent) * newExtendedPlace->poiContentCount);
            if (!newExtendedPlace->poiContent)
            {
                err = NE_NOMEM;
                goto error;
            }

            for (i = 0; i < newExtendedPlace->poiContentCount; i++)
            {
                data_pair* pdp = CSL_VectorGetPointer(pproxmatch->poi_content.vec_pairs, i);
                newExtendedPlace->poiContent[i].key = nsl_malloc(nsl_strlen(data_string_get(pDUS, &pdp->key)) + 1);
                newExtendedPlace->poiContent[i].value = nsl_malloc(nsl_strlen(data_string_get(pDUS, &pdp->value)) + 1);
                if (newExtendedPlace->poiContent[i].key == NULL ||
                    newExtendedPlace->poiContent[i].value == NULL)
                {
                    err = NE_NOMEM;
                    goto error;
                }
                nsl_strcpy(newExtendedPlace->poiContent[i].key, data_string_get(pDUS, &pdp->key));
                nsl_strcpy(newExtendedPlace->poiContent[i].value, data_string_get(pDUS, &pdp->value));
            }
        }
    }

    if (pproxmatch->poi_content.icon.image.size > 0 && pproxmatch->poi_content.icon.image.data != NULL)
    {
        //Incoming data is encoded with base64

        //Create buffer to hold copy of encoded image with NULL ended
        char* imageData = (char*)nsl_malloc(pproxmatch->poi_content.icon.image.size + 1);
        if (imageData == NULL)
        {
            err = NE_NOMEM;
            goto error;
        }

        //Create buffer to hold decoded image. When decoding base64 text 4 characters are typically converted back to 3 characters.
        //So, encoded text lenght will be enough for decoded buffer.
        newExtendedPlace->imageData = nsl_malloc(pproxmatch->poi_content.icon.image.size);
        if (!newExtendedPlace->imageData)
        {
            err = NE_NOMEM;
            nsl_free(imageData);
            goto error;
        }

        nsl_memcpy((void*)imageData, (void*)pproxmatch->poi_content.icon.image.data, pproxmatch->poi_content.icon.image.size);
        imageData[pproxmatch->poi_content.icon.image.size] = '\0';

        //Decode base64
        newExtendedPlace->imageDataSize = decode_base64((const char*)imageData, (char*)newExtendedPlace->imageData);

        nsl_free(imageData);
        imageData = NULL;

        if (newExtendedPlace->imageDataSize <= 0)
        {
            nsl_free(newExtendedPlace->imageData);
            newExtendedPlace->imageData = NULL;
            newExtendedPlace->imageDataSize = 0;
        }
    }

    // Assign data to output parameter
    *extendedPlace = newExtendedPlace;

    return NE_OK;

error:
    FreeNIMExtendedPlace(pDUS, newExtendedPlace);
    newExtendedPlace = NULL;
    *extendedPlace = NULL;
    return err;
}

/*! Clone an existing extended place structure.

Call FreeNIMExtendedPlace() to free the structure

@return NB_Error

@see FreeNIMExtendedPlace
*/
NB_Error
NB_ExtendedPlaceClone(data_util_state* dataState, NB_ExtendedPlace* sourceExtendedPlace, NB_ExtendedPlace** cloneExtendedPlace)
{
    NB_Error err = NE_NOMEM;
    NB_ExtendedPlace* newExtendedPlace = NULL;
    data_formatted_text* formattedText = NULL;
    int i = 0;

    newExtendedPlace = nsl_malloc(sizeof(NB_ExtendedPlace));
    if (!newExtendedPlace)
    {
        goto error;
    }
    nsl_memset(newExtendedPlace, 0, sizeof(NB_ExtendedPlace));

    newExtendedPlace->extendedInformation = sourceExtendedPlace->extendedInformation;

    if (sourceExtendedPlace->poiContentId)
    {
        newExtendedPlace->poiContentId = nsl_strdup(sourceExtendedPlace->poiContentId);
        if (!newExtendedPlace->poiContentId)
        {
            goto error;
        }
    }

    if (sourceExtendedPlace->tagline)
    {
        newExtendedPlace->tagline = nsl_strdup(sourceExtendedPlace->tagline);
        if (!newExtendedPlace->tagline)
        {
            goto error;
        }
    }

    if (sourceExtendedPlace->formattedContentHandle)
    {
        formattedText = nsl_malloc(sizeof(data_formatted_text));
        if (!formattedText)
        {
            goto error;
        }
        if (data_formatted_text_init(dataState, formattedText) != NE_OK ||
            data_formatted_text_copy(dataState, formattedText, (data_formatted_text*)sourceExtendedPlace->formattedContentHandle) != NE_OK)
        {
            nsl_free(formattedText);
            formattedText = NULL;
            goto error;
        }
        newExtendedPlace->formattedContentHandle = (NB_SearchFormattedTextContentHandle*)formattedText;
    }

    if (sourceExtendedPlace->poiContent && sourceExtendedPlace->poiContentCount)
    {
        newExtendedPlace->poiContent = nsl_malloc(sourceExtendedPlace->poiContentCount * sizeof(NB_POIContent));
        if (!newExtendedPlace->poiContent)
        {
            goto error;
        }
        nsl_memset(newExtendedPlace->poiContent, 0, sourceExtendedPlace->poiContentCount * sizeof(NB_POIContent));
        newExtendedPlace->poiContentCount = sourceExtendedPlace->poiContentCount;

        for (i = 0; i < sourceExtendedPlace->poiContentCount; i++)
        {
            newExtendedPlace->poiContent[i].key = nsl_strdup(sourceExtendedPlace->poiContent[i].key);
            newExtendedPlace->poiContent[i].value = nsl_strdup(sourceExtendedPlace->poiContent[i].value);
            if (!newExtendedPlace->poiContent[i].key || !newExtendedPlace->poiContent[i].value)
            {
                goto error;
            }
        }
    }

    if (sourceExtendedPlace->vendorContent && sourceExtendedPlace->vendorContentCount)
    {
        NB_VendorContent* vendorContent = (NB_VendorContent*) nsl_malloc(sizeof(NB_VendorContent) * sourceExtendedPlace->vendorContentCount);
        if (vendorContent == NULL)
        {
            goto error;
        }
        nsl_memset(vendorContent, 0, sizeof(NB_VendorContent) * sourceExtendedPlace->vendorContentCount);

        newExtendedPlace->vendorContent = vendorContent;

        for (i = 0; i < sourceExtendedPlace->vendorContentCount; ++i)
        {
            int ii = 0;
            NB_VendorContent* vc = &sourceExtendedPlace->vendorContent[i];

            vendorContent[i].content = (NB_POIContent*) nsl_malloc(vc->contentCount * sizeof(NB_POIContent));
            if (!vendorContent[i].content)
            {
                goto error;
            }

            for (ii = 0; ii < vc->contentCount; ++ii)
            {
                vendorContent[i].content[ii].key = nsl_strdup(vc->content[ii].key);
                vendorContent[i].content[ii].value = nsl_strdup(vc->content[ii].value);
                if (!vendorContent[i].content[ii].key || !vendorContent[i].content[ii].value)
                {
                    goto error;
                }
            }

            vendorContent[i].contentCount = vc->contentCount;

            vendorContent[i].overallRating.averageRating = vc->overallRating.averageRating;
            vendorContent[i].overallRating.ratingCount = vc->overallRating.ratingCount;

            vendorContent[i].name = nsl_strdup(vc->name);
            if (!vendorContent[i].name)
            {
                goto error;
            }
        }

        newExtendedPlace->vendorContentCount = sourceExtendedPlace->vendorContentCount;
    }

    newExtendedPlace->overallRating.averageRating = sourceExtendedPlace->overallRating.averageRating;
    newExtendedPlace->overallRating.ratingCount = sourceExtendedPlace->overallRating.ratingCount;

    if (sourceExtendedPlace->goldenCookie.providerId)
    {
        newExtendedPlace->goldenCookie.providerId = nsl_strdup(sourceExtendedPlace->goldenCookie.providerId);
    }
    if (sourceExtendedPlace->goldenCookie.state && sourceExtendedPlace->goldenCookie.stateSize)
    {
        newExtendedPlace->goldenCookie.state = nsl_malloc(sourceExtendedPlace->goldenCookie.stateSize);
        if (!newExtendedPlace->goldenCookie.state)
        {
            goto error;
        }
        newExtendedPlace->goldenCookie.stateSize = sourceExtendedPlace->goldenCookie.stateSize;
        nsl_memcpy(newExtendedPlace->goldenCookie.state, sourceExtendedPlace->goldenCookie.state, sourceExtendedPlace->goldenCookie.stateSize);
    }

    if (sourceExtendedPlace->placeEventCookie.providerId)
    {
        newExtendedPlace->placeEventCookie.providerId = nsl_strdup(sourceExtendedPlace->placeEventCookie.providerId);
    }
    if (sourceExtendedPlace->placeEventCookie.state && sourceExtendedPlace->placeEventCookie.stateSize)
    {
        newExtendedPlace->placeEventCookie.state = nsl_malloc(sourceExtendedPlace->placeEventCookie.stateSize);
        if (!newExtendedPlace->placeEventCookie.state)
        {
            goto error;
        }
        newExtendedPlace->placeEventCookie.stateSize = sourceExtendedPlace->placeEventCookie.stateSize;
        nsl_memcpy(newExtendedPlace->placeEventCookie.state, sourceExtendedPlace->placeEventCookie.state, sourceExtendedPlace->placeEventCookie.stateSize);
    }

    if (sourceExtendedPlace->imageData && sourceExtendedPlace->imageDataSize)
    {
        newExtendedPlace->imageData = nsl_malloc(sourceExtendedPlace->imageDataSize);
        if (!newExtendedPlace->imageData)
        {
            goto error;
        }
        newExtendedPlace->imageDataSize = sourceExtendedPlace->imageDataSize;
        nsl_memcpy(newExtendedPlace->imageData, sourceExtendedPlace->imageData, sourceExtendedPlace->imageDataSize);
    }

    *cloneExtendedPlace = newExtendedPlace;

    return NE_OK;

error:
    FreeNIMExtendedPlace(dataState, newExtendedPlace);
    newExtendedPlace = NULL;
    *cloneExtendedPlace = NULL;
    return err;
}


/*! Free a dynamically allocated extended place structure.

    Use this function to free extended place allocated by CreateNIMExtendedPlace()

    @return None

    @see CreateNIMExtendedPlace
*/
void
FreeNIMExtendedPlace(
    data_util_state* pDUS,
    NB_ExtendedPlace* extendedPlace   /*!< Extended place to free */
    )
{
    int i = 0;

    if (extendedPlace)
    {
        if (extendedPlace->poiContent)
        {
            for (i = 0; i < extendedPlace->poiContentCount; i++)
            {

                if (extendedPlace->poiContent[i].key)
                {
                    nsl_free(extendedPlace->poiContent[i].key);
                    extendedPlace->poiContent[i].key = NULL;
                }

                if (extendedPlace->poiContent[i].value)
                {
                    nsl_free(extendedPlace->poiContent[i].value);
                    extendedPlace->poiContent[i].value = NULL;
                }
            }

            nsl_free(extendedPlace->poiContent);
            extendedPlace->poiContent = NULL;
        }

        if (extendedPlace->vendorContentCount)
        {
            nsl_assert(extendedPlace->vendorContent);
            for (i = 0; i < extendedPlace->vendorContentCount; ++i)
            {
                NB_VendorContent* vendorContent = &extendedPlace->vendorContent[i];
                int ii = 0;

                for (ii = 0; ii < vendorContent->contentCount; ++ii)
                {
                    nsl_free(vendorContent->content[ii].key);
                    nsl_free(vendorContent->content[ii].value);
                }

                nsl_free(vendorContent->content);
                vendorContent->contentCount = 0;

                nsl_free(vendorContent->name);
            }

            nsl_free(extendedPlace->vendorContent);

            extendedPlace->vendorContent = NULL;
            extendedPlace->vendorContentCount = 0;
        }

        if (extendedPlace->poiContentId)
        {
            nsl_free(extendedPlace->poiContentId);
            extendedPlace->poiContentId = 0;
        }

        if (extendedPlace->tagline)
        {
            nsl_free(extendedPlace->tagline);
            extendedPlace->tagline = 0;
        }

        if (extendedPlace->goldenCookie.providerId)
        {
            nsl_free(extendedPlace->goldenCookie.providerId);
            extendedPlace->goldenCookie.providerId = 0;
        }

        if (extendedPlace->goldenCookie.state)
        {
            nsl_free(extendedPlace->goldenCookie.state);
            extendedPlace->goldenCookie.state = 0;
        }

        if (extendedPlace->placeEventCookie.providerId)
        {
            nsl_free(extendedPlace->placeEventCookie.providerId);
            extendedPlace->placeEventCookie.providerId = 0;
        }

        if (extendedPlace->placeEventCookie.state)
        {
            nsl_free(extendedPlace->placeEventCookie.state);
            extendedPlace->placeEventCookie.state = 0;
        }

        if (extendedPlace->formattedContentHandle)
        {
            data_formatted_text_free(pDUS, (data_formatted_text*)extendedPlace->formattedContentHandle);
            nsl_free(extendedPlace->formattedContentHandle);
            extendedPlace->formattedContentHandle = 0;
        }

        if (extendedPlace->imageData)
        {
            nsl_free(extendedPlace->imageData);
        }

        // Free the structure itself
        nsl_free(extendedPlace);
    }
}


/*! Clone a search filter.

The search filter has to be freed by calling NIMFreeSearchFilter

@return New created search filter if successful. NULL on failure.

@see NIMFreeSearchFilter
*/
data_search_filter*
NIMCloneSearchFilter(data_util_state* dataState, data_search_filter* searchFilter)
{
    data_search_filter* copy = NULL;

    if (searchFilter)
    {
        if (CSL_VectorGetLength(searchFilter->vec_pairs) != 0)
        {
            copy = nsl_malloc(sizeof(data_search_filter));

            if (copy && data_search_filter_init(dataState, copy) == NE_OK)
            {
                data_search_filter_copy(dataState, copy, searchFilter);
            }
        }
    }

    return copy;
}


/*! Free a search filter.

@return None

@see NIMCloneSearchFilter
*/
void
NIMFreeSearchFilter(data_util_state* dataState, data_search_filter* searchFilter)
{
    if (searchFilter)
    {
        data_search_filter_free(dataState, searchFilter);
        nsl_free(searchFilter);
    }
}

boolean
PlaceHasPhoneNumber(NB_Place* place)
{
    return (place && place->numphone > 0 && nsl_strlen(place->phone[0].number) > 0) ? TRUE : FALSE;
}



// Local Functions ...............................................................................

/*! Create a performance array based on an input vector.

    This function allocates a dynamic performance array and fills out the content based on the
    given input vector. The caller has to call FreeNIMPerformances() to free the data.

    This function is a subfunction of CreateNIMEventsFromVector().

    @return NE_NOMEM if allocation fails. NE_OK on success.

    @see CreateNIMEventsFromVector
    @see FreeNIMPerformances
*/
NB_Error
CreateNIMPerformancesFromVector(NB_EventPerformance** performanceArray,   /*!< On return the new performance array. */
                                int* count,                                 /*!< On return the number in the performance array. */
                                data_util_state* pDUS,
                                struct CSL_Vector* performanceVector               /*!< Input vector of data_event_performance objects. */
                                )
{
    int i = 0;
    NB_EventPerformance* performances = NULL;

    // Allocate array for event performances
    int performanceCount = CSL_VectorGetLength(performanceVector);
    if (performanceCount == 0)
    {
        // Set the return to zero but return success. This is a valid outcome.
        *performanceArray = NULL;
        *count = 0;
        return NE_OK;
    }

    performances = nsl_malloc(performanceCount * sizeof(NB_EventPerformance));
    if (!performances)
    {
        return NE_NOMEM;
    }

    // For all the performances in the input vector
    for (i = 0; i < performanceCount; i++)
    {
        // Get the performance from the input vector
        data_event_performance* performance = CSL_VectorGetPointer(performanceVector, i);

        // Copy the data to the output array
        performances[i].start_time      = performance->start_time;
        performances[i].end_time        = performance->end_time;
        performances[i].bargain_price   = performance->bargain_price;

        // What about the other members in the data_event_performance structure?
        // Don't we need them?
    }

    // Set return parameters
    *performanceArray = performances;
    *count = performanceCount;
    return NE_OK;
}


/*! Frees a performance array.

    This function has to be called to free the performance array created by CreateNIMPerformancesFromVector()

    @return None

    @see CreateNIMPerformancesFromVector
*/
void
FreeNIMPerformances(NB_EventPerformance* performanceArray)    /*!< Pointer to array to free */
{
    // Since the performance array has no sub-array this free is easy.
    // If the performance gets sub-array then we need to free them here, too.
    nsl_free(performanceArray);
}

const char* GetImageFormatString(NB_ImageFormat format)
{
    const char* formatString = 0;

    switch (format)
    {
    case NB_IF_PNG:
        formatString = IMAGE_FORMAT_PNG;
        break;
    case NB_IF_GIF:
        formatString = IMAGE_FORMAT_GIF;
        break;
    case NB_IF_BMP_Deflate:
        formatString = IMAGE_FORMAT_BMPZ;
        break;
    case NB_IF_JPG:
        formatString = IMAGE_FORMAT_JPG;
        break;
    case NB_IF_BMP:
    default:
        // Default to BMP
        formatString = IMAGE_FORMAT_BMP;
        break;
    }

    return formatString;
}

NB_ImageFormat GetImageFormat(const char* formatString)
{
    // Default to BMP
    NB_ImageFormat format = NB_IF_BMP;

    if (str_cmpx(formatString, IMAGE_FORMAT_BMP) == 0)
    {
        format = NB_IF_BMP;
    }
    else if (str_cmpx(formatString, IMAGE_FORMAT_BMPZ) == 0)
    {
        format = NB_IF_BMP_Deflate;
    }
    else if (str_cmpx(formatString, IMAGE_FORMAT_PNG) == 0)
    {
        format = NB_IF_PNG;
    }
    else if (str_cmpx(formatString, IMAGE_FORMAT_GIF) == 0)
    {
        format = NB_IF_GIF;
    }
    else if (str_cmpx(formatString, IMAGE_FORMAT_JPG) == 0)
    {
        format = NB_IF_JPG;
    }

    return format;
}


static NB_Error
SetPublicTransitStopInfoFromTransitPOI(NB_PublicTransitStopInfo* pPublicTransitStop, data_util_state* pDUS, data_transit_poi* pTransitPoi)
{
    NB_Error err = NE_OK;
    int len = 0;
    int i = 0;
    nb_boolean hasRouteInfo = FALSE;
    char transitRouteId[NB_PLACE_NAME_LEN+1] = {0};

    if (pPublicTransitStop == NULL || pDUS == NULL || pTransitPoi == NULL)
    {
        return NE_INVAL;
    }
    nsl_memset(pPublicTransitStop, 0, sizeof(*pPublicTransitStop));
    pPublicTransitStop->hoursOfOperationCount = len =CSL_VectorGetLength((pTransitPoi->vec_hours_of_operation));
    pPublicTransitStop->hoursOfOperation = nsl_malloc(sizeof(NB_Hours) * pPublicTransitStop->hoursOfOperationCount);
    if (pPublicTransitStop->hoursOfOperationCount != 0 && pPublicTransitStop->hoursOfOperation == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pPublicTransitStop->hoursOfOperation, 0, sizeof(NB_Hours) * pPublicTransitStop->hoursOfOperationCount);

    for (i = 0; i < len; i++)
    {
        err = err ? err : SetNIMHoursFromHoursOfOperation(pPublicTransitStop->hoursOfOperation + i, pDUS, CSL_VectorGetPointer(pTransitPoi->vec_hours_of_operation, i));
    }

    if (err != NE_OK)
    {
        return err;
    }

    //Set key-value pairs
    len = CSL_VectorGetLength(pTransitPoi->vec_detail);
    for (i = 0; i < len; i++)
    {
        uint32 uColor = 0;
        data_pair* pdp = (data_pair*)CSL_VectorGetPointer(pTransitPoi->vec_detail, i);
        const char* key = data_string_get(pDUS, &pdp->key);

         if (nsl_strcmp(key, "unique_stop_agency_id") == 0)
        {
            pPublicTransitStop->stopId = (uint32)nsl_strtoul(data_string_get(pDUS, &pdp->value), NULL, 10);
            hasRouteInfo = TRUE;
        }
        else if (nsl_strcmp(key, "agency-name") == 0)
        {
            data_string_get_copy(pDUS, &pdp->value, pPublicTransitStop->agencyName, sizeof(pPublicTransitStop->agencyName));
            hasRouteInfo = TRUE;
        }
        else if (nsl_strcmp(key, "stop-sequence") == 0)
        {
            pPublicTransitStop->sequence = (uint32)nsl_strtoul(data_string_get(pDUS, &pdp->value), NULL, 16);
            hasRouteInfo = TRUE;
        }
        else if (nsl_strcmp(key, "unique_route_agency_id") == 0)
        {
            pPublicTransitStop->routeId= (uint32)nsl_strtoul(data_string_get(pDUS, &pdp->value), NULL, 10);
            hasRouteInfo = TRUE;
        }
        else if (nsl_strcmp(key, "transit-route-short-name") == 0)
        {
            data_string_get_copy(pDUS, &pdp->value, pPublicTransitStop->routeShortName, sizeof(pPublicTransitStop->routeShortName));
            hasRouteInfo = TRUE;
        }
        else if (nsl_strcmp(key, "transit-route-long-name") == 0)
        {
            data_string_get_copy(pDUS, &pdp->value, pPublicTransitStop->routeLongName, sizeof(pPublicTransitStop->routeLongName));
            hasRouteInfo = TRUE;
        }
        else if (nsl_strcmp(key, "transit-route-color") == 0)
        {
            uColor = (uint32)nsl_strtoul(data_string_get(pDUS, &pdp->value), NULL, 16);
            pPublicTransitStop->routeColor = MAKE_NB_COLOR(NB_GET_BYTE_2(uColor), NB_GET_BYTE_1(uColor), NB_GET_BYTE_0(uColor));
            hasRouteInfo = TRUE;
        }
        else if (nsl_strcmp(key, "transit-route-text-color") == 0)
        {
            uColor = (uint32)nsl_strtoul(data_string_get(pDUS, &pdp->value), NULL, 16);
            pPublicTransitStop->routeTextColor = MAKE_NB_COLOR(NB_GET_BYTE_2(uColor), NB_GET_BYTE_1(uColor), NB_GET_BYTE_0(uColor));
            hasRouteInfo = TRUE;
        }
        else if (nsl_strcmp(key, "transit-route-type") == 0)
        {
            data_string_get_copy(pDUS, &pdp->value, pPublicTransitStop->routeType.code, sizeof(pPublicTransitStop->routeType.code));
            hasRouteInfo = TRUE;
        }
        else if (nsl_strcmp(key, "transit-route-id") == 0)
        {
            data_string_get_copy(pDUS, &pdp->value, transitRouteId, sizeof(transitRouteId));
            hasRouteInfo = TRUE;
        }
        else if (nsl_strcmp(key, "transit-stop-type") == 0)
        {
            pPublicTransitStop->stopType |= ConvertPublicTransitRouteType(data_string_get(pDUS, &pdp->value));
        }

    }

    if (nsl_strlen(pPublicTransitStop->routeShortName) == 0)
    {
        nsl_memset(pPublicTransitStop->routeShortName, 0, sizeof(pPublicTransitStop->routeShortName));
        nsl_strncpy(pPublicTransitStop->routeShortName, transitRouteId, sizeof(pPublicTransitStop->routeShortName) - 1);
    }

    if (!hasRouteInfo)
    {
        err = NE_NOENT;
    }
    return err;
}

NB_PublicTransitStopType
ConvertPublicTransitRouteType(const char *routeType)
{
    switch (nsl_strtoul(routeType, NULL, 10))
    {
        case 0:
            return NBPTST_Tram;
        case 1:
            return NBPTST_Subway;
        case 2:
            return NBPTST_Rail;
        case 3:
            return NBPTST_Bus;
        case 4:
            return NBPTST_Ferry;
        case 5:
            return NBPTST_CableCar;
    }
    return NBPTST_None;
}

NB_DEF NB_Error
CreatePublicTransitStopsFromVector(NB_PublicTransitStopInfo** pPublicTransitStops, int* publicTransitStopCount, data_util_state* pDUS, struct CSL_Vector* transitpoiVector)
{
    NB_Error err = NE_OK;
    NB_PublicTransitStopInfo* pStops = NULL;
    int transitPoiLen = 0;
    int i = 0;

    if (pPublicTransitStops == NULL || pDUS == NULL || transitpoiVector == NULL || publicTransitStopCount == NULL )
    {
        return NE_INVAL;
    }
    *pPublicTransitStops = NULL;

    transitPoiLen = CSL_VectorGetLength(transitpoiVector);
    if(transitPoiLen > 0)
    {
        pStops = nsl_malloc(sizeof(NB_PublicTransitStopInfo) * transitPoiLen);
        if(!pStops)
        {
           return NE_NOMEM;
        }
        for (i = 0; i < transitPoiLen; i++)
        {
            data_transit_poi* pTransitPoi = (data_transit_poi*)CSL_VectorGetPointer(transitpoiVector, i);
            err = err ? err : SetPublicTransitStopInfoFromTransitPOI(&pStops[i], pDUS, pTransitPoi);
        }
    }
    *pPublicTransitStops = pStops;
    *publicTransitStopCount = transitPoiLen;
    return err;
}

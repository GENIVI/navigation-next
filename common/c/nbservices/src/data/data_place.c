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
 * data_place.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_place.h"
#include "data_pair.h"
#include "vec.h"
#include "data_event.h"

NB_DEF NB_Error
data_place_init(data_util_state* pds, data_place* pdat)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(pdat, data_place);

    DATA_INIT(pds, err, &pdat->name, data_string);

    DATA_INIT(pds, err, &pdat->location, data_location);

    DATA_VEC_ALLOC(err, pdat->vec_phone, data_phone);

    DATA_VEC_ALLOC(err, pdat->vec_category, data_category);

    DATA_VEC_ALLOC(err, pdat->vec_detail, data_pair);

    DATA_VEC_ALLOC(err, pdat->vec_event, data_event);

    DATA_VEC_ALLOC(err, pdat->vec_fuel_product, data_fuelproduct);

    DATA_INIT(pds, err, &pdat->weather_conditions, data_weather_conditions);

    DATA_INIT(pds, err, &pdat->traffic_incident, data_traffic_incident);

    DATA_VEC_ALLOC(err, pdat->vec_weather_forecast, data_weather_forecast);

    DATA_INIT(pds, err, &pdat->camera, data_speed_camera);

    DATA_INIT(pds, err, &pdat->id, data_string);

    pdat->modificationTime = 0;

    pdat->elements = PlaceElement_None;

    if (err == NE_NOMEM)
        DATA_FREE(pds, pdat, data_place);

    return err;
}

NB_DEF void
data_place_free(data_util_state* pds, data_place* pdat)
{
    if(!pdat)
    {
        return;
    }
    DATA_VEC_FREE(pds, pdat->vec_weather_forecast, data_weather_forecast);

    DATA_FREE(pds, &pdat->weather_conditions, data_weather_conditions);

    DATA_VEC_FREE(pds, pdat->vec_fuel_product, data_fuelproduct);

    DATA_VEC_FREE(pds, pdat->vec_event, data_event);

    DATA_VEC_FREE(pds, pdat->vec_detail, data_pair);

    DATA_VEC_FREE(pds, pdat->vec_category, data_category);

    DATA_VEC_FREE(pds, pdat->vec_phone, data_phone);

    DATA_FREE(pds, &pdat->location, data_location);

    DATA_FREE(pds, &pdat->name, data_string);

    DATA_FREE(pds, &pdat->traffic_incident, data_traffic_incident);

    DATA_FREE(pds, &pdat->camera, data_speed_camera);

    DATA_FREE(pds, &pdat->id, data_string);
}

NB_DEF NB_Error
data_place_from_tps(data_util_state* pds, data_place* pdat, tpselt te)
{
    NB_Error err = NE_OK;
    boolean has_location = FALSE;
    tpselt ce;
    int iter;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pdat, data_place);

    if (err != NE_OK)
        return err;

    err = err ? err : data_string_from_tps_attr(pds, &pdat->name, te, "name");

    iter = 0;

    while (err == NE_OK && (ce = te_nextchild(te, &iter)) != NULL) {

        if (nsl_strcmp(te_getname(ce), "phone") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_phone, data_phone);
        }
        else if (nsl_strcmp(te_getname(ce), "category") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_category, data_category);
        }
        else if (nsl_strcmp(te_getname(ce), "pair") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_detail, data_pair);
        }
        else if (nsl_strcmp(te_getname(ce), "event") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_event, data_event);
        }
        else if (nsl_strcmp(te_getname(ce), "fuel-product") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_fuel_product, data_fuelproduct);
        }
         else if (nsl_strcmp(te_getname(ce), "weather-conditions") == 0) {

            DATA_FROM_TPS(pds, err, ce, &pdat->weather_conditions, data_weather_conditions);
            pdat->elements |= PlaceElement_WeatherCondition;
        }
        else if (nsl_strcmp(te_getname(ce), "weather-forecast") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_weather_forecast, data_weather_forecast);
            pdat->elements |= PlaceElement_WeatherForecast;
        } 
        else if (nsl_strcmp(te_getname(ce), "location") == 0) {
            DATA_FROM_TPS(pds, err, ce, &pdat->location, data_location);
            has_location = TRUE;
        }else if (nsl_strcmp(te_getname(ce), "traffic-incident") == 0) {

            DATA_FROM_TPS(pds, err, ce, &pdat->traffic_incident, data_traffic_incident);
            pdat->elements |= PlaceElement_TrafficIncident;
        }
        else if (nsl_strcmp(te_getname(ce), "speed-camera") == 0) {
            DATA_FROM_TPS(pds, err, ce, &pdat->camera, data_speed_camera);
            pdat->elements |= PlaceElement_SpeedCamera;
        }

        if (err != NE_OK)
            goto errexit;
    }

    if (!has_location)
        err = NE_INVAL;

errexit:

    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_place (error_code: %d)", err));
        data_place_free(pds, pdat);
    }

    return err;
}

NB_DEF tpselt
data_place_to_tps(data_util_state* pds, data_place* pdat)
{
    tpselt te;
    tpselt ce = NULL;
    int n,l;
    data_category* pcat = NULL;

    te = te_new("place");

    if (te == NULL)
        goto errexit;

    if (!te_setattrc(te, "name", data_string_get(pds, &pdat->name)))
        goto errexit;

    if ((ce = data_location_to_tps(pds, &pdat->location)) != NULL && te_attach(te, ce))
        ce = NULL;
    else
        goto errexit;

    if (pdat->vec_phone != NULL) {

        l = CSL_VectorGetLength(pdat->vec_phone);

        for (n=0; n < l; n++) {

            if ((ce = data_phone_to_tps(pds, CSL_VectorGetPointer(pdat->vec_phone, n))) != NULL && te_attach(te, ce))
                ce = NULL;
            else
                goto errexit;
        }
    }

    if (pdat->vec_category != NULL) {

        l = CSL_VectorGetLength(pdat->vec_category);

        for (n=0; n < l; n++) {

            if ((ce = data_category_to_tps(pds, CSL_VectorGetPointer(pdat->vec_category, n))) != NULL && te_attach(te, ce))
                ce = NULL;
            else
                goto errexit;
        }
    }
    if (pdat->vec_detail != NULL) {

        l = CSL_VectorGetLength(pdat->vec_detail);

        for (n=0; n < l; n++) {

            if ((ce = data_pair_to_tps(pds, CSL_VectorGetPointer(pdat->vec_detail, n))) != NULL && te_attach(te, ce))
                ce = NULL;
            else
                goto errexit;
        }
    }

    /*When speed cameras will be searchable via SB and a proper speed camera data feed,
    if vec_catory includes speed camera code "XSCAM", call data_speed_camera_to_tps() to support saving speed camera locations*/

    if (pdat->vec_category != NULL){

        l = CSL_VectorGetLength(pdat->vec_category);
        for (n=0; n < l; n++){

            pcat = CSL_VectorGetPointer(pdat->vec_category, n);
            if (pcat != NULL && nsl_strcmp(pcat->code, "XSCAM") == 0){                

                if ((ce = data_speed_camera_to_tps(pds, &pdat->camera)) != NULL && te_attach(te, ce))
                {
                    ce = NULL; break;
                }
                else
                    goto errexit;
            }
        }
    }

    return te;

errexit:

    te_dealloc(te);
    te_dealloc(ce);

    return NULL;
}

NB_DEF boolean
data_place_equal(data_util_state* pds, data_place* lhs, data_place* rhs)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &lhs->name, &rhs->name, data_string);

    DATA_EQUAL(pds, ret, &lhs->location, &rhs->location, data_location);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_phone, rhs->vec_phone, data_phone);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_category, rhs->vec_category, data_category);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_detail, rhs->vec_detail, data_pair);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_fuel_product, rhs->vec_fuel_product, data_fuelproduct);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_event, rhs->vec_event, data_event);

    DATA_EQUAL(pds, ret, &lhs->traffic_incident, &rhs->traffic_incident, data_traffic_incident);

    DATA_EQUAL(pds, ret, &lhs->weather_conditions, &rhs->weather_conditions, data_weather_conditions);    

    return (boolean) ret;
}

NB_DEF NB_Error
data_place_copy(data_util_state* pds, data_place* dst, data_place* src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, dst, data_place);

    DATA_COPY(pds, err, &dst->location, &src->location, data_location);

    DATA_VEC_COPY(pds, err, dst->vec_phone, src->vec_phone, data_phone);

    DATA_VEC_COPY(pds, err, dst->vec_category, src->vec_category, data_category);

    DATA_VEC_COPY(pds, err, dst->vec_detail, src->vec_detail, data_pair);

    DATA_VEC_COPY(pds, err, dst->vec_fuel_product, src->vec_fuel_product, data_fuelproduct);

    DATA_VEC_COPY(pds, err, dst->vec_event, src->vec_event, data_event);

    DATA_COPY(pds, err, &dst->name, &src->name, data_string);

    DATA_COPY(pds, err , &dst->traffic_incident, &src->traffic_incident, data_traffic_incident);

    DATA_COPY(pds, err, &dst->weather_conditions, &src->weather_conditions, data_weather_conditions);

    return err;
}

NB_DEF NB_Error
data_place_from_nimplace(data_util_state* pds, data_place* pdatl, const NB_Place* pPlace)
{
    NB_Error         err = NE_OK;
    int n,l;
    data_phone       phone;
    data_category    category;

    data_phone_init(pds, &phone);
    data_category_init(pds, &category);

    data_place_free(pds, pdatl);

    err = data_place_init(pds, pdatl);

    err = err ? err : data_string_set(pds, &pdatl->name, pPlace->name);
    err = err ? err : data_location_from_nimlocation(pds, &pdatl->location, &pPlace->location);

    l = pPlace->numphone;

    for (n=0;n<l && err == NE_OK;n++) {

        err = err ? err : data_phone_from_nimphone(pds, &phone, &pPlace->phone[n]);
        err = err ? err : CSL_VectorAppend(pdatl->vec_phone, &phone) ? NE_OK : NE_NOMEM;

        if (err)
            data_phone_free(pds, &phone);
        else
            data_phone_init(pds, &phone);
    }

    l = pPlace->numcategory;

    for (n=0;n<l && err == NE_OK;n++) {

        err = err ? err : data_category_from_nimcategory(pds, &category, &pPlace->category[n]);
        err = err ? err : CSL_VectorAppend(pdatl->vec_category, &category) ? NE_OK : NE_NOMEM;

        if (err)
            data_category_free(pds, &category);
        else
            data_category_init(pds, &category);
    }

    if (err != NE_OK)
        data_place_free(pds, pdatl);

    return err;
}

uint32   data_place_get_tps_size(data_util_state* pds, data_place* pdatl)
{
    uint32 size = 0;
    int i = 0;
    int length = 0;

    size += data_location_get_tps_size(pds, &pdatl->location);

    length = CSL_VectorGetLength(pdatl->vec_phone);
    size += sizeof(length);
    for(i = 0; i < length; i++)
    {
        size += data_phone_get_tps_size(pds, CSL_VectorGetPointer(pdatl->vec_phone, i));
    }

    length = CSL_VectorGetLength(pdatl->vec_category);
    size += sizeof(length);
    for(i = 0; i < length; i++)
    {
        size += data_category_get_tps_size(pds, CSL_VectorGetPointer(pdatl->vec_category, i));
    }

    length = CSL_VectorGetLength(pdatl->vec_detail);
    size += sizeof(length);
    for(i = 0; i < length; i++)
    {
        size += data_pair_get_tps_size(pds, CSL_VectorGetPointer(pdatl->vec_detail, i));
    }

    length = CSL_VectorGetLength(pdatl->vec_event);
    size += sizeof(length);
    for(i = 0; i < length; i++)
    {
        size += data_fuelproduct_get_tps_size(pds, CSL_VectorGetPointer(pdatl->vec_event, i));
    }

    length = CSL_VectorGetLength(pdatl->vec_fuel_product);
    size += sizeof(length);
    for(i = 0; i < length; i++)
    {
        size += data_event_get_tps_size(pds, CSL_VectorGetPointer(pdatl->vec_fuel_product, i));
    }

    size += data_traffic_incident_get_tps_size(pds, &pdatl->traffic_incident);
    size += data_weather_conditions_get_tps_size(pds, &pdatl->weather_conditions);

    length = CSL_VectorGetLength(pdatl->vec_weather_forecast);
    size += sizeof(length);
    for(i = 0; i < length; i++)
    {
        size += data_weather_forecast_get_tps_size(pds, CSL_VectorGetPointer(pdatl->vec_weather_forecast, i));
    }

    size += data_speed_camera_get_tps_size(pds, &pdatl->camera);
    
    size += data_string_get_tps_size(pds, &pdatl->name);
    size += data_string_get_tps_size(pds, &pdatl->id);
    size += sizeof(pdatl->modificationTime);

    size += sizeof(uint32); /*enum PlaceElement*/

    return size;
}

void     data_place_to_buf(data_util_state* pds, data_place* pdatl, struct dynbuf* pdb)
{
    uint32 elements = pdatl->elements;
    int len = 0;

    data_location_to_buf(pds, &pdatl->location, pdb);

    len = CSL_VectorGetLength(pdatl->vec_phone);
    dbufcat(pdb, (const byte*)&len, sizeof(len));
    if (len  > 0)
    {
        int i = 0;

        for (i = 0; i < len; i++)
        {
            data_phone* pair =
                CSL_VectorGetPointer(pdatl->vec_phone, i);

            data_phone_to_buf(pds, pair, pdb);
        }
    }

    len = CSL_VectorGetLength(pdatl->vec_category);
    dbufcat(pdb, (const byte*)&len, sizeof(len));
    if (len  > 0)
    {
        int i = 0;

        for (i = 0; i < len; i++)
        {
            data_category* pair =
                CSL_VectorGetPointer(pdatl->vec_category, i);

            data_category_to_buf(pds, pair, pdb);
        }
    }

    len = CSL_VectorGetLength(pdatl->vec_detail);
    dbufcat(pdb, (const byte*)&len, sizeof(len));
    if (len  > 0)
    {
        int i = 0;

        for (i = 0; i < len; i++)
        {
            data_pair* pair =
                CSL_VectorGetPointer(pdatl->vec_detail, i);

            data_pair_to_buf(pds, pair, pdb);
        }
    }

    len = CSL_VectorGetLength(pdatl->vec_event);
    dbufcat(pdb, (const byte*)&len, sizeof(len));
    if (len  > 0)
    {
        int i = 0;

        for (i = 0; i < len; i++)
        {
            data_event* pair =
                CSL_VectorGetPointer(pdatl->vec_event, i);

            data_event_to_buf(pds, pair, pdb);
        }
    }

    len = CSL_VectorGetLength(pdatl->vec_fuel_product);
    dbufcat(pdb, (const byte*)&len, sizeof(len));
    if (len  > 0)
    {
        int i = 0;

        for (i = 0; i < len; i++)
        {
            data_fuelproduct* pair =
                CSL_VectorGetPointer(pdatl->vec_fuel_product, i);

            data_fuelproduct_to_buf(pds, pair, pdb);
        }
    }

    data_traffic_incident_to_buf(pds, &pdatl->traffic_incident, pdb);
    data_weather_conditions_to_buf(pds, &pdatl->weather_conditions, pdb);

    len = CSL_VectorGetLength(pdatl->vec_weather_forecast);
    dbufcat(pdb, (const byte*)&len, sizeof(len));
    if (len  > 0)
    {
        int i = 0;

        for (i = 0; i < len; i++)
        {
            data_weather_forecast* pair =
                CSL_VectorGetPointer(pdatl->vec_weather_forecast, i);

            data_weather_forecast_to_buf(pds, pair, pdb);
        }
    }

    data_speed_camera_to_buf(pds, &pdatl->camera, pdb);

    data_string_to_buf(pds, &pdatl->name, pdb);
    data_string_to_buf(pds, &pdatl->id, pdb);
    dbufcat(pdb, (const byte*)&pdatl->modificationTime, sizeof(pdatl->modificationTime));

    dbufcat(pdb, (const byte*)&elements, sizeof(elements));
}

NB_Error data_place_from_binary(data_util_state* pds, data_place* pdatl, byte** pdata, size_t* pdatalen)
{
    uint32 elements = 0;
    NB_Error err = NE_OK;

    err = data_location_from_binary(pds, &pdatl->location, pdata, pdatalen);
    DATA_VEC_FROM_BINARY(pds, err, pdata, pdatalen, pdatl->vec_phone, data_phone);
    DATA_VEC_FROM_BINARY(pds, err, pdata, pdatalen, pdatl->vec_category, data_category);
    DATA_VEC_FROM_BINARY(pds, err, pdata, pdatalen, pdatl->vec_detail, data_pair);
    DATA_VEC_FROM_BINARY(pds, err, pdata, pdatalen, pdatl->vec_event, data_event);
    DATA_VEC_FROM_BINARY(pds, err, pdata, pdatalen, pdatl->vec_fuel_product, data_fuelproduct);


    err = err ? err : data_traffic_incident_from_binary(pds, &pdatl->traffic_incident, pdata, pdatalen);
    err = err ? err : data_weather_conditions_from_binary(pds, &pdatl->weather_conditions, pdata, pdatalen);

    DATA_VEC_FROM_BINARY(pds, err, pdata, pdatalen, pdatl->vec_weather_forecast, data_weather_forecast);

    err = err ? err : data_speed_camera_from_binary(pds, &pdatl->camera, pdata, pdatalen);
    
    err = err ? err : data_string_from_binary(pds, &pdatl->name, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pdatl->id, pdata, pdatalen);
    err = err ? err : data_uint64_from_binary(pds, &pdatl->modificationTime, pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &elements, pdata, pdatalen);
    if (err == NE_OK)
    {
        pdatl->elements = elements;
    }

    return err;
}


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

#include "nbcontext.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "nbtrafficparameters.h"
#include "data_traffic_notify_query.h"
#include "nbrouteinformationprivate.h"
#include "nbguidanceinformationprivate.h"

struct NB_TrafficParameters 
{   
    boolean     wantTrafficNotification;                 
    NB_Context* context;    
    data_traffic_notify_query	dataQuery;
};


static void 
cacheContentsNotifyAddItem(uint32 ts, byte* name, size_t namelen, byte* data, size_t datalen, void* puser)
{
    NB_Error err = NE_OK;
    NB_TrafficParameters* pThis = (NB_TrafficParameters*) puser;
    data_cache_item ce;
    data_util_state* dataState = NULL;
    int i = 0;
    int length = 0;
    
    if (!pThis)
    {
        return;
    }

    // check if pThis->dataQuery.cache_contents contain the cache item, if not, append to vec_cache_item
    length = CSL_VectorGetLength(pThis->dataQuery.cache_contents.vec_cache_item);
    for(i = 0; i < length; i++)
    {
        data_blob* tmpCacheItem = CSL_VectorGetPointer(pThis->dataQuery.cache_contents.vec_cache_item, i);
        if(tmpCacheItem->size == namelen && nsl_memcmp(tmpCacheItem->data, name, namelen) == 0)
        {
            return;
        }
    }

    dataState = NB_ContextGetDataState(pThis->context);

    
    err = data_cache_item_init(dataState, &ce);
    err = err ? err : data_blob_set(dataState, &ce.name, name, namelen);
    err = err ? err : data_cache_contents_add(dataState, &pThis->dataQuery.cache_contents, &ce);

    data_cache_item_free(dataState, &ce);
    
}

static NB_TrafficParameters* 
AllocateTrafficParameters(NB_Context* context, NB_RouteInformation* route)
{
    NB_TrafficParameters* pThis = NULL;
    data_util_state* dataState = NULL;

    if (!context)
        return NULL;

    dataState = NB_ContextGetDataState(context); 

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NULL;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    if ( data_traffic_notify_query_init(dataState, &pThis->dataQuery) != NE_OK )
    {
        nsl_free(pThis);
        return NULL;
    }

    data_blob_copy(dataState, &pThis->dataQuery.traffic_record_identifier.nav_progress.route_id, &route->dataReply.route_id);
    data_string_copy(dataState, &pThis->dataQuery.traffic_record_identifier.nav_progress.session_id, &route->dataReply.traffic_record_identifier.value);

    return pThis;
}

static void
AddPronunToQuery(NB_Context* context, NB_TrafficParameters* params, const char* voiceStyle)
{
    data_util_state* dataState = NB_ContextGetDataState(context);    
    CSL_Cache* voiceCache = NULL;

    // Do not include the chache-contents element as downloadeable prononunciation are not needed
    if (!nsl_strempty(params->dataQuery.want_extended_pronun_data.supported_phonetics_formats))
    {
        return;
    }

    voiceCache = NB_ContextGetVoiceCache(context);

    // set up pronun cache
    data_cache_contents_clear(dataState, &params->dataQuery.cache_contents);

    if (voiceStyle && nsl_strlen(voiceStyle) > 0) 
    {
        int i = 0;
        char name[5] = {0};

        if (voiceCache)
        {
            CSL_CacheForEach(voiceCache, cacheContentsNotifyAddItem, params, FALSE);
        }
        // NIM0-NIM7 should be present as cache-item
        for (i = 0;i < 8; ++i)
        {
            nsl_memset(name, 0, sizeof(name));
            nsl_sprintf(name, "NIM%d",i);
            cacheContentsNotifyAddItem(0, (byte*)name, nsl_strlen(name), NULL, 0, (void*)params);
        }

        data_string_set(dataState, &params->dataQuery.cache_contents.pronun_style_pair.key, "pronun-style");
        data_string_set(dataState, &params->dataQuery.cache_contents.pronun_style_pair.value, voiceStyle);
    }
}

NB_Error 
NB_TrafficParametersCreateFromTRI(NB_Context* context, NB_RouteInformation* route, uint32 navProgressPosition, const char* voiceStyle, const char* tri, NB_TrafficParameters** parameters)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = NB_ContextGetDataState(context);
    NB_TrafficParameters* pThis = NULL;    
    
    if (!context || !route || !parameters)
    {
        return NE_INVAL;
    }
    
    pThis = AllocateTrafficParameters(context, route);

    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->wantTrafficNotification = TRUE;
                   
    err = err ? err : data_string_set(dataState, &pThis->dataQuery.traffic_record_identifier.value, tri);    
    err = err ? err : data_string_set(dataState, &pThis->dataQuery.traffic_record_identifier.nav_progress.state, "in-progress");

    pThis->dataQuery.traffic_record_identifier.nav_progress.position = navProgressPosition;
    
    AddPronunToQuery(context, pThis, voiceStyle);

    (*parameters) = pThis;
    return err;
}

NB_DEF NB_Error
NB_TrafficParametersCreate(NB_Context* context, nb_boolean notify, nb_boolean startNotification, NB_RouteInformation* route, uint32 navProgressPosition, const char* voiceStyle, NB_TrafficParameters** parameters)
{
    NB_TrafficConfiguration trafficConfiguration = {0};
    NB_Error err = NE_OK;

    trafficConfiguration.notify = notify;
    trafficConfiguration.startNotification = startNotification;
    trafficConfiguration.route = route;
    trafficConfiguration.navProgressPosition = navProgressPosition;
    if (!nsl_strempty(voiceStyle))
    {
        trafficConfiguration.voiceStyle = nsl_strdup(voiceStyle);
    }

    err = NB_TrafficParametersCreateEx(context, &trafficConfiguration, parameters);

    if (trafficConfiguration.voiceStyle)
    {
        nsl_free(trafficConfiguration.voiceStyle);
    }

    return err;
}

NB_DEF NB_Error
NB_TrafficParametersCreateEx(NB_Context* context, NB_TrafficConfiguration* configuration, NB_TrafficParameters** parameters)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = NB_ContextGetDataState(context);
    NB_TrafficParameters* pThis = NULL;

    if (!context || !configuration || !parameters)
    {
        return NE_INVAL;
    }

    pThis = AllocateTrafficParameters(context, configuration->route);

    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->wantTrafficNotification = configuration->notify;

    err = err ? err : data_traffic_record_identifier_copy(dataState, &pThis->dataQuery.traffic_record_identifier, &configuration->route->dataReply.traffic_record_identifier);

    pThis->dataQuery.traffic_record_identifier.nav_progress.position = configuration->navProgressPosition;

    if (configuration->startNotification)
    {
        err = err ? err : data_string_set(dataState, &pThis->dataQuery.traffic_record_identifier.nav_progress.state, "start");
    }
    else
    {
        err = err ? err : data_string_set(dataState, &pThis->dataQuery.traffic_record_identifier.nav_progress.state, "in-progress");
    }

    AddPronunToQuery(context, pThis, configuration->voiceStyle);

    if (!nsl_strempty(configuration->supportedPhoneticsFormats))
    {
        err = err ? err : data_string_set(dataState,
            &pThis->dataQuery.want_extended_pronun_data.supported_phonetics_formats,
            configuration->supportedPhoneticsFormats);
    }

    (*parameters) = pThis;

    return err;
}

tpselt 
NB_TrafficParametersToTPSQuery(NB_TrafficParameters* parameters)
{
    return data_traffic_notify_query_to_tps( NB_ContextGetDataState(parameters->context), &parameters->dataQuery);
}

NB_DEF NB_Error 
NB_TrafficParametersDestroy(NB_TrafficParameters* parameters)
{
    data_util_state* dataState = NULL;
    if (parameters)
    {
        dataState = NB_ContextGetDataState(parameters->context);
        data_traffic_notify_query_free(dataState, &parameters->dataQuery);
        nsl_free(parameters);
        return NE_OK;
    }
    return NE_INVAL;
}

nb_boolean
NB_TrafficParametersIsIdentifierValid(NB_TrafficParameters* parameters)
{
    if (!parameters)
    {
        return FALSE;
    }
    if (nsl_strempty(parameters->dataQuery.traffic_record_identifier.value))
    {
        return FALSE;
    }
    return TRUE;
}
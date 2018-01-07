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

#include "palstdlib.h"
#include "palmath.h"
#include "nbexp.h"
#include "nbcontextprotected.h"
#include "nbcontextaccess.h"
#include "nbpronunparameters.h"
#include "nbpronunparametersprivate.h"
#include "nbrouteinformationprivate.h"
#include "data_pronun_query.h"
#include "cslcache.h"

struct NB_PronunParameters
{
    NB_Context* context;
    data_pronun_query dataQuery;
};

static NB_PronunParameters* AllocatePronunParameters(NB_Context* context);

NB_DEF NB_Error
NB_PronunParametersCreate(NB_Context* context, const char* pronunStyle, NB_RouteInformation* route, NB_PronunParameters** parameters)
{
    NB_Error err = NE_OK;

    data_util_state* dataState = NULL;
    NB_PronunParameters* pThis = NULL;
    CSL_Cache* voiceCache = NULL;
    data_nav_maneuver* pman = NULL;
    int nman=0, n=0;
    int counter = 0;

    if ( !context || !pronunStyle || !route)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(context);

    if (!dataState)
    {
        return NE_INVAL;
    }

    pThis = AllocatePronunParameters(context);

    if (!pThis)
    {
        return NE_NOMEM;
    }

    voiceCache = NB_ContextGetVoiceCache(context);
    
    if (!voiceCache)
    {
        NB_PronunParametersDestroy(pThis);
        return NE_INVAL;
    }

    err = err ? err : data_pronun_query_init(dataState, &pThis->dataQuery);

#define CHECK_AND_ADD_TO_PRONUN_REQUEST(PRONUN_DATA)                                                \
do                                                                                                  \
{                                                                                                   \
	if(PRONUN_DATA.data != NULL && PRONUN_DATA.size > 0)                                            \
	{                                                                                               \
		if (!CSL_CacheIsItemInCache(voiceCache, PRONUN_DATA.data, PRONUN_DATA.size) &&              \
				0 != nsl_strncmp((const char*)PRONUN_DATA.data, "NIM", 3))                          \
		{                                                                                           \
			data_pronun_query_add_voice(dataState, &pThis->dataQuery, PRONUN_DATA.data, PRONUN_DATA.size); \
			++counter;                                                                              \
		}                                                                                           \
	}                                                                                               \
}                                                                                                   \
while(0)

    nman = NB_RouteInformationGetManeuverCount(route);
    for (n = 0; n < nman; n++)
    {
        pman = NB_RouteInformationGetManeuver(route, n);
        if(pman)
        {
            CHECK_AND_ADD_TO_PRONUN_REQUEST(pman->current_roadinfo.pronun);
            CHECK_AND_ADD_TO_PRONUN_REQUEST(pman->turn_roadinfo.pronun);
            CHECK_AND_ADD_TO_PRONUN_REQUEST(pman->current_roadinfo.exit_number.pronun);
            CHECK_AND_ADD_TO_PRONUN_REQUEST(pman->turn_roadinfo.exit_number.pronun);
        }
    }

#undef CHECK_AND_ADD_TO_PRONUN_REQUEST

    // Do not create pronun parameters if nothing was added
    if (counter == 0)
    {
        NB_PronunParametersDestroy(pThis);
        return NE_NOENT;
    }

    err = err ? err : data_string_set(dataState, &pThis->dataQuery.pronun_style, pronunStyle);

    (*parameters) = pThis;
    return err;
}

NB_DEF NB_Error
NB_PronunParametersDestroy(NB_PronunParameters* parameters)
{
    if (parameters)
    {
        data_pronun_query_free(NB_ContextGetDataState(parameters->context), &parameters->dataQuery);

        nsl_free(parameters);
        return NE_OK;
    }
    return NE_INVAL;
}

tpselt
NB_PronunParametersToTPSQuery(NB_PronunParameters* parameters)
{
    return data_pronun_query_to_tps( NB_ContextGetDataState(parameters->context), &parameters->dataQuery);
}



static NB_PronunParameters*
AllocatePronunParameters(NB_Context* context)
{
    NB_PronunParameters* pThis = NULL;

    if (!context)
        return NULL;

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NULL;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    return pThis;
}


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

    @file     nbrastertiledatasourceinformation.c
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */


#include "nbrastertiledatasourceinformation.h"
#include "nbrastertiledatasourceinformationprivate.h"
#include "data_raster_tile_data_source_reply.h"
#include "nbcontextprotected.h"
#include "nbqalog.h"


struct NB_RasterTileDataSourceInformation
{
    NB_Context*                         context;
    data_raster_tile_data_source_reply  reply;
};


/* See nbrastertiledatasourceinformationprivate.h for description */
NB_Error
NB_RasterTileDataSourceInformationCreate(NB_Context* context,
                                         tpselt reply,
                                         NB_RasterTileDataSourceInformation** information)
{
    NB_RasterTileDataSourceInformation* pThis = 0;
    NB_Error result = NE_OK;

    if (!context || !reply || !information)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    // Convert the tps reply to the data source structure
    result = data_raster_tile_data_source_reply_from_tps(NB_ContextGetDataState(context), &pThis->reply, reply);
    if (result == NE_OK)
    {
        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_RasterTileDataSourceInformationGetDataSource(NB_RasterTileDataSourceInformation* information,
                                                NB_RasterTileDataSourceData* dataSource)
{
    // For now we just return the primary data source
    nsl_memcpy(dataSource, &information->reply.data, sizeof(*dataSource));

    // Check if QA logging is enabled
    if (NB_ContextGetQaLog(information->context))
    {
        /*
            The best way would be to log the data source reply when we get it. But
            that is difficult. We just log it when it gets retrieved. This function usually gets
            called in response to the download completion.

            We log all base and all template urls.
        */
        NB_QaLogDTSReply(information->context, NB_QLMT_Map, dataSource->templateMap.baseUrl);
        NB_QaLogDTSReply(information->context, NB_QLMT_Map, dataSource->templateMap.templateUrl);

        NB_QaLogDTSReply(information->context, NB_QLMT_Traffic, dataSource->templateTraffic.baseUrl);
        NB_QaLogDTSReply(information->context, NB_QLMT_Traffic, dataSource->templateTraffic.templateUrl);

        NB_QaLogDTSReply(information->context, NB_QLMT_Route, dataSource->templateRoute.baseUrl);
        NB_QaLogDTSReply(information->context, NB_QLMT_Route, dataSource->templateRoute.templateUrl);

        NB_QaLogDTSReply(information->context, NB_QLMT_Satellite, dataSource->templateSatellite.baseUrl);
        NB_QaLogDTSReply(information->context, NB_QLMT_Satellite, dataSource->templateSatellite.templateUrl);

        NB_QaLogDTSReply(information->context, NB_QLMT_PTRoute, dataSource->templatePTRoute.baseUrl);
        NB_QaLogDTSReply(information->context, NB_QLMT_PTRoute, dataSource->templatePTRoute.templateUrl);

        NB_QaLogDTSReply(information->context, NB_QLMT_Route, dataSource->templateRouteSelector1.baseUrl);
        NB_QaLogDTSReply(information->context, NB_QLMT_Route, dataSource->templateRouteSelector1.templateUrl);

        NB_QaLogDTSReply(information->context, NB_QLMT_Route, dataSource->templateRouteSelector2.baseUrl);
        NB_QaLogDTSReply(information->context, NB_QLMT_Route, dataSource->templateRouteSelector2.templateUrl);

        NB_QaLogDTSReply(information->context, NB_QLMT_Route, dataSource->templateRouteSelector3.baseUrl);
        NB_QaLogDTSReply(information->context, NB_QLMT_Route, dataSource->templateRouteSelector3.templateUrl);
    }

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_RasterTileDataSourceInformationDestroy(NB_RasterTileDataSourceInformation* information)
{
    if (information)
    {
        data_raster_tile_data_source_reply_free(NB_ContextGetDataState(information->context), &information->reply);
        nsl_free(information);
    }

    return NE_OK;
}

/*! @} */

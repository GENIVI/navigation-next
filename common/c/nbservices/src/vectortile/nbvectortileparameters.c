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

@file     vectortileparameters.c
@defgroup vectortileparameters Vector Map Implementation

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

#include "nbcontextprotected.h"
#include "nbvectortileparameters.h"
#include "vec.h"
#include "data_vector_tile_query.h"


/*! @{ */

/*! Vector map parameters. */
struct _NB_VectorTileParameters
{
    NB_Context* context;					                /*!< NAVBuilder Context Instance */
    NB_VectorTileParametersConfiguration config;
    struct CSL_Vector* tileIndexVector;
};

NB_DEF void
NB_VectorTileParametersConfigurationSetDefaults(
    NB_VectorTileParametersConfiguration* configuration)
{
    configuration->minPpriority			    = 1;
    configuration->maxPriority			    = 7;
    configuration->minScale				    = 3.0;
    configuration->zoomLevel			    = 0;
    configuration->passes				    = 0;
}

/* See header file for description. */
NB_DEF NB_Error
NB_VectorTileParametersCreate(
    NB_Context* context,
    NB_VectorTileParametersConfiguration* configuration,
    NB_VectorTileIndex* tile,
    NB_VectorTileParameters** parameters
    )
{
    NB_VectorTileParameters* pThis = NULL;
    NB_VectorTileIndex* index = NULL;
    NB_Error err = NE_OK;

    if (!context || !parameters || !tile || !configuration)
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
    pThis->config = *configuration;    

    pThis->tileIndexVector = CSL_VectorAlloc(sizeof(NB_VectorTileIndex*));
    if (pThis->tileIndexVector == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    index = nsl_malloc(sizeof(NB_VectorTileIndex));
    if (!index)
    {
        err = NE_NOMEM;
        goto errexit;
    }
    *index = *tile;
    if (!CSL_VectorAppend(pThis->tileIndexVector, &index))
    {
        err = NE_NOMEM;
        goto errexit;
    }

errexit:
    if (err != NE_OK)
    {
        if (pThis->tileIndexVector)
        {
            CSL_VectorDealloc(pThis->tileIndexVector);
            pThis->tileIndexVector = NULL;
        }

        if (index)
        {
            nsl_free(index);
            index = NULL;;
        }

        nsl_free(pThis);
        pThis = NULL;
    }

    *parameters = pThis;

    return err;
}


/* See header file for description. */
NB_DEF NB_Error
NB_VectorTileParametersAddTile(
    NB_VectorTileParameters* parameters,
    NB_VectorTileIndex* tile
    )
{
    NB_VectorTileIndex* index = NULL;

    if (!parameters || !tile)
    {
        return NE_INVAL;
    }

    index = nsl_malloc(sizeof(NB_VectorTileIndex));
    if (!index)
    {
        return NE_NOMEM;
    }
    *index = *tile;
    if (!CSL_VectorAppend(parameters->tileIndexVector, &index))
    {
        nsl_free(index);
        return NE_NOMEM;
    }

    return NE_OK;
}


/* See header file for description. */
NB_DEF NB_Error 
NB_VectorTileParametersDestroy(
    NB_VectorTileParameters* parameters
    )
{
    int count = 0;
    int i = 0;

    if (!parameters)
    {
        return NE_INVAL;
    }

    count = CSL_VectorGetLength(parameters->tileIndexVector);
    for (i = 0; i < count; i++)
    {
        void* p = 0;
        CSL_VectorGet(parameters->tileIndexVector, i, &p);
        nsl_free(p);
    }
    CSL_VectorDealloc(parameters->tileIndexVector);
    parameters->tileIndexVector = NULL;

    nsl_free(parameters);
    parameters = NULL;

    return NE_OK;
}


tpselt
NB_VectorTileParametersToTPSQuery(NB_VectorTileParameters* pThis, data_vector_tile_query* pvq)
{
    NB_Error err = NE_OK;
    int numberOfTilesRequested = 0;
    int i = 0;

    if (!pThis)
    {
        return 0;
    }

    err = data_vector_tile_query_init(NB_ContextGetDataState(pThis->context), pvq);
    if (err != NE_OK)
    {
        return 0;
    }

    // Add all the tiles from the parameters
    numberOfTilesRequested = CSL_VectorGetLength(pThis->tileIndexVector);
    for (i = 0; i < numberOfTilesRequested; i++)
    {
        NB_VectorTileIndex* index = 0;
        CSL_VectorGet(pThis->tileIndexVector, i, &index);
        err = data_vector_tile_add_descriptor(NB_ContextGetDataState(pThis->context), pvq, index->x, index->y, pThis->config.zoomLevel, pThis->config.minPpriority, pThis->config.maxPriority, pThis->config.minScale);
        if (err != NE_OK)
        {        
            return 0;
        }
    }

    if (pThis->config.passes == 0)
    {
        return 0;
    }
    pvq->passes = pThis->config.passes;

    return data_vector_tile_query_to_tps(NB_ContextGetDataState(pThis->context), pvq);
}


NB_Error
NB_VectorTileParametersGetConfigurationParameters(
    NB_VectorTileParameters* pThis,
    NB_VectorTileParametersConfiguration** config)
{
    if (!pThis || !config)
    {
        return NE_INVAL;
    }

    *config = &pThis->config;
    return NE_OK;
}


/*! @} */

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

@file     vectortileinformation.c
@defgroup vectortileinformation Vector Map Implementation

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
#include "nbvectortileinformation.h"
#include "nbvectortileinformationprivate.h"

#include "vec.h"
#include "data_vector_tile_reply.h"
#include "datautil.h"

/*! @{ */

// Local Types ...................................................................................

struct _NB_VectorTileInformation
{
	NB_Context*	context;
    struct CSL_Vector*	tilesVector;        /*!< Stores NB_VectorTile objects */
	data_vector_tile_reply reply;
};



// Private functions .............................................................................


// Public functions ..............................................................................

/* See header file for description. */
NB_DEF NB_Error 
NB_VectorTileInformationGetTile(
    NB_VectorTileInformation* pThis,
	uint32 tileIndex,
    NB_VectorTile** tile
    )
{
	if (!pThis || !tile)
	{
		return NE_INVAL;
	}

	CSL_VectorGet(pThis->tilesVector, tileIndex, tile);
    if (!*tile)
    {
        return NE_UNEXPECTED;
    }

	// Once the tile has been retrieved, it's removed
	// from the vector and its up to the callee to 
	// destroy the tile
    CSL_VectorRemove(pThis->tilesVector, tileIndex);

    return NE_OK;
}    

/* See header file for description. */
NB_DEF NB_Error 
NB_VectorTileInformationDestroyTile(
	NB_VectorTile* tile
	)
{
	if (!tile)
	{
		return NE_INVAL;
	}

	VectorTileDestroy(tile);

	return NE_OK;
}

/* See header file for description. */
NB_DEF NB_Error
NB_VectorTileInformationGetTileCount(
    NB_VectorTileInformation* information,
    uint32* count
    )
{
	if (!information)
	{
		return NE_INVAL;
	}

	*count = CSL_VectorGetLength(information->tilesVector);
    return NE_OK;
}    

/* See header file for description. */
NB_DEF NB_Error 
NB_VectorTileInformationDestroy(
    NB_VectorTileInformation* information
    )
{
	int count = 0;
	int i = 0;

	if (!information)
	{
		return NE_INVAL;
	}

	count = CSL_VectorGetLength(information->tilesVector);
	for (i = 0; i < count; i++)
	{
		NB_VectorTile* tile = NULL;
		CSL_VectorGet(information->tilesVector, i, &tile);
		VectorTileDestroy(tile);
	}
	CSL_VectorDealloc(information->tilesVector);
	nsl_free(information);
	information = NULL;

    return NE_OK;
}    


// ...............................................................................................    

NB_Error
NB_VectorTileInformationCreate(
	NB_Context* context,
    uint32 zoomLevel,
	uint32 passes,
	tpselt reply,
	NB_VectorTileInformation** information)
{
    NB_Error err = NE_OK;
	NB_VectorTileInformation* pThis = NULL;
	data_vector_tile_reply vtr = {0};
	data_vector_tile*      pdvt = NULL;
    VecTile_t* pVecTile_t = NULL;
    NB_VectorTile* vectorTile = NULL;
	int numTiles = 0;
	int i = 0;

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
    pThis->tilesVector = CSL_VectorAlloc(sizeof(NB_VectorTile*));
    if (pThis->tilesVector == NULL)
    {
        nsl_free(pThis);
        return NE_NOMEM;
    }
    
	err = data_vector_tile_reply_init(NB_ContextGetDataState(context), &vtr);
	err = data_vector_tile_reply_from_tps(NB_ContextGetDataState(context), &vtr, reply, passes);
	
	if (vtr.vec_vector_tiles != NULL)
	{
	    numTiles = CSL_VectorGetLength(vtr.vec_vector_tiles);
	}
	
    for (i = 0; i < numTiles; i++) 
    {
        pdvt = (data_vector_tile*) CSL_VectorGetPointer(vtr.vec_vector_tiles, i);

        pVecTile_t = VectorTileCreate(context, zoomLevel, pdvt, TRUE);
        if (pVecTile_t == NULL)
        {
            return NE_NOMEM;
        }

        vectorTile = VectorTileDecode(pVecTile_t);
        if (!vectorTile)
        {
            return NE_NOMEM;
        }

        if (!CSL_VectorAppend(pThis->tilesVector, &vectorTile))
		{
			return NE_NOMEM;
		}
    }

	// free the vector immediately
	if (vtr.vec_vector_tiles != NULL)
    {
		VEC_DEALLOC_FSA(&NB_ContextGetDataState(context)->vec_heap, vtr.vec_vector_tiles);
    }
	vtr.vec_vector_tiles = NULL;


	data_vector_tile_reply_free(NB_ContextGetDataState(context), &vtr);

	*information = pThis;

	return NE_OK;
}

	




/*! @} */

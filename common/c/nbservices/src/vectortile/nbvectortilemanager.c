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

@file     nbvectortilemanager.h
@defgroup nbvectortilemanager vector tile manager implementation

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
#include "nbvectortilehandler.h"
#include "nbvectortileparameters.h"
#include "nbvectortilemanager.h"
#include "nbqalog.h"

#include "csdict.h"
#include "palclock.h"
#include "vec.h"

/*! @{ */

// Constants .....................................................................................

#define INITIAL_DICTIONARY_SIZE                 64
#define MAX_TILES                               128
#define MAX_PENDING_TILES                       32
#define MAX_DOWNLOADING_TILES                   32
#define MAX_CONCURRENT_HANDLERS                 4
#define ALLOW_TILE_FETCH                        TRUE

#define VTM_KEY_LEN	                            100

#define NB_VECTOR_TILE_MANAGER_MAX_CALLBACKS    16

// Local types ...................................................................................

typedef struct  
{
    NB_VectorTileHandler* handler;
    nb_boolean  active;
} VectorTileManagerHandler;

typedef struct
{
    nb_unixTime		time;
    NB_VectorTile*  vectorTile;
    int             lockCount;
} VectorTileRecord;


struct find_oldest_data 
{
	NB_VectorTileManager*   vtm;
	VectorTileRecord*       oldest;
	VectorTileRecord*       oldest_unlocked;
	char                    key[VTM_KEY_LEN];
	char                    unlocked_key[VTM_KEY_LEN];
};


typedef struct 
{
	nb_unixTime			        time;
	uint32				        tx;
	uint32				        ty;
	uint32				        tz;

    NB_VectorTileHandler*       handler;

} VectorTileRequestRecord;

typedef struct 
{
    NB_VectorTileManagerCallback    callback;
    void*                           userData;
} NB_VectorTileManagerCallbackRecord;

struct _NB_VectorTileManager
{
    NB_Context*         context;
    NB_VectorTileManagerConfiguration config;

    NB_VectorTileConfiguration tileConfig;

    struct CSL_Dictionary*		tiles;
    struct CSL_Dictionary*		pendingTiles;
    struct CSL_Dictionary*		downloadingTiles;

    int                         maxPendingTilesOverride;
    int                         lockedTileCount;

   	uint32						discardedTiles;
	uint32	    				lastDataSize;
	uint32		    			totalDataSize;

    nb_gpsTime		    		lastQueryGPSTime;
	nb_gpsTime					lastReplyGPSTime;

    
    VectorTileManagerHandler* handlers;             /*!< Pointer to array of handlers */
    NB_RequestHandlerCallback handlerCallback;
       
    NB_VectorTileManagerCallbackRecord  callbacks[NB_VECTOR_TILE_MANAGER_MAX_CALLBACKS]; 
    int                                 numberOfCallbacks;
};



// Private functions .............................................................................

static NB_Error Initialize(NB_VectorTileManager* pThis, NB_VectorTileManagerConfiguration* configuration);
static NB_Error AddTile(NB_VectorTileManager* pThis, NB_VectorTile* tile);
static NB_Error AddPendingTile(NB_VectorTileManager* pThis, uint32 tx, uint32 ty, uint32 tz);
static NB_Error AddDownloadingTile(NB_VectorTileManager* pThis, uint32 tx, uint32 ty, uint32 tz, NB_VectorTileHandler* handler);
static NB_Error RemoveFromDownloadingList(NB_VectorTileManager* pThis, uint32 tx, uint32 ty, uint32 tz);
static NB_Error RemoveOldest(NB_VectorTileManager* pThis);
static NB_Error SetDownloading(NB_VectorTileManager* pThis, NB_VectorTileHandler* handler);
static void		ClearTiles(NB_VectorTileManager* pThis);
static VectorTileManagerHandler* FindFreeHandler(NB_VectorTileManager* pThis);
static VectorTileManagerHandler* GetVectorTileManagerHandler(NB_VectorTileManager* pThis, NB_VectorTileHandler* tileHandler);
static uint32   GetNumberOfActiveHandlers(NB_VectorTileManager* pThis);
static void     VectorTileManagerFree(NB_VectorTileManager* pThis);
static void     GetKey(int tx, int ty, int tz, char* key);
static int		FindOldestFunctionCallback(void* userdata, const char* key, const char *value, size_t vlen);
static int      ClearTilesIteratorFunction(void* userdata, const char* key, const char *value, size_t vlen);
static int		PendingTilesIteratorFunction(void* userdata, const char* key, const char *value, size_t vlen);
static int		SetDownloadingTilesIteratorFunction(void* userdata, const char* key, const char *value, size_t vlen);
static int		ResetDownloadingTilesIteratorFunction(void* userdata, const char* key, const char *value, size_t vlen);
static void     VectorTileHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
static void     QaLogTileState(NB_VectorTileManager* pThis);

// Public Functions ..............................................................................

/* See header file for description. */
NB_DEF NB_Error
NB_VectorTileManagerCreate(
    NB_Context* context,
    NB_VectorTileConfiguration* tileConfiguration,
    NB_VectorTileManagerConfiguration* managerConfiguration,
    NB_VectorTileManager** vectorTileManager
    )
{
    NB_Error err = NE_OK;
    NB_VectorTileManager* pThis = NULL;

    uint32 i = 0;
    
    if (!context || !tileConfiguration || /*!callback ||*/ !vectorTileManager)
    {
        return NE_INVAL;
    }
    
    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }          
    pThis->context = context;
    
    if (Initialize(pThis, managerConfiguration) != NE_OK)
    {
        return NE_NOMEM;
    }
    
    for (i=0; i<NB_VECTOR_TILE_MANAGER_MAX_CALLBACKS; i++)
    {
        pThis->callbacks[i].callback = NULL;
        pThis->callbacks[i].userData = NULL;
    }

    pThis->numberOfCallbacks = 0;
            
    pThis->handlerCallback.callback = VectorTileHandlerCallback;
    pThis->handlerCallback.callbackData = pThis;
    
    pThis->tileConfig = *tileConfiguration;

    pThis->maxPendingTilesOverride = -1; /* This value is a temporary override of the configured value */

    pThis->discardedTiles = 0;
	pThis->lastDataSize = 0;
	pThis->totalDataSize = 0;

    pThis->lastQueryGPSTime = PAL_ClockGetGPSTime();
	pThis->lastReplyGPSTime = PAL_ClockGetGPSTime();

    // Create handlers
    for (i = 0; i < pThis->config.maxConcurrentHandlers; i++)
    {
        err = NB_VectorTileHandlerCreate(pThis->context, &pThis->handlerCallback, &pThis->handlers[i].handler);
        if (err != NE_OK)
        {
            return NE_NOMEM;
        }
    }
        
    *vectorTileManager = pThis;

    return NE_OK;
}

NB_DEF NB_Error
NB_VectorTileManagerRegisterCallback(
    NB_VectorTileManager* vectorTileManager,
    NB_VectorTileManagerCallback callback,
    void* callbackUserData)
{
    int i = 0;

    if (vectorTileManager->numberOfCallbacks >= NB_VECTOR_TILE_MANAGER_MAX_CALLBACKS)
    {
        // too many callbacks are registered -- this is VERY unlikely
        return NE_BUSY; 
    }

    for (i=0; i<vectorTileManager->numberOfCallbacks; i++)
    {
        if ((vectorTileManager->callbacks[i].callback == callback) &&
            (vectorTileManager->callbacks[i].userData == callbackUserData))
        {
            return NE_EXIST;
        }
    }

    vectorTileManager->callbacks[vectorTileManager->numberOfCallbacks].callback = callback;
    vectorTileManager->callbacks[vectorTileManager->numberOfCallbacks].userData = callbackUserData;

    vectorTileManager->numberOfCallbacks++;

    return NE_OK;
}

NB_DEF NB_Error
NB_VectorTileManagerRemoveCallback(
    NB_VectorTileManager* vectorTileManager,
    NB_VectorTileManagerCallback callback,
    void* callbackUserData)
{
    int i = 0;
    nb_boolean found = FALSE;

    for (i=0; i<vectorTileManager->numberOfCallbacks; i++)
    {
        if ((vectorTileManager->callbacks[i].callback == callback) &&
            (vectorTileManager->callbacks[i].userData == callbackUserData))
        {
            found = TRUE;
        }

        if (found)
        {
            if (i<(vectorTileManager->numberOfCallbacks-1))
            {
                vectorTileManager->callbacks[i].callback = vectorTileManager->callbacks[i+1].callback;
                vectorTileManager->callbacks[i].userData = vectorTileManager->callbacks[i+1].userData;
            }
            else
            {
                vectorTileManager->callbacks[i].callback = NULL;
                vectorTileManager->callbacks[i].userData = NULL;
            }
        }
    }

    if (!found)
    {
        return NE_NOENT;
    }
    
    vectorTileManager->numberOfCallbacks--;

    return NE_OK;
}

/* See header file for description. */
NB_DEF NB_Error
NB_VectorTileManagerGetTile(
    NB_VectorTileManager* pThis,
    NB_VectorTileIndex* tileIndex,
    NB_VectorTile** tile,
    nb_boolean download
    )
{    
    NB_Error err = NE_OK;
    char key[VTM_KEY_LEN];
    VectorTileRecord* rec = NULL;
    size_t size = 0;
    
    // Check to see if we already have the tile
    GetKey(tileIndex->x, tileIndex->y, pThis->tileConfig.mapZoomLevel, key);
    rec = (VectorTileRecord*)CSL_DictionaryGet(pThis->tiles, key, &size);
    
    if (rec == NULL)
    {
        if (pThis->config.allowTileFetch && download)
        {
			err = AddPendingTile(pThis, tileIndex->x, tileIndex->y, pThis->tileConfig.mapZoomLevel); 
			if (err != NE_OK)
			{
				goto error;
			}
        }
        
        err = NE_NOENT;
		goto error;
    }
    else
    {
        if (rec->lockCount == 0)
        {
            pThis->lockedTileCount++;
        }

		rec->lockCount++;
		*tile = rec->vectorTile;
        rec->time = PAL_ClockGetUnixTime();  // update the time stamp
		err = NE_OK;
    }   

error:
	if (err != NE_OK)
	{
		*tile = NULL;
	}
    return err;
}


/* See header file for description. */
NB_DEF NB_Error
NB_VectorTileManagerDownloadPendingTiles(
    NB_VectorTileManager* pThis
    )
{
	NB_Error err = NE_OK;
	VectorTileManagerHandler* vtmHandler = NULL;
	NB_VectorTileParametersConfiguration config = {0};
	NB_VectorTileParameters* parameters = 0;
	struct CSL_Vector* tileIndexesVector = NULL;
	int i = 0;

	vtmHandler = FindFreeHandler(pThis);
	if (vtmHandler == NULL)
	{
		return NE_BUSY;
	}

	if (CSL_DictionaryLength(pThis->pendingTiles) == 0)
	{
		return NE_UNEXPECTED;
	}

	// Specify parameter configuration
	NB_VectorTileParametersConfigurationSetDefaults(&config);
	config.passes = pThis->tileConfig.mapPasses;
	config.zoomLevel = pThis->tileConfig.mapZoomLevel;

	// Create the request parameters
	tileIndexesVector = CSL_VectorAlloc(sizeof(NB_VectorTileIndex));
	if (tileIndexesVector == NULL)
	{
		err = NE_NOMEM;
		goto error;
	}
	CSL_DictionaryIteration(pThis->pendingTiles, PendingTilesIteratorFunction, &tileIndexesVector);
	if (CSL_VectorGetLength(tileIndexesVector) == 0)
	{
		err = NE_UNEXPECTED;
		goto error;
	}

	for (i = 0; i < CSL_VectorGetLength(tileIndexesVector); i++)
	{
		NB_VectorTileIndex tile = {0};
		CSL_VectorGet(tileIndexesVector, i, &tile);

		if (i == 0)
		{
			err = NB_VectorTileParametersCreate(pThis->context, &config, &tile, &parameters);
			if (err != NE_OK)
			{
				goto error;
			}
		}
		else
		{
			err = NB_VectorTileParametersAddTile(parameters, &tile);
			if (err != NE_OK)
			{
				goto error;
			}
		}
	}

	// Start the request
	err = NB_VectorTileHandlerStartRequest(vtmHandler->handler, parameters);
	vtmHandler->active = TRUE;

	// Mark pending tiles as downloading
	err = SetDownloading(pThis, vtmHandler->handler);

    QaLogTileState(pThis);

error:
	if (parameters)
	{
		NB_VectorTileParametersDestroy(parameters);
		parameters = NULL;
	}

	if (tileIndexesVector)
	{
		CSL_VectorDealloc(tileIndexesVector);
		tileIndexesVector = NULL;
	}

    return err;
}

/* See header file for description. */
NB_DEF NB_Error
NB_VectorTileManagerUnlockTile(
	NB_VectorTileManager* pThis,								
    NB_VectorTile* tile
    )
{
    char key[VTM_KEY_LEN];
    VectorTileRecord* rec = NULL;
    size_t size = 0;
    
    // Get tile record
	GetKey(tile->x, tile->y, tile->zoomLevel, key);
    rec = (VectorTileRecord*)CSL_DictionaryGet(pThis->tiles, key, &size);
    
    if (rec)
    {
		rec->lockCount--;
		if (rec->lockCount == 0)
		{
            pThis->lockedTileCount--;
		}
	}

    return NE_OK;
}

NB_DEF NB_Error
NB_VectorTileManagerAddVectorTileInformation(
    NB_VectorTileManager* vectorTileManager,
    NB_VectorTileInformation* vectorTileInformation
    )
{
    NB_Error    err = NE_OK;
    uint32      i;
    uint32      count;
	NB_VectorTile* tile = NULL;

	err = NB_VectorTileInformationGetTileCount(vectorTileInformation, &count);

	if (err != NE_OK)
	{
		goto error;
	}

	for (i = 0; i < count; i++)
	{
		err = NB_VectorTileInformationGetTile(vectorTileInformation, 0, &tile);

		if (err != NE_OK)
		{
			goto error;
		}

		RemoveFromDownloadingList(vectorTileManager, tile->x, tile->y, tile->zoomLevel);

        NB_QaLogReceiveVectorTile(vectorTileManager->context, NB_RECEIVE_TILE_INITIAL, tile->x, tile->y, tile->zoomLevel);

		err = AddTile(vectorTileManager, tile);

        if (err != NE_OK)
		{
			goto error;
		}

        tile = NULL;
	}

error:

    if (tile != NULL)
    {
        NB_VectorTileInformationDestroyTile(tile);
        tile = NULL;
    }

	if (vectorTileInformation != NULL)
	{
		NB_VectorTileInformationDestroy(vectorTileInformation);
		vectorTileInformation = NULL;
	}

    return err;
}

NB_DEF void
NB_VectorTileManagerSetMaximumPendingTilesOverride(
    NB_VectorTileManager* vectorTileManager, 
    int maximumPendingTiles
    )
{
    vectorTileManager->maxPendingTilesOverride = maximumPendingTiles;
}	

NB_DEF int
NB_VectorTileManagerGetMaximumPendingTilesOverride(
    NB_VectorTileManager* vectorTileManager
    )
{
	return vectorTileManager->maxPendingTilesOverride;
}

NB_DEF int
NB_VectorTileManagerGetNumberOfUnlockedSlots(
    NB_VectorTileManager* vectorTileManager
    )
{
	// Count a pending download as an unavailable slot
	return ((int)vectorTileManager->config.maxTiles) 
        - vectorTileManager->lockedTileCount 
        - CSL_DictionaryLength(vectorTileManager->pendingTiles) 
        - CSL_DictionaryLength(vectorTileManager->downloadingTiles);
}

NB_DEF int
NB_VectorTileManagerGetNumberOfPendingSlots(
    NB_VectorTileManager* vectorTileManager
    )
{
    int num = 0;

    if (vectorTileManager->maxPendingTilesOverride > 0)
    {
        num =  vectorTileManager->maxPendingTilesOverride - CSL_DictionaryLength(vectorTileManager->pendingTiles);
    }
    else
    {
        num = MAX_PENDING_TILES - CSL_DictionaryLength(vectorTileManager->pendingTiles);
    }

    if (num < 0)
    {
        num = 0;
    }

    return num;
}

NB_DEF void
NB_VectorTileManagerGetStatistics(
    NB_VectorTileManager* vectorTileManager, 
    uint32* totalNumberOfTiles, 
    uint32* numberOfValidTiles, 
    uint32* numberOfPendingTiles, 
    uint32* numberOfDownloadingTiles, 
    uint32* numberOfDiscardedTiles,
	uint32* totalDataSize, 
    uint32* lastDataSize, 
    uint32* numberOfActiveQueries, 
    nb_gpsTime* lastQueryGPSTime, 
    nb_gpsTime* lastReplyGPSTime
    )
{
    uint32 numberOfValidTilesLocal          = CSL_DictionaryLength(vectorTileManager->tiles);
    uint32 numberOfPendingTilesLocal        = CSL_DictionaryLength(vectorTileManager->pendingTiles);
    uint32 numberOfDownloadingTilesLocal    = CSL_DictionaryLength(vectorTileManager->downloadingTiles);
    uint32 totalNumberOfTilesLocal          = numberOfValidTilesLocal + numberOfPendingTilesLocal + numberOfDownloadingTilesLocal;

	if (totalNumberOfTiles)
		*totalNumberOfTiles = totalNumberOfTilesLocal;

	if (numberOfValidTiles)
		*numberOfValidTiles = numberOfValidTilesLocal;

	if (numberOfPendingTiles)
		*numberOfPendingTiles = numberOfPendingTilesLocal;

	if (numberOfDownloadingTiles)
		*numberOfDownloadingTiles = numberOfDownloadingTilesLocal;

	if (numberOfDiscardedTiles)
		*numberOfDiscardedTiles = vectorTileManager->discardedTiles;

	if (totalDataSize)
		*totalDataSize = vectorTileManager->totalDataSize;

	if (lastDataSize)
		*lastDataSize = vectorTileManager->lastDataSize;

	if (numberOfActiveQueries)
		*numberOfActiveQueries = GetNumberOfActiveHandlers(vectorTileManager);

	if (lastQueryGPSTime)
		*lastQueryGPSTime = vectorTileManager->lastQueryGPSTime;

	if (lastReplyGPSTime)
		*lastReplyGPSTime = vectorTileManager->lastReplyGPSTime;
}


/* See header file for description. */
NB_DEF NB_Error
NB_VectorTileManagerDestroy(
    NB_VectorTileManager* pThis
    )
{
    VectorTileManagerFree(pThis);
    return NE_OK;
}

/* See header file for description. */
NB_DEF void
NB_VectorTileManager_GetDefaultConfiguration(
    NB_VectorTileManagerConfiguration* pThis
    )
{   
    // Tile manager operating limits   
    // FUTURE: Get this from configuration someday 
    pThis->initialDictionarySize    = INITIAL_DICTIONARY_SIZE;
    pThis->maxTiles                 = MAX_TILES;
    pThis->maxPendingTiles          = MAX_PENDING_TILES;
    pThis->downloadingTiles         = MAX_DOWNLOADING_TILES;
    pThis->maxConcurrentHandlers    = MAX_CONCURRENT_HANDLERS;
    pThis->allowTileFetch           = ALLOW_TILE_FETCH;
    pThis->foregroundOnly           = FALSE;
} 

/* See header file for description. */
NB_DEC NB_VectorTileManagerConfiguration*
NB_VectorTileManagerGetConfiguration(
    NB_VectorTileManager* pThis
    )
{
    if (pThis)
    {
        return &pThis->config;
    }
    return NULL;
}

// Private functions .............................................................................

static void
TileDeleteFunction(void* userdata, const char* key, const char *value, size_t valueLength)
{
    VectorTileRecord* rec = (VectorTileRecord*) value;

    nsl_assert(rec->lockCount == 0);
    nsl_assert(valueLength == sizeof(VectorTileRecord));

    NB_VectorTileInformationDestroyTile(rec->vectorTile);
    rec->vectorTile = NULL;
}

static NB_Error     
Initialize(
    NB_VectorTileManager* pThis,
    NB_VectorTileManagerConfiguration* configuration
    )
{   
    NB_Error err = NE_OK;
    
    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->lockedTileCount = 0;
    
    if (configuration)
    {
        pThis->config = *configuration;
    }
    else
    {
        NB_VectorTileManager_GetDefaultConfiguration(&pThis->config);
    }
    
    // Number of concurrent handlers at any one time
    pThis->handlers = nsl_malloc(sizeof(VectorTileManagerHandler) * pThis->config.maxConcurrentHandlers);
    if (pThis->handlers == NULL)
    {
        err = NE_NOMEM;
        goto error;
    }
    nsl_memset(pThis->handlers, 0, sizeof(VectorTileManagerHandler) * pThis->config.maxConcurrentHandlers);
    
    pThis->tiles = CSL_DictionaryAllocEx(pThis->config.initialDictionarySize, TileDeleteFunction, pThis);
    if (pThis->tiles == NULL)
    {
        err =  NE_NOMEM;
        goto error;
    }
    
    pThis->pendingTiles = CSL_DictionaryAlloc(pThis->config.initialDictionarySize);
    if (pThis->pendingTiles == NULL)
    {
        err = NE_NOMEM;
        goto error;
    }
    
    pThis->downloadingTiles = CSL_DictionaryAlloc(pThis->config.initialDictionarySize);
    if (pThis->downloadingTiles == NULL)
    {
        err = NE_NOMEM;
        goto error;
    }
  
error:    
    
    if (err != NE_OK)
    {
        VectorTileManagerFree(pThis);
    }
    
    return err;
}    

static void 
VectorTileManagerFree(
    NB_VectorTileManager* pThis)
{
    uint32 i = 0;
    
    // Cancel all outstanding requests
    for (i =0; i < pThis->config.maxConcurrentHandlers; i++)
    {
        if (pThis->handlers[i].handler != NULL)
        {
            NB_VectorTileHandlerDestroy(pThis->handlers[i].handler);
            pThis->handlers[i].active = FALSE;       
        }
    }    
    
	if (pThis->tiles != NULL)
    {
        ClearTiles(pThis);
        CSL_DictionaryDealloc(pThis->tiles);
    }
    
    if (pThis->pendingTiles != NULL)
    {
        CSL_DictionaryDealloc(pThis->pendingTiles);
    }
    
    if (pThis->downloadingTiles != NULL)
    {
        CSL_DictionaryDealloc(pThis->downloadingTiles);
    }

	if (pThis->handlers)
	{
		nsl_free(pThis->handlers);
	}
  
    nsl_free(pThis);
    pThis = NULL;    
}


static NB_Error 
AddTile(NB_VectorTileManager* pThis, NB_VectorTile* tile)
{
	NB_Error err = NE_OK;
    char key[VTM_KEY_LEN];
	VectorTileRecord rec = {0};

	while (((CSL_DictionaryLength(pThis->tiles)+1) - (int) pThis->config.maxTiles) > 0 && err == NE_OK)
	{
		err = RemoveOldest(pThis);
    }

    if (err != NE_OK)
    {
        /* Couldn't make space for the new tile */
        return err;
    }

	GetKey(tile->x, tile->y, tile->zoomLevel, key);
	rec.vectorTile = tile;
	rec.lockCount = 0;
	rec.time = PAL_ClockGetUnixTime();

	if (!CSL_DictionarySet(pThis->tiles, key, (const char*)&rec, sizeof(rec)))
	{
		NB_VectorTileInformationDestroyTile(tile);		
		err = NE_NOMEM;
	}

	DEBUGLOG(LOG_SS_NAVUI, LOG_SEV_INFO, ("Adding Tile: %d, %d, %d err=%d", tile->x, tile->y, tile->zoomLevel, err));
	
	return err;
}


static NB_Error 
AddPendingTile(NB_VectorTileManager* pThis, uint32 tx, uint32 ty, uint32 tz)
{
    NB_Error err = NE_OK;
    char key[VTM_KEY_LEN];
	VectorTileRequestRecord rec = {0};

	GetKey(tx, ty, tz, key);

	if (CSL_DictionaryGet(pThis->pendingTiles, key, NULL) != NULL) 
    {
		DEBUGLOG(LOG_SS_NAVUI, LOG_SEV_INFO, ("Add Pending Tile - Tile Already Pending: %d, %d, %d", tx, ty, tz));
		return err;
	}

	if (CSL_DictionaryGet(pThis->downloadingTiles, key, NULL) != NULL) 
    {
		DEBUGLOG(LOG_SS_NAVUI, LOG_SEV_INFO, ("Add Pending Tile - Tile Already Downloading: %d, %d, %d", tx, ty, tz));
		return err;
	}

	if (pThis->maxPendingTilesOverride > 0)
    {
        if (CSL_DictionaryLength(pThis->pendingTiles) >= pThis->maxPendingTilesOverride)
        {
		    return NE_AGAIN;
        }
    }
    else if (CSL_DictionaryLength(pThis->pendingTiles) >= MAX_PENDING_TILES)
    {
        return NE_AGAIN;
    }

	rec.time = PAL_ClockGetUnixTime();
	rec.tx = tx;
	rec.ty = ty;
	rec.tz = tz;
    rec.handler = NULL;

	if (!CSL_DictionarySet(pThis->pendingTiles, key, (const char*)&rec, sizeof(rec)))
    {
		err = NE_NOMEM;
    }

	return err;
}


static NB_Error 
AddDownloadingTile(NB_VectorTileManager* pThis, uint32 tx, uint32 ty, uint32 tz, NB_VectorTileHandler* handler)
{
    NB_Error err = NE_OK;
    char key[VTM_KEY_LEN];
	VectorTileRequestRecord rec = {0};

	GetKey(tx, ty, tz, key);

	if (CSL_DictionaryGet(pThis->downloadingTiles, key, NULL) != NULL) 
    {
		DEBUGLOG(LOG_SS_NAVUI, LOG_SEV_INFO, ("Add Downloading Tile - Tile Already Downloading: %d, %d, %d", tx, ty, tz));
		return err;
	}

    NB_QaLogRequestVectorTile(pThis->context, tx, ty, tz);

	rec.time = PAL_ClockGetUnixTime();
	rec.tx = tx;
	rec.ty = ty;
	rec.tz = tz;
    rec.handler = handler;

	if (!CSL_DictionarySet(pThis->downloadingTiles, key, (const char*)&rec, sizeof(rec)))
    {
		err = NE_NOMEM;
    }

	return err;
}



static VectorTileManagerHandler* 
FindFreeHandler(NB_VectorTileManager* pThis)
{
	uint32 i = 0;

	for (i = 0; i < pThis->config.maxConcurrentHandlers; i++)
	{
		if (!pThis->handlers[i].active)
		{
			return &pThis->handlers[i];
		}
	}

	return NULL;
}

static VectorTileManagerHandler*
GetVectorTileManagerHandler(NB_VectorTileManager* pThis, NB_VectorTileHandler* tileHandler)
{
	uint32 i = 0;

	for (i = 0; i < pThis->config.maxConcurrentHandlers; i++)
	{
        if (pThis->handlers[i].handler == tileHandler)
		{
			return &pThis->handlers[i];
		}
	}

	return NULL;

}

static uint32
GetNumberOfActiveHandlers(NB_VectorTileManager* pThis)
{
	uint32 i = 0;
    uint32 n = 0;

	for (i = 0; i < pThis->config.maxConcurrentHandlers; i++)
	{
        if (pThis->handlers[i].active)
		{
			n++;
		}
	}

	return n;
}


static void 
ClearTiles(NB_VectorTileManager* pThis)
{
    (void) CSL_DictionaryIteration(pThis->tiles, ClearTilesIteratorFunction, pThis);
}

struct SetDownloadingTilesData
{
    NB_VectorTileManager* vtm;
    NB_VectorTileHandler* handler;
};

static NB_Error
SetDownloading(NB_VectorTileManager* pThis, NB_VectorTileHandler* handler)
{
    struct SetDownloadingTilesData data;

    data.vtm = pThis;
    data.handler = handler;

	(void) CSL_DictionaryIteration(pThis->pendingTiles, SetDownloadingTilesIteratorFunction, &data);

	return NE_OK;
}

static NB_Error
ResetDownloading(NB_VectorTileManager* pThis, NB_VectorTileHandler* handler)
{
    struct SetDownloadingTilesData data;

    data.vtm = pThis;
    data.handler = handler;

	(void) CSL_DictionaryIteration(pThis->downloadingTiles, ResetDownloadingTilesIteratorFunction, &data);

	return NE_OK;
}

static NB_Error 
RemoveFromDownloadingList(NB_VectorTileManager* pThis, uint32 tx, uint32 ty, uint32 tz)
{
	char key[VTM_KEY_LEN];

	GetKey(tx, ty, tz, key);
	CSL_DictionaryDelete(pThis->downloadingTiles, key);

	return NE_OK;
}


static int
FindOldestFunctionCallback(void* userdata, const char* key, const char *value, size_t vlen)
{
	struct find_oldest_data* pdata = (struct find_oldest_data*) userdata;
	VectorTileRecord* rec = (VectorTileRecord*) value;
    
	if (pdata->oldest == NULL || rec->time < pdata->oldest->time) 
    {
		pdata->oldest = rec;
		nsl_strlcpy(pdata->key, key, sizeof(pdata->key));
	}

    if (rec->lockCount == 0)    // unlocked tiles
    {
        if ((pdata->oldest_unlocked == NULL) || (rec->time < pdata->oldest_unlocked->time))
        {
            pdata->oldest_unlocked = rec;
            nsl_strlcpy(pdata->unlocked_key, key, sizeof(pdata->unlocked_key));
        }
    }

	return 1;
}


static NB_Error 
RemoveOldest(NB_VectorTileManager* pThis)
{
	struct find_oldest_data oldest_data = {0};
    
	if (CSL_DictionaryLength(pThis->tiles) == 0)
    {
		return NE_OK;
    }
    
	// find the oldest tile
	oldest_data.vtm = pThis;
	(void) CSL_DictionaryIteration(pThis->tiles, FindOldestFunctionCallback, &oldest_data);
    
	if (oldest_data.oldest_unlocked != NULL) 
    {       
        pThis->discardedTiles++;

        NB_QaLogDiscardVectorTile(pThis->context, oldest_data.oldest->vectorTile->x, 
            oldest_data.oldest->vectorTile->y, pThis->tileConfig.mapZoomLevel);

		CSL_DictionaryDelete(pThis->tiles, oldest_data.unlocked_key);
	}
    /* Never delete a locked tile since the user is counting on it remaining in memory
	else if (oldest_data.oldest != NULL) 
    {
        pThis->discardedTiles++;

        NB_QaLogDiscardVectorTile(pThis->context, oldest_data.oldest->vectorTile->x, 
            oldest_data.oldest->vectorTile->y, pThis->tileConfig.mapZoomLevel);
        
        CSL_DictionaryDelete(pThis->tiles, oldest_data.key);
	}
    */
	else
    {
		return NE_NOENT;
    }
    
	return NE_OK;
}

static int
SetDownloadingTilesIteratorFunction(void* userdata, const char* key, const char *value, size_t vlen)
{
	struct SetDownloadingTilesData* data = userdata;
	VectorTileRequestRecord* rec = (VectorTileRequestRecord*)value;

	AddDownloadingTile(data->vtm, rec->tx, rec->ty, rec->tz, data->handler);

    // delete the item
	return 0;
}

static int
ResetDownloadingTilesIteratorFunction(void* userdata, const char* key, const char *value, size_t vlen)
{
	struct SetDownloadingTilesData* data = userdata;
	VectorTileRequestRecord* rec = (VectorTileRequestRecord*)value;

	if (rec->handler == data->handler)
    {
        // delete the item
        return 0;
    }

	return 1;
}

static int 
ClearTilesIteratorFunction(void* userdata, const char* key, const char *value, size_t vlen)
{
    // delete the tile - the destroy function will free the tile data.
	return 0;
}


static int 
PendingTilesIteratorFunction(void* userdata, const char* key, const char *value, size_t vlen)
{
	struct CSL_Vector* tileIndexVector = *(struct CSL_Vector**)userdata;
	VectorTileRequestRecord* rec = (VectorTileRequestRecord*)value;
	NB_VectorTileIndex tile = {0};

	tile.x = rec->tx;
	tile.y = rec->ty;

	CSL_VectorAppend(tileIndexVector, &tile);


	return 1;
}

static void 
GetKey(int tx, int ty, int tz, char* key)
{
    nsl_sprintf(key, "%d,%d,%d", tx, ty, tz);
}

static void
VectorTileHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
 	NB_VectorTileInformation* info = NULL;
	NB_VectorTileManager* pThis = userData;
    VectorTileManagerHandler* vtmHandler = GetVectorTileManagerHandler(pThis, (NB_VectorTileHandler*)handler);
    int i;

    if ((!up) && (status != NB_NetworkRequestStatus_Progress))
    {

        if (status == NB_NetworkRequestStatus_Success)
        {
			NB_Error err = NE_OK;

			err = NB_VectorTileHandlerGetTileInformation(handler, &info);

			if (err != NE_OK)
			{
				goto error;
			}

            
			err = NB_VectorTileManagerAddVectorTileInformation(pThis, info);

            if (err != NE_OK)
            {
                /* Ignore any errors that happen while adding the tiles.  There isn't anything that we can do if there are no slots available due to all tiles being locked. */

                NB_QaLogAppState(pThis->context, "Error adding downloaded tile");
                err = NE_OK;
            }

            info = NULL; // this is taken over by NB_VectorTileManagerAddVectorTileInformation
        }

        /* Make sure any remaining tiles are removed from the downloading list */
        err = ResetDownloading(pThis, vtmHandler->handler);

		vtmHandler->active = FALSE;

    }

error:
	if (info != NULL)
	{
		NB_VectorTileInformationDestroy(info);
		info = NULL;
	}

    for (i=0; i<pThis->numberOfCallbacks; i++)
    {
        if (pThis->callbacks[i].callback)
        {
            (pThis->callbacks[i].callback)(pThis, err, err == NE_OK ? percent : -1, pThis->callbacks[i].userData);
        }
    }

    QaLogTileState(pThis);
}

static void
QaLogTileState(NB_VectorTileManager* pThis
    )
{   
    uint32      totalNumberOfTiles;
    uint32      numberOfValidTiles; 
    uint32      numberOfPendingTiles;
    uint32      numberOfDownloadingTiles;
    uint32      numberOfDiscardedTiles;
	uint32      totalDataSize;
    uint32      lastDataSize;
    uint32      numberOfActiveQueries;
    nb_gpsTime  lastQueryGPSTime;
    nb_gpsTime  lastReplyGPSTime;

    NB_VectorTileManagerGetStatistics(pThis, &totalNumberOfTiles, &numberOfValidTiles, 
        &numberOfPendingTiles, &numberOfDownloadingTiles, &numberOfDiscardedTiles,
	    &totalDataSize, &lastDataSize, &numberOfActiveQueries, &lastQueryGPSTime, &lastReplyGPSTime);

    NB_QaLogVectorTileState(pThis->context, totalNumberOfTiles, numberOfValidTiles, numberOfPendingTiles,
        numberOfDownloadingTiles, numberOfDiscardedTiles, lastDataSize, totalDataSize);
}

/*! @} */




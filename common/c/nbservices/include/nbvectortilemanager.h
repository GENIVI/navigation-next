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

/*!--------------------------------------------------------------------------

@file     nbvectortilemanager.h

This interface lets you retrieve vector tiles from the server. 

The vector tiles use Mercator Projection coordinates. To convert coordinates 
from/to latitude/longitude values use the spatial interface.

For a description of the Mercator Projections see:
http://en.wikipedia.org/wiki/Mercator_projection

To use the interface follow these steps.

* Set NB_VectorTileConfiguration to desired settings

* Create a VectorTileManager object by calling NB_VectorTileManagerCreate()

* Call NB_VectorTileManagerGetTile() to queue tiles for download (if they are cached
they will be returned immediately).

* Call NB_VectorTileManagerDownloadPendingTiles() to download all requested tiles.

* Call NB_VectorTileManagerGetTile() again for all tiles which were not retrieved
by the first call.

* Unlock all the tiles by calling NB_VectorTileManagerUnlockTile()

* Finally, call NB_VectorTileManagerDestroy() when you are finish with
the vector tile manager

*/
/*
(C) Copyright 2009 by Networks In Motion, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef VECTORTILEMANAGER_H
#define VECTORTILEMANAGER_H

#include "paltypes.h"
#include "nbexp.h"
#include "nbvectortile.h"
#include "nbvectortileinformation.h"

/*! 
	@addtogroup nbvectortilemanager
	@{ 
*/

// Public Types ..................................................................................

/*! @struct NB_VectorTileManager
A NB_VectorTileManager is used to get a vector tile 
*/
typedef struct _NB_VectorTileManager NB_VectorTileManager;

/*! @struct NB_VectorTileManagerConfiguration
A NB_VectorTileManagerConfiguration is used to configure VectorTileManager operating limits 
*/
typedef struct
{
    uint32              initialDictionarySize;
    uint32              maxTiles;
    uint32              maxPendingTiles;
    uint32              downloadingTiles;
    uint32              maxConcurrentHandlers;
    nb_boolean          allowTileFetch;
    nb_boolean          foregroundOnly;
} NB_VectorTileManagerConfiguration;

/*! VectorTileManager configuration. Has to be passed to NB_VectorTileManagerCreate(). */
typedef struct 
{
    /*! bit precision for coordinate calculation. Allowed values are between 1 and 16. Suggested value is 8. */
    int mapPasses;

    /*! Zoom level from 0 to 17. Currently only zoom levels 14 and 17 are supported. */
    int mapZoomLevel;

} NB_VectorTileConfiguration;


/*! Callback function used for tile download.

Pass a pointer to this callback function to NB_VectorTileManagerCreate(). It will show the progress of
tile downloads.

@param vectorTileManager A pointer to VectorTileManager instance.
@param result Error code for download.
@param percent Percentage of tile download. -1 on error 
@param userCallbackData User data passed into NB_VectorTileManager()
*/
typedef void (*NB_VectorTileManagerCallback)(NB_VectorTileManager* vectorTileManager, NB_Error result, int percent, void* userCallbackData);


// Public Functions ..............................................................................



/*! Create and initialize a new VectorTileManager

Creates a VectorTileManager object.

Call NB_VectorTileManagerDestroy to destroy the instance.

@param context NB_Context
@param tileConfiguration Configuration of tiles to request
@param managerConfiguration Configuration of the tile manager   
@param callback Callback used for tile downloads
@param callbackUserData User data that will be passed to the callback function 
@param vectorTileManager On success, the newly created VectorTileManager; NULL otherwise.
@returns NB_Error
*/
NB_DEC NB_Error
NB_VectorTileManagerCreate(
    NB_Context* context,
    NB_VectorTileConfiguration* tileConfiguration,   
    NB_VectorTileManagerConfiguration* managerConfiguration,
    NB_VectorTileManager** vectorTileManager	
    );     

/*! Register a callback function for notifications of tile download activity.

The set of callback function and user data represents a unique callback registration.  
If the same function and data are passed in a

@param vectorTileManager A pointer to VectorTileManager instance.
@param callback A pointer to the callback function to call with download activity
@param callbackUserData The user data associated with the callback function
*/
NB_DEC NB_Error
NB_VectorTileManagerRegisterCallback(
    NB_VectorTileManager* vectorTileManager,
    NB_VectorTileManagerCallback callback,
    void* callbackUserData);

/*! Remove a callback function for notifications of tile download activity.

@param vectorTileManager A pointer to VectorTileManager instance.
@param callback A pointer to the callback function to call with download activity
@param callbackUserData The user data associated with the callback function
*/
NB_DEC NB_Error
NB_VectorTileManagerRemoveCallback(
    NB_VectorTileManager* vectorTileManager,
    NB_VectorTileManagerCallback callback,
    void* callbackUserData);

/*! Get a vector tile.

This function is used to request a vector tile map. The number of vector tile maps of the entire world is 
dependent on the zoom level. There are (4 ^ zoomLevel) tiles for the entire world. E.g. there are 
4 ^ 16 = 4,294,967,296 tiles for zoom level 16. The tile x/y index starts at 0/0 from the top/left corner.
Pass in the tile x/y-index to get the corresponding tile. Use the spatial interface to get a tile x/y-index based
on a latitide/longitude. The zoomlevel gets set in the configuration when calling NB_VectorTileManagerCreate().

If the function succeeds, a tile is returned ***locked***. The user needs to call NB_VectorTileManagerUnlockTile to
release the tile after its use.

@param vectorTileManager A pointer to VectorTileManager instance.
@param tileIndex Specifies the x/y index of the tile to get. 
@param download If set to TRUE, tile will be downloaded if its not in the cache
@param tile On Success, the requested vector tile object; NULL otherwise.
@returns NB_Error
*/
NB_DEC NB_Error
NB_VectorTileManagerGetTile(
    NB_VectorTileManager* vectorTileManager,
    NB_VectorTileIndex* tileIndex,
    NB_VectorTile** tile,
    nb_boolean download
    );


/*! Download all pending vector tiles.

Call this function after several vector tiles have been queued for download by calling NB_VectorTileManagerGetTile().
The download is initiated and the status for every tile is available through the callback set in NB_VectorTileManagerCreate().
This function can be called even before all tiles have been finished downloading. If too many downloads are in 
progress then this function will return NE_BUSY

@param vectorTileManager A pointer to VectorTileManager instance.
@returns NB_Error
*/
NB_DEC NB_Error
NB_VectorTileManagerDownloadPendingTiles(
    NB_VectorTileManager* vectorTileManager
    );


/*! Unlock a vector tile object.

An unlocked tile can then be disposed by the VectorTileManager
This function has to be called after NB_VectorTileManagerGetTile().

@param vectorTileManager A pointer to VectorTileManager instance.
@param tile A pointer to VectorTile instance.
@returns NB_Error
*/
NB_DEC NB_Error
NB_VectorTileManagerUnlockTile(
	NB_VectorTileManager* vectorTileManager,
	NB_VectorTile* tile
	);

/*! Add the tiles contained in an NB_VectorTileInformation object

Add the tiles contained in an NB_VectorTileInformation object.  The vector 
tile manager will take ownership of this NB_VectorTileInformation object and
release it when done with it.

@param vectorTileManager A pointer to VectorTileManager instance.
@param vectorTileInformation An NB_VectorTileInformation object containing the tiles to add
*/
NB_DEC NB_Error
NB_VectorTileManagerAddVectorTileInformation(
    NB_VectorTileManager* vectorTileManager,
    NB_VectorTileInformation* vectorTileInformation
    );

/*! Override the maximum number of pending tile downloads.

@param vectorTileManager A pointer to VectorTileManager instance.
@param maximumPendingTiles The number of pending tiles to allow, or -1 to reset to the default
*/
NB_DEC void
NB_VectorTileManagerSetMaximumPendingTilesOverride(
    NB_VectorTileManager* vectorTileManager, 
    int maximumPendingTiles
    );

/*! Get the current override the maximum number of pending tile downloads.

@param vectorTileManager A pointer to VectorTileManager instance.
@return int The current override of the number of pending tiles to allow, or -1 for none.
*/
NB_DEF int
NB_VectorTileManagerGetMaximumPendingTilesOverride(
    NB_VectorTileManager* vectorTileManager
    );

/*! Gets the number of unlocked slots available to download tiles

@param vectorTileManager A pointer to VectorTileManager instance.
@returns int
*/
NB_DEC int
NB_VectorTileManagerGetNumberOfUnlockedSlots(
    NB_VectorTileManager* vectorTileManager
    );

/*! Gets the number of slots available to download tiles

@param vectorTileManager A pointer to VectorTileManager instance.
@returns int
*/
NB_DEC int
NB_VectorTileManagerGetNumberOfPendingSlots(
    NB_VectorTileManager* vectorTileManager
    );

NB_DEC void
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
    );

/*! Destroy the NB_VectorTileManager object.

@param vectorTileManager A pointer to VectorTileManager instance.
@returns NB_Error
*/
NB_DEC NB_Error
NB_VectorTileManagerDestroy(
    NB_VectorTileManager* vectorTileManager
    );

/*! Retrieve the default .

@param configuration A pointer to a NB_VectorTileManagerConfiguration structure.
*/
NB_DEC void
NB_VectorTileManager_GetDefaultConfiguration(
    NB_VectorTileManagerConfiguration* configuration
    );

/*! Retrieve the current configuration of vector tile manager.

@param vectorTileManager A pointer to VectorTileManager instance.
@returns A pointer to a NB_VectorTileManagerConfiguration structure.
*/
NB_DEC NB_VectorTileManagerConfiguration*
NB_VectorTileManagerGetConfiguration(
    NB_VectorTileManager* vectorTileManager
    );

/*! @} */

#endif // VECTORTILEMANAGER_H

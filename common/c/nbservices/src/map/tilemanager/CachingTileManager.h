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

    @file       CachingTileManager.h

    Class CachingTileManager inherits from TileManager interface. When
    user requests a tile, the CachingTileManager object checks if the
    tile is existing in the cache. If yes the tile is returned from the
    cache. If the tile is not existing in the cache the CachingTileManager
    object calls the offboard or onboard tile manager to request the tile
    and saves the tile in the cache if the request is successful.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef CACHINGTILEMANAGER_H
#define CACHINGTILEMANAGER_H

/*!
    @addtogroup nbmap
    @{
*/

extern "C"
{
#include "nbcontext.h"
#include "nbcontextprotected.h"
}

#include "base.h"
#include "TileType.h"
#include "TileManager.h"
#include "Cache.h"
#include "FileOperatingTaskManager.h"


namespace nbmap
{

// Constants ....................................................................................


// Types ........................................................................................

class CachingTileRequest;

/*! Caching tile manager for getting tiles */
class CachingTileManager : public Base,
                           public TileManager
{
public:
    // Public functions .........................................................................

    /*! CachingTileManager constructor */
    CachingTileManager();

    /*! CachingTileManager destructor */
    virtual ~CachingTileManager();

    /*! Initialize a CachingTileManager object

        This function should be called before user requests or cancels tiles.

        @return NE_OK if success
    */
    NB_Error
    Initialize(NB_Context* context,                 /*!< Pointer to current context */
               TileTypePtr tileType,                /*!< A TileType object specified the URL template and
                                                         content ID template */
               TileManagerPtr tileManager,          /*!< An offboard or onboard tile manager to request
                                                         the tile if the tile is not existing in the
                                                         cache */
               shared_ptr<string> cachingType, /*!< Caching type of the tiles */
               shared_ptr<string> datasetId,   /*!< Dataset generation ID */
               TileLayerInfoPtr m_tileLayerInfo     /*!< Common information shared by tiles
                                                         of same layer.*/
               );

    /* See description in TileManager.h */
    virtual void GetTile(shared_ptr<map<string, string> > templateParameters,
                         uint32 priority);
    virtual void GetTile(shared_ptr<map<string, string> > templateParameters,
                         shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > callback,
                         uint32 priority,
                         bool fastLoadOnly = false);
    virtual void RemoveAllTiles();
    virtual NB_Error SetCommonParameterMap(shared_ptr<map<string, string> > commonParameters);
    /*! This function will be called when client calls NB_ContextSendMasterClear

        This is a callback function that will be registered with NB_context.
        When client calls NB_ContextSendMasterClear, NB_Context will call this function
        This function will clear the cache.

     */
    static void MasterClear(void* pTileManagerObject /*!< void pointer to be converted to the class*/
                            );

    CachingTileManager* CloneWithLayerInfo(TileLayerInfoPtr info);

    // Callback used by CachedTileOperationCallback
    void TileCacheReadSuccess(uint32 requestId, nbcommon::CacheOperationEntityPtr entity);
    void TileCacheReadError(uint32 requestId, NB_Error error);

    virtual std::string str() const;

private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    CachingTileManager(const CachingTileManager& manager);
    CachingTileManager& operator=(const CachingTileManager& tileManager);
    void UpdateTileLayerInfo(TileLayerInfoPtr info);
    /* See source file for description */

    bool IsInitialized() const;
    void Reset();
    shared_ptr<string> GetContentId(shared_ptr<map<string, string> > templateParameters);

    void GetTileFromDTS(StringMapPtr templateParameters,
                        shared_ptr<AsyncCallbackWithRequest<StringMapPtr, TilePtr> > callback,
                        shared_ptr<string> contentId,
                        uint32 priority,
                        bool fastLoadOnly,
                        nbcommon::DataStreamPtr origData = nbcommon::DataStreamPtr(),
                        AdditionalDataPtr additionalData = AdditionalDataPtr());

    static void EventTaskCallback(PAL_Instance* pal, void* userData);

    void CCC_TileCacheReadSuccess(uint32 requestId, nbcommon::CacheOperationEntityPtr entry);
    void CCC_TileCacheReadError(uint32 requestId, NB_Error error);

    // Private members ..........................................................................
    NB_Context* m_context;                  /*!< Pointer to current context */
    NB_ContextCallback m_contextCallback;   /*!< A context callback object */
    TileLayerInfoPtr m_tileLayerInfo;       /*!< Common information shared by all tiles of
                                                 same layer.*/
    TileTypePtr m_tileType;                 /*!< Draw order for the layer*/
    TileManagerPtr m_tileManager;           /*!< An offboard or onboard tile manager to request
                                                 the tile if the tile is not existing in the
                                                 cache */
    nbcommon::CachePtr m_cache;             /*!< A Cache object to save the tiles */
    shared_ptr<bool> m_callbackValid;       /*!< Flag to identify if the callbacks for the tile
                                                 requests are valid. It is set to false when
                                                 the function RemoveAllTiles is called. */
    shared_ptr<string> m_cachingType;       /*!< Caching type of the tiles */

    shared_ptr<string> m_datasetId;    /*!< Dataset generation ID */

    string m_commonParametersContentIdTemplate;    /*!< Content ID template formatted
                                                             by common parameters */
    nbcommon::FileOperatingTaskManagerPtr m_fileOperationManager;
    uint32 m_currentRequestId;
    map<uint32, shared_ptr<CachingTileRequest> > m_requests;
};

typedef shared_ptr<CachingTileManager> CachingTileManagerPtr;

};  // namespace nbmap

/*! @} */

#endif  // CACHINGTILEMANAGER_H

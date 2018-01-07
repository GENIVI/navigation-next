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

    @file       OffboardTileManager.h

    Class OffboardTileManager inherits from TileManager interface. An
    OffboardTileManager object is used to request tiles by HTTP
    downloading. The URLs of tile requests are formatted from
    metadata-source protocol.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef OFFBOARDTILEMANAGER_H
#define OFFBOARDTILEMANAGER_H

/*!
    @addtogroup nbmap
    @{
*/

extern "C"
{
#include "paltypes.h"
#include "pal.h"
#include "paltimer.h"
#include "palclock.h"
#include "nberror.h"
#include "nbcontextprotected.h"
}

#include "base.h"
#include "TileManager.h"
#include "TileType.h"
#include "OffboardTileRequest.h"
#include "UnifiedTileManager.h"

#include <vector>
#include <map>

namespace nbcommon
{
class DownloadManager;
};

namespace nbmap
{

// Constants ....................................................................................

/* According to the metadata protocol, valid range of download-priority is 0 (for download first)
   to 255 (for download last).
*/
#define DEFAULT_DOWNLOAD_PRIORITY 255

// Default port of the tile requests
#define DEFAULT_TILE_REQUEST_PORT 80

// Default retry times of the tile requests
#define DEFAULT_TILE_REQUEST_RETRY_TIMES 3


// Types ........................................................................................

class TileConverter;

/*! Tile manager for getting tiles by HTTP downloading */
class OffboardTileManager : public Base,
                            public UnifiedTileManager
{
public:
    // Public functions .........................................................................

    /*! OffboardTileManager constructor */
    OffboardTileManager();

    /*! OffboardTileManager destructor */
    virtual ~OffboardTileManager();

    /*! Initialize an OffboardTileManager object

        This function should be called before user requests or cancels tiles.

        @return NE_OK if success
    */
    virtual NB_Error Initialize(NB_Context* context,
                                shared_ptr<protocol::UrlArgsTemplate> urlArgsTemplate,
                                bool compressed,
                                uint32 retryDelay,
                                uint32 maximumTileRequestCount,
                                UnifiedTileTypePtr tileType,
                                TileLayerInfoPtr info,
                                shared_ptr<string> clientGuid
                                );

    virtual void SetTileConverter(shared_ptr<TileConverter> tileConverter);

    /*! Notify a request is added to the download queue

        The tile manager keeps this request until users cancels the request or the request
        is finished (the function RequestRemovedInDownloadQueue is called).

        @return None
        @see RequestRemovedInDownloadQueue
    */
    void
    RequestAddedToDownloadQueue(nbcommon::DownloadRequestPtr request        /*!< Request added to the download queue */
                                );

    /*! Notify a request is removed from the download queue

        This function is called when the request succeeds or fails with error.

        @return None
        @see RequestAddedInDownloadQueue
    */
    void
    RequestRemovedFromDownloadQueue(nbcommon::DownloadRequestPtr request    /*!< Request removed from the download queue */
                                    );

    /*! This function will be called when client calls NB_ContextSendMasterClear

        This is a callback function that will be registered with NB_context.
        When client calls NB_ContextSendMasterClear, NB_Context will call this function
        This function will clear the cache.

        @param pTileManagerObject void pointer to be converted to the class
        @return void
    */
    static void MasterClear(void* pTileManagerObject);

    /* See description in TileManager.h */
    virtual void GetTile(shared_ptr<map<string, string> > templateParameters,
                         uint32 priority);
    virtual void GetTile(shared_ptr<map<string, string> > templateParameters,
                         shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > callback,
                         uint32 priority,
                         bool fastLoadOnly = false);
    virtual void RemoveAllTiles();
    virtual NB_Error SetCommonParameterMap(shared_ptr<map<string, string> > commonParameters);

    /*! Schedule a retry of download request.

        @return true if scheduled successfully.
    */
    bool ScheduleRetry(OffboardTileRequest* request);

    /*! Remove request from record list of this manager.

      @return None.
    */
    void RemoveRequestFromList(nbcommon::DownloadRequest* request);

    /*! Clone this TileManager.

        @return Pointer of OffboardTileManager instance.
    */
    virtual OffboardTileManager* Clone();

    void UpdateTileLayerInfo(TileLayerInfoPtr info);

    virtual std::string str() const;

protected:
    bool IsInitialized() const;
    void UnregisterMasterClearCallback();
    void Reset();
    OffboardTileManager(const OffboardTileManager& tileManager);

    shared_ptr<bool> m_valid;

private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    OffboardTileManager& operator=(const OffboardTileManager& tileManager);

    void StartTileRequests(const vector<nbcommon::DownloadRequestPtr>& requests);
    /* See source file for description */

    nbcommon::DownloadManager* GetGenericHttpDownloadManager();
    NB_Error CreateRequest(uint32 priority,
                           shared_ptr<map<string, string> > templateParameters,
                           shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > callback,
                           nbcommon::DownloadRequest** request);

    virtual nbcommon::DownloadRequest*
    CreateRealRequest(int priority,
                      NB_Context* context,
                      bool compressed,
                      uint16 port,
                      uint32 retryTimes,
                      nbcommon::DownloadProtocol protocol,
                      TileLayerInfoPtr info,
                      shared_ptr<string> hostname,
                      shared_ptr<string> url,
                      shared_ptr<string> contentID,
                      shared_ptr<string> clientGuid,
                      shared_ptr<map<string, string> > templateParameters,
                      shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > callbackWithRequest);

    void ClearRequests(bool inProgress);
    void LimitRequestCount();
    NB_Error InitializeTileRequest(OffboardTileRequest* request,
                                   shared_ptr<string> url,
                                   shared_ptr<string> contentID,
                                   shared_ptr<map<string, string> > templateParameters,
                                   shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > callbackWithRequest);
    bool IsTheSameRequest(nbcommon::DownloadRequest* request1, nbcommon::DownloadRequestPtr request2);
    static void ExpiringTimerCallback(PAL_Instance * pal,
                                      void * userData,
                                      PAL_TimerCBReason reason);

    // Private members ..........................................................................

    NB_Context* m_context;                  /*!< Pointer to current context */
    NB_ContextCallback m_contextCallback;   /*!< A context callback object */
    TileLayerInfoPtr m_tileLayerInfo;       /*!< Common information shared by tiles of same
                                                 layer.*/
    bool m_compressed;                      /*!< Flag to identify if the data is compressed */
    uint16 m_port;                          /*!< Port of the tile requests */
    uint32 m_maximumTileRequestCount;       /*!< Maximum count of requests to download tiles */
    uint32 m_maxRetryTimes;                 /*!< Retry times of the tile requests */
    uint32 m_retryDelay;                    /*!< tile interval of retry Delay  */
    uint32 m_downloadLevel;                 /*!< Download level of the tile requests for the HTTP
                                                 download manager */
    nbcommon::DownloadProtocol m_protocol;  /*!< Protocol of the tile requests */
    TileTypePtr m_tileType;                 /*!< A TileType object specified the URL template and
                                                 content ID template */
    shared_ptr<string> m_clientGuid;        /*!< guid of client, needed by DTS */
    shared_ptr<string> m_hostname;          /*!< Hostname of the tile requests */
    string m_commonParametersUrlTemplate;   /*!< URL template formatted by common parameters */
    string m_commonParametersContentIdTemplate;  /*!< Content ID template formatted by
                                                      common parameters */

    map<uint32, nbcommon::DownloadRequestPtr> m_requestsInDownloadQueue; /*!< Download queue added to HttpDownloadManager, sorted by priority. */
    map<string, nbcommon::DownloadRequestPtr> m_waitRetryList; /*!< List of requests that is
                                                                   watting to be retried. */
    map<string, nbcommon::DownloadRequestPtr> m_requestList;   /*!< Request list generated
                                                                 by this TileManager, when a
                                                                 download failed, we can get
                                                                 it from this list and
                                                                 schedule retry for it */
    shared_ptr<TileConverter> m_tileConverter;
};

typedef shared_ptr<OffboardTileManager> OffboardTileManagerPtr;

}  // namespace nbmap

/*! @} */

#endif  // OFFBOARDTILEMANAGER_H

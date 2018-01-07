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

    @file       OffboardTileRequest.cpp

    See header file for description.
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

/*! @{ */

extern "C"
{
#include "palunzip.h"
#include "palfile.h"
#include "nbcontextprotected.h"
#include "nbqalog.h"
#include "cslqarecorddefinitions.h"
}

#include <iterator>
#include "OffboardTileRequest.h"
#include "OffboardTileManager.h"
#include "TileConverter.h"
#include "TileImpl.h"
#include "datastreamimplementation.h"
#include "StringUtility.h"
#include "analyticsengine.h"

using namespace std;
using namespace nbcommon;
using namespace nbmap;

// Local Constants ..............................................................................

static const uint16 DEFAULT_PORT = 80;
//static const uint32 BUFFER_SIZE = 4096;
//static const uint32 MAX_CACHE_PATH_LENGTH = 512;

static const char HTTP_HEADER_NAME_ETAG[]          = "ETag";
static const char HTTP_HEADERCLIENT_GUID_HEADER[]  = "X-Navbuilder-Clientid";
static const char HTTP_HEADER_NAME_IF_NONE_MATCH[] = "If-None-Match";
static const char HTTP_HEADER_NAME_ACTIVE_SESSION[] = "active-session";

// OffboardTileRequest Public functions .........................................................

/* See header file for description */
OffboardTileRequest::OffboardTileRequest(uint32 priority)
: DownloadRequest(priority),
  m_compressed(false),
  m_tileManager(NULL),
  m_context(NULL),
  m_port(DEFAULT_PORT),
  m_protocol(DOWNLOAD_PROTOCOL_UNDEFINED)
{
    // Nothing to do here.
}

/* See header file for description */
OffboardTileRequest::~OffboardTileRequest()
{
    if (m_callbackWithRequest)
    {
        // m_callback is not called, tell caller about this.
        m_callbackWithRequest->Error(m_templateParameters, NE_IGNORED);
        m_callbackWithRequest.reset();
    }
    Reset();
}

/* See header file for description */
NB_Error
OffboardTileRequest::Initialize(NB_Context* context,
                                bool compressed,
                                uint16 port,
                                uint32 retryTimes,
                                DownloadProtocol protocol,
                                TileLayerInfoPtr info,
                                shared_ptr<string> hostname,
                                shared_ptr<string> url,
                                shared_ptr<string> contentId,
                                shared_ptr<std::string> guid,
                                shared_ptr<map<string, string> > templateParameters,
                                shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > callbackWithRequest,
                                TileConverterPtr tileConveter)
{
    NB_Error error = NE_OK;

    /* The parameter 'callbackWithRequest' can be NULL if user do not care the returned tile.
       The parameter 'fullPath' can be NULL or empty if user do not want to save the data to a file.
    */
    if ((!context) || (!hostname) || (!url) || (!contentId))
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (IsInitialized())
    {
        return NE_EXIST;
    }

    Reset();

    // Add the ETAG to the HTTP request headers.
    if (templateParameters)
    {
        const string& etag = (*templateParameters)[TILE_ADDITIONAL_KEY_ETAG];
        if (!(etag.empty()))
        {
            error = AddContentForRequestHeaderName(HTTP_HEADER_NAME_IF_NONE_MATCH, etag);
            if (error != NE_OK)
            {
                Reset();
                return error;
            }
        }
    }

    if (guid && !guid->empty())
    {
        error = AddContentForRequestHeaderName(HTTP_HEADERCLIENT_GUID_HEADER, *guid);
    }

    AnalyticsEngine* analyticsEngine = AnalyticsEngine::GetInstance(context);
    if(analyticsEngine)
    {
        std::string sessionID = analyticsEngine->GetSessionId();
        std::string sessionIDBinaryString = StringUtility::BinaryBufferToString(reinterpret_cast<const unsigned char*>(sessionID.c_str()), sessionID.length());
        if(!sessionIDBinaryString.empty())
        {
            AddContentForRequestHeaderName(HTTP_HEADER_NAME_ACTIVE_SESSION, sessionIDBinaryString);
        }
    }

    // Initialize the members of this object.
    m_context    = context;
    m_compressed = compressed;
    m_port       = port;
    m_protocol   = protocol;
    m_tileLayerInfo = info;
    m_hostname   = hostname;
    m_url        = url;
    m_contentId  = contentId;
    m_templateParameters  = templateParameters;
    m_callbackWithRequest = callbackWithRequest;
    m_tileConveter = tileConveter;

    SetMaxRetryTime(retryTimes);

    return NE_OK;
}

/* See header file for description */
void
OffboardTileRequest::SetTileManager(OffboardTileManager* tileManager, shared_ptr<bool> valid)
{
    m_tileManager = tileManager;
    m_valid = valid;
}

/* See header file for description */
bool
OffboardTileRequest::WhetherToCancel()
{
    /* If one of the current request or the duplicated requests is valid, this
       function returns false.
    */
    if (m_valid && (*m_valid))
    {
        return false;
    }

    // Check if one of the duplicated requests is valid.
    vector<DownloadRequestPtr>::const_iterator requestIterator = m_duplicatedRequests.begin();
    vector<DownloadRequestPtr>::const_iterator requestEnd = m_duplicatedRequests.end();
    for (; requestIterator != requestEnd; ++requestIterator)
    {
        DownloadRequestPtr duplicatedRequest = *requestIterator;
        if (duplicatedRequest)
        {
            OffboardTileRequest* offboardTileRequest = static_cast<OffboardTileRequest*>(duplicatedRequest.get());
            if (offboardTileRequest &&
                (offboardTileRequest->m_valid) &&
                (*(offboardTileRequest->m_valid)))
            {
                return false;
            }
        }
    }

    // Clear all duplicated requests. Because they are invalid.
    m_duplicatedRequests.clear();
    return true;
}

/* See description in downloadrequest.h */
DownloadProtocol
OffboardTileRequest::GetProtocol()
{
    return m_protocol;
}

/* See description in downloadrequest.h */
shared_ptr<string>
OffboardTileRequest::GetHostname()
{
    return m_hostname;
}

/* See description in downloadrequest.h */
uint16
OffboardTileRequest::GetPort()
{
    return m_port;
}

/* See description in downloadrequest.h */
shared_ptr<string>
OffboardTileRequest::GetUrl()
{
    return m_url;
}

/* See description in downloadrequest.h */
void
OffboardTileRequest::DuplicatedRequestWillBeIgnored(nbcommon::DownloadRequestPtr duplicatedRequest)
{
    // Check if the duplicated request is valid.
    if ((!duplicatedRequest) || (duplicatedRequest.get() == this))
    {
        return;
    }

    OffboardTileRequest* offboardTileRequest = static_cast<OffboardTileRequest*>(duplicatedRequest.get());
    if (!offboardTileRequest)
    {
        return;
    }

    // Move the duplicated requests to this request.
    vector<DownloadRequestPtr>& requestsToMove = offboardTileRequest->m_duplicatedRequests;
    if (!(requestsToMove.empty()))
    {
        vector<DownloadRequestPtr>::const_iterator requestIterator = offboardTileRequest->m_duplicatedRequests.begin();
        vector<DownloadRequestPtr>::const_iterator requestEnd = offboardTileRequest->m_duplicatedRequests.end();
        for (; requestIterator != requestEnd; ++requestIterator)
        {
            DownloadRequestPtr movedRequest = *requestIterator;
            if (movedRequest && (movedRequest.get() != this))
            {
                m_duplicatedRequests.push_back(movedRequest);
            }
        }
        requestsToMove.clear();
    }

    // Add the parameter request to the duplicated requests.
    m_duplicatedRequests.push_back(duplicatedRequest);
}

/* See description in downloadrequest.h */
void
OffboardTileRequest::AddedToDownloadQueue(DownloadRequestPtr request)
{
    if (m_tileManager && m_valid && (*m_valid))
    {
        m_tileManager->RequestAddedToDownloadQueue(request);
    }
}

/* See description in downloadrequest.h */
void
OffboardTileRequest::RemovedFromDownloadQueue(DownloadRequestPtr request)
{
    if (m_tileManager && m_valid && (*m_valid))
    {
        m_tileManager->RequestRemovedFromDownloadQueue(request);
    }
}

NB_Error OffboardTileRequest::GetDataFromStream(uint8** buffer, uint32* size)
{
    NB_Error error = NE_OK;
    uint8* downloadedData = NULL;
    do
    {
        if (!buffer)
        {
            error = NE_INVAL;
            break;
        }

        // Check if the pointer to the context is NULL.
        if (!m_context || !NB_ContextGetPal(m_context))
        {
            error = NE_NOTINIT;
            break;
        }

        // Get the data stream.
        DataStreamPtr downloadedDataStream = GetDataStream();
        if (!downloadedDataStream)
        {
            error = NE_BADDATA;
            break;
        }

        // Get the size of the downloaded data.
        uint32 downloadedDataSize = downloadedDataStream->GetDataSize();
        if (downloadedDataSize == 0)
        {
            error = NE_BADDATA;
            break;
        }

        // Create a new buffer as the source buffer to decompress.
        downloadedData = new uint8[downloadedDataSize];
        if (!downloadedData)
        {
            error = NE_NOMEM;
            break;
        }

        // copy the downloaded data to the source buffer.
        uint32 gotDataSize = downloadedDataStream->GetData(downloadedData, 0,
                                                           downloadedDataSize);
        if (gotDataSize != downloadedDataSize)
        {
            error = NE_BADDATA;
            break;
        }

        /* Create a new buffer as the destination buffer to receive the decompressed data
           and the size of the decompressed data is saved in the first 4 bytes (uint32) of
           the downloaded data.
        */
        uint32 decompressedDataSize = m_compressed ?
                                      *((uint32*) downloadedData) : downloadedDataSize;
        uint8* decompressedData = (uint8*)nsl_malloc(decompressedDataSize);
        if (!decompressedData)
        {
            error =NE_NOMEM;
            break;
        }

        if (m_compressed)
        {
            // Decompress the data.
            gotDataSize = 0;
            PAL_Error palError = PAL_DecompressData(downloadedData + sizeof(uint32),
                                                    downloadedDataSize - sizeof(uint32),
                                                    decompressedData,
                                                    decompressedDataSize,
                                                    &gotDataSize);
            switch (palError)
            {
                case PAL_Ok:
                {
                    error = NE_OK;
                    break;
                }
                case PAL_ErrNoMem:
                {
                    error = NE_NOMEM;
                    break;
                }
                case PAL_ErrRange:
                {
                    error = NE_RANGE;
                    break;
                }
                case PAL_ErrNoData:
                default:
                {
                    error = NE_BADDATA;
                    break;
                }
            }

            if ((error == NE_OK) && (gotDataSize != decompressedDataSize))
            {
                error = NE_BADDATA;
            }
        }
        else
        {
            nsl_memcpy(decompressedData, downloadedData, decompressedDataSize*sizeof(uint8));
        }

        if (error != NE_OK) // @todo:
        {
            nsl_free(decompressedData);
            decompressedData = NULL;
        }
        else
        {
            *buffer = decompressedData;
            *size   = decompressedDataSize;
        }

    } while (0);

    if (downloadedData)
    {
        delete [] downloadedData;
        downloadedData = NULL;
    }

    return error;
}

/* See description in downloadrequest.h */
void
OffboardTileRequest::DataAvailable(uint32 size)
{
    if ((!m_valid) || !(*m_valid))
    {
        /* The duplicated requests are notified NE_IGNORED in the destructor, and
           the data may not be read from cache, the user needs to request again.
        */
        return;
    }

    // If the parameter 'size' is 0 downloading is complete.
    if (size != 0)
    {
        // The request is downloading in progress.
        return;
    }

    // Skip data if response code is not 200.
    if (m_respnseError != NE_OK)
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo,
                    "%s: %p: skipped download data as a result of error: %d\n",
                    __FUNCTION__, this, (int)m_respnseError);
        return;
    }

    DataStreamPtr decompressedDataStream;

    uint32   decompressedDataSize = 0;
    uint8*   decompressedData     = NULL;
    NB_Error error                = NE_OK;
    do
    {
        error = GetDataFromStream(&decompressedData, &decompressedDataSize);
        if (error != NE_OK)
        {
            break;
        }

        // Create a new data stream to save the decompressed data.
        decompressedDataStream = DataStreamPtr(new DataStreamImplementation());
        if (!decompressedDataStream)
        {
            error = NE_NOMEM;
            break;
        }

        if (m_tileConveter) // Needs to convert tiles.
        {
            TileKeyPtr key = GetTileKeyFromTemplateParameters();
            if (!key)
            {
                error = NE_UNEXPECTED;
                break;
            }

            // Convert data to NBM format
            ConverterTilePtr tile(new ConverterTile());

            tile->xPos       = key->m_x;
            tile->yPos       = key->m_y;
            tile->zoomLevel  = key->m_zoomLevel;
            tile->buffer     = decompressedData;
            tile->bufferSize = decompressedDataSize;

            ConverterDataBufferPtr dataBuffer(new ConverterDataBuffer());
            if (m_tileConveter->Convert(tile, dataBuffer) != NE_OK)
            {
                error = NE_NOMEM;
                break;
            }
            error = decompressedDataStream->AppendData(dataBuffer->GetData(),
                                                       dataBuffer->GetSize());
        }
        else
        {
            // Append extracted data to new data stream.
            error = decompressedDataStream->AppendData(decompressedData, decompressedDataSize);
        }

    } while (0);

    if (decompressedData)
    {
        nsl_free(decompressedData);
        decompressedData = NULL;
    }

    if (error != NE_OK)
    {
        DataError(error);
    }
    else
    {
        WriteQaLogTileDownloadSuccess(decompressedDataStream->GetDataSize());
        NotifySuccessWithData(decompressedDataStream);
        m_tileManager->RemoveRequestFromList(this);
    }
}

/* See description in downloadrequest.h */
void
OffboardTileRequest::DataError(NB_Error error)
{
    if ((!m_callbackWithRequest) || (!m_tileManager) || (!m_valid) || !(*m_valid))
    {
        return;
    }

    switch (error)
    {
        case NE_HTTP_NOT_MODIFIED:
        {
            /* Notify the request is successful. Because this error indicates the resource
               has not been modified since last requested. And the data of the tile will be
               read from the cache.
            */
            NotifySuccessWithData(DataStreamPtr());
            break;
        }
        case NE_HTTP_NO_CONTENT:
        case NE_HTTP_INTERNAL_SERVER_ERROR:
        case NE_HTTP_BAD_GATEWAY:
        case NE_HTTP_BAD_REQUEST:
        case NE_HTTP_RESOURCE_NOT_FOUND:
        {
            // If server returns 204, 400, 404, 500 or 502, don't retry.
            WriteQaLogTileDownloadFailure(error);
            m_tileManager->RemoveRequestFromList(this);
            NotifyError(error);
            break;
        }
        default:
        {
            // Other error codes, schedule a retry if possible.
            if (!RetriedTimeIncreased() || !m_tileManager->ScheduleRetry(this))
            {
                WriteQaLogTileDownloadFailure(error);
                m_tileManager->RemoveRequestFromList(this);
                NotifyError(error);
            }
            break;
        }
    }
}


// OffboardTileRequest Private functions ........................................................

/*! Check if this OffboardTileRequest object is initialized

    This function should be called in all public functions of class OffboardTileRequest.

    @return Flag if this OffboardTileRequest object is initialized
*/
bool
OffboardTileRequest::IsInitialized() const
{
    // Check if hostname or URL is NULL.
    if ((!m_context) || (!m_hostname) || (!m_url))
    {
        return false;
    }

    return true;
}

/*! Reset an OffboardTileRequest object to be uninitialized

    Clear all members in this OffboardTileRequest object.

    @return None
*/
void
OffboardTileRequest::Reset()
{
    // Clear the HTTP request and response headers.
    SetRequestHeaders(shared_ptr<string>());
    SetResponseHeaders(shared_ptr<string>());

    // Clear all members in this OffboardTileRequest object.
    m_compressed = false;
    m_port = DEFAULT_PORT;
    m_protocol = DOWNLOAD_PROTOCOL_UNDEFINED;
    m_tileManager = NULL;

    m_hostname.reset();
    m_url.reset();
    m_contentId.reset();
    m_templateParameters.reset();
    m_callbackWithRequest.reset();
    m_duplicatedRequests.clear();
    m_context = NULL;
}

/*! Notify success with the data of the tile

    @return None
*/
void
OffboardTileRequest::NotifySuccessWithData(DataStreamPtr dataStream, /*!< Data of the tile. It can be NULL if
                                                                         the data needs to read from cache. */
                                           bool duplicated           /*!< Flag duplicated. For to avoid Same Tile
                                                                         Rewrite Cache.*/
                                           )
{
    // Check if the request has a callback.
    if (m_callbackWithRequest)
    {
        /* Create a tile object to return the data. The caching path is set to NULL and
           will be set in class CachingTileCallbackWithRequest.
        */
        TileImplPtr tile(new TileImpl(m_contentId, m_tileLayerInfo, duplicated));
        if (!tile)
        {
            DataError(NE_NOMEM);
            return;
        }

        // Set the tile data.
        tile->SetData(dataStream);


        // Add the ETAG to the additional data of the tile.
        map<string, shared_ptr<string> > tmpAdditionalData;
        shared_ptr<string> value;
        if ((value = GetContentByResponseHeaderName(HTTP_HEADER_NAME_ETAG)))
        {
            tmpAdditionalData.insert(make_pair(TILE_ADDITIONAL_KEY_ETAG, value));
        }

        // Add TileRefresh, this indicate when shall this tile becomes stale.
        if ((value = GetContentByResponseHeaderName(HTTP_HEADER_TILE_REFRESH)))
        {
            tmpAdditionalData.insert(make_pair(HTTP_HEADER_TILE_REFRESH, value));
        }

        if (!tmpAdditionalData.empty())
        {
            shared_ptr<map<string, shared_ptr<string> > > additionalData =
                    tile->GetAdditionalData();
            if (!additionalData)
            {
                additionalData.reset(new map<string, shared_ptr<string> >());
                if (!additionalData)
                {
                    DataError(NE_NOMEM);
                    return;
                }
            }

            copy(tmpAdditionalData.begin(), tmpAdditionalData.end(),
                 inserter(*additionalData, additionalData->end()));

            // Set the additional data to the tile.
            tile->SetAdditionalData(additionalData);
        }

        // Notify the request is successful.
        m_callbackWithRequest->Success(m_templateParameters, tile);
        m_callbackWithRequest.reset();

        /* Notify success to the duplicated requests.

           TRICKY: Below function must be called here. For example, the request 2
                   ignores the request 1, so the request 2 saves the request 1 as
                   a duplicated request. Then the request 3 ignores the request 2,
                   so the request 3 saves the request 2 as a duplicated request.
                   The request 1, 2, 3 all need to be notified when success.
        */
        NotifySuccessToDuplicatedRequests(dataStream);
    }
    else
    {
        /* Cannot be moved outside of the if condition. Because this function may
           return an error in the above if condition.
        */
        NotifySuccessToDuplicatedRequests(dataStream);
    }
}

/*! Notify an error to the request

    @return None
    @see NotifySuccessWithData
*/
void
OffboardTileRequest::NotifyError(NB_Error error)
{
    if (m_callbackWithRequest)
    {
        m_callbackWithRequest->Error(m_templateParameters, error);
        m_callbackWithRequest.reset();
    }

    /* Notify an error to the duplicated requests. Below function must be called
       here. Please see the comments in the function NotifySuccessWithData.
    */
    NotifyErrorToDuplicatedRequests(error);
}

void  OffboardTileRequest::WriteQaLogTileDownloadSuccess(uint32 tileSize)
{
    // Log event through QaLog.
    TileKeyPtr tileKey = GetTileKeyFromTemplateParameters();
    if (tileKey && m_tileLayerInfo->tileDataType)
    {
        const char* layerId = m_tileLayerInfo->tileDataType->c_str();
        if ((*(m_tileLayerInfo->tileDataType) != TILE_TYPE_COMMON_MATERIALS)
            && tileSize > 0)
        {
            NB_QaLogTileDownloadSuccess(m_context, (uint32)tileKey->m_x,
                                        (uint32)tileKey->m_y, (uint32)tileKey->m_zoomLevel,
                                        layerId, tileSize, 0);
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo,"TILE_DOWNLOAD_SUCCESS: X:%d Y:%d Z:%d LAYER:%s TileSize:%d",
                                       (uint32)tileKey->m_x,
                                       (uint32)tileKey->m_y, (uint32)tileKey->m_zoomLevel,
                                       layerId, tileSize);
        }
        else
        {
            // @todo: no id is provided to record the successfully download of MAT.
        }
    }
}

void OffboardTileRequest::WriteQaLogTileDownloadFailure(NB_Error error)
{
    if (m_tileLayerInfo->tileDataType)
    {
        if (*(m_tileLayerInfo->tileDataType) != TILE_TYPE_COMMON_MATERIALS)
        {
            TileKeyPtr tileKey = GetTileKeyFromTemplateParameters();
            if (tileKey)
            {
                NB_QaLogTileDownloadFailure(m_context, (uint32)tileKey->m_x,
                                            (uint32)tileKey->m_y, (uint32)tileKey->m_zoomLevel,
                                            m_tileLayerInfo->tileDataType->c_str(),
                                            (uint32)error);
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError,"TILE_DOWNLOAD_FAILURE: X:%d Y:%d Z:%d LAYER:%s NB_ERROR:%d",
                		(uint32)tileKey->m_x,
                        (uint32)tileKey->m_y, (uint32)tileKey->m_zoomLevel,
                        m_tileLayerInfo->tileDataType->c_str(),
                        (uint32)error);
            }
        }
        else // Common material download failed, treat as config init failure.
        {
            unsigned char failureType = GETTING_MATERIAL_FAILED;
            // We have been retried 3 times before HttpDownloadManager reported error herer.
            NB_QaLogUnifiedMapConfigInitializationFailure(m_context, failureType,
                                                          3, (uint32) error);

            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError,"COMMON_MAT_FAILED: NB_ERROR:%d",error);
        }
    }
}

/*! Notify success to the duplicated requests

    TRICKY: This function notifies success with empty data stream to the duplicated
            requests (like the logic for NE_HTTP_NOT_MODIFIED), then the
            CachingTileCallbackWithRequest will try to read the data from the cache
            and return.

    @return None
*/
void
OffboardTileRequest::NotifySuccessToDuplicatedRequests(DataStreamPtr dataStream /*!< Data of the tile. It can be NULL if
                                                                                 the data needs to read from cache. */
                                                       )
{
    vector<DownloadRequestPtr>::const_iterator requestIterator = m_duplicatedRequests.begin();
    vector<DownloadRequestPtr>::const_iterator requestEnd = m_duplicatedRequests.end();
    for (; requestIterator != requestEnd; ++requestIterator)
    {
        DownloadRequestPtr request = *requestIterator;
        if (request)
        {
            // The duplicated request must be the type of OffboardTileRequest.
            OffboardTileRequest* offboardTileRequest = static_cast<OffboardTileRequest*>(request.get());
            if (offboardTileRequest)
            {
                /* Notifies success with an data stream.

                   TRICKY: Cannot return the data for the duplicated requests. Otherwise
                           the multiple requests of the different map views may append
                           the same data to the cache.
                */
                offboardTileRequest->NotifySuccessWithData(dataStream, true);

                /* There is no need to remove the duplicated request from the tile manager
                   by the function OffboardTileManager::RemoveRequestFromList. Because the
                   duplicated requests have not been added to the request list of the tile
                   manager before.
                */
            }
        }
    }

    // Clear the duplicated requests.
    m_duplicatedRequests.clear();
}

/*! Notify an error to the duplicated requests

    @return None
*/
void
OffboardTileRequest::NotifyErrorToDuplicatedRequests(NB_Error error)
{
    if (m_duplicatedRequests.empty())
    {
        return;
    }

    vector<DownloadRequestPtr>::const_iterator requestIterator = m_duplicatedRequests.begin();
    vector<DownloadRequestPtr>::const_iterator requestEnd = m_duplicatedRequests.end();
    for (; requestIterator != requestEnd; ++requestIterator)
    {
        DownloadRequestPtr request = *requestIterator;
        if (request)
        {
            // The duplicated request must be the type of OffboardTileRequest.
            OffboardTileRequest* offboardTileRequest = static_cast<OffboardTileRequest*>(request.get());
            if (offboardTileRequest)
            {
                offboardTileRequest->NotifyError(error);
            }
        }
    }

    // Clear the duplicated requests.
    m_duplicatedRequests.clear();
}

TileKeyPtr OffboardTileRequest::GetTileKeyFromTemplateParameters()
{
    TileKeyPtr tileKey;
    do
    {
        if (!m_templateParameters)
        {
            break;
        }

        int x = 0;
        int y = 0;
        int zoomLevel = 0;
        map<string, string>::const_iterator endOfParameters = m_templateParameters->end();
        map<string, string>::const_iterator iter;

        if ((iter = m_templateParameters->find(PARAMETER_KEY_TILE_X)) == endOfParameters)
        {
            break;
        }
        nbcommon::StringUtility::StringToNumber(iter->second, x);
        if ((iter = m_templateParameters->find(PARAMETER_KEY_TILE_Y)) == endOfParameters)
        {
            break;
        }
        nbcommon::StringUtility::StringToNumber(iter->second, y);
        if ((iter = m_templateParameters->find(PARAMETER_KEY_ZOOM_LEVEL)) == endOfParameters)
        {
            break;
        }
        nbcommon::StringUtility::StringToNumber(iter->second, zoomLevel);
        tileKey.reset(new TileKey(x, y, zoomLevel));
    } while (0);

    return tileKey;
}

/*! @} */

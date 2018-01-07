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

    @file       OffboardTileRequest.h

    Class OffboardTileRequest inherits from DownloadRequest interface. An
    OffboardTileRequest object is used to request or cancel tiles by the
    functions of DownloadManager.
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

#ifndef OFFBOARDTILEREQUEST_H
#define OFFBOARDTILEREQUEST_H

/*!
    @addtogroup nbmap
    @{
*/

extern "C"
{
#include "nberror.h"
#include "nbcontext.h"
}

#include "downloadrequest.h"
#include "AsyncCallback.h"
#include "TileInternal.h"
#include <string>
#include <map>

namespace nbmap
{

// Constants ....................................................................................

// Types ........................................................................................
class OffboardTileManager;
class TileConverter;

/*! An OffboardTileRequest object is used to request or cancel tiles */
class OffboardTileRequest : public nbcommon::DownloadRequest,
                            public Base
{
public:
    // Public functions .........................................................................

    /*! OffboardTileRequest constructor */
    OffboardTileRequest(uint32 priority     /*!< Download priority of the request */
                        );

    /*! OffboardTileRequest destructor */
    virtual ~OffboardTileRequest();

    /*! Initialize an OffboardTileRequest object

        This function should be called before user calls other public functions.

        @return NE_OK if success
    */
    virtual NB_Error
    Initialize(NB_Context* context,                 /*!< Pointer to current context */
               bool compressed,                     /*!< Flag to identify if the data is compressed */
               uint16 port,                         /*!< Port of the request */
               uint32 retryTimes,                   /*!< Retry times of the request */
               nbcommon::DownloadProtocol protocol, /*!< Protocol of the request */
               TileLayerInfoPtr info,               /*!< Tile info shared by this layer */
               shared_ptr<std::string> hostname,    /*!< Hostname of the request */
               shared_ptr<std::string> url,         /*!< URL of the request */
               shared_ptr<std::string> contentId,   /*!< Content ID of the request */
               shared_ptr<std::string> guid,        /*!< GUID of this client */
               shared_ptr<std::map<std::string, std::string> > templateParameters,  /*!< A parameter map for request of callback.
                                                                                         It can be NULL. */
               shared_ptr<AsyncCallbackWithRequest<shared_ptr<std::map<std::string, std::string> >, TilePtr> > callbackWithRequest,  /*!< An AsyncCallbackWithRequest object with a parameter
                                                                                                                                         map for request and a Tile object for response. It
                                                                                                                                         can be NULL if user do not care the returned tile. */
               shared_ptr<TileConverter> tileConveter        /*!< Tile converter object */
               );

    /*! Set a pointer to an OffboardTileManager object

        The parameter 'tileManager' can be NULL when the OffboardTileManager object is destroyed.

        @return None
    */
    void
    SetTileManager(OffboardTileManager* tileManager, /*!< A reference of pointer of pointer to an OffboardTileManager object.
                                                           It can be NULL. This strange */
                   shared_ptr<bool> valid
                   );

    /*! Check if the request could be canceled

        This function checks the flag m_valid of this request and the duplicated requests.
        If all flags m_valid of the above requests are false, this function returns true.
        Otherwise it returns false.

        @return true if the request could be canceled, false otherwise.
    */
    bool
    WhetherToCancel();


    /* See description in downloadrequest.h */
    virtual nbcommon::DownloadProtocol GetProtocol();
    virtual shared_ptr<std::string> GetHostname();
    virtual uint16 GetPort();
    virtual shared_ptr<std::string> GetUrl();
    virtual void DuplicatedRequestWillBeIgnored(nbcommon::DownloadRequestPtr duplicatedRequest);
    virtual void AddedToDownloadQueue(nbcommon::DownloadRequestPtr request);
    virtual void RemovedFromDownloadQueue(nbcommon::DownloadRequestPtr request);
    virtual void DataAvailable(uint32 size);
    virtual void DataError(NB_Error error);

    /*! Report how many times this request has been retried.

        OffboardTileManager will use this value to decide whether to retry this request or not.

        @return retried times.
    */
    uint32  GetRetriedTimes();

protected:
    NB_Error GetDataFromStream(uint8** buffer, uint32* size);
    void     WriteQaLogTileDownloadSuccess(uint32 tileSize);
    void     WriteQaLogTileDownloadFailure(NB_Error error);
    void     NotifySuccessWithData(nbcommon::DataStreamPtr dataStream, bool duplicated = false);
    void     NotifyError(NB_Error error);
    void     NotifySuccessToDuplicatedRequests(nbcommon::DataStreamPtr dataStream);
    void     NotifyErrorToDuplicatedRequests(NB_Error error);
    void     Reset();

    bool m_compressed;                      /*!< Flag to identify if the data is compressed */
    shared_ptr<std::map<std::string, std::string> > m_templateParameters;   /*!< A parameter map for request of callback.
                                                                              It can be NULL. */
    shared_ptr<AsyncCallbackWithRequest<shared_ptr<std::map<std::string, std::string> >, TilePtr> > m_callbackWithRequest;  /*!< An AsyncCallbackWithRequest object with a parameter
                                                                                                                              map for request and a Tile object for response. It
                                                                                                                              can be NULL if user do not care the returned tile. */

    OffboardTileManager* m_tileManager;     /*!< A pointer to an OffboardTileManager object.
                                                 It can be NULL. */
    NB_Context* m_context;                  /*!< Pointer to current context */
    TileLayerInfoPtr m_tileLayerInfo;
    uint16 m_port;                          /*!< Port of the request */
    nbcommon::DownloadProtocol m_protocol;  /*!< Protocol of the request */
    shared_ptr<std::string> m_hostname;     /*!< Hostname of the request */
    shared_ptr<std::string> m_url;          /*!< URL of the request */
    shared_ptr<std::string> m_contentId;    /*!< Content ID of the request */

    shared_ptr<bool> m_valid;               /*!< Flag to identify if this request is valid */

    /* TRICKY: If this request succeeds, notify success with empty data stream to the
               duplicated requests (like the logic for NE_HTTP_NOT_MODIFIED), then the
               CachingTileCallbackWithRequest will try to read the data from the cache
               and return.
               If this request fails, notify the error to the duplicated requests.
    */
    std::vector<nbcommon::DownloadRequestPtr> m_duplicatedRequests; /*!< The requests are duplicated with this request,
                                                                         and they have been ignored by the download manager. */


private:
    // Private functions ...................................................................
    // Copy constructor and assignment operator are not supported.
    OffboardTileRequest(const OffboardTileRequest& request);
    OffboardTileRequest& operator=(const OffboardTileRequest& request);

    /* See source file for description */

    bool IsInitialized() const;
    TileKeyPtr GetTileKeyFromTemplateParameters();

    // Private members .....................................................................
    shared_ptr<TileConverter> m_tileConveter;     /*!< Tile converter object */

};

typedef shared_ptr<OffboardTileRequest> OffboardTileRequestPtr;

};  // namespace nbmap

/*! @} */

#endif  // OFFBOARDTILEREQUEST_H

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

    @file       downloadrequest.h

    Class 'DownloadRequest' is an interface. User should inherit from
    this interface and implement the pure virtual functions. A
    'DownloadRequest' object is used to request or cancel downloading
    data by the functions of 'DownloadManager'.
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

#ifndef DOWNLOADREQUEST_H
#define DOWNLOADREQUEST_H

/*!
    @addtogroup nbcommon
    @{
*/

extern "C"
{
#include "paltypes.h"
#include "nberror.h"
}

#include "datastream.h"
#include <string>

namespace nbcommon
{

// Constants ....................................................................................


/*! Supported download protocols */
typedef enum
{
    DOWNLOAD_PROTOCOL_UNDEFINED = 0,    /*!< Protocol is undefined. Download manager need to get
                                             protocol from hostname. If the hostname does not
                                             contain protocol, HTTP download manager sets it to HTTP. */
    DOWNLOAD_PROTOCOL_HTTP,             /*!< HTTP protocol */
    DOWNLOAD_PROTOCOL_HTTPS             /*!< HTTPS protocol */
} DownloadProtocol;


// Types ........................................................................................

class DownloadRequest;

typedef shared_ptr<DownloadRequest> DownloadRequestPtr;

/*! Download request interface */
class DownloadRequest
{
public:
    // Interface functions ......................................................................

    /*! Get protocol

        If this function returns DOWNLOAD_PROTOCOL_UNDEFINED , HTTP download
        manager tries to get protocol from hostname. If the hostname does not
        contain protocol, HTTP download manager sets it to HTTP.

        @return Protocol
    */
    virtual DownloadProtocol
    GetProtocol() = 0;

    /*! Get hostname

        @return Hostname
    */
    virtual shared_ptr<std::string>
    GetHostname() = 0;

    /*! Get port

        @return Port
    */
    virtual uint16
    GetPort() = 0;

    /*! Get URL

        @return URL
    */
    virtual shared_ptr<std::string>
    GetUrl() = 0;

    /*! Notify this request in the download queue that there is another duplicated
        request which will be ignored by the download manager

        The parameter duplicatedRequest will be ignored by the download manager.
        User needs to copy the information from the duplicated request to this
        request if the duplicated request needs to be notified when the request
        succeeds or fails.

        @return None
    */
    virtual void
    DuplicatedRequestWillBeIgnored(DownloadRequestPtr duplicatedRequest     /*!< The duplicated request which will be
                                                                                 ignored by the download manager */
                                   ) = 0;

    /*! Notify the request is added to the download queue

        The request is added to the download queue when user requests data with
        this object.

        @return None
    */
    virtual void
    AddedToDownloadQueue(DownloadRequestPtr request     /*!< A shared pointer to this request */
                         ) = 0;

    /*! Notify the request is removed from the download queue

        The request is removed from the download queue when the request succeeds
        or fails with an error.

        @return None
    */
    virtual void
    RemovedFromDownloadQueue(DownloadRequestPtr request /*!< A shared pointer to this request */
                             ) = 0;

    /*! Notify download data available

        @return None
    */
    virtual void
    DataAvailable(uint32 size       /*!< Size of data is available. 0 if end of file. */
                  ) = 0;

    /*! Notify download error

        This function returns NE_NO_TRAFFIC_ROUTE_INFO for HTTP 204 and
        NE_HTTP_NOT_MODIFIED for HTTP 304.

        @return None
    */
    virtual void
    DataError(NB_Error error        /*!< Download error */
              ) = 0;


public:
    // Public functions .........................................................................

    /*! DownloadRequest destructor */
    virtual ~DownloadRequest();

    /*! Get download priority of the request

        @return Download priority of the request
    */
    uint32
    GetPriority() const;

    /*! Get request headers to be added to request

        @return Request headers
    */
    shared_ptr<std::string>
    GetRequestHeaders() const;

    /*! Add a field content for the specified field name of the request header

        Reference the common forms from Hypertext Transfer Protocol:

            CR             = <carriage return>
            LF             = <linefeed>
            SP             = <space>
            HT             = <horizontal-tab>
            LWS            = [CRLF] 1*( SP | HT )

            message-header = field-name ":" [ field-value ]
            field-name     = token
            field-value    = *( field-content | LWS )
            field-content  = <the OCTETs making up the field-value
                             and consisting of either *TEXT or combinations
                             of token, separators, and quoted-string>

        The functions of request and response headers follow these rules.
        Find the details in http://www.ietf.org/rfc/rfc2616.txt

        @return NE_OK if success
    */
    NB_Error
    AddContentForRequestHeaderName(const std::string& name,     /*!< A field name of request header to add */
                                   const std::string& content   /*!< A field content of request header to add */
                                   );

    /*! Set request headers

        Set an empty pointer if user want to clear headers.

        @return None
    */
    void
    SetRequestHeaders(shared_ptr<std::string> requestHeaders    /*!< A request headers to set */
                       );

    /*! Set response headers

        Set an empty pointer if user want to clear headers.

        @return None
    */
    void
    SetResponseHeaders(shared_ptr<std::string> responseHeaders   /*!< A response headers to set */
                       );

    /*! Get a field content by the specified field name of the response header

        @return Field content related with the field name of the response header
        @see AddContentForRequestHeaderName
    */
    shared_ptr<std::string>
    GetContentByResponseHeaderName(const std::string& name  /*!< A field name of response header to get
                                                                 field content */
                                   );

    /*! Get flag if request is downloading in progress

        @return Flag if request is downloading in progress
    */
    nb_boolean
    GetInProgressFlag() const;

    /*! Set flag if request is downloading in progress

        @return None
    */
    void
    SetInProgressFlag(nb_boolean isInProgress       /*!< Is request downloading in progress? */
                      );

    /*! Increase retried time of this request.

        @return true if DownlaodManager should retry this request.
    */
    bool
    RetriedTimeIncreased();

    /*! Get the 'DataStream' object

        @return 'DataStream' object
    */
    DataStreamPtr
    GetDataStream() const;

    /*! Set a 'DataStream' object

        @return None
    */
    void
    SetDataStream(DataStreamPtr dataStream          /*! A 'DataStream' object to set */
                  );


    /*! Set response error code.

     User of this request should skip received data if error is not NE_OK;

     @return None.
     */
    void SetResponseError(NB_Error error)
    {
        m_respnseError = error;
    }

protected:
    // Protected functions ......................................................................

    /*! DownloadRequest constructor */
    DownloadRequest(uint32 priority /*!< Download priority of the request */
                    );


    /*! Set maximum retry times of this request. */
    void SetMaxRetryTime(uint32 retryTimes);

    NB_Error m_respnseError;

private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    DownloadRequest(const DownloadRequest& request);
    DownloadRequest& operator=(const DownloadRequest& request);

    /* See source file for description */

    int FindStringCaseInsensitive(const std::string& sourceString,
                                  const std::string& stringToFind,
                                  int start);
    int FilterLinearWhiteSpace(const std::string& sourceString,
                               int start);

private:

    uint32        m_priority;            /*!< Download priority of the request */
    DataStreamPtr m_downloadData;        /*!< DataStream object contained download data */
    uint32        m_maxRetryTimes;       /*!< Number of maximum retry times  */
    uint32        m_currentRetriedTimes; /*!< Number of current retried times  */
    nb_boolean    m_isInProgress;        /*!< Is request downloading in progress? */

    shared_ptr<string> m_requestHeaders;   /*!< Request headers of the request */
    shared_ptr<string> m_responseHeaders;  /*!< Response headers of the request */

};

};  // namespace nbmap

/*! @} */

#endif  // DOWNLOADREQUEST_H

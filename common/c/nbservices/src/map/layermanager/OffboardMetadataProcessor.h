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
   @file        MetadataProcessor.h
   @defgroup    nbmap

   Description: MetadataProcessor is responsible for the processing of
   metadata, including: request, serializing, desrializing, retry, and so on.
   This class should be singleton for a NB_Context, and is shared by several
   objects that need to know about metadata, such as OffboardLayerProvder,
   UnifiedLayerManager and so on.
*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */

#ifndef _METADATAPROCESSOR_H_
#define _METADATAPROCESSOR_H_

extern "C"
{
#include "paltimer.h"
}

#include "Handler.h"
#include "UnifiedMetadataProcessor.h"

namespace nbmap
{

class MetadataAsyncCallback;
typedef shared_ptr<MetadataAsyncCallback> MetadataAsyncCallbackPtr;

/*! Flag to indicate status of metadata changing. */
typedef enum _MetadataChangedFlag
{
    MCF_Invalid = 0,   /*!< Metadata is invalid, needs to retrieve from server again. */
    MCF_Changed,       /*!< Flag to indicate metadata changed  */
    MCF_Unchanged,     /*!< Flag to indicate metadata is not changed  */
} MetadataChangedFlag;

// MetadataProcessor

class OffboardMetadataProcessor : public UnifiedMetadataProcessor
{
public:
    OffboardMetadataProcessor(NB_Context* context);
    virtual ~OffboardMetadataProcessor(void);

    /*! Reset some statistic information.

        @return None.
    */
    virtual void Reset(void);

public:
    /*! Check whether metadata can be retried, if so, schedule a retry.

        @return true if it should be retried.
    */
    bool PrepareRetry(NB_Error error, shared_ptr<AsyncCallback <bool> > callback);

    /*! Updates some statistic information based on inputted flat.

        @return None.
    */
    void UpdateStatistics(bool succeed = true);

    /*! Update MetadataSourceInformation stored locally.

        @return MCF_Changed if metadata is changed, MCF_Unchanged if unchanged,
                or MCF_Invalid if response is invalid.
    */
    MetadataChangedFlag
    UpdateMetadataSourceInformation(protocol::MetadataSourceInformationSharedPtr response);

    /*! Report the network error event when metadata request faile. */
    void ReportNetworkErrorEvent(NB_Error error);

protected:
    // Protected member functions
    virtual bool IsNeedUpdateMetadataForTime(shared_ptr<MetadataConfiguration> config,
                                             bool skipTimeInterval);

    virtual void StartMetadataRequestWithCallback(shared_ptr<AsyncCallback<bool> > callback);

    /*! Start real metadata request.

      @return None.
    */
    void StartMetadataRequest(void);

    /*! Callback for Timer.

      This function will start a metadata request using variables stored by this class.

      @return None.
    */
    static void
    ExpiringTimerCallback(PAL_Instance *pal,        /*!< PAL instance */
                          void *userData,           /*!< User data will be passed to callback */
                          PAL_TimerCBReason reason  /*!< Reason for callback being called */
                          );

protected:
    // Protected member fields
    int    m_currentRetryTimes;         /*!< Number of current retry times  */
    int    m_metadataMaxRetryTime;      /*!< Number of m metadata max retry time  */

    shared_ptr<protocol::Handler<protocol::MetadataSourceParameters,
                                 protocol::MetadataSourceInformation> > m_pMetadataSourceHandler;

    MetadataAsyncCallbackPtr m_pMetadataAsyncCallback; /*!< Asynchronous callback */
};

typedef shared_ptr<OffboardMetadataProcessor> OffboardMetadataProcessorPtr;

}

#endif /* _METADATAPROCESSOR_H_ */

/*! @} */

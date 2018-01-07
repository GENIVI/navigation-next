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

    @file     Network.h

    Description: Network handle the tps request and switch thread.
*/
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "nbtaskqueue.h"
#include "nbcontext.h"
#include "cslnetwork.h"
#include "smartpointer.h"
#include "Lock.h"
#include <list>
#include "AsyncCallback.h"
#include "NetworkRequest.h"
#include "MutableTPSElement.h"
#include "nberror.h"

//@todo: should have namespace here!

typedef struct _RequestQuery
{
    NB_NetworkQuery     networkQuery;
    void*               pThis;
    uint32              requestID;
}RequestQuery;

/*! Simple Binary data used to encapsulate data buffer and length. */
class BinaryData
{
public:
    BinaryData(const char* data, uint32 size);
    virtual ~BinaryData();

    const char* GetData() const;
    uint32      GetSize() const;
    bool        IsValid() const ;
private:
    const char* m_data;
    uint32 m_size;
};

typedef shared_ptr<BinaryData> BinaryDataPtr;

class NetworkCallback : public nbmap::AsyncCallbackWithRequest<uint32, BinaryDataPtr>
{
public:
    NetworkCallback() {}
    virtual ~NetworkCallback() {}

    // AsyncCallback functions ..............................................................................

    /* See AsyncCallback.h for description */
    virtual void Success(uint32 id, BinaryDataPtr response) {}
    virtual void Error(uint32 id, NB_Error error) {}
    virtual bool Progress(int percentage) {return false;}
    /*! Override Progress funtion to return request id to client. */
    virtual bool Progress(uint32 id, float percentage) { return false; }
};


typedef shared_ptr<NetworkCallback> RequestCallbackPtr;

class NetworkTask;
class Network
{
public:
    Network(NB_Context* context);
    ~Network();

    /*! Send network request. */
    void Request(shared_ptr<NetworkRequest> request, RequestCallbackPtr callback);

    /*! Cancel network request. */
    bool Cancel(uint32 requestID);

    /*! Cancel all network requests. */
    void CancelPendingRequests();

    NB_Context* GetNbContext() const { return m_context; }

private:
    bool FindRequest(list<RequestQuery*>& requestList, uint32 requestID);
    bool FindRequestAndRemove(uint32 requestID);

    // Callback functions from the CCC thread...........................................................................

    /*! Send network request. */
    void CCC_Request(const NetworkTask* pTask);

    /*! Cancel network request. */
    void CCC_Cancel(const NetworkTask* pTask);

    /*! Cancel all network requests. */
    void CCC_CancelAll(const NetworkTask* pTask);

    static void StaticNetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
    static void StaticNetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);

private:
    NB_Context* m_context; /*!< Pointer of NB_Context instance */
    shared_ptr<EventTaskQueue> m_eventQueue;  /*!< CCC worker thread */
    list<RequestQuery*> m_requestList; /*!< Request list queue. */
    PAL_Lock* m_lock;   /*!< lock for request list. */
    RequestCallbackPtr m_callback;  /*!< Callback class to handle network request. */
};


#endif //_NETWORK_H_

/*! @} */

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
    @file     NetworkTask.h

    Description: Network task
*/
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef _NETWORK_TASK_
#define _NETWORK_TASK_

extern "C"
{
#include "nberror.h"
#include "palevent.h"
#include "paltaskqueue.h"
}

#include "nbtaskqueue.h"
#include "nbuitaskqueue.h"
#include <vector>
#include <string>
#include "Network.h"

class Network;
class NetworkTask;
typedef void (Network::*TaskFunction)(const NetworkTask* pTask);

/*! Basic used to add to task queue by Network, refer to class Task for more description */
class NetworkTask : public Task
{
public:
    NetworkTask(Network* handler, TaskFunction function);
    virtual ~NetworkTask();

    // Refer to class Task for description.
    virtual void Execute(void);

private:
    Network*         m_handler;
    TaskFunction           m_function;
};

/*! Task used for request. */
class NetworkTaskRequest : public NetworkTask
{
public:
    NetworkTaskRequest(Network* handler, TaskFunction function, shared_ptr<NetworkRequest> request, RequestCallbackPtr callback) : NetworkTask(handler, function), m_request(request), m_callback(callback) {}
    virtual ~NetworkTaskRequest() {}

    shared_ptr<NetworkRequest> m_request;  /*!< Network request. */
    RequestCallbackPtr m_callback;        /*!< Network callbcak. */
};

/*! Task used for cancel request. */
class NetworkTaskCancel : public NetworkTask
{
public:
    NetworkTaskCancel(Network* handler, TaskFunction function, uint32 id) : NetworkTask(handler, function), m_id(id) {}
    virtual ~NetworkTaskCancel() {}

    uint32 m_id;     /*!< request identifier. */
};

/*! Task used for cancel all requests. */
class NetworkTaskCancelAll : public NetworkTask
{
public:
    NetworkTaskCancelAll(Network* handler, TaskFunction function) : NetworkTask(handler, function) {}
    virtual ~NetworkTaskCancelAll() {}

};

#endif //_NETWORK_TASK_

/*! @} */

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
   @file         CacheTasks.h
   @defgroup     nbcommon
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */
#ifndef _CACHETASKS_H_
#define _CACHETASKS_H_

#include "nbtaskqueue.h"
#include "CacheOperationCallback.h"
#include <functional>
#include "Cache.h"

namespace nbcommon
{

class CacheTask;
typedef void (Cache::*TaskFunction)(const CacheTask* pTask);

/*! Basic class used to schedule tasks. */
class CacheTask : public Task
{
public:
    CacheTask(Cache* instance, TaskFunction function);
    virtual ~CacheTask();
    virtual void Execute();
private:
    Cache*           m_instance;
    mem_fun1_t<void, Cache, const CacheTask*> m_functor;
    shared_ptr<bool> m_isValid;
};


/*! Task to process cache full. */
class CacheTaskProcessCacheFull : public CacheTask
{
public:
    CacheTaskProcessCacheFull(Cache*             instance,
                              TaskFunction       function,
                              shared_ptr<string> type,
                              shared_ptr<string> name,
                              uint32             entryCountToAdd);
    virtual ~CacheTaskProcessCacheFull();
    shared_ptr<string> m_type;
    shared_ptr<string> m_name;
    uint32             m_entryCountToAdd;
};


class CacheTaskGetData : public CacheTask
{
public:
    CacheTaskGetData(Cache*                  instance,
                     TaskFunction            function,
                     shared_ptr <string>     type,
                     shared_ptr <string>     name,
                     CacheOperationCallbackPtr callback);
    virtual ~CacheTaskGetData();

    shared_ptr <string>     m_type;
    shared_ptr <string>     m_name;
    CacheOperationCallbackPtr m_callback;
};

/*! Task to save some data into cache. */
class CacheTaskSaveData : public CacheTask
{
public:
    CacheTaskSaveData(Cache*              instance,
                      TaskFunction        function,
                      shared_ptr <string> type,
                      shared_ptr <string> name,
                      DataStreamPtr       data,
                      AdditionalDataPtr   additionalData,
                      CacheOperationCallbackPtr callback);
    virtual ~CacheTaskSaveData();

    shared_ptr <string>       m_type;
    shared_ptr <string>       m_name;
    DataStreamPtr             m_data;
    AdditionalDataPtr         m_additionalData;
    CacheOperationCallbackPtr m_callback;
};

/*! Task to delete some data from cache. */
class CacheTaskRemoveData : public CacheTask
{
public:
    CacheTaskRemoveData(Cache*                    instance,
                        TaskFunction              function,
                        shared_ptr <string>       type,
                        shared_ptr <string>       name,
                        CacheOperationCallbackPtr callback);
    virtual ~CacheTaskRemoveData();

    shared_ptr <string>       m_type;
    shared_ptr <string>       m_name;
    CacheOperationCallbackPtr m_callback;
};

}
#endif                                  /* _CACHETASKS_H_ */
/*! @} */

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
   @file         CacheTasks.cpp
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
#include "CacheTasks.h"
#include "Cache.h"

using namespace nbcommon;

/* See description in header file. */
CacheTask::CacheTask(Cache* instance, TaskFunction function)
        : m_instance(instance),
          m_functor(function)
{
    if (instance)
    {
        m_isValid = instance->IsValid();
    }
}

/* See description in header file. */
CacheTask::~CacheTask()
{
}

/* See description in header file. */
void CacheTask::Execute()
{
    if (m_instance && m_isValid && *m_isValid)
    {
        m_functor(m_instance, this);
    }
    delete this;
}

/* See description in header file. */
CacheTaskProcessCacheFull::CacheTaskProcessCacheFull(Cache*              instance,
                                                     TaskFunction        function,
                                                     shared_ptr <string> type,
                                                     shared_ptr <string> name,
                                                     uint32              entryCountToAdd)
        : CacheTask(instance, function),
          m_type(type),
          m_name(name),
          m_entryCountToAdd(entryCountToAdd)
{
}

/* See description in header file. */
CacheTaskProcessCacheFull::~CacheTaskProcessCacheFull()
{
}

/* See description in header file. */
CacheTaskGetData::CacheTaskGetData(Cache*                    instance,
                                   TaskFunction              function,
                                   shared_ptr <string>       type,
                                   shared_ptr <string>       name,
                                   CacheOperationCallbackPtr callback)
        : CacheTask(instance, function),
          m_type(type),
          m_name(name),
          m_callback(callback)
{
}

/* See description in header file. */
CacheTaskGetData::~CacheTaskGetData()
{
}

/* See description in header file. */
CacheTaskSaveData::CacheTaskSaveData(Cache*                    instance,
                                     TaskFunction              function,
                                     shared_ptr <string>       type,
                                     shared_ptr <string>       name,
                                     DataStreamPtr             data,
                                     AdditionalDataPtr         additionalData,
                                     CacheOperationCallbackPtr callback)
        : CacheTask(instance, function),
          m_type(type),
          m_name(name),
          m_data(data),
          m_additionalData(additionalData),
          m_callback(callback)
{
}

/* See description in header file. */
CacheTaskSaveData::~CacheTaskSaveData()
{
}

/* See description in header file. */
CacheTaskRemoveData::CacheTaskRemoveData(Cache*                    instance,
                                         TaskFunction              function,
                                         shared_ptr <string>       type,
                                         shared_ptr <string>       name,
                                         CacheOperationCallbackPtr callback)
        : CacheTask(instance, function),
          m_type(type),
          m_name(name),
          m_callback(callback)
{
}

/* See description in header file. */
CacheTaskRemoveData::~CacheTaskRemoveData()
{
}
/*! @} */

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
   @file         CacheOperationCallback.h
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

#ifndef _CACHEOPERATIONCALLBACK_H_
#define _CACHEOPERATIONCALLBACK_H_


extern "C"
{
#include "palstdlib.h"
}

#include "smartpointer.h"
#include <map>
#include "datastream.h"
#include "AsyncCallback.h"

typedef shared_ptr<string>    StringPtr;
typedef map<string, string>   StringMap;
typedef shared_ptr<StringMap> StringMapPtr;

//@todo: Maybe better to encapsulate AddtionalData into a class.
typedef map<string, shared_ptr<string> >                AdditionalData;
typedef shared_ptr<map<string, shared_ptr<string> > >   AdditionalDataPtr;

namespace nbcommon
{


/*! CacheOperationEntry is used in callbacks to notify user the result of cache options. */
class CacheOperationEntity
{
public:
    CacheOperationEntity() : m_error(NE_OK){}
    virtual ~CacheOperationEntity(){}
    NB_Error          m_error;          /*!< error code of cache operation */
    DataStreamPtr     m_data;           /*!< data stream to be read or written */
    AdditionalDataPtr m_additionalData; /*!< Additional Data to save. */
    StringPtr         m_type;           /*!< Type of cache entry */
    StringPtr         m_name;           /*!< Name of cache entry */
    StringPtr         m_cachingPath;    /*!< Path of cache entry. */
};

typedef shared_ptr<CacheOperationEntity> CacheOperationEntityPtr;

/*! callback used by cache operations. */
class CacheOperationCallback : public nbmap::AsyncCallback<CacheOperationEntityPtr>
{
public:
    virtual ~CacheOperationCallback(){}

    // @tricky:
    // Errors should be returned by setting CacheOperationEntity::m_error and call
    // CacheOperationCallback::Success() if possible. The reason for this is
    // CacheOperationCallback::Error() can not return extra information for caller to parse
    // the reason of failure. Hence, if some operation fails, CacheOperationEntity::m_error
    // and CacheOperationEntity::m_additionalData will be set and caller can check those two
    // fields to decide what to do with the error.
    virtual void Success(CacheOperationEntityPtr entity) = 0;
    virtual void Error(NB_Error error) = 0;
};

typedef shared_ptr<CacheOperationCallback> CacheOperationCallbackPtr;

}

#endif /* _CACHEOPERATIONCALLBACK_H_ */
/*! @} */

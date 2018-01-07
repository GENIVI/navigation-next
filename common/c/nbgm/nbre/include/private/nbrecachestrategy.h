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

@file nbrecachestrategy.h
*/
/*
(C) Copyright 2012 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBRE_CACHE_STRATEGY_H_
#define _NBRE_CACHE_STRATEGY_H_

#include "paltypes.h"

template<typename Key, typename Value>
class NBRE_Cache;

/** \addtogroup NBRE_Core
*  @{
*/

//! Cache strategy class
/**
*/

template<typename Key, typename Value>
class NBRE_ICacheStrategy
{

friend class NBRE_Cache<Key, Value>;

public:
    virtual ~NBRE_ICacheStrategy(){;}

protected:
    virtual PAL_Error GetRemovedKeys(NBRE_Vector<Key>& removedKeys)const   = 0;
    virtual void OnPut(const Key& key, const Value& val)                   = 0;
    virtual void OnGet(const Key& key)                                     = 0;
    virtual void OnTouch(const Key& key)                                   = 0;
    virtual nb_boolean OnRealse(const Key& key)                            = 0;
    virtual void OnRemove(const Key& key)                                  = 0;
    virtual void OnDestroy(Value& val)                                     = 0;
    virtual void OnRemoveAll()                                             = 0;
    virtual void Destroy()                                                 = 0;
    virtual void OnUpdate(const Key& key, const Value& val, Value& oldVal) = 0;
    virtual void OnLock(const Key& /*key*/)  {;}
    virtual void OnUnlock(const Key& /*key*/){;}
};

/** @} */
#endif

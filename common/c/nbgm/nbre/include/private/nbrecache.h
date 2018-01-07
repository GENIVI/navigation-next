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

    @file nbrecache.h
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
#ifndef _NBRE_CACHE_H_
#define _NBRE_CACHE_H_

#include "paltypes.h"
#include "nbrecachestrategy.h"

/** \addtogroup NBRE_Core
*  @{
*/

//! Cache class
/**
*/
template<typename Key, typename Value>
class NBRE_Cache
{
    typedef NBRE_ICacheStrategy<Key, Value> Strategy;
public:
    NBRE_Cache(Strategy* strategy);
    ~NBRE_Cache();

public:
    /// Append an element, return error if exited
    PAL_Error Put(const Key& key, const Value& item);
    /// Append an element and fill the remove keys, return error if existed
    PAL_Error Put(const Key& key, const Value& item, NBRE_Vector<Key>& removedKeys);
    /// Get an element, return error if not exit
    PAL_Error Get(const Key& k, Value& v);
    /// Try to remove an element
    nb_boolean Release(const Key& k);
    /// Remove elements which meet the auto-deletion condition
    void Refresh();
    /// Lock the element to avoid being auto deleted
    void Lock(const Key& k);
    /// Unlock the element
    void Unlock(const Key& k);
    /// Raise the priority of the element
    void Touch(const Key& k);
    /// Make the cache empty
    void RemoveAll();
    /// Return the real size of the cache
    uint32 GetSize()const { return static_cast<uint32>(mItemMap.size());}
    /// Is element existed?
    nb_boolean Contains(const Key& k)const;
    /// Return all the key in cache
    void GetAllKeys(NBRE_Vector<Key>& ks) const;
    /// Update current key and value
    void Update(const Key& key, const Value& item);

private:
    /// Remove a key from cache
    void Remove(const Key& key);
    /// Remove keys from cache
    void Remove(const NBRE_Vector<Key> &keys);

private:
    typedef NBRE_Map<Key, Value> ItemMap;
    ItemMap    mItemMap;
    Strategy*  mStrategy;
};

#include "nbrecache.inl"

/** @} */
#endif

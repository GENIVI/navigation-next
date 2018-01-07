/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

template<typename Key, typename Value>
class DefalutCacheStrategy :public NBRE_ICacheStrategy<Key, Value>
{
private:
    virtual PAL_Error GetRemovedKeys(NBRE_Vector<Key>& /*removedKeys*/)const { return PAL_Ok; }
    virtual void OnPut(const Key& /*key*/, const Value& /*val*/) {;}
    virtual void OnGet(const Key& /*key*/){;}
    virtual void OnTouch(const Key& /*key*/){;}
    virtual nb_boolean OnRealse(const Key& /*key*/) { return TRUE; }
    virtual void OnRemoveAll(){;}
    virtual void Destroy(){ NBRE_DELETE this; }
    virtual void OnLock(const Key& /*key*/)  {;}
    virtual void OnUnlock(const Key& /*key*/){;}
    virtual void OnRemove(const Key& /*key*/){;}
    virtual void OnDestroy(Value& /*val*/){;}
    virtual void OnUpdate(const Key& /*key*/, const Value& /*val*/, Value& /*oldVal*/) {;}
};

template <typename Key, typename Value>
inline NBRE_Cache<Key, Value>::NBRE_Cache(Strategy* strategy)
:mStrategy(strategy)
{
    if (strategy == NULL)
    {
        mStrategy = NBRE_NEW DefalutCacheStrategy<Key, Value>();
    }
}

template <typename Key, typename Value>
inline NBRE_Cache<Key, Value>::~NBRE_Cache()
{
    RemoveAll();
    mStrategy->Destroy();
}

template <typename Key, typename Value>
inline nb_boolean
NBRE_Cache<Key, Value>::Contains(const Key& k)const
{
    return mItemMap.find(k) != mItemMap.end();
}

template <typename Key, typename Value>
inline void
NBRE_Cache<Key, Value>::RemoveAll()
{
    mStrategy->OnRemoveAll();

    for(typename ItemMap::iterator it = mItemMap.begin(); it != mItemMap.end(); ++it)
    {
        mStrategy->OnDestroy(it->second);
    }

    mItemMap.clear();
}

template <typename Key, typename Value>
void
NBRE_Cache<Key, Value>::Refresh()
{
    NBRE_Vector<Key> ks;
    if (mStrategy->GetRemovedKeys(ks) == PAL_Ok)
    {
        Remove(ks);
    }
}

template <typename Key, typename Value>
inline void
NBRE_Cache<Key, Value>::Lock(const Key& k)
{
    mStrategy->OnLock(k);
}

template <typename Key, typename Value>
inline void
NBRE_Cache<Key, Value>::Unlock(const Key& k)
{
    mStrategy->OnUnlock(k);
}

template <typename Key, typename Value>
inline PAL_Error
NBRE_Cache<Key, Value>::Get(const Key& k, Value& e)
{
    PAL_Error err = PAL_ErrNotFound;
    typename ItemMap::iterator it = mItemMap.find(k);
    if (it != mItemMap.end())
    {
        e = it->second;
        mStrategy->OnGet(k);
        err = PAL_Ok;
    }
    return err;
}


template <typename Key, typename Value>
inline nb_boolean
NBRE_Cache<Key, Value>::Release(const Key& k)
{
    nb_boolean result = mStrategy->OnRealse(k);
    if (result)
    {
        Remove(k);
    }
    return result;
}

template <typename Key, typename Value>
inline void
NBRE_Cache<Key, Value>::Remove(const Key& key)
{
    Value v = mItemMap[key];
    mItemMap.erase(key);
    mStrategy->OnRemove(key);
    mStrategy->OnDestroy(v);
}

template <typename Key, typename Value>
inline void
NBRE_Cache<Key, Value>::Remove(const NBRE_Vector<Key> &keys)
{
   for (typename NBRE_Vector<Key>::const_iterator i = keys.begin(); i != keys.end(); i++)
   {
       Remove(*i);
   }
}

template <typename Key, typename Value>
inline void
NBRE_Cache<Key, Value>::Touch(const Key& k)
{
    mStrategy->OnTouch(k);
}

template <typename Key, typename Value>
PAL_Error
NBRE_Cache<Key, Value>::Put(const Key& k, const Value& v)
{
    if (mItemMap.find(k) != mItemMap.end())
    {
        return PAL_ErrAlreadyExist;
    }

    NBRE_Vector<Key> ks;
    PAL_Error err = mStrategy->GetRemovedKeys(ks);
    if (err != PAL_Ok)
    {
        return err;
    }
    Remove(ks);
    mStrategy->OnPut(k, mItemMap.insert(typename ItemMap::value_type(k, v)).first->second);
    return PAL_Ok;
}

template <typename Key, typename Value>
PAL_Error
NBRE_Cache<Key, Value>::Put(const Key& k, const Value& v, NBRE_Vector<Key>& removedKeys)
{
    if (mItemMap.find(k) != mItemMap.end())
    {
        return PAL_ErrAlreadyExist;
    }

    removedKeys.clear();
    PAL_Error err = mStrategy->GetRemovedKeys(removedKeys);
    if (err != PAL_Ok)
    {
        return err;
    }
    Remove(removedKeys);
    mStrategy->OnPut(k, mItemMap.insert(typename ItemMap::value_type(k, v)).first->second);
    return PAL_Ok;
}

template <typename Key, typename Value>
void
NBRE_Cache<Key, Value>::GetAllKeys(NBRE_Vector<Key>& ks) const
{
    ks.clear();
    for(typename ItemMap::const_iterator it = mItemMap.begin(); it != mItemMap.end(); ++it)
    {
        ks.push_back(it->first);
    }
}

template <typename Key, typename Value>
void
NBRE_Cache<Key, Value>::Update(const Key& key, const Value& item)
{
    if (mItemMap.find(key) == mItemMap.end())
    {
        Put(key, item);
    }
    else
    {
        Value v = mItemMap[key];
        mItemMap.erase(key);
        mItemMap.insert(typename ItemMap::value_type(key, item));
        mStrategy->OnUpdate(key, item, v);
    }
}

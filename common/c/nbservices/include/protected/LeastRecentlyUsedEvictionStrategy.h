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

    @file       LeastRecentlyUsedEvictionStrategy.h

    Class LeastRecentlyUsedEvictionStrategy inherits from
    EvictionStrategy interface. It implements the strategy that find the
    least recently used index and remove this index. It maintains a list
    of indexes by the order of recently used and always returns the tail
    of the list if the function GetFirstToRemove is called.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef LEASTRECENTLYUSEDEVICTIONSTRATEGY_H
#define LEASTRECENTLYUSEDEVICTIONSTRATEGY_H

/*!
    @addtogroup nbcommon
    @{
*/

#include "base.h"
#include "EvictionStrategy.h"
#include "nblist.h"
#include <map>

namespace nbcommon
{

// Constants ....................................................................................


// Types ........................................................................................

/*! A strategy that find the least recently used index and remove this index */
template <class T>
class LeastRecentlyUsedEvictionStrategy : public Base,
                                          public EvictionStrategy<T>
{
public:
    // Public Functions .........................................................................

    /*! LeastRecentlyUsedEvictionStrategy constructor */
    LeastRecentlyUsedEvictionStrategy();

    /*! LeastRecentlyUsedEvictionStrategy destructor */
    virtual ~LeastRecentlyUsedEvictionStrategy();

    /* See description in EvictionStrategy.h */
    virtual void IndexOperated(T index,
                               IndexOperation operation);
    virtual T GetFirstToRemove();
    virtual void Clear();


private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    LeastRecentlyUsedEvictionStrategy(const LeastRecentlyUsedEvictionStrategy& strategy);
    LeastRecentlyUsedEvictionStrategy& operator=(const LeastRecentlyUsedEvictionStrategy& strategy);


private:
    // Private members ..........................................................................

    List<T> m_recentlyUsedList;                         /*!< A list of indexes by the order of recently
                                                             used. The tail is least recently used. */
    std::map<T, shared_ptr<ListNode<T> > > m_indexMap;  /*!< A map of indexes */
};


// Public functions definition ..................................................................

/* See declaration for description */
template <class T>
LeastRecentlyUsedEvictionStrategy<T>::LeastRecentlyUsedEvictionStrategy() : m_recentlyUsedList(),
                                                                            m_indexMap()
{
    // Nothing to do here.
}

/* See declaration for description */
template <class T>
LeastRecentlyUsedEvictionStrategy<T>::~LeastRecentlyUsedEvictionStrategy()
{
    // Nothing to do here.
}

/* See declaration for description */
template <class T>
void
LeastRecentlyUsedEvictionStrategy<T>::IndexOperated(T index,
                                                    IndexOperation operation)
{
    switch (operation)
    {
        case ADDING_INDEX:
        {
            shared_ptr<ListNode<T> > listNode = m_recentlyUsedList.PushFront(index);
            if (listNode)
            {
                std::pair<typename std::map<T, shared_ptr<ListNode<T> > >::iterator, bool> result = m_indexMap.insert(std::pair<T, shared_ptr<ListNode<T> > >(index, listNode));
                if (!(result.second))
                {
                    // The element already exists in the map. So remove the front of the list.
                    m_recentlyUsedList.PopFront();
                }
            }
            break;
        }
        case REMOVING_INDEX:
        {
            typename std::map<T, shared_ptr<ListNode<T> > >::iterator found = m_indexMap.find(index);
            if (found != m_indexMap.end())
            {
                shared_ptr<ListNode<T> > listNode = found->second;
                if (listNode)
                {
                    m_recentlyUsedList.Erase(listNode);
                }
                m_indexMap.erase(found);
            }
            break;
        }
        case USING_INDEX:
        {
            typename std::map<T, shared_ptr<ListNode<T> > >::iterator found = m_indexMap.find(index);
            if (found != m_indexMap.end())
            {
                shared_ptr<ListNode<T> > listNode = found->second;
                if (listNode)
                {
                    listNode = m_recentlyUsedList.Erase(listNode);
                    if (listNode)
                    {
                        listNode = m_recentlyUsedList.PushFront(listNode->m_data);
                    }
                }

                if(listNode)
                {
                    // Replace the value with new added list node.
                    found->second = listNode;
                }
                else
                {
                    // Remove the index in the map if saved list node is NULL.
                    m_indexMap.erase(found);
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

/* See declaration for description */
template <class T>
T
LeastRecentlyUsedEvictionStrategy<T>::GetFirstToRemove()
{
    shared_ptr<ListNode<T> > listNode = m_recentlyUsedList.Back();

    if (listNode)
    {
        return listNode->m_data;
    }

    return T();
}

/* See declaration for description */
template <class T>
void
LeastRecentlyUsedEvictionStrategy<T>::Clear()
{
    m_recentlyUsedList.Clear();
    m_indexMap.clear();
}

};  // namespace nbcommon

/*! @} */

#endif  // LEASTRECENTLYUSEDEVICTIONSTRATEGY_H

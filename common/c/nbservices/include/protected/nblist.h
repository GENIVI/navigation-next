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

    @file       nblist.h

    Class List implements the functions of a linked list. It is not
    same with std::list. User could keep a ListNode using shared_ptr even
    if it is erased from the list (both m_previous and m_next are NULL).
    But keeping the std::list<T>::iterator is not safe if the element is
    erased. And user could keep and use a ListNode to erase the element
    efficiently.
    For example user keeps the element in both a list and a std::map.
    User needs find the element by a key and erase this element from
    both the std::list and map. So user could use the List<VALUE> and
    std::map<KEY, shared_ptr<ListNode<VALUE> > > to erase the element
    efficiently.
    If the above requirements are not neccessary, std::list should be
    always used.
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

#ifndef LIST_H
#define LIST_H

/*!
    @addtogroup nbcommon
    @{
*/

extern "C"
{
}

#include "base.h"
#include "smartpointer.h"

namespace nbcommon
{

// Constants ....................................................................................


// Types ........................................................................................

/*! List node

    User could keep a ListNode using shared_ptr even if it is erased from the list (both
    m_previous and m_next are NULL). I suggest using shared_ptr wrapping the data (
    ListNode<shared_ptr<DATA> >).
*/
template <class T>
class ListNode : public Base
{
public:
    // Public functions .........................................................................

    /*! ListNode constructor */
    ListNode(const T& data);

    /*! ListNode destructor */
    virtual ~ListNode();

    // Public members ...........................................................................

    shared_ptr<ListNode> m_previous;/*!< Previous node */
    shared_ptr<ListNode> m_next;    /*!< Next node */
    T m_data;                       /*!< Stored data */
};


/*! List used to erase an element efficiently */
template <class T>
class List : public Base
{
public:
    // Public functions .........................................................................

    /*! List constructor */
    List();

    /*! List destructor */
    virtual ~List();

    /*! Check if the list is empty

        @return true if the list is empty, false otherwise.
    */
    bool
    Empty() const;

    /*! Get the first element

        @return Head list node
    */
    shared_ptr<ListNode<T> >
    Front();

    /*! Get the last element

        @return Tail list node
    */
    shared_ptr<ListNode<T> >
    Back();

    /*! Add data to the front

        This function returns NULL if an error occurs.

        @return Added list node
    */
    shared_ptr<ListNode<T> >
    PushFront(const T& data     /*!< Data to be added */
              );

    /*! Remove first element

        @return Removed list node
    */
    shared_ptr<ListNode<T> >
    PopFront();

    /*! Add data to the end

        This function returns NULL if an error occurs.

        @return Added list node
    */
    shared_ptr<ListNode<T> >
    PushBack(const T& data      /*!< Data to be added */
             );

    /*! Remove last element

        @return Removed list node
    */
    shared_ptr<ListNode<T> >
    PopBack();

    /*! Insert given value into the list before specified list node

        @return Inserted list node
    */
    shared_ptr<ListNode<T> >
    Insert(shared_ptr<ListNode<T> > position,   /*!< A position to insert */
           const T& data                        /*!< Data to be inserted */
           );

    /*! Remove element at given position

        @return Removed list node
    */
    shared_ptr<ListNode<T> >
    Erase(shared_ptr<ListNode<T> > position     /*!< A position to erase */
          );

    /*! Erase all the elements

        @return None
    */
    void
    Clear();

    uint32 Size() const
    {
        return m_size;
    }

private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    List(const List& list);
    List& operator=(const List& list);


    // Private members ..........................................................................

    shared_ptr<ListNode<T> > m_head;    /*!< Head of the list */
    uint32 m_size;
};


// ListNode functions definition ................................................................

/* See declaration for description */
template <class T>
ListNode<T>::ListNode(const T& data) : m_previous(),
                                       m_next(),
                                       m_data(data)
{
    // Nothing to do here.
}

/* See declaration for description */
template <class T>
ListNode<T>::~ListNode()
{
    // Nothing to do here.
}


// List functions definition ....................................................................

/* See declaration for description */
template <class T>
List<T>::List() : m_head(), m_size(0)
{
    // Nothing to do here.
}

/* See declaration for description */
template <class T>
List<T>::~List()
{
    Clear();
}

/* See declaration for description */
template <class T>
bool
List<T>::Empty() const
{
    return (m_head == NULL);
}

/* See declaration for description */
template <class T>
shared_ptr<ListNode<T> >
List<T>::Front()
{
    return m_head;
}

/* See declaration for description */
template <class T>
shared_ptr<ListNode<T> >
List<T>::Back()
{
    if (m_head)
    {
        return m_head->m_previous;
    }

    return shared_ptr<ListNode<T> >();
}

/* See declaration for description */
template <class T>
shared_ptr<ListNode<T> >
List<T>::PushFront(const T& data)
{
    return Insert(Front(), data);
}

/* See declaration for description */
template <class T>
shared_ptr<ListNode<T> >
List<T>::PopFront()
{
    return Erase(Front());
}

/* See declaration for description */
template <class T>
shared_ptr<ListNode<T> >
List<T>::PushBack(const T& data)
{
    return Insert(shared_ptr<ListNode<T> >(), data);
}

/* See declaration for description */
template <class T>
shared_ptr<ListNode<T> >
List<T>::PopBack()
{
    return Erase(Back());
}

/* See declaration for description */
template <class T>
shared_ptr<ListNode<T> >
List<T>::Insert(shared_ptr<ListNode<T> > position,
                const T& data)
{
    shared_ptr<ListNode<T> > newNode(new ListNode<T>(data));
    if (!newNode)
    {
        // Out of memory.
        return shared_ptr<ListNode<T> >();
    }

    // Replace the head with the new node if 'position' is the head.
    if (position == m_head)
    {
        // Set the previous and next to itself if the list is empty.
        if (!m_head)
        {
            newNode->m_previous = newNode;
            newNode->m_next = newNode;
        }

        m_head = newNode;
    }
    else if (!position)
    {
        // Add the new node to the end of list if the 'position' is empty.
        position = m_head;
    }

    // Insert the new node at the position.
    if (position)
    {
        if (position->m_previous)
        {
            // Connect 'position->m_previous' with the new node.
            position->m_previous->m_next = newNode;
            newNode->m_previous = position->m_previous;
        }

        // Connect 'position' with the new node.
        position->m_previous = newNode;
        newNode->m_next = position;
    }


    ++m_size;
    return newNode;
}

/* See declaration for description */
template <class T>
shared_ptr<ListNode<T> >
List<T>::Erase(shared_ptr<ListNode<T> > position)
{
    if (!position)
    {
        return position;
    }

    // Disconnect 'position->m_previous' with 'position'.
    if (position->m_previous)
    {
        position->m_previous->m_next = position->m_next;
    }

    // Disconnect 'position->m_next' with 'position'.
    if (position->m_next)
    {
        position->m_next->m_previous = position->m_previous;
    }

    // Replace the head with 'position->m_next' if 'position' is the head.
    if (position == m_head)
    {
        // Check if there is only one element in the list.
        if (position == position->m_next)
        {
            nsl_assert(position == position->m_previous);
            m_head.reset();
        }
        else
        {
            m_head = position->m_next;
        }
    }

    // Clear the previous and next of 'position'.
    position->m_previous.reset();
    position->m_next.reset();

    --m_size;

    return position;
}

/* See declaration for description */
template <class T>
void
List<T>::Clear()
{
    while (!Empty())
    {
        Erase(Front());
    }
}

};  // namespace nbcommon

/*! @} */

#endif  // LIST_H

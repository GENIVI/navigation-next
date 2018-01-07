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

    @file     ListenerList.cpp
    @date     05/05/2011
    @defgroup NBLOCATIONSERVICES Location Services

    NBList class is single-linked list implementation
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */


#include "listenerlist.h"

ListenerList::ListenerList()
  : m_head(NULL), m_cursor(NULL)
{
}

ListenerList::~ListenerList()
{
    if (m_head == NULL)
    {
        return;
    }

    Node* q = NULL;
    while (m_head != NULL)
    {
        q = m_head->link;
        nsl_free(m_head);
        m_head = q;
    }
}

void ListenerList::Append(NB_LS_Listener* elem)
{
    Node *q = NULL, *tail = NULL;

    if (m_head == NULL)
    {
        m_head = (Node*)nsl_malloc(sizeof(Node));
        if (m_head)
        {
            m_head->data = elem;
            m_head->link = NULL;
        }
    }
    else
    {
        q = m_head;
        while (q->link != NULL)
        {
            q = q->link;
        }

        tail = (Node*)nsl_malloc(sizeof(Node));
        if (tail)
        {
            tail->data = elem;
            tail->link = NULL;
        }
        q->link = tail;
    }
}

void ListenerList::Remove(NB_LS_Listener* elem)
{
    if (m_head == NULL)
    {
        return;
    }

    Node *q = NULL, *r = NULL;
    q = m_head;
    if (q->data == elem)
    {
        m_head = q->link;
        nsl_free(q);
        return;
    }

    r = q;
    while (q != NULL)
    {
        if (q->data == elem)
        {
            r->link = q->link;
            nsl_free(q);
            break;
        }

        r = q;
        q = q->link;
    }
}

NB_LS_Listener* ListenerList::First()
{
    m_cursor = NULL;
    if (m_head == NULL)
    {
        return NULL;
    }

    m_cursor = m_head->link;
    return m_head->data;
}

NB_LS_Listener* ListenerList::Next()
{
    if (m_cursor == NULL)
    {
        return NULL;
    }

    NB_LS_Listener* ptr = m_cursor->data;
    m_cursor = m_cursor->link;
    return ptr;
}

int ListenerList::Count()
{
    Node *q;
    int count = 0;
    for( q = m_head ; q != NULL ; q = q->link )
    {
        count++;
    }

    return count;
}

/*! @} */

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

    @file     palqueue.h
    @date     08/25/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for queue.

    Windows implementation for PAL queue.
*/
/*
    (C) Copyright 2011 by Telecommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Telecommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Telecommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef PALQUEUE_H
#define PALQUEUE_H

#include <deque>

using namespace std;

namespace nimpal
{
    template<class T>
    class palqueue
    {
    public:
        palqueue(void) {}
        ~palqueue(void) {}

        T& front(void) { return m_deque.front(); }
        const T& front() const { return m_deque.front(); }

        T& back(void) { return m_deque.back(); }
        const T& back() const { return m_deque.back(); }

        T& at(size_t idx) { return m_deque.at(idx); }
        const T& at(size_t idx) const { return m_deque.at(idx); }

        void push(const T& elem) { m_deque.push_back(elem); }
        void pop(void) { m_deque.pop_front(); }

        void erase(size_t idx);
        void clear(void) { m_deque.clear(); }

        size_t size(void) const { return m_deque.size(); }
        bool empty(void) const { return m_deque.empty(); }

        void swap(deque<T> temp) { m_deque.swap(temp); }

    private:
        // Forbid to copy class object
        palqueue(const palqueue&);
        palqueue& operator=(const palqueue&);

    private:
        deque<T> m_deque;
    };

    template<class T>
    void palqueue<T>::erase(size_t idx)
    {
        if (idx < m_deque.size())
        {
            m_deque.erase(m_deque.begin() + idx);
        }
    }
}

#endif

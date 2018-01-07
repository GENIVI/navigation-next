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
@file     DoubleTapGestureHandler.cpp
*/
/*
(C) Copyright 2013 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems, is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#include "DoubleTapGestureHandler.h"
#include "ScaleGestureHandler.h"

DoubleTapGestureDetector::DoubleTapGestureDetector():m_pListener(NULL),
    m_isInited(false)
{
    m_viewSize.cx = 0;
    m_viewSize.cy = 0;
}

DoubleTapGestureDetector::~DoubleTapGestureDetector()
{
    delete m_pListener;
}

void
DoubleTapGestureDetector::SetListener(IDoublTapGestureListener* listener)
{
    if(m_pListener != NULL)
    {
        delete m_pListener;
        m_pListener = NULL;
    }
    m_pListener = listener;
}

bool
DoubleTapGestureDetector::OnTouchEvent(const GestureEvent& event)
{
    if(m_pListener == NULL)
    {
        return false;
    }

    bool ret = false;
    switch(event.message)
    {
    case WM_SIZE:
        {
            m_viewSize.cx = LOWORD(event.lParam);
            m_viewSize.cy = HIWORD(event.lParam);
            m_isInited = true;
        }
        break;
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        {
            if(m_isInited)
            {
                int x = LOWORD(event.lParam);
                int y = HIWORD(event.lParam);
                m_pListener->OnDoubleTap((event.message == WM_LBUTTONDBLCLK), x, y, m_viewSize.cx, m_viewSize.cy);
                ret = true;
            }
        }
        break;
    default:
        break;
    }
    return ret;
}
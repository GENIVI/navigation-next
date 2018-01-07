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

    @file     uithreadwin32.cpp
    @date     09/06/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for Win32 UI thread.

    Windows implementation for PAL UI thread.
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

#include "uithreadwin32.h"
#include "palstdlib.h"
#include <tchar.h>

// !!!! test: Currently extern for testing. @todo: make static again
LRESULT CALLBACK UIThreadWindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


namespace nimpal
{
    namespace taskqueue
    {
        static const WPARAM MSG_NEW_TASK = WM_USER + 100;
        static const WPARAM MSG_CANCEL_TASK = WM_USER + 101;


        LPCWSTR NIM_CLASS_NAME = _T("PALUIWINDOW");
        LPCWSTR NIM_TITLE_NAME = _T("PAL UI MESSAGE WINDOW");


        UiThreadWin32::UiThreadWin32(PAL_Instance* pal, TaskQueue* taskQueue, ThreadStateListener* listener)
            : Thread(pal, taskQueue, listener),
              m_palWnd(NULL),
              m_uiThreadId( GetCurrentThreadId() )
        {
        }

        UiThreadWin32::~UiThreadWin32(void)
        {
            if (m_palWnd != NULL)
            {
                DestroyWindow(m_palWnd);
                UnregisterClass(NIM_CLASS_NAME, NULL);
                m_palWnd = NULL;
            }
        }

        void UiThreadWin32::NewTaskNotify(void)
        {
            PostMessage(m_palWnd, MSG_NEW_TASK, reinterpret_cast<WPARAM>(this), 0);
        }

        void UiThreadWin32::CancelRunningTaskNotify(void)
        {
            PostMessage(m_palWnd, MSG_CANCEL_TASK, reinterpret_cast<WPARAM>(this), 0);
        }

        PAL_Error UiThreadWin32::StartThread(void)
        {
            if (GetState() != Invalid)
            {
                // Window was already created
                return PAL_Ok;
            }

            PAL_Error err = PAL_Failed;

            if (RegisterWindowClass() != 0)
            {
                m_palWnd = CreateWindow(NIM_CLASS_NAME, NIM_TITLE_NAME, 0,
                                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                        CW_USEDEFAULT, NULL, NULL, NULL, NULL);

                if (m_palWnd != NULL)
                {
                    err = PAL_Ok;
                    SetState(Pending);
                }
            }

            return err;
        }

        bool UiThreadWin32::WindowProc(UINT message)
        {
            if (message == MSG_NEW_TASK)
            {
                ExecuteTasks();
                return true;
            }
            else if (message == MSG_CANCEL_TASK)
            {
                // TODO: to figure out what to do
                return true;
            }

            return false;
        }

        ATOM UiThreadWin32::RegisterWindowClass()
        {
            WNDCLASS wc = {0};

            wc.style         = 0;
            wc.lpfnWndProc   = &UIThreadWindowProcedure;
            wc.cbClsExtra    = 0;
            wc.cbWndExtra    = 0;
            wc.hInstance     = 0;
            wc.hIcon         = 0;
            wc.hCursor       = 0;
            wc.hbrBackground = 0;
            wc.lpszMenuName  = 0;
            wc.lpszClassName = NIM_CLASS_NAME;

            return RegisterClass(&wc);
        }

        nb_threadId UiThreadWin32::ThreadId(void) const
        {
            return m_uiThreadId;
        }
    }
}

LRESULT CALLBACK UIThreadWindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    nimpal::taskqueue::UiThreadWin32* pThis = reinterpret_cast<nimpal::taskqueue::UiThreadWin32*>(wParam);
    if (pThis && pThis->WindowProc(message))
    {
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

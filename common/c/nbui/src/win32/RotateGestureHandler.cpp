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
@file     RotateGestureHandler.cpp
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

#include "RotateGestureHandler.h"
#include <cmath>

#define ENABLE_DEBUG_DRAW  0
#define DEBUG_LINE_LENGTH  12

#define PI_HALF            1.5707963f
#define PI                 3.1415926f
#define PI_TWO             6.2831852f
#define MIN_ANGLE          0.0019000f
#define MAX_ANGLE          0.7853981f
#define INF_ANGLE          9999999.0f
#define INERTIA_FACTOR     3.0000000f


RotateGestureDetector::RotateGestureDetector():m_pListener(NULL),
    m_isInited(false),
    m_lastAngle(INF_ANGLE),
    m_lastDir(0.f),
    m_isActived(false)
{
    m_centerPoint.x = 0;
    m_centerPoint.y = 0;
    m_lastPoint.x = 0;
    m_lastPoint.y = 0;
}

void
RotateGestureDetector::SetListener(IRotateGestureListener* listener)
{
    if(m_pListener != NULL)
    {
        delete m_pListener;
        m_pListener = NULL;
    }
    m_pListener = listener;
}

float
RotateGestureDetector::GetRotateAngle(int ptX, int ptY)const
{
    float x = static_cast<float>(ptX - m_centerPoint.x);
    float y = static_cast<float>(ptY - m_centerPoint.y);
    if(abs(x) < 1.f && abs(y) < 1.f)
    {
        return 0.f;
    }

    float tempX = static_cast<float>(abs(x));
    float tempY = static_cast<float>(abs(y));
    float tan = 0.f;

    if(tempX > tempY)
    {
        tan = tempY / tempX;
    }
    else
    {
        tan = tempX / tempY;
    }

    float theta = static_cast<float>(atan(tan));

    if(tempX < tempY)
    {
        theta = static_cast<float>(PI_HALF - theta);
    }
    if(x < 0.f)
    {
        theta = static_cast<float>(PI - theta);
    }
    if(y < 0.f)
    {
        theta = static_cast<float>(PI_TWO - theta);
    }

    return theta;
}

void
RotateGestureDetector::DebugDraw(HDC hDC)const
{
    HPEN hpen = ::CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
    HGDIOBJ original = ::SelectObject(hDC, hpen);

    ::MoveToEx(hDC, m_centerPoint.x-DEBUG_LINE_LENGTH, m_centerPoint.y, NULL);
    ::LineTo(hDC, m_centerPoint.x+DEBUG_LINE_LENGTH, m_centerPoint.y);

    ::MoveToEx(hDC, m_centerPoint.x, m_centerPoint.y-DEBUG_LINE_LENGTH, NULL);
    ::LineTo(hDC, m_centerPoint.x, m_centerPoint.y+DEBUG_LINE_LENGTH);

    ::MoveToEx(hDC, m_lastPoint.x-DEBUG_LINE_LENGTH, m_lastPoint.y, NULL);
    ::LineTo(hDC, m_lastPoint.x+DEBUG_LINE_LENGTH, m_lastPoint.y);

    ::MoveToEx(hDC, m_lastPoint.x, m_lastPoint.y-DEBUG_LINE_LENGTH, NULL);
    ::LineTo(hDC, m_lastPoint.x, m_lastPoint.y+DEBUG_LINE_LENGTH);

    ::SelectObject(hDC, original);
    ::DeleteObject(hpen);
}

bool
RotateGestureDetector::OnTouchEvent(const GestureEvent& event)
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
            m_centerPoint.x = LOWORD(event.lParam)/2;
            m_centerPoint.y = HIWORD(event.lParam)/2;
            m_lastAngle = INF_ANGLE;
            m_lastDir = 0.f;
            m_isInited = true;
        }
        break;
#if ENABLE_DEBUG_DRAW == 1
     case WM_PAINT:
        {
            if(m_isActived)
            {
                static PAINTSTRUCT ps;
                HDC hDC = ::BeginPaint(event.wnd, &ps);
                DebugDraw(hDC);
                ::EndPaint(event.wnd, &ps);
            }
        }

        break;
#endif
    case WM_LBUTTONDOWN:
        {
            if(m_isInited && (event.wParam & MK_CONTROL))
            {
                m_pListener->OnRotate(GRS_Began,
                    -GetRotateAngle(LOWORD(event.lParam), HIWORD(event.lParam)),
                    0.f);
                m_lastPoint.x = m_centerPoint.x;
                m_lastPoint.y = m_centerPoint.y;
                m_lastAngle = INF_ANGLE;
                m_lastDir = 0.f;
                m_isActived = true;
                ret = true;
            }
        }
        break;
    case WM_LBUTTONUP:
        {
            if(m_isActived)
            {
                m_pListener->OnRotate(GRS_Ended,
                    0.f,
                    m_lastDir);
                m_isActived = false;
                ret = true;
            }
        }
        break;
    case WM_MOUSEMOVE:
        {
            if(m_isActived && (event.wParam & MK_LBUTTON) && (event.wParam & MK_CONTROL))
            {
                int x = LOWORD(event.lParam);
                int y = HIWORD(event.lParam);
                if(m_lastPoint.x != x || m_lastPoint.y != y)
                {
                    float angle = -GetRotateAngle(x, y);
                    m_pListener->OnRotate(GRS_Changed,
                        angle,
                        0.f);
                    if(m_lastAngle != INF_ANGLE)
                    {
                        float det = abs(angle - m_lastAngle);
                        if(det > MIN_ANGLE)
                        {
                            det = (det > MAX_ANGLE) ? MAX_ANGLE: det;
                            det *= INERTIA_FACTOR;
                            if(angle < PI_HALF && m_lastAngle > PI)
                            {
                                m_lastDir = -det;
                            }
                            else if(angle > PI && m_lastAngle < PI_HALF)
                            {
                                m_lastDir = det;
                            }
                            else
                            {
                                m_lastDir = ((angle - m_lastAngle))>0.f?det:-det;
                            }
                        }
                    }
                    m_lastAngle = angle;
                    m_lastPoint.x = x;
                    m_lastPoint.y = y;
                    ret = true;
                }
#if ENABLE_DEBUG_DRAW == 1
                InvalidateRect(event.wnd, NULL, TRUE);
#endif
            }
        }
        break;
    default:
        break;
    }

    return ret;
}
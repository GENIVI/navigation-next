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

#include "doubletapgesturerecognizer.h"
#include <QLineF>

static float TAP_DISTANCE = 30.0f;
static int TAP_TIMEOUT = 500;

DoubleTapGesture::DoubleTapGesture(QObject *parent)
    : QGesture(parent)
{

}

DoubleTapGesture::~DoubleTapGesture()
{

}

DoubleTapGestureRecognizer::DoubleTapGestureRecognizer()
    :m_tapNum(0)
{

}

DoubleTapGestureRecognizer::~DoubleTapGestureRecognizer()
{

}

QGestureRecognizer::Result DoubleTapGestureRecognizer::recognize(QGesture *state, QObject *watched, QEvent *event)
{
    DoubleTapGesture *gesture = static_cast<DoubleTapGesture *>(state);

    const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);

    QGestureRecognizer::Result result = QGestureRecognizer::Ignore;

    if (ev->target() != watched)
    {
        return QGestureRecognizer::Ignore;
    }

    switch (event->type())
    {
    case QEvent::TouchBegin:
    {
        const QTouchEvent::TouchPoint& point0 = ev->touchPoints().front();
        int ms = QTime::currentTime().msecsSinceStartOfDay() - m_lastPressTime;
        if (ms > TAP_TIMEOUT && m_tapNum > 0)
        {
            // reset
            m_tapNum = 0;
        }

        if (m_tapNum == 0)
        {
            if (ev->touchPoints().size() == 1)
            {
                m_lastPressTime = QTime::currentTime().msecsSinceStartOfDay();
                m_lastPressPosition = point0.pos();
                m_touchNum = 1;
                m_firstPressed = true;
                result = QGestureRecognizer::MayBeGesture;
                //qDebug("****************double detector: MayBeGesture m_tapNum=%d m_lastPressTime=%d", m_tapNum, m_lastPressTime);
            }
        }
        else
        {
            result = QGestureRecognizer::CancelGesture;
            if (m_touchNum == 1 && ev->touchPoints().size() == 1)
            {
                QLineF line(ev->touchPoints().front().pos(), m_lastPressPosition);

                //qDebug("****************double detector: FinishGesture m_tapNum=%d length=%f ms=%d", m_tapNum, line.length(), ms);
                if (line.length() <= TAP_DISTANCE)
                {
                    result = QGestureRecognizer::FinishGesture;
                    gesture->m_position = m_lastPressPosition;
                }
            }
            m_tapNum = 0;
            m_firstPressed = false;
        }
        break;
    }
    case QEvent::TouchEnd:
        if (m_firstPressed)
        {
            ++m_tapNum;
        }
        break;
    case QEvent::TouchUpdate:
    {
        if (ev->touchPoints().size() != 1)
        {
            m_touchNum = ev->touchPoints().size();
            m_tapNum = 0;
            m_firstPressed = false;
            result = QGestureRecognizer::CancelGesture;
            //qDebug("double detector: TouchUpdate m_tapNum=%d", m_tapNum);
        }
        break;
    }
    default:
        result = QGestureRecognizer::Ignore;
        break;
    }

    //qDebug("eventType=%d, result=%d event=%p", event->type(), result, event);
    return result;
}

QGesture *DoubleTapGestureRecognizer::create(QObject *target)
{
    return new DoubleTapGesture();
}

void DoubleTapGestureRecognizer::reset(QGesture *state)
{
    DoubleTapGesture *g = static_cast<DoubleTapGesture *>(state);
    g->m_position.setX(0);
    g->m_position.setY(0);
    QGestureRecognizer::reset(state);
}

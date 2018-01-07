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

#include "twofingerstapgesturerecognizer.h"
#include <QLine>

static float TAP_DISTANCE = 10.0f;
static int TAP_TIMEOUT = 300;

TwoFingersTapGesture::TwoFingersTapGesture(QObject *parent)
    : QGesture(parent)
{

}

TwoFingersTapGesture::~TwoFingersTapGesture()
{

}

TwoFingersTapGestureRecognizer::TwoFingersTapGestureRecognizer()
{

}

TwoFingersTapGestureRecognizer::~TwoFingersTapGestureRecognizer()
{

}

QGestureRecognizer::Result TwoFingersTapGestureRecognizer::recognize(QGesture *state, QObject *watched, QEvent *event)
{
    TwoFingersTapGesture *gesture = static_cast<TwoFingersTapGesture *>(state);

    const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
    if (ev->target() != watched)
    {
        return QGestureRecognizer::Ignore;
    }

    QGestureRecognizer::Result result;

    switch (event->type())
    {
    case QEvent::TouchBegin:
    {
        result = QGestureRecognizer::MayBeGesture;
        m_touches.clear();

        if (ev->touchPoints().size() > 0)
        {
            const QTouchEvent::TouchPoint& p = ev->touchPoints().first();
            TouchState state;
            state.m_touchPoint = p.pos();
            state.m_touchStartTime = QTime::currentTime().msecsSinceStartOfDay();
            m_touches[p.id()] = state;
        }
        break;
    }
    case QEvent::TouchEnd:
    {
        result = QGestureRecognizer::CancelGesture;
        if (gesture->state() != Qt::NoGesture &&
            gesture->state() != Qt::GestureCanceled)
        {
            if (ev->touchPoints().size() > 0)
            {
                QTouchEvent::TouchPoint point0 = ev->touchPoints().at(0);

                QMap<int,TouchState>::iterator it = m_touches.find(point0.id());
                if (it != m_touches.end())
                {
                    QLineF line(point0.pos(), point0.startPos());
                    int deltaTime = QTime::currentTime().msecsSinceStartOfDay() - it->m_touchStartTime;
                    if (line.length() <= TAP_DISTANCE && deltaTime <= TAP_TIMEOUT)
                    {
                        result = QGestureRecognizer::FinishGesture;
                    }
                }
            }
        }
        break;
    }
    case QEvent::TouchUpdate:
    {
        result = QGestureRecognizer::CancelGesture;

        foreach (const QTouchEvent::TouchPoint& p, ev->touchPoints())
        {
            switch (p.state())
            {
            case Qt::TouchPointPressed:
            {
                if (m_touches.size() == 1)
                {
                    gesture->m_offset = (p.pos() + m_touches.first().m_touchPoint) * 0.5f;

                    TouchState state;
                    state.m_touchPoint = p.pos();
                    state.m_touchStartTime = QTime::currentTime().msecsSinceStartOfDay();
                    m_touches[p.id()] = state;

                    result = QGestureRecognizer::TriggerGesture;
                }
                break;
            }
            default:
                result = QGestureRecognizer::Ignore;
                break;
            }
        }

        break;
    }
    default:
        result = QGestureRecognizer::Ignore;
        break;
    }

    return result;
}

QGesture *TwoFingersTapGestureRecognizer::create(QObject *target)
{
    return new TwoFingersTapGesture();
}

void TwoFingersTapGestureRecognizer::reset(QGesture *state)
{
    TwoFingersTapGesture *g = static_cast<TwoFingersTapGesture *>(state);
    g->m_offset.setX(0);
    g->m_offset.setY(0);
    QGestureRecognizer::reset(state);
}

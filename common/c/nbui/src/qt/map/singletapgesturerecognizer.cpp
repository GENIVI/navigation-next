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

#include "singletapgesturerecognizer.h"
#include <QLineF>

static float TAP_DISTANCE = 10.0f;
static int TAP_TIMEOUT = 300;

SingleTapGesture::SingleTapGesture(QObject *parent)
    : QGesture(parent)
{

}

SingleTapGesture::~SingleTapGesture()
{

}

SingleTapGestureRecognizer::SingleTapGestureRecognizer()
{

}

SingleTapGestureRecognizer::~SingleTapGestureRecognizer()
{

}

QGestureRecognizer::Result SingleTapGestureRecognizer::recognize(QGesture *state, QObject *watched, QEvent *event)
{
    SingleTapGesture *gesture = static_cast<SingleTapGesture *>(state);

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
        result = QGestureRecognizer::MayBeGesture;
        m_lastPressTime = QTime::currentTime().msecsSinceStartOfDay();
        m_touchNum = 1;
        break;
    }
    case QEvent::TouchEnd:
    {
        result = QGestureRecognizer::CancelGesture;
        if (gesture->state() != Qt::GestureCanceled &&
            m_touchNum == 1 &&
            ev->touchPoints().size() == 1)
        {
            QTouchEvent::TouchPoint point0 = ev->touchPoints().at(0);
            QLineF line(point0.pos(), point0.startPos());
            if (line.length() <= TAP_DISTANCE)
            {
                int ms = QTime::currentTime().msecsSinceStartOfDay() - m_lastPressTime;
                if (ms <= TAP_TIMEOUT)
                {
                    result = QGestureRecognizer::FinishGesture;
                    gesture->m_position = point0.startPos();
                }
            }
        }
        break;
    }
    case QEvent::TouchUpdate:
    {
        if (gesture->state() != Qt::NoGesture)
        {
            if (ev->touchPoints().size() != 1)
            {
                m_touchNum = ev->touchPoints().size();
                result = QGestureRecognizer::CancelGesture;
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

QGesture *SingleTapGestureRecognizer::create(QObject *target)
{
    return new SingleTapGesture();
}

void SingleTapGestureRecognizer::reset(QGesture *state)
{
    SingleTapGesture *g = static_cast<SingleTapGesture *>(state);
    g->m_position.setX(0);
    g->m_position.setY(0);
    QGestureRecognizer::reset(state);
}

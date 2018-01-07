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

#include "pangesturerecognizer.h"
#include <QLine>
#include <QWidget>
#include <QVector2D>

PanGesture::PanGesture(QObject *parent)
    : QGesture(parent)
{

}

PanGesture::~PanGesture()
{

}

PanGestureRecognizer::PanGestureRecognizer()
{

}

PanGestureRecognizer::~PanGestureRecognizer()
{

}

QGestureRecognizer::Result PanGestureRecognizer::recognize(QGesture *state, QObject *watched, QEvent *event)
{
    PanGesture *gesture = static_cast<PanGesture *>(state);

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
        m_startMove = false;
        m_lastPosition = ev->touchPoints().first().pos();
        m_startWatchPosition = ev->touchPoints().first().pos();
        m_lastTime = QTime::currentTime().msecsSinceStartOfDay();
        m_touchPoints = 1;
        m_velocityHistory.clear();
        break;
    }
    case QEvent::TouchEnd:
    {
        if (gesture->state() != Qt::NoGesture)
        {
            result = QGestureRecognizer::FinishGesture;

            if (m_startMove)
            {
                float ms = QTime::currentTime().msecsSinceStartOfDay() - m_lastTime;
                if (ms > 0)
                {
                    m_velocityHistory.push_back(QVector2D((ev->touchPoints().first().pos() - m_lastPosition) / (ms * 0.001f)));
                    m_lastTime = QTime::currentTime().msecsSinceStartOfDay();
                }

                QVector2D v(0,0);
                int num = 0;
                const int VELOCITY_SAMPLE_COUNT = 10;
                for (int i = 0; i < VELOCITY_SAMPLE_COUNT && i < m_velocityHistory.size(); ++i)
                {
                    v += m_velocityHistory[m_velocityHistory.size() - 1 - i];
                    ++num;
                }
                if (num > 0)
                {
                    v *= 1.0f / num;
                    const float MAX_SWIPE_SPEED = 5000.0f;
                    if (v.length() > MAX_SWIPE_SPEED)
                    {
                        v.normalize();
                        v *= MAX_SWIPE_SPEED;
                    }
                    gesture->m_velocity = v.toPointF();
                }
                else
                {
                    gesture->m_velocity = QPointF(0,0);
                }
            }
            else
            {
                gesture->m_velocity = QPointF(0, 0);
            }
        }
        else
        {
            result = QGestureRecognizer::CancelGesture;
        }
        m_startMove = false;
        break;
    }
    case QEvent::TouchUpdate:
    {
        if (m_touchPoints == 1 && ev->touchPoints().size() == 1)
        {
            QTouchEvent::TouchPoint point0 = ev->touchPoints().at(0);
            QLineF line(point0.pos(), m_startWatchPosition);
            if (m_startMove || line.length() > 50.0f)
            {
                result = QGestureRecognizer::TriggerGesture;
                m_startMove = true;
            }

            float ms = QTime::currentTime().msecsSinceStartOfDay() - m_lastTime;
            if (ms > 0)
            {
                QVector2D v((point0.pos() - m_lastPosition) / (ms * 0.001f));
                m_velocityHistory.push_back(v);
                m_lastTime = QTime::currentTime().msecsSinceStartOfDay();
            }

            gesture->m_deltaPosition = point0.pos() - m_lastPosition;
            gesture->m_offset = point0.pos();
            m_lastPosition = point0.pos();
        }
        else
        {
            if (gesture->state() != Qt::NoGesture)
            {
                m_velocityHistory.clear();
                m_startMove = false;
                result = QGestureRecognizer::FinishGesture;
                gesture->m_velocity = QPointF(0,0);
            }
            else
            {
                result = QGestureRecognizer::Ignore;
            }
            m_startWatchPosition = ev->touchPoints().first().pos();
        }
        if (ev->touchPoints().size() > m_touchPoints)
        {
            m_touchPoints = ev->touchPoints().size();
        }
        break;
    }
    default:
        result = QGestureRecognizer::Ignore;
        break;
    }

    return result;
}

QGesture *PanGestureRecognizer::create(QObject *target)
{
    if (target && target->isWidgetType()) {
        static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
    }
    QGesture* g = new PanGesture();
    g->setGestureCancelPolicy(QGesture::CancelAllInContext);
    return g;
}

void PanGestureRecognizer::reset(QGesture *state)
{
    PanGesture *g = static_cast<PanGesture *>(state);
    g->m_deltaPosition.setX(0);
    g->m_deltaPosition.setY(0);
    QGestureRecognizer::reset(state);
}

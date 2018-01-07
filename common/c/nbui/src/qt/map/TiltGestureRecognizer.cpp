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

    @file TiltGestureRecognizer.cpp
*/
/*
* (C) Copyright 2014 by TeleCommunication Systems, Inc.
*
* The information contained herein is confidential, proprietary
* to TeleCommunication Systems, Inc., and considered a trade secret
* as defined in section 499C of the penal code of the State of
* California. Use of this information by anyone other than
* authorized employees of TeleCommunication Systems is granted only
* under a written non-disclosure agreement, expressly prescribing
* the scope and manner of such use.
*
--------------------------------------------------------------------------*/

//#include <QEvent>
#include <QWidget>
#include "TiltGestureRecognizer.h"
#include <math.h>
#include <QTime>

TiltGesture::TiltGesture(QObject *parent)
    : QGesture(parent)
    , m_tiltAngle(0.0f)
{

}

TiltGesture::~TiltGesture()
{

}

TiltGestureRecognizer::TiltGestureRecognizer()
    : m_gestureRecognized(false)
    , m_lastTime(0)
{
}

TiltGestureRecognizer::~TiltGestureRecognizer()
{

}

QGesture* TiltGestureRecognizer::create(QObject *target)
{
    if (target && target->isWidgetType())
    {
        static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
    }

    return new TiltGesture();
}

static float getRotateAngle(const QTouchEvent::TouchPoint& p0, const QTouchEvent::TouchPoint& p1)
{
    return -QLineF(p0.pos(), p1.pos()).angle() + QLineF(p0.startPos(), p1.startPos()).angle();
}

static float getScale(const QTouchEvent::TouchPoint& p0, const QTouchEvent::TouchPoint& p1)
{
    return QLineF(p0.pos(), p1.pos()).length() / QLineF(p0.startPos(), p1.startPos()).length();
}

static float getTilt(const QTouchEvent::TouchPoint& point0, const QTouchEvent::TouchPoint& point1)
{
    float startY = (point0.startPos().y() + point1.startPos().y()) * 0.5f;
    float endY = (point0.pos().y() + point1.pos().y()) * 0.5f;
    return endY - startY;
}

QGestureRecognizer::Result TiltGestureRecognizer::recognize(QGesture *state,
                                                            QObject *watched,
                                                            QEvent *event)
{
    TiltGesture *gesture = static_cast<TiltGesture *>(state);

    const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
    if (ev->target() != watched)
    {
        return QGestureRecognizer::Ignore;
    }

    QGestureRecognizer::Result result = QGestureRecognizer::Ignore;

    switch (event->type())
    {
    case QEvent::TouchBegin:
    {
        result = QGestureRecognizer::MayBeGesture;
        m_gestureRecognized = false;
        m_scaleSpeedHistory.clear();
        m_rotateSpeedHistory.clear();
        break;
    }
    case QEvent::TouchEnd:
    {
        if (gesture->state() != Qt::NoGesture)
        {
            result = QGestureRecognizer::FinishGesture;
        }
        else
        {
            result = QGestureRecognizer::CancelGesture;
        }
        m_gestureRecognized = false;
        break;
    }
    case QEvent::TouchUpdate:
    {
        gesture->m_rotateSpeed = 0;
        gesture->m_scaleSpeed = 0;
        if (ev->touchPoints().size() == 2)
        {
            QTouchEvent::TouchPoint point0 = ev->touchPoints().at(0);
            QTouchEvent::TouchPoint point1 = ev->touchPoints().at(1);
            QPointF centerPoint((point0.pos().x() + point1.pos().x()) * 0.5f, (point0.pos().y() + point1.pos().y()) * 0.5f);

            if (m_gestureRecognized)
            {
                if (m_motionState == TiltGesture::Tilt)
                {
                    gesture->m_motionState = TiltGesture::Tilt;
                    gesture->m_tiltAngle = getTilt(point0, point1);
                    gesture->m_centerPoint = centerPoint;
                }
                else if (m_motionState == TiltGesture::Scale)
                {
                    gesture->m_motionState = TiltGesture::Scale;
                    gesture->m_scale = getScale(point0, point1);
                    gesture->m_centerPoint = centerPoint;

                    int now = QTime::currentTime().msecsSinceStartOfDay();
                    int ms = now - m_lastTime;
                    if (ms > 0)
                    {
                        m_scaleSpeedHistory.push_back((gesture->m_scale - m_lastScale) * 1000.0f / ms);
                        m_lastScale = gesture->m_scale;
                        m_lastTime = now;
                    }
                }
                else if (m_motionState == TiltGesture::Rotate)
                {
                    gesture->m_motionState = TiltGesture::Rotate;
                    gesture->m_rotateAngle = getRotateAngle(point0, point1);
                    gesture->m_centerPoint = centerPoint;

                    int now = QTime::currentTime().msecsSinceStartOfDay();
                    int ms = now - m_lastTime;
                    if (ms > 0)
                    {
                        QVector2D curDir = QVector2D(point1.pos() - point0.pos()).normalized();
                        float r = (float)acos(QVector2D::dotProduct(m_lastDir, curDir)) * 180.0f / 3.1415926f;
                        float cross = m_lastDir.x() * curDir.y() - m_lastDir.y() * curDir.x();
                        if (cross < 0)
                        {
                            r = -r;
                        }
                        m_lastDir = curDir;
                        m_rotateSpeedHistory.push_back(r * 1000.0f / ms);

                        m_lastTime = now;
                    }
                }

                if (point0.state() == Qt::TouchPointReleased || point1.state() == Qt::TouchPointReleased)
                {
                    int now = QTime::currentTime().msecsSinceStartOfDay();
                    int ms = now - m_lastTime;
                    if (ms > 0)
                    {
                        m_scaleSpeedHistory.push_back((gesture->m_scale - m_lastScale) * 1000.0f / ms);
                        m_lastScale = gesture->m_scale;

                        QVector2D curDir = QVector2D(point1.pos() - point0.pos()).normalized();
                        float r = (float)acos(QVector2D::dotProduct(m_lastDir, curDir)) * 180.0f / 3.1415926f;
                        float cross = m_lastDir.x() * curDir.y() - m_lastDir.y() * curDir.x();
                        if (cross < 0)
                        {
                            r = -r;
                        }
                        m_lastDir = curDir;
                        m_rotateSpeedHistory.push_back(r * 1000.0f / ms);

                        m_lastTime = now;
                    }

                    const float MAX_ROTATE_SPEED = 540.0f;
                    const float SCALE_FACTOR = 0.03f;
                    const int VELOCITY_SAMPLES = 10;
                    int num = 0;
                    float v = 0.0f;
                    for (int i = 0; i < VELOCITY_SAMPLES && i < (int)m_scaleSpeedHistory.size(); ++i)
                    {
                        v += m_scaleSpeedHistory[m_scaleSpeedHistory.size() - 1 - i];
                        ++num;
                    }

                    if (m_motionState == TiltGesture::Scale && num > 0)
                    {
                        gesture->m_scaleSpeed = v / num * SCALE_FACTOR;
                    }
                    else
                    {
                        gesture->m_scaleSpeed = 0.0f;
                    }

                    {
                        int num = 0;
                        float v = 0.0f;
                        for (int i = 0; i < VELOCITY_SAMPLES && i < (int)m_rotateSpeedHistory.size(); ++i)
                        {
                            v += m_rotateSpeedHistory[m_rotateSpeedHistory.size() - 1 - i];
                            ++num;
                        }

                        if (m_motionState == TiltGesture::Rotate && num > 0)
                        {
                            float rotateSpeed = v / num;
                            if (rotateSpeed > MAX_ROTATE_SPEED)
                            {
                                rotateSpeed = MAX_ROTATE_SPEED;
                            }
                            if (rotateSpeed < -MAX_ROTATE_SPEED)
                            {
                                rotateSpeed = -MAX_ROTATE_SPEED;
                            }
                            gesture->m_rotateSpeed = -rotateSpeed;
                        }
                        else
                        {
                            gesture->m_rotateSpeed = 0.0f;
                        }
                    }
                    result = QGestureRecognizer::FinishGesture;
                }
                else
                {
                    result = QGestureRecognizer::TriggerGesture;
                }
            }
            else
            {
                if (point0.state() == Qt::TouchPointPressed ||
                    point1.state() == Qt::TouchPointPressed)
                {
                    mStartPos0 = point0.pos();
                    mStartPos1 = point1.pos();
                }

                if (point0.state() != Qt::TouchPointPressed &&
                    point0.state() != Qt::TouchPointReleased &&
                    point1.state() != Qt::TouchPointPressed &&
                    point1.state() != Qt::TouchPointReleased)
                {
                    if (isTilt(ev))
                    {
                        gesture->m_tiltAngle = getTilt(point0, point1);
                        gesture->m_centerPoint = centerPoint;
                        gesture->m_motionState = TiltGesture::Tilt;
                        result = QGestureRecognizer::TriggerGesture;
                        m_gestureRecognized = true;
                        m_lastTime = QTime::currentTime().msecsSinceStartOfDay();
                        m_motionState = TiltGesture::Tilt;
                    }
                    else if (isRotate(ev))
                    {
                        gesture->m_rotateAngle = getRotateAngle(point0, point1);
                        gesture->m_centerPoint = centerPoint;
                        gesture->m_motionState = TiltGesture::Rotate;
                        result = QGestureRecognizer::TriggerGesture;
                        m_gestureRecognized = true;
                        m_lastTime = QTime::currentTime().msecsSinceStartOfDay();
                        m_lastDir = QVector2D(point1.pos() - point0.pos()).normalized();
                        m_motionState = TiltGesture::Rotate;

                        m_rotateSpeedHistory.clear();
                    }
                    else if (isZoom(ev))
                    {
                        gesture->m_scale = getScale(point0, point1);
                        gesture->m_centerPoint = centerPoint;
                        gesture->m_motionState = TiltGesture::Scale;
                        result = QGestureRecognizer::TriggerGesture;
                        m_gestureRecognized = true;
                        m_lastTime = QTime::currentTime().msecsSinceStartOfDay();
                        m_lastScale = gesture->m_scale;
                        m_motionState = TiltGesture::Scale;

                        m_scaleSpeedHistory.clear();
                    }
                }

            }
        }
        else
        {
            if (gesture->state() != Qt::NoGesture)
            {
                result = QGestureRecognizer::FinishGesture;
            }
            else
            {
                result = QGestureRecognizer::CancelGesture;
            }
            m_gestureRecognized = false;
        }
        break;
    }
    default:
        result = QGestureRecognizer::Ignore;
        break;
    }

    return result;
}

void TiltGestureRecognizer::reset(QGesture *state)
{
    TiltGesture *tilt = static_cast<TiltGesture *>(state);
    tilt->m_tiltAngle = 0;
    tilt->m_rotateAngle = 0;
    tilt->m_scale = 1.0;
    tilt->m_motionState = TiltGesture::None;
    QGestureRecognizer::reset(state);
}

bool TiltGestureRecognizer::isTilt(const QTouchEvent *ev)
{
    QTouchEvent::TouchPoint point0 = ev->touchPoints().at(0);
    QTouchEvent::TouchPoint point1 = ev->touchPoints().at(1);

    float delta_y0 = point0.pos().y() - mStartPos0.y();
    float delta_y1 = point1.pos().y() - mStartPos1.y();
    float TILT_Y_MOVE = 40.0f;

    if (delta_y0 * delta_y1 > 0.0f &&
        fabs(delta_y0 + delta_y1) * 0.5f >= TILT_Y_MOVE)
    {
        return true;
    }
    return false;
}

bool TiltGestureRecognizer::isZoom(const QTouchEvent *ev)
{
    float SCALE_DELTA = 150.0f;
    QTouchEvent::TouchPoint point0 = ev->touchPoints().at(0);
    QTouchEvent::TouchPoint point1 = ev->touchPoints().at(1);

    float distStart = QLineF(mStartPos0, mStartPos1).length();
    float distCur = QLineF(point0.pos(), point1.pos()).length();

    if (fabs(distCur - distStart) >= SCALE_DELTA)
    {
        return true;
    }
    return false;
}

bool TiltGestureRecognizer::isRotate(const QTouchEvent *ev)
{
    float ROTATE_DELTA = 10.0f;
    float ROTATE_MOVE_DISTANCE = 80.0f;
    QTouchEvent::TouchPoint point0 = ev->touchPoints().at(0);
    QTouchEvent::TouchPoint point1 = ev->touchPoints().at(1);

    QVector2D v0(mStartPos1 - mStartPos0);
    v0.normalize();
    QVector2D v1(point1.pos() - point0.pos());
    v1.normalize();

    float d0 = QLineF(mStartPos0,point0.pos()).length();
    float d1 = QLineF(mStartPos1,point1.pos()).length();

    float angle = acos(QVector2D::dotProduct(v0, v1)) * 180.0f / 3.1415926f;
    if (angle >= ROTATE_DELTA &&
        (d0 > ROTATE_MOVE_DISTANCE || d1 > ROTATE_MOVE_DISTANCE))
    {
        return true;
    }
    return false;
}

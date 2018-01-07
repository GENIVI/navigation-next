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

#include "gesturedetector.h"
#include <QDebug>
#include <QLineF>

#ifndef _INTEL_NUC
#define _INTEL_NUC
#include <math.h>
#endif _INTEL_NUC

#ifndef M_PI
#define M_PI       3.14159265358979323846   // pi
#endif

static float PAN_TOLERANCE = 50;
static int PAN_VELOCITY_SAMPLES = 3;
static float MAX_SWIPE_SPEED = 5000.0f;
static float LONG_PRESS_TOLERANCE = 50;
static float LONG_PRESS_INTERVAL = 700;
static float SCALE_TOLERANCE = 50;
static float TAP_TOLERANCE = 50;
static float TAP_INTERVAL = 400;
static float DOUBLE_TAP_INTERVAL = 400;
static float PINCH_TOLERANCE = 70;
static int PINCH_VELOCITY_SAMPLES = 3;
static float PINCH_DETECT_ROTATE_ANGLE = 20.0f;
static float PINCH_DETECT_INVTERVAL = 100.0f;
static float MAX_ROTATE_SPEED = 540.0f;

GestureDetector::GestureDetector()
    :mCurrent(NULL)
{
}

void GestureDetector::touchEvent(QTouchEvent *event)
{
    switch (event->type())
    {
    case QEvent::TouchBegin:
    {
        mTime.restart();
        break;
    }
    case QEvent::TouchEnd:
    {
        break;
    }
    case QEvent::TouchCancel:
    {
        break;
    }
    case QEvent::TouchUpdate:
    {
        break;
    }
    default:
        break;
    }

    for (int i = 0; i < mRecognizers.size(); ++i)
    {
        GestureRecognizer* r = mRecognizers[i];
        if (r == mCurrent)
        {
            break;
        }
        if (r->onTouch(event, mHistory))
        {
            if (mCurrent)
            {
                stopCurrentGesture();
            }
            startGesture(r);
            break;
        }
    }
    if (mCurrent)
    {
        if (!mCurrent->onTouch(event, mHistory))
        {
            stopCurrentGesture();
        }
    }
    saveHistory(event);
}

void GestureDetector::saveHistory(QTouchEvent* event)
{
    GestureTouch touch;
    foreach(const QTouchEvent::TouchPoint& tp, event->touchPoints())
    {
        touch.points.push_back(tp.pos());
    }
    touch.t = mTime.elapsed();
    mHistory.push_back(touch);
}

void GestureDetector::stopCurrentGesture()
{
    mCurrent->stopGesture();
    mCurrent = NULL;
}

void GestureDetector::startGesture(GestureRecognizer* r)
{
    r->startGesture();
    mCurrent = r;
}

void GestureDetector::addGestureRecognizer(GestureRecognizer* r)
{
    mRecognizers.push_back(r);
}
//****************************************************
GestureRecognizer::GestureRecognizer(QObject* parent)
    :QObject(parent)
    ,mGestureStarted(false)
{
}

GestureRecognizer::~GestureRecognizer()
{
}

void GestureRecognizer::startGesture()
{
    mGestureStarted = true;
}

void GestureRecognizer::stopGesture()
{
    mGestureStarted = false;
}
//****************************************************
static float distance(const QPointF& p0, const QPointF& p1)
{
    return QVector2D(p0).distanceToPoint(QVector2D(p1));
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
//****************************************************
PinchRecognizer::PinchRecognizer(QObject* parent)
    :GestureRecognizer(parent)
    ,mState(StateNone)
    ,mZoomSpeed(0)
    ,mRotateSpeed(0)
    ,mStartLength(1)
    ,mStartAngle(0)
    ,mStartTilt(0)
    ,mLastLength(1)
    ,mLastAngle(0)
    ,mLastTilt(0)
{
}

bool PinchRecognizer::onTouch(QTouchEvent* event, QList<GestureTouch>& history)
{
    if (event->touchPoints().size() != 2)
    {
        return false;
    }

    QTouchEvent::TouchPoint tp0 = event->touchPoints()[0];
    QTouchEvent::TouchPoint tp1 = event->touchPoints()[1];

    if (tp0.state() == Qt::TouchPointReleased || tp1.state() == Qt::TouchPointReleased)
    {
        return false;
    }

    if (mGestureStarted)
    {
        switch (mState)
        {
        case StateZoom:
        {
            float length = distance(tp0.pos(), tp1.pos());
            mLastLength = length;
            emit zoom(mCenter, length / mStartLength);
            break;
        }
        case StateRotate:
        {
            float angle = getRotateAngle(tp0, tp1);
            mLastAngle = angle;
            emit rotate(mCenter, angle - mStartAngle);
            break;
        }
        case StateTilt:
        {
            float t = getTilt(tp0, tp1);
            mLastTilt = t;
            emit tilt(t - mStartTilt);
            break;
        }
        default:
            break;
        }

        if (mPoints.size() >= PINCH_VELOCITY_SAMPLES)
        {
            mPoints.pop_front();
        }
        Touch touch;
        touch.rotate = mLastAngle;
        touch.length = mLastLength;
        touch.time = mTime.elapsed();
        mPoints.push_back(touch);

        return true;
    }
    else
    {
        if (distance(tp0.pos(), tp0.startPos()) >= PINCH_TOLERANCE || distance(tp1.pos(), tp1.startPos()) >= PINCH_TOLERANCE)
        {
            float lastLength = distance(tp0.startPos(), tp1.startPos());
            float length = distance(tp0.pos(), tp1.pos());
            float deltaLength = length - lastLength;
            if (deltaLength < 0)
            {
                deltaLength = -deltaLength;
            }

            float maxLength = length;
            if (maxLength < lastLength)
            {
                maxLength = lastLength;
            }
            float rotateAngle = getRotateAngle(tp0, tp1);
            if (rotateAngle < 0)
            {
                rotateAngle = -rotateAngle;
            }
            float deltaRotate = M_PI * maxLength * rotateAngle / 180.0f;

            float tilt = getTilt(tp0, tp1);
            if (tilt < 0)
            {
                tilt = -tilt;
            }

            //qDebug("detect pinch: scale=%f, rotate=%f, tilt=%f", deltaLength, deltaRotate, tilt);

            mCenter = (tp0.startPos() + tp1.startPos()) * 0.5f;
            if (tilt > deltaLength && tilt > deltaRotate)
            {
                mState = StateTilt;
                mLastTilt = mStartTilt = getTilt(tp0, tp1);
            }
            else if (deltaRotate > tilt && deltaRotate > deltaLength)
            {
                mState = StateRotate;
                mLastAngle = mStartAngle = getRotateAngle(tp0, tp1);
            }
            else
            {
                mState = StateZoom;
                mLastLength = mStartLength = length;
            }
            return true;
        }
        mState = StateNone;
        return false;
    }
}

void PinchRecognizer::startGesture()
{
    if (mState == StateZoom)
    {
        emit zoomStart(mCenter);
    }
    else if (mState == StateRotate)
    {
        emit rotateStart(mCenter);
    }
    else if (mState == StateTilt)
    {
        emit tiltStart();
    }
    mGestureStarted = true;

    mTime.restart();
    mPoints.clear();
    Touch touch;
    touch.rotate = mLastAngle;
    touch.length = mLastLength;
    touch.time = 0;
    mPoints.push_back(touch);
    mZoomSpeed = 0;
    mRotateSpeed = 0;
}

void PinchRecognizer::stopGesture()
{
    if (mGestureStarted)
    {
        if (mState == StateZoom)
        {
            float total = 0;
            int n = 0;
            for (int i = 1; i < mPoints.size(); ++i)
            {
                const Touch& t0 = mPoints[i - 1];
                const Touch& t1 = mPoints[i];
                int t = t1.time - t0.time;
                if (t > 0)
                {
                    float l = t1.length - t0.length;
                    total += l;
                    ++n;
                }
            }
            if (n > 0)
            {
                mZoomSpeed = (total / n) / mLastLength;
            }
            emit zoomEnd(mCenter, mLastLength / mStartLength, mZoomSpeed);
        }
        else if (mState == StateRotate)
        {
            float total = 0;
            int n = 0;
            if (mPoints.size() >= 2)
            {
                const Touch& t0 = mPoints.first();
                const Touch& t1 = mPoints.last();
                int t = t1.time - t0.time;
                if (t > 0)
                {
                    float a0 = t0.rotate * M_PI / 180.0f;
                    float a1 = t1.rotate * M_PI / 180.0f;
                    QVector2D lastDir(cos(a0), sin(a0));
                    QVector2D curDir(cos(a1), sin(a1));
                    float r = (float)acos(QVector2D::dotProduct(lastDir, curDir)) * 180.0f / M_PI;
                    float cross = lastDir.x() * curDir.y() - lastDir.y() * curDir.x();
                    if (cross < 0)
                    {
                        r = -r;
                    }
                    //qDebug() << "r0=" << t0.rotate << "t1=" << t1.rotate << "r=" << r;
                    r = r / t * 1000.0f;
                    total += r;
                    ++n;
                }
            }
            if (n > 0)
            {
                mRotateSpeed = total / n;
                if ( mRotateSpeed > MAX_ROTATE_SPEED)
                {
                    mRotateSpeed = MAX_ROTATE_SPEED;
                }
                else if (mRotateSpeed < -MAX_ROTATE_SPEED)
                {
                    mRotateSpeed = -MAX_ROTATE_SPEED;
                }
            }
            emit rotateEnd(mCenter, mLastAngle - mStartAngle, mRotateSpeed);
        }
        else if (mState == StateTilt)
        {
            emit tiltEnd();
        }
    }
    mGestureStarted = false;
    mState = StateNone;
}

//****************************************************
PanRecognizer::PanRecognizer(QObject* parent)
    :GestureRecognizer(parent)
    ,mMaxTouchPoints(0)
{
}

bool PanRecognizer::onTouch(QTouchEvent* event, QList<GestureTouch>& history)
{
    if (event->type() == QEvent::TouchBegin)
    {
        mMaxTouchPoints = 1;
    }
    int touchPointNum = event->touchPoints().size();
    if (mMaxTouchPoints < touchPointNum)
    {
        mMaxTouchPoints = touchPointNum;
    }
    if (mMaxTouchPoints != 1)
    {
        return false;
    }

    QTouchEvent::TouchPoint pt = event->touchPoints().first();
    if (mGestureStarted)
    {
        mPosition = pt.pos();
        if (event->type() == QEvent::TouchEnd || event->type() == QEvent::TouchCancel)
        {
            return false;
        }
        if (mPoints.size() >= PAN_VELOCITY_SAMPLES)
        {
            mPoints.pop_front();
        }
        Touch touch;
        touch.pos = mPosition;
        touch.time = mTime.elapsed();
        mPoints.push_back(touch);
        emit pan(mPosition);
        return true;
    }
    else
    {
        if (distance(pt.pos(),pt.startPos()) >= PAN_TOLERANCE)
        {
            mPosition = pt.pos();
            return true;
        }
        return false;
    }
}

void PanRecognizer::startGesture()
{
    emit panStart(mPosition);
    mGestureStarted = true;
    mTime.restart();
    mPoints.clear();
    Touch touch;
    touch.pos = mPosition;
    touch.time = 0;
    mPoints.push_back(touch);
    mTranslateVelocity = QPointF(0,0);
}

void PanRecognizer::stopGesture()
{
    if (mGestureStarted)
    {
        QPointF totalV(0,0);
        int n = 0;
        for (int i = 1; i < mPoints.size(); ++i)
        {
            const Touch& t0 = mPoints[i - 1];
            const Touch& t1 = mPoints[i];
            int t = t1.time - t0.time;
            if (t > 0)
            {
                QVector2D v(t1.pos - t0.pos);
                v /= t * 0.001;
                if (v.length() > MAX_SWIPE_SPEED)
                {
                    v.normalize();
                    v *= MAX_SWIPE_SPEED;
                }
                totalV += v.toPointF();
                ++n;
            }
        }
        if (n > 0)
        {
            mTranslateVelocity = totalV * (1.0/n);
        }
        emit panEnd(mPosition, mTranslateVelocity);
    }
    mGestureStarted = false;
}

//****************************************************
TwoFingerTapRecognizer::TwoFingerTapRecognizer(QObject* parent)
    :GestureRecognizer(parent)
{
}

bool TwoFingerTapRecognizer::onTouch(QTouchEvent* event, QList<GestureTouch>& history)
{
    if (event->type() == QEvent::TouchBegin)
    {
        mStopped = false;
        mMaxTouchPoints = 1;
        mPressTime.restart();
    }
    else if (event->type() == QEvent::TouchEnd)
    {
        QTouchEvent::TouchPoint tp = event->touchPoints().first();
        if (!mStopped && mMaxTouchPoints == 2 &&
            mPressTime.elapsed() < TAP_INTERVAL &&
            distance(tp.startPos(), tp.pos()) <= TAP_TOLERANCE)
        {
            mPositions[1] = tp.pos();
            emit twoFingerTap(((mPositions[0] + mPositions[1])) * 0.5f);
        }
    }
    else if (event->type() == QEvent::TouchUpdate)
    {
        if (!mStopped)
        {
            if (event->touchPoints().size() > 2)
            {
                mStopped = true;
            }
            else if (event->touchPoints().size() == 2)
            {
                QTouchEvent::TouchPoint tp0 = event->touchPoints()[0];
                QTouchEvent::TouchPoint tp1 = event->touchPoints()[1];
                if (distance(tp0.startPos(), tp0.pos()) > TAP_TOLERANCE ||
                    distance(tp1.startPos(), tp1.pos()) > TAP_TOLERANCE)
                {
                    mStopped = true;
                    return false;
                }

                if (tp0.state() == Qt::TouchPointReleased)
                {
                    mPositions[0] = tp0.pos();
                }
                else if (tp1.state() == Qt::TouchPointReleased)
                {
                    mPositions[0] = tp1.pos();
                }
            }

            if (event->touchPoints().size() > mMaxTouchPoints)
            {
                mMaxTouchPoints = event->touchPoints().size();
            }
        }
    }
    else
    {
        mStopped = true;
    }
    return false;
}
//****************************************************
TapRecognizer::TapRecognizer(QObject* parent)
    :GestureRecognizer(parent)
    ,mStopped(true)
{
    mLastTapTime.start();
}

bool TapRecognizer::onTouch(QTouchEvent* event, QList<GestureTouch>& history)
{
    QTouchEvent::TouchPoint tp = event->touchPoints().first();
    if (event->type() == QEvent::TouchBegin)
    {
        if (mLastTapTime.elapsed() < DOUBLE_TAP_INTERVAL &&
            distance(mLastTapPosition, tp.pos()) < TAP_TOLERANCE)
        {
            emit doubleTap(tp.pos());
            mStopped = true;
            return false;
        }
        mStopped = false;
        mPressTime.restart();
    }
    else if (event->type() == QEvent::TouchEnd)
    {
        if (!mStopped && mPressTime.elapsed() < TAP_INTERVAL)
        {
            mLastTapTime.restart();
            mLastTapPosition = tp.pos();
            emit tap(tp.pos());
        }
    }
    else
    {
        if (event->touchPoints().size() != 1
           || distance(tp.pos(), tp.startPos()) > TAP_TOLERANCE)
        {
            mStopped = true;
        }
    }
    return false;
}

void TapRecognizer::stopGesture()
{
    mStopped = true;
}

//****************************************************
LongPressRecognizer::LongPressRecognizer(QObject* parent)
    :GestureRecognizer(parent)
    ,mTimeout(LONG_PRESS_INTERVAL)
{
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

bool LongPressRecognizer::onTouch(QTouchEvent* event, QList<GestureTouch>& history)
{
    QTouchEvent::TouchPoint tp = event->touchPoints().first();
    if (event->type() == QEvent::TouchBegin)
    {
        mPosition = tp.pos();
        mTimer->start(mTimeout);
        return true;
    }
    else if (event->type() == QEvent::TouchEnd)
    {
        mTimer->stop();
        return false;
    }
    else
    {
        if (event->touchPoints().size() != 1
           || distance(tp.pos(), tp.startPos()) > LONG_PRESS_TOLERANCE)
        {
            mTimer->stop();
            return false;
        }
        else
        {
            return true;
        }
    }
}

void LongPressRecognizer::onTimeout()
{
    mTimer->stop();
    emit longPress(mPosition);
}

void LongPressRecognizer::stopGesture()
{
    mTimer->stop();
}

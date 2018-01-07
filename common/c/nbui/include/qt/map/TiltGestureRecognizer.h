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

    @file TiltGestureRecognizer.h
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
#ifndef _TILT_GESTURE_RECOGNIZER_H_
#define _TILT_GESTURE_RECOGNIZER_H_

#include <QGestureRecognizer>
#include <QGesture>
#include <QTouchEvent>
#include <QVector2D>

class TiltGesture : public QGesture
{
public:
    enum MotionState
    {
        None,
        Scale,
        Rotate,
        Tilt
    };

    TiltGesture(QObject *parent = 0);
    ~TiltGesture();

    float angle() const { return m_tiltAngle; }
    float totalScaleFactor() const { return m_scale; }
    float totalRotationAngle() const { return m_rotateAngle; }
    MotionState motionState() const { return m_motionState; }
    const QPointF& centerPoint() const { return m_centerPoint; }
    float rotateSpeed() const { return m_rotateSpeed; }
    float scaleSpeed() const { return m_scaleSpeed; }

    QPointF m_centerPoint;
    float m_tiltAngle;
    float m_rotateAngle;
    float m_scale;
    MotionState m_motionState;
    float m_rotateSpeed;
    float m_scaleSpeed;
};

class TiltGestureRecognizer : public QGestureRecognizer
{
public:
    TiltGestureRecognizer();
    ~TiltGestureRecognizer();

private:
    virtual QGesture *create(QObject *target);
    virtual Result recognize(QGesture *state, QObject *watched,
                             QEvent *event);
    virtual void reset(QGesture *state);
    bool isTilt(const QTouchEvent *ev);
    bool isZoom(const QTouchEvent *ev);
    bool isRotate(const QTouchEvent *ev);

private:
    TiltGesture::MotionState m_motionState;
    bool m_gestureRecognized;
    int m_lastTime;
    QVector2D m_lastDir;
    float m_lastScale;
    QList<float> m_scaleSpeedHistory;
    QList<float> m_rotateSpeedHistory;
    QPointF mStartPos0;
    QPointF mStartPos1;

    float m_lastDeltaX;
};

#endif // _TILT_GESTURE_RECOGNIZER_H_

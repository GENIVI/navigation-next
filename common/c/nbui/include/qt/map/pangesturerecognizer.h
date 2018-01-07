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

#ifndef PANGESTURERECOGNIZER_H
#define PANGESTURERECOGNIZER_H
#include <QGestureRecognizer>
#include <QGesture>
#include <QTouchEvent>

class PanGesture : public QGesture
{
public:
    PanGesture(QObject *parent = 0);
    ~PanGesture();

    const QPointF& deltaPosition() const { return m_deltaPosition; }
    const QPointF& offset() const { return m_offset; }
    const QPointF& velocity() const { return m_velocity; }

    QPointF m_offset;
    QPointF m_deltaPosition;
    QPointF m_velocity;
};

class PanGestureRecognizer:
        public QGestureRecognizer
{
public:
    PanGestureRecognizer();
    ~PanGestureRecognizer();

    Result recognize(QGesture *state, QObject *watched, QEvent *event);
    QGesture *create(QObject *target);
    void reset(QGesture *state);

private:
    QPointF m_lastPosition;
    int m_lastTime;
    bool m_startMove;
    int m_touchPoints;
    QPointF m_startWatchPosition;
    QList<QVector2D> m_velocityHistory;
};

#endif // PANGESTURERECOGNIZER_H

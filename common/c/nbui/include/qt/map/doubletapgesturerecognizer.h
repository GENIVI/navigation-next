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

#ifndef DOUBLETAPGESTURERECOGNIZER_H
#define DOUBLETAPGESTURERECOGNIZER_H
#include <QGestureRecognizer>
#include <QGesture>
#include <QTouchEvent>

class DoubleTapGesture : public QGesture
{
public:
    DoubleTapGesture(QObject *parent = 0);
    ~DoubleTapGesture();

    const QPointF& position() const { return m_position; }

    QPointF m_position;
};


class DoubleTapGestureRecognizer : public QGestureRecognizer
{
public:
    DoubleTapGestureRecognizer();
    ~DoubleTapGestureRecognizer();

    Result recognize(QGesture *state, QObject *watched, QEvent *event);
    QGesture *create(QObject *target);
    void reset(QGesture *state);

private:
    QPointF m_lastPressPosition;
    int m_lastPressTime;
    int m_touchNum;
    int m_tapNum;
    bool m_firstPressed;
};

#endif // DOUBLETAPGESTURERECOGNIZER_H

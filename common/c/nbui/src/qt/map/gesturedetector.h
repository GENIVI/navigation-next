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

#ifndef GESTUREDETECTOR_H
#define GESTUREDETECTOR_H
#include <QTouchEvent>
#include <QList>
#include <QTime>
#include <QTimer>

struct GestureTouch
{
    QList<QPointF> points;
    float t;
};

class GestureRecognizer: public QObject
{
    Q_OBJECT
public:
    GestureRecognizer(QObject* parent = NULL);
    virtual ~GestureRecognizer();
    virtual bool onTouch(QTouchEvent* event, QList<GestureTouch>& history) = 0;
    virtual void startGesture();
    virtual void stopGesture();
protected:
    bool mGestureStarted;
};

class GestureDetector
{
public:
    GestureDetector();

    void touchEvent(QTouchEvent *event);
    void startGestureRecognizer(GestureRecognizer* r);
    void stopCurrentGesture();
    void addGestureRecognizer(GestureRecognizer* r);

private:
    void saveHistory(QTouchEvent* event);
    void startGesture(GestureRecognizer* r);

signals:

private:
    QList<GestureTouch> mHistory;
    QTime mTime;
    GestureRecognizer* mCurrent;
    QList<GestureRecognizer*> mRecognizers;
};

//gestures
class PinchRecognizer: public GestureRecognizer
{
    Q_OBJECT
public:
    PinchRecognizer(QObject* parent = 0);
    bool onTouch(QTouchEvent* event, QList<GestureTouch>& history);
    void startGesture();
    void stopGesture();

signals:
    void zoomStart(QPointF pos);
    void zoom(QPointF pos, float scale);
    void zoomEnd(QPointF pos, float scale, float speed);
    void rotateStart(QPointF pos);
    void rotate(QPointF pos, float angle);
    void rotateEnd(QPointF pos, float angle, float speed);
    void tiltStart();
    void tilt(float pixels);
    void tiltEnd();

private:
    enum State
    {
        StateNone,
        StateZoom,
        StateRotate,
        StateTilt
    };
    struct Touch
    {
        float rotate;
        float length;
        int time;
    };

    float getDistanceSpeed();
    float getAngularSpeed();
    float getCenterSpeed();
private:
    float mZoomSpeed;
    float mRotateSpeed;
    QPointF mTranslateVelocity;
    State mState;
    QPointF mCenter;
    float mStartLength;
    float mStartAngle;
    float mStartTilt;
    float mLastLength;
    float mLastAngle;
    float mLastTilt;
    QTime mTime;
    QList<Touch> mPoints;
};

class TwoFingerTapRecognizer: public GestureRecognizer
{
    Q_OBJECT
public:
    TwoFingerTapRecognizer(QObject* parent = 0);
    bool onTouch(QTouchEvent* event, QList<GestureTouch>& history);

signals:
    void twoFingerTap(QPointF center);

private:
    bool mStopped;
    QTime mPressTime;
    int mMaxTouchPoints;
    QPointF mPositions[2];
};

class TapRecognizer: public GestureRecognizer
{
    Q_OBJECT
public:
    TapRecognizer(QObject* parent = 0);
    bool onTouch(QTouchEvent* event, QList<GestureTouch>& history);
    void stopGesture();

signals:
    void tap(QPointF center);
    void doubleTap(QPointF center);
private:
    QTime mPressTime;
    QTime mLastTapTime;
    bool mStopped;
    QPointF mLastTapPosition;
};


class PanRecognizer: public GestureRecognizer
{
    Q_OBJECT
public:
    PanRecognizer(QObject* parent = NULL);
    bool onTouch(QTouchEvent* event, QList<GestureTouch>& history);
    void startGesture();
    void stopGesture();

signals:
    void panStart(QPointF position);
    void pan(QPointF position);
    void panEnd(QPointF position, QPointF velocity);

private:
    struct Touch
    {
        QPointF pos;
        int time;
    };

    QPointF mPosition;
    QPointF mTranslateVelocity;
    int mMaxTouchPoints;
    QTime mTime;
    QList<Touch> mPoints;
};

class LongPressRecognizer: public GestureRecognizer
{
    Q_OBJECT
public:
    LongPressRecognizer(QObject* parent = NULL);
    bool onTouch(QTouchEvent* event, QList<GestureTouch>& history);
    void stopGesture();

public slots:
    void onTimeout();
signals:
    void longPress(QPointF center);
private:
    int mTimeout;
    QTimer* mTimer;
    QPointF mPosition;
    QPointF mTotal;
    int mCount;
};


#endif // GESTUREDETECTOR_H

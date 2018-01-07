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

#include "navigationoptionbutton.h"
#include <QDir>
#include <QCoreApplication>
#include <QPainter>
#include <QBitmap>

NavigationOptionButton::NavigationOptionButton(QWidget *parent, const QString imageName) :
    QPushButton(parent)
{
    setWindowFlags( Qt::FramelessWindowHint );
    setAutoFillBackground( true );
    hide();

    QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    QString strRoot = dir.absolutePath();

    QString strPng = strRoot + "/resource/images/button_off.png";
    mMouseOffBackGround = new QPixmap;
    mMouseOffBackGround->load( strPng );

    strPng = strRoot + "/resource/images/button_pressed.png";
    mPressedBackGround = new QPixmap;
    mPressedBackGround->load( strPng );

    strPng = strRoot + "/resource/images/button_focus.png";
    mFocusBackGround = new QPixmap;
    mFocusBackGround->load( strPng );

    strPng = strRoot + "/resource/images/" + imageName;
    mForeGround = new QPixmap;
    mForeGround->load( strPng );

    mBackGround = mMouseOffBackGround;

    setFixedSize( mBackGround->size() );
    setMask( mBackGround->mask() );
}

NavigationOptionButton::~NavigationOptionButton()
{
}

void NavigationOptionButton::enterEvent(QEvent *)
{
    mBackGround = mFocusBackGround;
    Repaint();
}

void NavigationOptionButton::leaveEvent(QEvent *)
{
    mBackGround = mMouseOffBackGround;
    Repaint();
}

void NavigationOptionButton::mousePressEvent(QMouseEvent *)
{
    mBackGround = mPressedBackGround;
    Repaint();
}

void NavigationOptionButton::mouseReleaseEvent(QMouseEvent *)
{
    mBackGround = mFocusBackGround;
    Repaint();
    emit clicked();
}

void NavigationOptionButton::paintEvent(QPaintEvent *paintEvent)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, *mBackGround);
    painter.drawPixmap(0, 0, *mForeGround);
}

void NavigationOptionButton::Repaint()
{
    setFixedSize( mBackGround->size() );
    setMask( mBackGround->mask() );
    update();
}

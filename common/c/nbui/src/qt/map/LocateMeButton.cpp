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

    @file locatemebutton.cpp
    @date 08/26/2014
    @defgroup nbui
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
 */

#include <QDir>
#include <QCoreApplication>
#include <QBitmap>
#include <QtEvents>
#include <QPainter>
#include "LocateMeButton.h"
#include <QDebug>

LocateMeButton::LocateMeButton(QWidget *parent)
    : QPushButton(parent)
    , mEnabled(false)
{
    setWindowFlags( Qt::FramelessWindowHint );
    setAutoFillBackground( true );
    hide();

    QString strPng = GetCurrentPath() + "/resource/images/followme_off.png";
    mPixmapFollowmeOff = new QPixmap;
    mPixmapFollowmeOff->load( strPng );

    strPng = GetCurrentPath() + "/resource/images/followme_on.png";
    mPixmapFollowmeOn = new QPixmap;
    mPixmapFollowmeOn->load( strPng );

    strPng = GetCurrentPath() + "/resource/images/followme_compass.png";
    mPixmapFollowmeCompass = new QPixmap;
    mPixmapFollowmeCompass->load( strPng );

    mCurrentPixmap = mPixmapFollowmeOff;

    strPng = GetCurrentPath() + "/resource/images/button_off.png";
    mBackgroudPixmap = new QPixmap();
    mBackgroudPixmap->load(strPng);
}

LocateMeButton::~LocateMeButton()
{
    mPixmapFollowmeOff->detach();
    mPixmapFollowmeOn->detach();
    mPixmapFollowmeCompass->detach();
    mBackgroudPixmap->detach();
    delete mPixmapFollowmeOff;
    delete mPixmapFollowmeOn;
    delete mPixmapFollowmeCompass;
    delete mBackgroudPixmap;
}

void LocateMeButton::SetEnabled(bool enabled)
{
    mEnabled = enabled;
}

void LocateMeButton::SetShowMode(ShowMode mode)
{
    if(!mEnabled)
    {
        return;
    }

    switch (mode)
    {
    case SM_LOCATE_ME:
    {
        mCurrentPixmap = mPixmapFollowmeOff;
        break;
    }
    case SM_FOLLOW_ME_ANYHEADING:
    {
        mCurrentPixmap = mPixmapFollowmeOn;
        break;
    }
    case SM_FOLLOW_ME:
    {
        mCurrentPixmap = mPixmapFollowmeCompass;
        break;
    }
    default:
    {
        break;
    }
    }

    Repaint();
    show();
}

void LocateMeButton::paintEvent(QPaintEvent *paintEvent)
{
    QPainter painter(this);

    if(mBackgroudPixmap)
    {
        painter.drawPixmap(0, 0, *mBackgroudPixmap);
    }

    if(mCurrentPixmap)
    {
        painter.drawPixmap(0, 0, *mCurrentPixmap);
    }
}

void LocateMeButton::mousePressEvent(QMouseEvent *)
{
    QString strPng = GetCurrentPath() + "/resource/images/button_pressed.png";
    mBackgroudPixmap->load(strPng);
    Repaint();
}

void LocateMeButton::mouseReleaseEvent(QMouseEvent *)
{
    QString strPng = GetCurrentPath() + "/resource/images/button_off.png";
    mBackgroudPixmap->load(strPng);
    Repaint();
    emit clicked();
}

void LocateMeButton::Repaint()
{
    setFixedSize( mBackgroudPixmap->size() );
    setMask( mBackgroudPixmap->mask() );
    update();
}

QString LocateMeButton::GetCurrentPath()
{
    QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    return dir.absolutePath();
}

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
#include "defaultbubble.h"
#include <QIcon>
#include <QBitmap>
#include <QPalette>
//#include <QWidget>
#include "pinparameters.h"

using namespace locationtoolkit;

DefaultBubble::DefaultBubble(QString title,
                             QString content,
                             QString bubbleImagePath,
                             QWidget *parent):
//    mParent( parent ),
//    mWidget( NULL ),
    mTitle( title ),
    mContent( content ),
    mBubbleImagePath(bubbleImagePath)
{
}

DefaultBubble::~DefaultBubble()
{
    Remove();
}


void DefaultBubble::Show(float x, float y, Qt::Orientation /*orientation*/)
{
//    if( mWidget == NULL )
//    {
//        CreateInternalWidget();
//    }

//    mWidget->setGeometry(x - mWidget->width() / 2,
//                         y - mWidget->height(),
//                         mWidget->width(),
//                         mWidget->height());
//    mWidget->show();
}

void DefaultBubble::Update(float x, float y, Qt::Orientation /*orientation*/)
{
//    if( mWidget )
//    {
//        mWidget->setGeometry(x - mWidget->width() / 2,
//                             y - mWidget->height(),
//                             mWidget->width(),
//                             mWidget->height());
//    }
}

void DefaultBubble::Hide()
{
//    if( mWidget )
//    {
//        mWidget->hide();
//    }
}

void DefaultBubble::Remove()
{
//    if( mWidget )
//    {
//        mWidget->hide();
//        mWidget->deleteLater();
//        mWidget = NULL;
//    }
}

void DefaultBubble::CreateInternalWidget()
{
//    mWidget = new QWidget( mParent );
//    mWidget->setAutoFillBackground(true);

//    mTitleLabel = new QLabel(mWidget);
//    mTitleLabel->setText( mTitle );
//    mTitleLabel->setStyleSheet("background-color: rgba(255, 255, 255, 0); font: 14pt;");
//    mTitleLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
//    mTitleLabel->adjustSize();
//    mTitleLabel->setGeometry(30,
//                             15,
//                             mTitleLabel->width() + 30,
//                             mTitleLabel->height());

//    mSubTitleLabel = new QLabel(mWidget);
//    mSubTitleLabel->setText( mContent );
//    mSubTitleLabel->setStyleSheet("background-color: rgba(255, 255, 255, 0); font: 9pt;");
//    mSubTitleLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
//    mSubTitleLabel->adjustSize();
//    mSubTitleLabel->setGeometry(mTitleLabel->x(),
//                                mTitleLabel->height() + 30,
//                                mSubTitleLabel->width() + 30,
//                                mSubTitleLabel->height());

//    QPixmap backgroundSourceImage(mBubbleImagePath, "PNG");
//    int width = mTitleLabel->width() > mSubTitleLabel->width()?mTitleLabel->width():mSubTitleLabel->width();
//    QPixmap backgroundImage(backgroundSourceImage.scaled(width + 60,
//                                                         backgroundSourceImage.height()));

//    mWidget->setGeometry(0, 0, backgroundImage.width(), backgroundImage.height());
//    QPalette palette;
//    palette.setBrush(QPalette::Background, QBrush(backgroundImage));
//    mWidget->setPalette(palette);
//    mWidget->setMask(backgroundImage.mask());
//    mWidget->hide();
}

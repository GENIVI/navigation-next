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

#include "minimapbutton.h"
#include <QPainter>
#include <QBitmap>
#include <QDir>
#include <QCoreApplication>
#include <QPaintEvent>
#include <QColor>

using namespace locationtoolkit;

MiniMapButton::MiniMapButton(const QString& background,
                             const QString& foreground,
                             QWidget *parent)
    : QPushButton(parent)
    , mBackgroudPixmap(NULL)
    , mForegroundPixmap(NULL)
{
    mBackgroudPixmap = new QPixmap(background);
    QPixmap foregroundImage(foreground);
    mForegroundPixmap = new QPixmap(foregroundImage.scaled(foregroundImage.width() / 2,
                                                           foregroundImage.height() / 2));
    setMask(mBackgroudPixmap->mask());
    update();
}

MiniMapButton::~MiniMapButton()
{
    if(mBackgroudPixmap)
    {
        mBackgroudPixmap->detach();
        delete mBackgroudPixmap;
    }

    if(mForegroundPixmap)
    {
        mForegroundPixmap->detach();
        delete mForegroundPixmap;
    }
}

void MiniMapButton::paintEvent(QPaintEvent *paintEvent)
{
    QPainter painter(this);
    QColor color("gray");
    painter.fillRect(this->rect(), color);
    if(mBackgroudPixmap)
    {
        painter.drawPixmap(0, 0, *mBackgroudPixmap);
    }

    if(mForegroundPixmap)
    {
        painter.drawPixmap(0, 0, *mForegroundPixmap);
    }
}

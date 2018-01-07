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

#include "private/manlistdelegate.h"
#include "mainwindow.h"
#include <QFontMetrics>


ManListDelegate::ManListDelegate(QObject *parent)
{
    mSelectIndex = 255;
}

void ManListDelegate::SetSelectIndex(int index) //tell delegate which item was selected
{
    mSelectIndex = index;
}
void ManListDelegate::SetFontName(QString font)
{
    mFontName = font;
}

void ManListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect r = option.rect;

    if (index.row() == mSelectIndex) //for highlight the row which had been clicked
    {
        painter->setBrush(QColor(55, 55, 55));
    }
    else{
         painter->setBrush(QColor(25,25,25));
    }

    painter->setPen(QColor(55,55,55));

    painter->drawRect(r);

    //ICON
    QString command = index.data(Qt::UserRole).toString();
    r.setRect(option.rect.x(),option.rect.y(),100,option.rect.height());
    QFont maneuverFont(mFontName,24);

    QPen fontPen(QColor::fromRgb(251,251,251), 1, Qt::SolidLine);
    painter->setPen(fontPen);
    painter->setFont(maneuverFont);
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignVCenter|Qt::AlignLeft, command, &r);

    //NAME
    QString name = index.data(Qt::DisplayRole).toString();
    painter->setFont(QFont("MS Shell Dlg2",12));

    r.setRect(option.rect.x() + (IMAGELENGTH),option.rect.y(),option.rect.width() - (IMAGELENGTH+DESTLENGTH),option.rect.height());
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignVCenter|Qt::AlignLeft, name, &r);

    //DISTANCE
    QString dist;
    dist = index.data(Qt::DecorationRole).toString();
    QPen distPen(QColor::fromRgb(200,200,200), 1, Qt::SolidLine);
    painter->setPen(distPen);
    r.setRect(option.rect.width() - (DESTLENGTH),option.rect.y(),(DESTLENGTH),option.rect.height());
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignVCenter|Qt::AlignRight, dist, &r);
}

QSize ManListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(300, 80); // very dumb value
}

ManListDelegate::~ManListDelegate()
{
}

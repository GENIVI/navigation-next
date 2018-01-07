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

#include "listdelegate.h"

ListDelegate::ListDelegate(QObject *parent)
: QAbstractItemDelegate(parent)
{
}


void ListDelegate::paint(QPainter *painter,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) const
{
    QRect r = option.rect;
    int width = r.width();
    painter->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );

    QString title = index.data(Qt::DisplayRole).toString();
    QString description = index.data(Qt::UserRole).toString();

    QString fontFamliy = "Lucida Grande";
    painter->setFont( QFont( fontFamliy, 14, QFont::Normal) );
    painter->drawText(r.left()+6, r.top(), r.width(), r.height()-20, Qt::AlignVCenter|Qt::AlignLeft, title, &r);

    painter->setFont( QFont( fontFamliy, 10, QFont::Normal) );
    painter->drawText(r.left(), r.bottom(), width, r.height(), Qt::AlignVCenter|Qt::AlignLeft, description, &r);
}

QSize ListDelegate::sizeHint(const QStyleOptionViewItem &option,
                       const QModelIndex &index) const
{
    return QSize( 280, 60 );
}

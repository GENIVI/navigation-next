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

#ifndef PIXMAPPROVIDER_H
#define PIXMAPPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QPainter>
#include "util.h"

class PixmapProvider : public QQuickImageProvider {
public:
    PixmapProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}    

//    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
//        qulonglong d = id.toULongLong();
//        if (d) {
//            QPixmap * p = reinterpret_cast<QPixmap *>(d);
//            return *p;
//        } else {
//            return QPixmap();
//        }
//    }

    /*QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
    {
        int width = 100;
        int height = 50;

        if (size)
            *size = QSize(width, height);
        QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
                       requestedSize.height() > 0 ? requestedSize.height() : height);
        pixmap.fill(QColor(id).rgba());

        // write the color name
        QPainter painter(&pixmap);
        QFont f = painter.font();
        f.setPixelSize(20);
        painter.setFont(f);
        painter.setPen(Qt::black);
        if (requestedSize.isValid())
            painter.scale(requestedSize.width() / width, requestedSize.height() / height);
        painter.drawText(QRectF(0, 0, width, height), Qt::AlignCenter, id);

        return pixmap;
    }*/

    QPixmap requestPixmap(const QString &imageId, QSize *size, const QSize &requestedSize)
    {
        QString directionFontPath = LTKSampleUtil::GetResourceFolder() + "font/LG_NexGen.ttf";
        int id = QFontDatabase::addApplicationFont(directionFontPath);
        QString directionFontFamily = QFontDatabase::applicationFontFamilies(id).at(0);
        QFont LG_NextGen(directionFontFamily);

        QString backgroundImg = LTKSampleUtil::GetResourceFolder() + "/images/RouteNavigation_images/maneuverBackground.png";
        QPixmap *pix = new QPixmap(backgroundImg);
        QPainter *paint = new QPainter(pix);

        QLinearGradient gradient;
        gradient.setColorAt(0, "#DDF8FF");
        gradient.setColorAt(1, "#0ED9FD");

        QBrush brush(gradient);
        QPen pen;
        pen.setBrush(brush);
        paint->setPen(pen);

        LG_NextGen.setPointSize(60);
        LG_NextGen.setBold(true);
        paint->setFont( LG_NextGen );
        paint->drawText(5,17,200,100, Qt::AlignCenter,imageId);

        return *pix;
    }

//    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
//    {
//        int width = 100;
//        int height = 50;

//        if (size)
//            *size = QSize(width, height);
//        QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
//                       requestedSize.height() > 0 ? requestedSize.height() : height);
//        pixmap.fill(QColor(id).rgba());

//        return pixmap;
//    }
};

#endif // PIXMAPPROVIDER_H

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

#ifndef MAINITEM_H
#define MAINITEM_H

#include "mapwidget.h"
#include <QQuickItem>

namespace locationtoolkit {
class MapWidget;
}


class MainItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString workFolder READ workFolder WRITE setWorkFolder)
    Q_PROPERTY(QPointF avatarPosition READ avatarPosition WRITE setAvatarPosition)
    Q_PROPERTY(float tilt READ tilt WRITE setTilt)
    Q_PROPERTY(float zoom READ zoom WRITE setZoom)

public:
    MainItem();

signals:

public slots:
    void init();    
    void setWorkFolder(QString path);
    QString workFolder();
    void onMapReady();
    QPointF avatarPosition();
    void setAvatarPosition(QPointF p);

    float tilt();
    void setTilt(float t);

    float zoom();
    void setZoom(float t);
    // QQuickItem interface

    void OnMapLongClick(const locationtoolkit::Coordinates&);
protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
    void addRoute();
    void addPin(const locationtoolkit::Coordinates &coordinate);

private:
    locationtoolkit::MapWidget* mMapWidget;
    bool mMapReady;
    QPointF mAvatarPosition;
    float mTilt;
    float mZoom;
};

#endif // MAINITEM_H

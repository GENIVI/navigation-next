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

#include "mapview.h"
#include "mapwidget.h"
#include "util.h"
#include "globalsetting.h"
#include<iostream>
using namespace std;

MapView::MapView(QQuickItem *parent):
    QQuickItem(parent),
    mInitDone(false),
    mMapReady(false),
    mTilt(15.0f),
    mZoom(19.0f)
{
    setFlag(ItemHasContents, true);
}

MapView::~MapView()
{
}

QSGNode* MapView::updatePaintNode(QSGNode *n, UpdatePaintNodeData *d)
{
    if (!mInitDone)
    {
        QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
    }
    return QQuickItem::updatePaintNode(n, d);
}
void MapView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size() && !oldGeometry.size().isNull())
    {
        cout<<"inside resizes";
        if(globalMapWidget)
        {
           globalMapWidget->Resize(newGeometry.size().toSize());
           globalMapWidget->setProperty("x", 0);
           globalMapWidget->setProperty("y", 0);
        }
    }
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}


void MapView::init()
{
    if(mInitDone)
        return;
    cout<<"MapView::init\n";
    GlobalSetting::InitInstance();
    GlobalSetting& settings = GlobalSetting::GetInstance();

    // setToken
    if(mToken.length() > 0)
        settings.setToken(mToken);

    QSettings setting(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
    setting.beginGroup("hybrid");
    QString dataPath = setting.value(gOnBoardDataPath).toString();
    bool IsOnBoard = setting.value("onBoard","false").toBool();

    qDebug()<<"MainViewController::init::"<<dataPath<<IsOnBoard;
    if(IsOnBoard)
        InitLTKHybridManager(IsOnBoard, dataPath);
    else
        InitLTKHybridManager(IsOnBoard);

    globalMapWidget = GetMapWidget();
    if(globalMapWidget == NULL)
    {
        qDebug( "c1reate mapkit3d widget failed!!!\n" );
        return;
    }

    globalMapWidget->setParentItem(this);
    globalMapWidget->setSize(QSize(width(),height()));

    connect(globalMapWidget, &locationtoolkit::MapWidget::MapCreated, this, &MapView::onMapReady);

    mInitDone = true;
    //mtimer.start(1000);
    connect( &mtimer, SIGNAL(timeout()), this, SLOT(onTimer()) );
    //abort();
    connect(this,SIGNAL(screenSizeChanged(double,double)),this,SLOT(onScreenSizeChanged(double,double)));

}
void MapView::onScreenSizeChanged(double width,double height)
{
    if(globalMapWidget)
    {
        cout<<"inside on screen size changed in mapview"<<width;
       globalMapWidget->Resize(QSize(width,height));
       globalMapWidget->setProperty("x", 0);
       globalMapWidget->setProperty("y", 0);
    }
}

void MapView::onTimer()
{
    static int i = 0;
    cout<<"timer started..."<<i;

    if(i == 10)
    {
        abort();

    }
     i++;
}

void MapView::setWorkFolder(QString path)
{
    qDebug("MapView::SetWorkFolder %s", path.toStdString().c_str());
    LTKSampleUtil::SetWorkFolder(path);
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    engine->rootContext()->setContextProperty("applicationDirPath1", path);
}

QString MapView::workFolder()
{
    return LTKSampleUtil::GetWorkFolder();
}

void MapView::onMapReady()
{
    mMapReady = true;

    globalMapWidget->SetReferenceCenter(QPoint(width()/2,height() * 2 / 3));
    // updateAvatarLocation();

    locationtoolkit::Coordinates pos;
    pos.setLatitude(33.604f);
    pos.setLongitude(-117.689f);
    locationtoolkit::CameraParameters cp(pos);
    globalMapWidget->GetCameraPosition(cp);
    cp.SetPosition(pos);
    cp.SetTiltAngle(mTilt);
    cp.SetZoomLevel(mZoom);
    globalMapWidget->MoveTo(cp);

    globalMapWidget->SetNightMode(locationtoolkit::MapWidget::NM_NIGHT);

}

float MapView::tilt()
{
    return mTilt;
}

void MapView::setTilt(float t)
{
    mTilt = t;
}

float MapView::zoom()
{
    return mZoom;
}

void MapView::setZoom(float t)
{
    mZoom = t;
}

QString MapView::token()
{
    return mToken;
}

void MapView::setToken(QString t)
{
    mToken = t;
}

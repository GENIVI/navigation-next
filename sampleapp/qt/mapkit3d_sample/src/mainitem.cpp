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

#include "mainitem.h"
#include "util.h"
//#include "devtooldialog.h"
#include "globalsetting.h"
#include "cameraparameters.h"
#include "coordinate.h"

MainItem::MainItem()
    : mMapReady(false),
      mAvatarPosition(-118.25,34.05),
      mTilt(15.0f),
      mZoom(19.0f)
{
    setFlag(ItemHasContents, true);
}

QSGNode* MainItem::updatePaintNode(QSGNode *n, UpdatePaintNodeData *d)
{
   qDebug("MainItem::updatePaintNode w=%f,h=%f", width(), height());
   QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
   return QQuickItem::updatePaintNode(n, d);
}
void MainItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size() && !oldGeometry.size().isNull())
    {
        mMapWidget->Resize(newGeometry.size().toSize());
    }
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}
void MainItem::init()
{
    GlobalSetting::InitInstance();
    GlobalSetting& settings = GlobalSetting::GetInstance();
    GetLTKContext()->StartQaLog();

    //InitLTKHybridManager(true, "/home/wliu/mapdataCA/mapdata");
    InitLTKHybridManager(false);

    mMapWidget = GetMapWidget();
    if( mMapWidget == NULL )
    {
        qDebug( "c1reate mapkit3d widget failed!!!\n" );
        return;
    }

    mMapWidget->setParentItem(this);
    mMapWidget->setSize(QSize(width(),height()));

    connect(mMapWidget, &locationtoolkit::MapWidget::MapCreated, this, &MainItem::onMapReady);
}

void MainItem::setWorkFolder(QString path)
{
    LTKSampleUtil::SetWorkFolder(path);
}

QString MainItem::workFolder()
{
    return LTKSampleUtil::GetWorkFolder();
}

void MainItem::onMapReady()
{
//    mMapWidget->SetGpsMode(locationtoolkit::MapWidget::GM_FOLLOW_ME_ANY_HEADING);
    mMapWidget->SetReferenceCenter(QPoint(width()/2,height()/2));

    locationtoolkit::Coordinates pos(mAvatarPosition.y(), mAvatarPosition.x());
    locationtoolkit::CameraParameters cp(pos);
    cp.SetTiltAngle(30.0f);
    cp.SetZoomLevel(16.0f);
    mMapWidget->MoveTo(cp);

    connect(mMapWidget, SIGNAL(MapLongClicked(const locationtoolkit::Coordinates&)),
                this, SLOT(OnMapLongClick(const locationtoolkit::Coordinates&)) );

    addRoute();

    mMapReady = true;
}


QPointF MainItem::avatarPosition()
{
    return mAvatarPosition;
}

void MainItem::setAvatarPosition(QPointF p)
{
    qDebug() << "MapView::setAvatarPosition" << p;
    if (p != mAvatarPosition)
    {
        mAvatarPosition = p;
        if (mMapReady)
        {
            locationtoolkit::Location loc;
            loc.latitude = mAvatarPosition.y();
            loc.longitude = mAvatarPosition.x();
            loc.valid = 3;
            mMapWidget->GetAvatar().SetLocation(loc);
        }
    }
}

float MainItem::tilt()
{
    return mTilt;
}

void MainItem::setTilt(float t)
{
    mTilt = t;
}

float MainItem::zoom()
{
    return mZoom;
}

void MainItem::setZoom(float t)
{
    mZoom = t;
}


void MainItem::OnMapLongClick(const locationtoolkit::Coordinates &coordinate)
{
    mMapWidget->DeleteAllPins();

    addPin(coordinate);
}

void MainItem::addPin(const locationtoolkit::Coordinates &coordinate)
{
    locationtoolkit::CameraParameters param( coordinate );
    QPixmap selectImage, unselectImage;

    QString resourceFolder = LTKSampleUtil::GetResourceFolder();
    selectImage.load(resourceFolder + "images/pin_blue.png");
    unselectImage.load(resourceFolder + "images/pin_normal.png");

    locationtoolkit::PinImageInfo selectedImage;
    selectedImage.SetPixmap(selectImage);
    selectedImage.SetPinAnchor(50, 100);
    locationtoolkit::PinImageInfo unSelectedImage;
    unSelectedImage.SetPixmap(unselectImage);
    unSelectedImage.SetPinAnchor(50, 100);

    locationtoolkit::RadiusParameters radiusPara(0, 0x6721D826);

    QString title("Title");
    QString subtitle("Sub Title");
    locationtoolkit::Bubble* bubble = NULL;
    bool visible = true;

    locationtoolkit::PinParameters pinpara(coordinate,
                                           selectedImage,
                                           unSelectedImage,
                                           radiusPara,
                                           title,
                                           subtitle,
                                           bubble,
                                           visible);
    mMapWidget->CreatePin(pinpara);
}

void MainItem::addRoute()
{
    QList<locationtoolkit::Coordinates> mPolyPoints;
    double pts[] = {-118.25,34.05,
                    -118.24,34.05,
                    -118.23,34.15};
    int n = sizeof(pts)/sizeof(double)/2;
    for (int i = 0; i < n; ++i)
    {
        locationtoolkit::Coordinates c(pts[i * 2 + 1], pts[i * 2]);
        mPolyPoints.push_back(c);
    }

    int polylineWidth = 10;
    locationtoolkit::CapParameter mStartCap;
    mStartCap.type = locationtoolkit::CPT_Nothing;
    mStartCap.radius = polylineWidth /2;
    mStartCap.width = 0;
    mStartCap.length = 0;
    locationtoolkit::CapParameter mEndCap;
    mEndCap.type = locationtoolkit::CPT_Nothing;
    mEndCap.radius = polylineWidth /2;
    mEndCap.width = 0;
    mEndCap.length = 0;
    locationtoolkit::PolylineParameters para;

    para.SetPoints(mPolyPoints);

    QList<locationtoolkit::SegmentAttribute*>* mSegAttr = new QList<locationtoolkit::SegmentAttribute*>();;
    locationtoolkit::ColorSegment* segattr = new locationtoolkit::ColorSegment(mPolyPoints.count()-1, QColor(Qt::black));
    mSegAttr->append(segattr);
    para.SetSegmentAttributes(*mSegAttr);
    para.SetUnhighlightColor(QColor(30,50,155));
    para.SetWidth(polylineWidth);
    para.SetZOrder(16);
    para.SetVisible(true);
    para.SetStartCap(mStartCap);
    para.SetEndCap(mEndCap);
    para.setOutlineColor(QColor(Qt::black));
    para.setOutlineWidth(0);
    para.SetAttributeType(locationtoolkit::PolylineParameters::SAT_ColorSegment);
    mMapWidget->AddPolyline(para);

    mMapWidget->DeleteAllPins();
    addPin(mPolyPoints.first());
    addPin(mPolyPoints.last());
}

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

#include "navigationuimapimpl.h"
#include <QDir>
#include <QCoreApplication>
#include "animationparameters.h"
#include "camerafactory.h"
#include "ltknkuitypeimpl.h"

using namespace locationtoolkit;

#define POLYLINE_WIDTH 10
#define AVATAR_REFERENCE_BOTTOM_GAP 200

NavigationUIMapImpl::NavigationUIMapImpl(NavigationUIMapController* pNavigationUIMapController):
    mNavigationUIMapController(pNavigationUIMapController)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();

    connect( mapwidget, SIGNAL(CameraUpdated(locationtoolkit::CameraParameters)), this, SLOT(OnCameraUpdated(locationtoolkit::CameraParameters)) );
    connect( mapwidget, SIGNAL(CameraAnimationDone(int,locationtoolkit::MapWidget::AnimationStatus)), this, SLOT(OnCameraAnimationDone(int,locationtoolkit::MapWidget::AnimationStatus)) );
    connect( mapwidget, SIGNAL(Unlocked()), this, SLOT(OnUnlocked()) );
    connect( mapwidget, SIGNAL(PinClicked(locationtoolkit::Pin)), this, SLOT(OnPinClicked(locationtoolkit::Pin)) );
    connect( mapwidget, SIGNAL(PolylineClick(QList<locationtoolkit::Polyline*>)), this, SLOT(OnPolylineClick(QList<locationtoolkit::Polyline*>)) );
}

void NavigationUIMapImpl::SetCamera(const LTKNKUICameraPosition& camera, const LTKNKUIAnimationParameters& animation)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        CameraParameters cameraParam( camera.coordinate, camera.zoomLevel, camera.tiltAngle, camera.headingAngle );
        if( animation.duration > 0 )
        {
            AnimationParameters::AccelerationType aniType =
                    animation.accelerationType == AAT_Linear ? AnimationParameters::AC_Linear : AnimationParameters::AC_Deceleration;
            AnimationParameters aniParam( aniType, animation.duration );
            aniParam.SetAnimationId( animation.animationId );

            mapwidget->AnimateTo( cameraParam, aniParam );
        }
        else
        {
            mapwidget->MoveTo( cameraParam );
        }
    }
}

void NavigationUIMapImpl::MoveCamera(const LTKNKUICoordinateBounds& boundingbox, const LTKNKUIAnimationParameters& animation)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        LatLngBound latlngBound;
        latlngBound.topLeftLatitude      = boundingbox.northEast.latitude;
        latlngBound.topLeftLongitude     = boundingbox.southWest.longitude;
        latlngBound.bottomRightLatitude  = boundingbox.southWest.latitude;
        latlngBound.bottomRightLongitude = boundingbox.northEast.longitude;
        QSharedPointer<CameraParameters>  cameraParameters =  CameraFactory::CreateCamera( mapwidget->width(), mapwidget->height(), latlngBound );

        if( animation.duration > 0 )
        {
            AnimationParameters::AccelerationType aniType =
                    animation.accelerationType == AAT_Linear ? AnimationParameters::AC_Linear : AnimationParameters::AC_Deceleration;
            AnimationParameters aniParam( aniType, animation.duration );
            aniParam.SetAnimationId( animation.animationId );
            mapwidget->AnimateTo( *cameraParameters, aniParam );
        }
        else
        {
            mapwidget->MoveTo( *cameraParameters );
        }
    }
}

void NavigationUIMapImpl::SetAvatarMode(AvatarMode avatarMode)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        if( avatarMode != AM_Remain )
        {
            Avatar::AvatarMode mod = avatarMode == AM_MapMode ? Avatar::AM_MAP : Avatar::AM_ARROW;
            mapwidget->GetAvatar().SetMode( mod );
        }
    }

}

void NavigationUIMapImpl::SetAvatarLocationAndCameraPosition(const Location& location, const LTKNKUICameraPosition& camera, const LTKNKUIAnimationParameters& animation)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        mapwidget->BeginAtomicUpdate();

        QPoint pos( mapwidget->width()/2, mapwidget->height() - AVATAR_REFERENCE_BOTTOM_GAP );
        mapwidget->SetReferenceCenter( pos );
        QSharedPointer<CameraParameters> scrolledCamera =
                CameraFactory::CreateCameraParametersForCoordinatesAtPosition( pos,
                                                                               mapwidget->width(),
                                                                               mapwidget->height(),
                                                                               camera.coordinate,
                                                                               camera.tiltAngle,
                                                                               camera.zoomLevel,
                                                                               camera.headingAngle );

        if( animation.duration > 0 )
        {
            AnimationParameters::AccelerationType aniType =
                    animation.accelerationType == AAT_Linear ? AnimationParameters::AC_Linear : AnimationParameters::AC_Deceleration;
            AnimationParameters aniParam( aniType, animation.duration );
            aniParam.SetAnimationId( animation.animationId );
            mapwidget->AnimateTo( *scrolledCamera, aniParam );
        }
        else
        {
            mapwidget->MoveTo( *scrolledCamera );
        }

        mapwidget->GetAvatar().SetLocation( location );

        mapwidget->EndAtomicUpdate();
    }
}

void NavigationUIMapImpl::SetAvatarLocation(const Location& location)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        mapwidget->GetAvatar().SetLocation( location );
    }

}

LTKNKUIPolyline* NavigationUIMapImpl::AddRoute(const LTKNKUIPolylineParameter& polyLineParameter)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        QList<SegmentAttribute> segAttrs;
        QList<Coordinates> points;
        for( int i = 0; i < polyLineParameter.points.size(); i++ )
        {
            points.append( polyLineParameter.points[i] );

            ColorSegment* segattr = new ColorSegment();
            *segattr = polyLineParameter.colorSegments[0];
            segAttrs.append(*segattr);
        }

        PolylineParameters para;
        para.SetPoints(points).SetSegmentAttributes(segAttrs).SetZOrder(16).SetUnhighlightColor(QColor(Qt::green)).SetWidth(POLYLINE_WIDTH).SetVisible(true).setOutlineColor(QColor(Qt::gray)).setOutlineWidth(5);

        Polyline& polylineObj =  mapwidget->AddPolyline( para );

        LTKNKUIPolyline* retVal = new LTKNKUIPolylineImpl();
        retVal->id       = polylineObj.GetID();
        retVal->selected = polylineObj.IsSelected();
        retVal->visible  = polylineObj.IsVisible();
        retVal->zOrder   = polylineObj.GetZOrder();

        mPolyLinesMap[&polylineObj] = retVal;
        return retVal;
    }
    return NULL;
}

void NavigationUIMapImpl::RemoveRoutes(QVector<LTKNKUIPolyline*>& polyline)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        QMap<Polyline*, LTKNKUIPolyline*>::iterator iter;
        for( iter = mPolyLinesMap.begin(); iter != mPolyLinesMap.end(); )
        {
             if( polyline.contains( iter.value() ) )
            {
                iter.key()->Remove();
                LTKNKUIPolylineImpl* pImpl = dynamic_cast<LTKNKUIPolylineImpl*>( iter.value() );
                delete pImpl;
                iter = mPolyLinesMap.erase( iter );
            }
            else
            {
                ++iter;
            }
        }
    }
}

LTKNKUIPolyline* NavigationUIMapImpl::AddManeuverArrow(const LTKNKUIManeuverArrowParameter& maneuverArrowParameter)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        CapParameter mStartCap;
        mStartCap.type = locationtoolkit::CPT_Nothing;
        mStartCap.radius = maneuverArrowParameter.maneuverwidth /2;
        mStartCap.width = 0;
        mStartCap.length = 0;
        CapParameter mEndCap;
        mEndCap.type = locationtoolkit::CPT_Arrow;
        mEndCap.radius = maneuverArrowParameter.maneuverwidth /2;
        mEndCap.width = maneuverArrowParameter.arrowWidth;
        mEndCap.length = maneuverArrowParameter.arrowWidth;

        QList<Coordinates> polyPoints;
        QList<SegmentAttribute> segAttrs;
        for( int i = 0; i < maneuverArrowParameter.points.size(); i++ )
        {
            polyPoints.append( maneuverArrowParameter.points[i] );
            ColorSegment* segattr = new ColorSegment(maneuverArrowParameter.points.size()-1, QColor(Qt::blue));
            segAttrs.append( *segattr );
        }

        PolylineParameters para;

        para.SetPoints(polyPoints).SetSegmentAttributes(segAttrs).SetUnhighlightColor(QColor(Qt::darkBlue)).SetWidth(maneuverArrowParameter.maneuverwidth).SetZOrder(16).SetVisible(true).SetStartCap(mStartCap).SetEndCap(mEndCap);
        Polyline& polylineObj = mapwidget->AddPolyline(para);
        mArrowPolyLines.push_back( &polylineObj );

        LTKNKUIPolyline* retVal = new LTKNKUIPolylineImpl();
        retVal->id       = polylineObj.GetID();
        retVal->selected = polylineObj.IsSelected();
        retVal->visible  = polylineObj.IsVisible();
        retVal->zOrder   = polylineObj.GetZOrder();
        return retVal;
    }

    return NULL;
}

void NavigationUIMapImpl::RemoveManeuverArrow(LTKNKUIPolyline* maneuverArrow)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        QList<Polyline *>::iterator iter;
        for( iter = mArrowPolyLines.begin(); iter != mArrowPolyLines.end(); )
        {
            Polyline* pointer = *iter;
            if( maneuverArrow->id == pointer->GetID() )
            {
                pointer->Remove();
                LTKNKUIPolylineImpl* pImpl = dynamic_cast<LTKNKUIPolylineImpl*>( maneuverArrow );
                delete pImpl;
                iter = mArrowPolyLines.erase( iter );
            }
            else
            {
                ++iter;
            }
        }
    }
}

LTKNKUIPin* NavigationUIMapImpl::AddPin(const LTKNKUIPinParameter& pinParameters)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        QPixmap selectImage, unselectImage;
        QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
        QString strRoot = dir.absolutePath();
        QString imgPath = strRoot + "/resource/images/pin_blue.png";
        selectImage.load(imgPath);
        imgPath = strRoot + "/resource/images/pin_normal.png";
        unselectImage.load(imgPath);

        locationtoolkit::PinImageInfo selectedImage;
        selectedImage.SetPixmap(selectImage);
        selectedImage.SetPinAnchor(50, 100);
        locationtoolkit::PinImageInfo unSelectedImage;
        unSelectedImage.SetPixmap(unselectImage);
        unSelectedImage.SetPinAnchor(50, 100);

        locationtoolkit::RadiusParameters radiusPara(50, 0x6721D826);

        PinParameters pin(const_cast<Coordinates&>( pinParameters.coordinate ),
                          selectedImage,
                          unSelectedImage,
                          radiusPara,
                          "",
                          "",
                          NULL,
                          true );
        Pin& pinObj = mapwidget->AddPin( pin );

        LTKNKUIPin* retVal = new LTKNKUIPin;
        retVal->coordinate = pinObj.GetPosition();
        retVal->visible = pinObj.IsVisible();
        mPinMap[&pinObj] = retVal;
        return retVal;
    }

    return NULL;
}

void NavigationUIMapImpl::RemovePins()
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        mapwidget->RemoveAllPins();
    }
    mPinMap.clear();
}

LTKNKUIBreadCrumb* NavigationUIMapImpl::AddBreadCrumb(const LTKNKUIBreadCrumbParameter& breadCrumbParameters)
{
    return NULL;
}

void NavigationUIMapImpl::RemoveAllBreadCrumb()
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {

    }
}

void NavigationUIMapImpl::LockCameraPosition(QBool locked)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
    }
}

qreal NavigationUIMapImpl::MetersPerPixel(qreal expectedLatitude)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        return mapwidget->GetMapProjection().MetersPerPixel();
    }

    return 0.0f;
}

QPoint NavigationUIMapImpl::ToScreenLocation(const Coordinates& coordinate)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        int x, y;
        mapwidget->GetMapProjection().ToScreenPosition( coordinate.latitude, coordinate.longitude, x, y );
        return QPoint( x, y );
    }

    QPoint tempPoint(0,0);
    return tempPoint;
}

Coordinates NavigationUIMapImpl::FromScreenLocation(const QPoint& point)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        double lat, lon;
        mapwidget->GetMapProjection().FromScreenPosition( point.x(), point.y(), lat, lon );
        return Coordinates( lat, lon );
    }

    Coordinates tempCoordinates;
    return tempCoordinates;
}

void NavigationUIMapImpl::SetCompassStateAndPosition(QBool isEnable, qreal screenX, qreal screenY)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        mapwidget->SetCompassPosition( screenX, screenY );
    }
}

void NavigationUIMapImpl::Prefetch(QVector<Coordinates> points, qreal zoomLevel)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget && points.size() > 0 )
    {
        CameraParameters param( points[0] );
        param.SetZoomLevel( zoomLevel );
        mapwidget->Prefetch( param );
    }
}

void NavigationUIMapImpl::EnableReferenCecenter(QBool enable)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        if( enable == true )
        {
            QPoint pos( mapwidget->width()/2, mapwidget->height() - AVATAR_REFERENCE_BOTTOM_GAP );
            mapwidget->SetReferenceCenter( pos );
        }
        mapwidget->EnableReferenceCenter( enable );
    }
}

LTKNKUICameraPosition NavigationUIMapImpl::CameraFromBoundingBox(const LTKNKUICoordinateBounds& boundingbox)
{
    LTKNKUICameraPosition returnVal;

    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        LatLngBound latlngBound;
        latlngBound.topLeftLatitude      = boundingbox.northEast.latitude;
        latlngBound.topLeftLongitude     = boundingbox.southWest.longitude;
        latlngBound.bottomRightLatitude  = boundingbox.southWest.latitude;
        latlngBound.bottomRightLongitude = boundingbox.northEast.longitude;

        QSharedPointer<CameraParameters>  cameraParameters =  CameraFactory::CreateCamera( mapwidget->width(), mapwidget->height(), latlngBound );

        returnVal.coordinate = cameraParameters->GetPosition();
        returnVal.zoomLevel  = cameraParameters->GetZoomLevel();
        returnVal.tiltAngle  = cameraParameters->GetTiltAngle();
        returnVal.headingAngle = cameraParameters->GetHeadingAngle();
    }

    return returnVal;
}

void NavigationUIMapImpl::SetGeometry(const QRect &rect)
{
    MapWidget* mapwidget = mNavigationUIMapController->GetMapWidget();
    if( mapwidget )
    {
        mapwidget->setGeometry( rect );
    }
}

void NavigationUIMapImpl::OnCameraUpdated(const locationtoolkit::CameraParameters &cameraParameters)
{
    LTKNKUICameraPosition position;
    position.coordinate = cameraParameters.GetPosition();
    position.zoomLevel  = cameraParameters.GetZoomLevel();
    position.tiltAngle  = cameraParameters.GetTiltAngle();
    position.headingAngle = cameraParameters.GetHeadingAngle();

    emit CameraUpdated( position );
}

void NavigationUIMapImpl::OnCameraAnimationDone(int cameraAnimationId, locationtoolkit::MapWidget::AnimationStatus animationStatus)
{
    qint32 aniID = cameraAnimationId;
    AnimationState aniState = AS_UNKNOWN;
    switch (animationStatus)
    {
    case locationtoolkit::MapWidget::AS_FINISHED:
        aniState = AS_FINISHED;
        break;
    case locationtoolkit::MapWidget::AS_INTERRUPTED:
        aniState = AS_INTERRUPTED;
        break;
    case locationtoolkit::MapWidget::AS_UNKNOWN:
        aniState = AS_UNKNOWN;
        break;
    default:
        break;
    }

    emit CameraAnimationDone( aniID, aniState );
}

void NavigationUIMapImpl::OnUnlocked()
{
    emit Unlocked();
}

void NavigationUIMapImpl::OnPinClicked(const locationtoolkit::Pin &pin)
{
    locationtoolkit::Pin* pointer = const_cast<locationtoolkit::Pin*>(&pin);
    if( mPinMap.find( pointer ) != mPinMap.end() )
    {
        emit PinSelected( mPinMap[pointer] );
    }
}

void NavigationUIMapImpl::OnPolylineClick(const QList<locationtoolkit::Polyline *> polylines)
{
    QVector<LTKNKUIPolyline*> param;
    for( int i = 0; i < polylines.size(); i++ )
    {
        if( mPolyLinesMap.find( polylines[i] ) != mPolyLinesMap.end() )
        {
            param.push_back( mPolyLinesMap[ polylines[i] ] );
        }
    }
    emit PolylineClicked( param );
}

void NavigationUIMapImpl::OnMapClicked(const locationtoolkit::Coordinates &coordinate)
{
    emit DidTapAtCoordinate( coordinate );
}

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

    @file     followmemapwidget.h
    @date     10/10/2014
    @defgroup MAINWINDOW_H
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef FOLLOWMEMAPWIDGET_H
#define FOLLOWMEMAPWIDGET_H

#include <QWidget>
#include <QTime>
#include <QTimer>
#include <QList>
#include <QVBoxLayout>
#include "mapcontainer.h"
#include "toolbarfollowme.h"
#include "interpolator.h"
#include "polyline.h"

class FollowMeMapWidget : public MapContainer
{
    Q_OBJECT
public:
    explicit FollowMeMapWidget(QWidget *parent = 0);
    void SetDefaultGPS( QString gps );
    void StartFollowMe( QString gpsFile );

    virtual void SetMapWidget( locationtoolkit::MapWidget* mapwidget, const QString& gpsfile );
    virtual void ReleaseMapWidget();

    // interface defined by LocationListener
    virtual void LocationUpdated( const locationtoolkit::Location& location );
    virtual void ProviderStateChanged( LocationProviderState state );
    virtual void OnLocationError(int errorCode);

Q_SIGNALS:

private Q_SLOTS:
    void OnTimer();
    void OnPolylineClick(const QList<locationtoolkit::Polyline*> polyline);

private:
    void StopFollowMe();

private:
    ToolBarFollowMe*             mToolBar;
    QString                      mDefaultGPS;

    locationtoolkit::LocationConfiguration mLocationConfiguration;
    QTime mInterpolatorTime;
    QTimer mInterpolatorTimer;
    locationtoolkit::Location mLastLocation;
    Interpolator mLatInterpolator;
    Interpolator mLonInterpolator;
    Interpolator mHeadingInterpolator;
    static const int mInterpolateTimerGap;
    QBool mPolylineReady;
    QList<locationtoolkit::Coordinates> mPolyPoints;
    QList<locationtoolkit::SegmentAttribute>* mSegAttr;
};

#endif // FOLLOWMEMAPWIDGET_H

/*! @} */

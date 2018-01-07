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

    @file     layeroptionwidget.h
    @date     12/31/2014
    @defgroup navigationuikit sample

    linear and angle interpolator definition
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

#ifndef LAYEROPTIONWIDGET_H
#define LAYEROPTIONWIDGET_H

#include "mapwidget.h"
#include <QDeclarativeView>
#include <QDeclarativeContext>

class LayerOptionWidget : public QDeclarativeView
{
    Q_OBJECT
    Q_PROPERTY(bool footprintsChecked READ footprintsChecked WRITE setFootprintsChecked NOTIFY footprintsCheckedChanged)
    Q_PROPERTY(bool weatherChecked READ weatherChecked WRITE setWeatherChecked NOTIFY weatherChanged)
    Q_PROPERTY(bool landmarksChecked READ landmarksChecked WRITE setLandmarksChecked NOTIFY landmarksCheckedChanged)
    Q_PROPERTY(bool pointsOfInterestChecked READ pointsOfInterestChecked WRITE setPointsOfInterestChecked NOTIFY pointsOfInterestCheckedChanged)
    Q_PROPERTY(bool satelliteChecked READ satelliteChecked WRITE setSatelliteChecked NOTIFY satelliteCheckedChanged)
    Q_PROPERTY(bool trafficChecked READ trafficChecked WRITE setTrafficChecked NOTIFY trafficCheckedChanged)
    Q_PROPERTY(bool footprintsVisible READ footprintsVisible  NOTIFY footprintsVisibleChanged)
    Q_PROPERTY(bool weatherVisible READ weatherVisible  NOTIFY weatherVisibleChanged)
    Q_PROPERTY(bool landmarksVisible READ landmarksVisible  NOTIFY landmarksVisibleChanged)
    Q_PROPERTY(bool pointsOfInterestVisible READ pointsOfInterestVisible  NOTIFY pointsOfInterestVisibleChanged)
    Q_PROPERTY(bool satelliteVisible READ satelliteVisible  NOTIFY satelliteVisibleChanged)
    Q_PROPERTY(bool trafficVisible READ trafficVisible  NOTIFY trafficVisibleChanged)

public:
    LayerOptionWidget(QWidget* parent, locationtoolkit::MapWidget* mapWidget);

    bool footprintsChecked() const;
    bool weatherChecked() const;
    bool landmarksChecked() const;
    bool pointsOfInterestChecked() const;
    bool satelliteChecked() const;
    bool trafficChecked() const;
    bool footprintsVisible() const;
    bool weatherVisible() const;
    bool landmarksVisible() const;
    bool pointsOfInterestVisible() const;
    bool satelliteVisible() const;
    bool trafficVisible() const;

    void setFootprintsChecked( bool b );
    void setWeatherChecked( bool b );
    void setLandmarksChecked( bool b );
    void setPointsOfInterestChecked( bool b );
    void setSatelliteChecked( bool b );
    void setTrafficChecked( bool b );

Q_SIGNALS:
    void footprintsCheckedChanged();
    void weatherChanged();
    void landmarksCheckedChanged();
    void pointsOfInterestCheckedChanged();
    void satelliteCheckedChanged();
    void trafficCheckedChanged();
    void footprintsVisibleChanged();
    void weatherVisibleChanged();
    void landmarksVisibleChanged();
    void pointsOfInterestVisibleChanged();
    void satelliteVisibleChanged();
    void trafficVisibleChanged();

public Q_SLOTS:
    void OnClose();

private:
    enum LayerTypes
    {
        LAYER_FOOT_PRINTS = 0,
        LAYER_WEATHER,
        LAYER_lAND_MARK,
        LAYER_POI,
        LAYER_SATELLITE,
        LAYER_TRAFFIC
    };

    bool mFootprintsChecked;
    bool mWeatherChecked;
    bool mLandmarksChecked;
    bool mPointsOfInterestChecked;
    bool mSatelliteChecked;
    bool mTrafficChecked;
    bool mFootprintsVisible;
    bool mWeatherVisible;
    bool mLandmarksVisible;
    bool mPointsOfInterestVisible;
    bool mSatelliteVisible;
    bool mTrafficVisible;
    locationtoolkit::MapWidget* mMapWidget;
    QMap<LayerTypes, QString> mLayerTypeToName;
};

#endif // LAYEROPTIONWIDGET_H

/*! @} */

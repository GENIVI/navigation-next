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

    @file     navigationmenu.h
    @date     09/25/2014
    @defgroup navkitsample
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
#ifndef __NAVIGATION_MENU_H__
#define __NAVIGATION_MENU_H__

#include <QWidget>
#include "coordinate.h"
#include "mainwindow.h"
#include "locationlistener.h"
#include "place.h"

namespace Ui {
class NavigationMenu;
}

using namespace locationtoolkit;
class NavigationMenu : public QWidget, public LocationListener
{
    Q_OBJECT
public:
    explicit NavigationMenu(QWidget *parent = 0);
    ~NavigationMenu();

    void SetStartButtonDisabled(bool isDisabled);

    /*! interface defined by LocationListener */
    virtual void LocationUpdated( const Location& location );
    virtual void ProviderStateChanged( LocationProviderState state );
    virtual void OnLocationError(int errorCode);

Q_SIGNALS:
    void StartNavigation( const Place& place, const RouteOptions& routeoption, const Preferences& preference );
    void UpdateGpsFile(QString gpsfile);
    void UpdateMeasurmentOption(Preferences::Measurement);
    void EnableTTS(bool);
    void ChangeServer(const QString& servername);
    void ShowMessageBox(QString, MessageType);

private slots:
    void on_StartButton_clicked();
    void on_comboBox_GPSFile_currentIndexChanged(const QString &arg1);
    void on_comboBox_voiceGuid_currentIndexChanged(const QString &arg1);
    void on_comboBox_vehicleType_currentIndexChanged(const QString &arg1);
    void on_comboBox_measurmentOption_currentIndexChanged(const QString &arg1);
    void on_comboBox_routeType_currentIndexChanged(const QString &arg1);
    void on_comboBox_offRouteSens_currentIndexChanged(const QString &arg1);
    void on_checkBox_TTS_clicked(bool checked);
    void on_checkBox_tollroads_clicked(bool checked);
    void on_checkBox_highways_clicked(bool checked);
    void on_checkBox_hovlanes_clicked(bool checked);
    void on_checkBox_ferry_clicked(bool checked);

    void on_comboBox_Server_activated(int index);

private:
    void Initialize();
    void InitGPSFileList();
    void InitServerList();

private:
    Ui::NavigationMenu *ui;
    QString mCurrentGPSFile;
    QString mCurrentVoiceGuide;
    quint32 mCurrentTrafficOption;
    quint32 mCurrentOffRouteSensitivity;
    quint32 mCurrentAvoid;
    TransportationMode mCurrentVehicleType;
    Preferences::Measurement mCurrentMeasurmentOption;
    RouteType mCurrentRouteType;
    bool    mPolylineReady;
    QString mLastGPGFile;

    QList<Coordinates> mPolyPoints;
    quint32     mSelectedServerIndex;
};

#endif // __NAVIGATION_MENU_H__

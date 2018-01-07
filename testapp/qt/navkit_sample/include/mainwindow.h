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

    @file     mainwindow.h
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
#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include <QMainWindow>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QResizeEvent>
#include <QMessageBox>
#include "private/waitdialog.h"
#include "private/interpolator.h"
#include "ltkcontext.h"
#include "place.h"
#include "preferences.h"
#include "routeoptions.h"
#include "maneuverlist.h"

namespace Ui
{
    class MainWindow;
}
class NavigationMenu;
class ManeuversList;
class NavigationSession;
using namespace locationtoolkit;

enum MessageType
{
    MT_Normal,
    MT_CancelNavigation
};
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
Q_SIGNALS:
    void StartNavigation(LTKContext&, const Place&, const RouteOptions&, const Preferences&);
    void StopNavigation();
    void PlayAnounce(int index);
    void EnableTTS(bool);
    void UpdateGpsFile(QString gpsfile);
    void UpdateMeasurmentOption(Preferences::Measurement);

    void RouteReceived();
    void TripRemainingTime(quint32 time);
    void TripRemainingDistance(qreal);
    void UpdateManeuverList(const ManeuverList& maneuverlist);
    void ManeuverRemainingDistance(qreal distance);

public slots:
    void OnShowWidget( int type );
    void OnStartNavigation(const Place&, const RouteOptions&, const Preferences&);
    void OnStopNavigation();
    void OnBackButtonClicked();
    void OnShowMessageBox(QString message, MessageType type);
    void OnChangeServer(const QString& servername);

protected:
    void resizeEvent(QResizeEvent* event);
    virtual void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow* ui;
    NavigationMenu* mNavigationMenu;
    ManeuversList*  mManeuverList;
    WaitDialog*     mWaitDialog;
    QMessageBox*    mMessageBox;

    LTKContext*     mLTKContext;
    NavigationSession* mNavigationImpl;
};

#endif // __MAIN_WINDOW_H__

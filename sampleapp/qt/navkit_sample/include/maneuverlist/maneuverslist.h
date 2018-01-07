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

    @file     ManeuversList.h
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
#ifndef __MANEUVERS_LIST_H__
#define __MANEUVERS_LIST_H__

#include <QWidget>
#include <QMessageBox>
#include <QModelIndex>
#include "mainwindow.h"

namespace Ui {
class ManeuverList;
}

using namespace locationtoolkit;

class ManeuversList : public QWidget
{
    Q_OBJECT

public:
    explicit ManeuversList(QWidget *parent = 0);
    ~ManeuversList();

Q_SIGNALS:
    void ShowMessageBox(QString, MessageType);
    void ShowWidget( int type );
    void PlayAnounce( int index );
    void StopNavigation();

public slots:
    void OnRouteReceived();
    void OnTripRemainingTime(quint32 time);
    void OnTripRemainingDistance(qreal distance);
    void OnManeuverRemainingDistance(qreal distance);
    void OnUpdateManeuverList(const ManeuverList& maneuverlist);
    void OnUpdateMeasurmentOption(Preferences::Measurement measurement);

private slots:
    void on_Backbutton_clicked();
    void on_listWidget_pressed(const QModelIndex &index);

private:
    QString FormatNavTime(quint32 time);
    QString FormatDistance(double dist, bool metric, bool yard);

private:
    Ui::ManeuverList *ui;

    QString mManeuverFontName;
    bool mIsMetric;
    bool mIsYard;
};

#endif // __MANEUVERS_LIST_H__

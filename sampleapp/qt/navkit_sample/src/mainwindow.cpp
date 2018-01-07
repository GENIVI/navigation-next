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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "mainwindow.h"
#include <QDir>
#include "ui_mainwindow.h"
#include "mainmenu/navigationmenu.h"
#include "maneuverlist/maneuverslist.h"
#include "private/navigationsession.h"
#include "util.h"
#include "globalsetting.h"
#include "qaloguploaddialog.h"
#include "ltkhybridmanager.h"
#include "modeselectordialog.h"

using namespace locationtoolkit;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setFixedSize(500,480);
    mLTKContext = GetLTKContext();
    ShowModeSelector();

    mNavigationImpl = new NavigationSession(this);
    mManeuverList = new ManeuversList(this);
    mNavigationMenu = new NavigationMenu(this);
    mWaitDialog = new WaitDialog(size(), this);
    mWaitDialog->hide();

    mMessageBox = new QMessageBox(this);
    mMessageBox->hide();

    ui->stackedWidget->addWidget(mNavigationMenu);
    ui->stackedWidget->addWidget(mManeuverList);
    ui->stackedWidget->setCurrentIndex(0);

    QObject::connect(this,SIGNAL(RouteReceived()), mManeuverList, SLOT(OnRouteReceived()));
    QObject::connect(this,SIGNAL(TripRemainingTime(quint32)), mManeuverList, SLOT(OnTripRemainingTime(quint32)));
    QObject::connect(this,SIGNAL(TripRemainingDistance(qreal)), mManeuverList, SLOT(OnTripRemainingDistance(qreal)));
    QObject::connect(this,SIGNAL(ManeuverRemainingDistance(qreal)), mManeuverList, SLOT(OnManeuverRemainingDistance(qreal)) );
    QObject::connect(this,SIGNAL(UpdateManeuverList(const ManeuverList&)), mManeuverList, SLOT(OnUpdateManeuverList(const ManeuverList&)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mNavigationMenu;
    delete mManeuverList;
    delete mNavigationImpl;
    delete mMessageBox;
}

void MainWindow::OnShowWidget( int type )
{
    switch ( type )
    {
    case 0:    //WIDGET_NAVMENU
        ui->stackedWidget->setCurrentIndex(0);
        mNavigationMenu->SetStartButtonDisabled(false);
        mWaitDialog->hide();
        break;
    case 1:    //WIDGET_MANEUVER_LIST
        ui->stackedWidget->setCurrentIndex(1);
        mWaitDialog->hide();
        break;
    case 2:    //WIDGET_WAITING_DIALOG
        mWaitDialog->SetText(" Start Navigation");
        mWaitDialog->show();
        break;
    case 3:    //WIDGET_RECALCULATE
        mWaitDialog->SetText("  Recalculating");
        mWaitDialog->show();
        break;
    case 4:    //WIDGET_DETOUR
        mWaitDialog->SetText("   Detour");
        mWaitDialog->show();
        break;
    default:
        break;
    }
}

void MainWindow::OnStartNavigation(const Place& place, const RouteOptions& routeoption, const Preferences& preference)
{
    emit StartNavigation(*mLTKContext, place,routeoption,preference);
}

void MainWindow::OnStopNavigation()
{
    emit StopNavigation();
    OnShowWidget(0);
}

void MainWindow::OnBackButtonClicked()
{
    OnShowWidget(0);
}

void MainWindow::OnShowMessageBox(QString message, MessageType type)
{
    mMessageBox->setIcon(QMessageBox::Information);
    mMessageBox->setText(message);
    mMessageBox->removeButton(mMessageBox->button(QMessageBox::Yes));
    mMessageBox->removeButton(mMessageBox->button(QMessageBox::No));

    if(type == MT_Normal)
    {
        mMessageBox->addButton(QMessageBox::Yes);
    }
    else if(type == MT_CancelNavigation)
    {
        mMessageBox->addButton(QMessageBox::Yes);
        mMessageBox->addButton(QMessageBox::No);
    }
    mMessageBox->exec();
    if(mMessageBox->clickedButton() == mMessageBox->button(QMessageBox::Yes))
    {
        if(type == MT_CancelNavigation)
        {
            emit StopNavigation();
        }
        OnShowWidget(0);
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    mWaitDialog->SetPosion((width() - mWaitDialog->width())/2, (height() - mWaitDialog->height())/2);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    QaLogUploadDialog qaDlg( GetLTKContext()->GetQaLogDelegate(), this );
    qaDlg.exec();
}

void MainWindow::ShowModeSelector()
{
    RunMode mode;
    int items = MSD_NavMode;
    ModeSelectorDialog dlg(mode,items);
    dlg.exec();

    InitLTKHybridManager(mode.enableOnboard, mode.dataFile);
//    InitLTKHybridManager(false, mode.dataFile);
}

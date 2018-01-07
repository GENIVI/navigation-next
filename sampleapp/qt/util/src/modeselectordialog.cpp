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


#include "modeselectordialog.h"
#include "ui_modeselectordialog.h"
#include "globalsetting.h"
#include "GPSFile.h"
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>

using namespace locationtoolkit;

ModeSelectorDialog::ModeSelectorDialog(RunMode& mode, int items, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModeSelectorDialog),
    mMode(mode),
    mItems(items)
{
    ui->setupUi(this);

    if ((mItems & MSD_NavMode) == 0)
    {
        delete ui->verticalLayoutNavMode;
        ui->verticalLayoutNavMode = NULL;
        delete ui->labNavMode;
        ui->labNavMode = NULL;
        delete ui->radioBtnOnboard;
        ui->radioBtnOnboard = NULL;
        delete ui->radioBtnOffboard;
        ui->radioBtnOffboard = NULL;
    }
    if ((mItems & MSD_Destination) == 0)
    {
        delete ui->verticalLayoutDest;
        ui->verticalLayoutDest = NULL;
        delete ui->labDestination;
        ui->labDestination = NULL;
        delete ui->labLat;
        ui->labLat = NULL;
        delete ui->latLineEdit;
        ui->latLineEdit = NULL;
        delete ui->labLon;
        ui->labLon = NULL;
        delete ui->lonLineEdit;
        ui->lonLineEdit = NULL;
        delete ui->RTScheckBox;
        ui->RTScheckBox = NULL;
    }
    else
    {
        QRegExp rx1("^-?(90|[1-8]?\\d(\\.\\d+)?)$");
        QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
        ui->latLineEdit->setValidator( pReg1 );
        QRegExp rx2("^-?(180|1?[0-7]?\\d(\\.\\d+)?)$");
        QRegExpValidator *pReg2 = new QRegExpValidator(rx2, this);
        ui->lonLineEdit->setValidator( pReg2 );
    }
    if ((mItems & MSD_GpsSelector) == 0)
    {
        delete ui->verticalLayoutGps;
        ui->verticalLayoutGps = NULL;
        delete ui->labSelectGps;
        ui->labSelectGps = NULL;
        delete ui->GPSComboBox;
        ui->GPSComboBox = NULL;
    }
    else
    {
        ui->GPSComboBox->setMaxVisibleItems(40);

        QStringList filter;
        filter << "*.gps";
        QDir dirGPS( QFileInfo( QCoreApplication::applicationFilePath() ).dir().absolutePath()
                     + "/resource/gpsfiles");
        QStringList gpsFiles = dirGPS.entryList( filter );

        SetGPSFileList( gpsFiles, "US_SNA.gps");
    }

    QString path = GlobalSetting::GetInstance().GetMapDataPath();
    if(!path.isEmpty())
    {
        ui->path->setText(path);
    }

    connect(ui->pathButton, SIGNAL(clicked()), this, SLOT(OnPathButtonclicked()));

}

ModeSelectorDialog::~ModeSelectorDialog()
{
    delete ui;
}

void ModeSelectorDialog::OnTimer()
{
    accept();
}

void ModeSelectorDialog::SetGPSFileList(const QStringList &GPSFiles, const QString defaultGPS)
{
    ui->GPSComboBox->addItems( GPSFiles );

    int defaultIndex = 0;
    for( int i = 0; i < GPSFiles.size(); i++ )
    {
        if( GPSFiles[i] == defaultGPS )
        {
            defaultIndex = i;
            break;
        }
    }
    ui->GPSComboBox->setCurrentIndex( defaultIndex );
    mSelectedGPS = GPSFiles[ defaultIndex ];
}

void ModeSelectorDialog::on_GPSComboBox_currentIndexChanged(const QString &arg1)
{
    mSelectedGPS = arg1;
    QString filepath = QFileInfo( QCoreApplication::applicationFilePath() ).dir().absolutePath()
                 + "/resource/gpsfiles/" + mSelectedGPS;
    GetDefaultDestinationAndOrigin(filepath.toStdString(), &mMode.dest, &mMode.orig);
    if (mItems & MSD_Destination)
    {
        ui->latLineEdit->setText(QString::number(mMode.dest.GetLocation().center.latitude, 'f', 6));
        ui->lonLineEdit->setText(QString::number(mMode.dest.GetLocation().center.longitude, 'f', 6));
    }
}

void ModeSelectorDialog::accept()
{
    if (mItems & MSD_GpsSelector)
    {
        mMode.gpsFile = mSelectedGPS;
    }
    if (mItems & MSD_NavMode)
    {
        mMode.enableOnboard = ui->radioBtnOnboard->isChecked() ? true : false;
    }

    if (mItems & MSD_Destination)
    {
        mMode.enableRTS = ui->RTScheckBox->isChecked()? true : false;
        MapLocation location;
        if(ui->latLineEdit->text().isEmpty())
        {
            QMessageBox box( QMessageBox::NoIcon, "input error", "please input latitude value" );
            box.exec();
            return;
        }
        if(ui->lonLineEdit->text().isEmpty())
        {
            QMessageBox box( QMessageBox::NoIcon, "input error", "please input longtitude value" );
            box.exec();
            return;
        }
        location.center.latitude = ui->latLineEdit->text().toDouble();
        location.center.longitude = ui->lonLineEdit->text().toDouble();
        mMode.dest.SetLocation(location);
    }
    QString path = ui->path->text();
    QFileInfo info(path);
    if(info.exists())
    {
        GlobalSetting::GetInstance().SetMapDataPath(path);
        mMode.dataFile = path;
    }
    else
    {
        mMode.dataFile = QString();
    }

    QDialog::accept();
}

void ModeSelectorDialog::reject()
{
    exit(0);
}

void ModeSelectorDialog::on_ModeSelector_accepted()
{
    accept();
}

void ModeSelectorDialog::OnPathButtonclicked()
{
    QFileDialog* fd = new QFileDialog(this);
    fd->setWindowTitle("Choose map data folder path");
    fd->setFileMode(QFileDialog::Directory);
    fd->setOption(QFileDialog::ShowDirsOnly);
    QFileInfo info(GlobalSetting::GetInstance().GetMapDataPath());
    if(info.exists())
    {
        fd->setDirectory(info.dir());
    }
    if(fd->exec())
    {
        ui->path->setText(fd->selectedFiles()[0]);
    }
}

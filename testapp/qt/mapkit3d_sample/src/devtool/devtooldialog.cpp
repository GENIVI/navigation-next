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

#include "devtooldialog.h"
#include "ui_devtooldialog.h"
#include "globalsetting.h"

DevToolDialog::DevToolDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DevToolDialog)
{
    ui->setupUi(this);
    ui->GPSComboBox->setMaxVisibleItems(40);
}

DevToolDialog::~DevToolDialog()
{
    delete ui;
}

void DevToolDialog::SetServerList(const QStringList &servers, const QString defaultServer)
{
    ui->serverComboBox->addItems( servers );

    int defaultIndex = 0;
    for( int i = 0; i < servers.size(); i++ )
    {
        if( servers[i] == defaultServer )
        {
            defaultIndex = i;
            break;
        }
    }
    ui->serverComboBox->setCurrentIndex( defaultIndex );
    mSelectedServer = servers[ defaultIndex ];
}

void DevToolDialog::SetGPSFileList(const QStringList &GPSFiles, const QString defaultGPS)
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

QString DevToolDialog::GetSelectedServer()
{
    return mSelectedServer;
}

QString DevToolDialog::GetSelectedGPS()
{
    return mSelectedGPS;
}

void DevToolDialog::on_serverComboBox_currentIndexChanged(const QString &arg1)
{
    mSelectedServer = arg1;
}

void DevToolDialog::on_GPSComboBox_currentIndexChanged(const QString &arg1)
{
    mSelectedGPS = arg1;
}

void DevToolDialog::on_okButton_clicked()
{
    accept();
}

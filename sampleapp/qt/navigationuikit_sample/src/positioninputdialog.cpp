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

#include "positioninputdialog.h"
#include "ui_positioninputdialog.h"
#include <QMessageBox>

PositionInputDialog::PositionInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PositionInputDialog),
    mLat(0.0),
    mLon(0.0)
{
    ui->setupUi(this);

    // set the limit to the input value
    QRegExp rx1("^-?(90|[1-8]?\\d(\\.\\d+)?)$");
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->latLineEdit->setValidator( pReg1 );

    QRegExp rx2("^-?(180|1?[0-7]?\\d(\\.\\d+)?)$");
    QRegExpValidator *pReg2 = new QRegExpValidator(rx2, this);
    ui->lonLineEdit->setValidator( pReg2 );
}

PositionInputDialog::~PositionInputDialog()
{
    delete ui;
}

qreal PositionInputDialog::GetLatitude()
{
    return mLat;
}

qreal PositionInputDialog::GetLongitude()
{
    return mLon;
}

void PositionInputDialog::SetLatitude(qreal latitude)
{
    mLat = latitude;
    ui->latLineEdit->setText(QString::number(mLat));
}

void PositionInputDialog::SetLongitude(qreal longitude)
{
    mLon = longitude;
    ui->lonLineEdit->setText(QString::number(mLon));
}

bool PositionInputDialog::EnableRTS()
{
    return mEnableRTS;
}

void PositionInputDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if( ui->buttonBox->standardButton( button ) == QDialogButtonBox::Ok )
    {
        if( ui->latLineEdit->text().length() == 0 )
        {
            QMessageBox box( QMessageBox::NoIcon, "input error", "please input latitude value" );
            box.exec();
        }
        else if( ui->lonLineEdit->text().length() == 0 )
        {
            QMessageBox box( QMessageBox::NoIcon, "input error", "please input longtitude value" );
            box.exec();
        }

        mLat = ui->latLineEdit->text().toDouble();
        mLon = ui->lonLineEdit->text().toDouble();
        mEnableRTS = ui->checkBox->isChecked();
    }
}

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

#include "qaloguploaddialog.h"
#include "ui_qaloguploaddialog.h"
#include "util.h"

QaLogUploadDialog::QaLogUploadDialog(QaLogListenerDelegate* logDelegate, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QaLogUploadDialog),
    mLogDelegate(logDelegate)
{
    ui->setupUi(this);
    if( mLogDelegate != NULL )
    {
        connect( mLogDelegate, SIGNAL(QaLogFileEmpty()), this, SLOT(OnQaLogFileEmpty()) );
        connect( mLogDelegate, SIGNAL(QaUploadCanceled()), this, SLOT(OnQaUploadCanceled()) );
        connect( mLogDelegate, SIGNAL(QaUploadCompleted(QString)), this, SLOT(OnQaUploadCompleted(QString)) );
        connect( mLogDelegate, SIGNAL(QaUploadFailed(int)), this, SLOT(OnQaUploadFailed(int)) );
        connect( mLogDelegate, SIGNAL(QaUploadProgress(int)), this, SLOT(OnQaUploadProgress(int)) );
        connect( mLogDelegate, SIGNAL(QaUploadStarted()), this, SLOT(OnQaUploadStarted()) );
    }
}

QaLogUploadDialog::~QaLogUploadDialog()
{
    delete ui;
}

void QaLogUploadDialog::OnQaUploadStarted()
{
    ui->labelPercentage->setText( "0%" );
}

void QaLogUploadDialog::OnQaUploadCanceled()
{
    ui->labelPercentage->setText( "upload cancled" );
}

void QaLogUploadDialog::OnQaUploadCompleted(const QString &logid)
{
    ui->labelPercentage->setText( "upload completed, log id = " + logid );
}

void QaLogUploadDialog::OnQaUploadProgress(int percentage)
{
    QString strPercent;
    strPercent.sprintf( "%d%%", percentage );
    ui->labelPercentage->setText( strPercent );
}

void QaLogUploadDialog::OnQaLogFileEmpty()
{
    ui->labelPercentage->setText( "log file is empty" );
}

void QaLogUploadDialog::OnQaUploadFailed(int error)
{
    QString strText;
    strText.sprintf( "upload failed, error code = %d", error );
    ui->labelPercentage->setText( strText );
}

void QaLogUploadDialog::on_pushButtonOK_clicked()
{
    GetLTKContext()->UploadQaLog();
}

void QaLogUploadDialog::on_pushButtonCancel_clicked()
{
    close();
}

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

#ifndef QALOGUPLOADDIALOG_H
#define QALOGUPLOADDIALOG_H

#include "qaloglistenerdelegate.h"
#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class QaLogUploadDialog;
}

class QaLogUploadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QaLogUploadDialog(QaLogListenerDelegate* logDelegate, QWidget *parent = 0);
    ~QaLogUploadDialog();

public Q_SLOTS:
    void OnQaUploadStarted();
    void OnQaUploadCanceled();
    void OnQaUploadCompleted(const QString& logid);
    void OnQaUploadProgress(int percentage);
    void OnQaLogFileEmpty();
    void OnQaUploadFailed (int error);

private slots:
    void on_pushButtonOK_clicked();

    void on_pushButtonCancel_clicked();

private:
    Ui::QaLogUploadDialog *ui;
    QaLogListenerDelegate* mLogDelegate;
};

#endif // QALOGUPLOADDIALOG_H

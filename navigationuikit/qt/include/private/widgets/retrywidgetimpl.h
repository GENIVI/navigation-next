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
    @file retrywidgetimpl.h
    @date 10/31/2014
    @addtogroup navigationuikit
*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary to
    TeleCommunication Systems, Inc., and considered a trade secret as defined
    in section 499C of the penal code of the State of California. Use of this
    information by anyone other than authorized employees of TeleCommunication
    Systems is granted only under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#ifndef _RETRYWIDGETIMPL_H_
#define _RETRYWIDGETIMPL_H_

#include "widgetbase.h"
#include "RetryWidget.h"
#include "NKUIPresenter.h"
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeContext>
#include <QTimer>

namespace locationtoolkit
{
class RetryWidgetImpl : public WidgetBase, public nkui::RetryWidget
{
    Q_OBJECT
    Q_PROPERTY(QString Message READ GetMessage WRITE SetMessage NOTIFY MessageChange)
    Q_PROPERTY(QString Confirm READ GetConfirmText WRITE SetConfirmText NOTIFY ConfirmTextChange)
    Q_PROPERTY(QString Cancel READ GetCancelText WRITE SetCancelText NOTIFY CancelTextChange)
public:
    RetryWidgetImpl();
    virtual ~RetryWidgetImpl();

    QString GetMessage() const;
    QString GetConfirmText() const;
    QString GetCancelText() const;

    void SetMessage(const QString& message);
    void SetConfirmText(const QString& confirm);
    void SetCancelText(const QString& cancel);

    void DisableTimer();
signals:
    void MessageChange(QString);
    void ConfirmTextChange(QString);
    void CancelTextChange(QString);

public slots:
    void OnNavigationRequestRetry();
    void OnNavigationRequestCancel();

private slots:
    void TimerUpDate();

private:
    /*! get widget id . */
    virtual nkui::WidgetID GetWidgetID();
    /*! set presenter to widget. */
    virtual void SetPresenter(nkui::NKUIPresenter* presenter);
    /*! show a widget */
    virtual void Show();
    virtual void Hide();

    // override WidgetBase
    virtual QWidget* GetNativeView();
    // override WidgetBase
    virtual void CreateNativeView(QWidget* parentView);
    virtual void OnSetPresenter();

    // override RetryWidget
    virtual void ShowWithParameters(nkui::RetryWidgetParameters& parameters);

private:
    QWidget* mWidget;
    QDeclarativeView* mRetryViewer;
    QObject* mRetryObject;
    QString  mMessage;
    QString  mConfirmText;
    QString  mCancelText;
    QTimer*  mTimer;
    nkui::RetryWidgetParameters mRetryPar;
};
}

#endif /* _RETRYWIDGETIMPL_H_ */

/*! @} */

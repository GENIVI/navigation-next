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
    @file footerwidgetbase.h
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

#ifndef _FOOTERWIDGETBASE_H_
#define _FOOTERWIDGETBASE_H_

#include <iostream>
#include "FooterWidget.h"
#include "widgetbase.h"

class QDeclarativeView;

namespace locationtoolkit
{
class FooterWidgetBase: public WidgetBase, public nkui::FooterWidget
{
    Q_OBJECT
    Q_PROPERTY(QString leftText READ GetLeftText WRITE SetLeftText NOTIFY LeftTextChanged)
    Q_PROPERTY(QString midText READ GetMidText WRITE SetMidText NOTIFY MidTextChanged)
    Q_PROPERTY(QString rightText READ GetRightText WRITE SetRightText NOTIFY RightTextChanged)
    Q_PROPERTY(QString statusText READ GetStatusText WRITE SetStatusText NOTIFY StatusTextChanged)
    Q_PROPERTY(bool footerOptionVisible READ GetFooterOptionVisible WRITE SetFooterOptionVisible NOTIFY FooterOptionVisibleChanged)

public:
    enum
    {
        FBT_None = 0,
        FBT_Left,
        FBT_Mid,
        FBT_Right
    }FooterButtonType;

public:
    explicit FooterWidgetBase(nkui::WidgetID widgetID);
    virtual ~FooterWidgetBase();

public:
    QString GetLeftText() const;
    QString GetMidText() const;
    QString GetRightText() const;
    QString GetStatusText() const;
    bool GetFooterOptionVisible() const;

    void SetLeftText(const QString& leftText);
    void SetMidText(const QString& midText);
    void SetRightText(const QString& rightText);
    void SetStatusText(const QString& statusText);
    void SetFooterOptionVisible(bool visible);

signals:
    void LeftTextChanged();
    void MidTextChanged();
    void RightTextChanged();
    void StatusTextChanged();
    void FooterOptionVisibleChanged();

    void ShowFooterOption(bool onoff);

public slots:
    virtual void OnFooterButtonSignal(int index) = 0;

private:
    /* inherit widget */
    virtual void Hide();

    virtual nkui::WidgetID GetWidgetID();

    virtual void SetPresenter(nkui::NKUIPresenter* presenter);

    virtual void Show();

    /* inherit WidgetBase */
    virtual QWidget* GetNativeView();
    virtual void CreateNativeView(QWidget* parentView);

    /* inherit Widget FooterWidget*/
    /** Show progress bar
     */
    virtual void ShowProgress(float progress);

    /** Hide progress bar
     */
    virtual void HideProgress();

    /** Show indicator view
     */
    virtual void ShowIndicator();

    /** Hide indicator view
     */
    virtual void HideIndicator();

    /*! set messge show at footer.
     */
    virtual void ShowMessage(const std::string& message);

    /*! hide message.
     */
    virtual void HideMessage();

    /*! show menu button
     */
    virtual void ShowMenuButton();

    /*! Hide menu button
     */
    virtual void HideMenuButton();

    /*! show list button.
     */
    virtual void ShowListButton();

    /*! Hide list button.
     */
    virtual void HideListButton();

    /*! show cancel button.
     */
    virtual void ShowcancelButton();
    /*! Hide cancel button.
     */
    virtual void HidecancelButton();
    /*! Set list button status
     *
     *  @param isOpen: true if opened.
     *  @return: None
     */
    virtual void SetListButtonStatus(bool isOpen);
    /*! set title. */
    virtual void SetTitle(std::string title);

protected:
    QDeclarativeView* mFooterViewer;
    QString mLeftText;
    QString mMidText;
    QString mRightText;
    QString mStatusText;
    bool mFooterOptionVisible;
};
}

#endif /* _FOOTERWIDGETBASE_H_ */

/*! @} */

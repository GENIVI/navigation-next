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
    @file routeselectbubblelayerimpl.h
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
    --------------------------------------------------------------------------
*/

/*! @{ */

#ifndef _ROUTESELECTBUBBLELAYERIMPL_H_
#define _ROUTESELECTBUBBLELAYERIMPL_H_

#include "widgetbase.h"
#include "NKUIPresenter.h"
#include "RouteSelectBubbleLayer.h"
#include <vector>
#include <QPoint>

namespace locationtoolkit
{
class RouteSelectBubbleInternal;
class RouteSelectBubbleLayoutHelper;
class RouteSelectBubbleInfo;
class RouteSelectBubbleImpl : public QObject, public nkui::RouteSelectBubble
{
    Q_OBJECT
public:
    explicit RouteSelectBubbleImpl(QWidget* parentView);
    virtual ~RouteSelectBubbleImpl();

public:
    void Update(const RouteSelectBubbleInfo& info);
    void Hide();
    void Show();
    const QPoint& GetPosition();
    int GetWidth();
    int GetHeight();
    virtual void SetPosition(const nkui::ScreenPoint& position);

public slots:
    void OnBubbleClicked();

signals:
    void BubbleClicked(const RouteSelectBubbleImpl* bubble);

private:
    // override RouteSelectBubble
    virtual void SetSelected(bool bSelected);
    virtual void SetText(const std::string& text);
private:
    RouteSelectBubbleInternal* mWidget;
    QPoint mPosition;
};

class RouteSelectBubbleLayerImpl  : public WidgetBase, public nkui::RouteSelectBubbleLayer
{
    Q_OBJECT
public:
    RouteSelectBubbleLayerImpl();
    virtual ~RouteSelectBubbleLayerImpl();

public slots:
    void UpdateLayout();
    void OnBubbleClicked(const RouteSelectBubbleImpl* bubble);

private:
    /* inherit widget */
    virtual void Hide();

    virtual nkui::WidgetID GetWidgetID();

    virtual void SetPresenter(nkui::NKUIPresenter* presenter);

    // override WidgetBase
    virtual void Show();
    virtual QWidget* GetNativeView();

    // override RouteSelectBubbleLayer
    virtual nkui::RouteSelectBubble* CreateBubble(const nkui::ScreenPoint& position);
    virtual void ClearBubbles();
    virtual void UpdateLayout(bool bRightNow);

    // override WidgetBase
    virtual void CreateNativeView(QWidget* parentView);
    virtual void OnSetPresenter();
    virtual void SetFrame(nkui::WidgetFrame viewFrame);

private:
    std::vector<RouteSelectBubbleImpl*> mBubbles;
    QWidget* mWidget;
    RouteSelectBubbleLayoutHelper* mHelper;
    bool haveSetFrame;
};
}

#endif /* _ROUTESELECTBUBBLELAYERIMPL_H_ */

/*! @} */

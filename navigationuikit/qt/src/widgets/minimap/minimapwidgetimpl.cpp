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
    @file minimapwidgetimpl.cpp
    @date 10/20/2014
    @addtogroup navigationuikit
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
---------------------------------------------------------------------------*/

#include "minimapwidgetimpl.h"
#include "navigationuikitwrapper.h"
#include "NKUIEvent.h"
#include "minimapbutton.h"
#include <QDir>
#include <QCoreApplication>

using namespace locationtoolkit;

MiniMapWidgetImpl::MiniMapWidgetImpl(nkui::WidgetID widgetID)
    : WidgetBase(widgetID)
    , mMiniMapButton(NULL)
{
}

MiniMapWidgetImpl::~MiniMapWidgetImpl()
{
}

void MiniMapWidgetImpl::LoadImage()
{
    QString appPath = NavigaitonUIKitWrapper::GetResourcePath();
    mBackgroundImage = appPath + "/images/routeoverview.png";
}

void MiniMapWidgetImpl::OnMiniMapClick()
{
    if (mPresenter)
    {
        mPresenter->HandleEvent(nkui::EVT_TAP_MINIMAP, NULL);
    }
}

void MiniMapWidgetImpl::CreateNativeView(QWidget *parentView)
{
    LoadImage();
    mMiniMapButton = new MiniMapButton(mBackgroundImage, mForegroundImage, parentView);
    QObject::connect(mMiniMapButton, SIGNAL(clicked()), this, SLOT(OnMiniMapClick()));
}

void MiniMapWidgetImpl::updateTraffic(nbnav::TrafficEvent::TrafficSeverity trafficSerity,
                                      nbnav::TrafficEvent::Type type)
{
}

void MiniMapWidgetImpl::HideTraffic()
{
}

QWidget *MiniMapWidgetImpl::GetNativeView()
{
    return mMiniMapButton;
}

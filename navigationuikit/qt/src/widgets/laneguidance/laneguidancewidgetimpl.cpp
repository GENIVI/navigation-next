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
    @file laneguidancewidgetimpl.cpp
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

#include "laneguidancewidgetimpl.h"
#include "laneguidancewidgetinternal.h"
#include <QDebug>

using namespace locationtoolkit;

LaneGuidanceWidgetImpl::LaneGuidanceWidgetImpl(nkui::WidgetID widgetID)
    : WidgetBase(widgetID)
    , mLaneGuidanceWidget(NULL)
{
}

LaneGuidanceWidgetImpl::~LaneGuidanceWidgetImpl()
{
}

void LaneGuidanceWidgetImpl::Hide()
{
    if(mVisible)
    {
        mLaneGuidanceWidget->hide();
        mVisible = false;
    }

    if(mPresenter)
    {
        mPresenter->HandleEvent(nkui::EVT_SHOW_CURRENTROAD, NULL);
    }
}

nkui::WidgetID LaneGuidanceWidgetImpl::GetWidgetID()
{
    return mWidgetID;
}

void LaneGuidanceWidgetImpl::SetPresenter(nkui::NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void LaneGuidanceWidgetImpl::Show()
{
    if(!mVisible)
    {
        mLaneGuidanceWidget->show();
        mVisible = true;
    }

    mPresenter->HandleEvent(nkui::EVT_HIDE_CURRENTROAD, NULL);
}

void LaneGuidanceWidgetImpl::UpdateLaneGuidance(std::vector<nbnav::Lane> lanes, int32 selectStart, int32 selectCount)
{
    if (lanes.empty())
    {
        return;
    }

    std::vector<LaneData> laneDataList;
    std::vector<nbnav::Lane>::iterator iterLane = lanes.begin();
    while (iterLane != lanes.end())
    {
        nbnav::Lane& lane = *iterLane;

        std::string highlightString = lane.GetHighlighted();
        std::string nonhighlightString = lane.GetNonHighlighted();
        if(highlightString.length() > LANE_MAX)
        {
            return;
        }

        if(nonhighlightString.length() > LANE_MAX)
        {
            return;
        }

        LaneData laneData;
        for(int i = 0; i < highlightString.length(); ++i)
        {
            laneData.highlightCharaters.push_back(QString(highlightString[i]));
        }

        for(int i = 0; i < nonhighlightString.length(); ++i)
        {
            laneData.nonhighlightCharaters.push_back(QString(nonhighlightString[i]));
        }

        laneDataList.push_back(laneData);

        iterLane ++;
    }

    mLaneGuidanceWidget->DrawLanes(laneDataList);
}

void LaneGuidanceWidgetImpl::SetType(nkui::LGW_LaneGuidanceWidgetType type)
{
}

QWidget *LaneGuidanceWidgetImpl::GetNativeView()
{
    return mLaneGuidanceWidget;
}

void LaneGuidanceWidgetImpl::CreateNativeView(QWidget *parentView)
{
    mLaneGuidanceWidget = new LaneGuidanceWidgetInternal(parentView);
    mLaneGuidanceWidget->setAutoFillBackground(true);
}

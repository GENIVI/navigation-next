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
    @file routeselectbubblelayouthelper.cpp
    @date 12/29/2014
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

#include "routeselectbubblelayouthelper.h"
#include <QDebug>
#include <QWidget>

using namespace locationtoolkit;

RouteSelectBubbleLayoutHelper::RouteSelectBubbleLayoutHelper(QWidget* parent)
    : mBubbleWidth(0)
    , mBubbleHeight(0)
    , mScreen()
{
}

RouteSelectBubbleLayoutHelper::~RouteSelectBubbleLayoutHelper()
{
}

std::vector<RouteSelectBubbleInfo> RouteSelectBubbleLayoutHelper::GetFourBubbles(const QPoint &centerPoint)
{
    std::vector<RouteSelectBubbleInfo> bubbleInfos;

    RouteSelectBubbleInfo info;
    info.rect = GetLeftTopBubble(centerPoint);
    info.type = RouteSelectBubbleInfo::BT_LeftTopOff;
    bubbleInfos.push_back(info);

    info.rect = GetRightTopBubble(centerPoint);
    info.type = RouteSelectBubbleInfo::BT_RightTopOff;
    bubbleInfos.push_back(info);

    info.rect = GetLeftBottomBubble(centerPoint);
    info.type = RouteSelectBubbleInfo::BT_LeftBottomOff;
    bubbleInfos.push_back(info);

    info.rect = GetRightBottomBubble(centerPoint);
    info.type = RouteSelectBubbleInfo::BT_RightBottomOff;
    bubbleInfos.push_back(info);

    return bubbleInfos;
}

std::vector<RouteSelectBubbleInfo> RouteSelectBubbleLayoutHelper::GetBestLayouts(const QPoint &centerPoint1,
                                                                                 const QPoint &centerPoint2,
                                                                                 const QPoint &centerPoint3)
{
    int minArea = mBubbleWidth * mBubbleHeight * 3;
    int totalOverlapArea = 0;
    int totalOutsideArea = 0;
    int totalArea = 0;
    int bubble1 = 0;
    int bubble2 = 0;
    int bubble3 = 0;
    std::vector<RouteSelectBubbleInfo> bubbleRects1 = GetFourBubbles(centerPoint1);
    std::vector<RouteSelectBubbleInfo> bubbleRects2 = GetFourBubbles(centerPoint2);
    std::vector<RouteSelectBubbleInfo> bubbleRects3 = GetFourBubbles(centerPoint3);

    for(size_t i = 0; i < bubbleRects1.size(); ++i)
    {
        for(size_t j = 0; j < bubbleRects2.size(); ++j)
        {
            for(size_t k = 0; k < bubbleRects3.size(); ++k)
            {
                totalOutsideArea = GetTotalOutsiddArea(bubbleRects1[i].rect, bubbleRects2[j].rect, bubbleRects3[k].rect, mScreen);
                totalOverlapArea = GetTotalOverlapArea(bubbleRects1[i].rect, bubbleRects2[j].rect, bubbleRects3[k].rect);
                totalArea = totalOutsideArea + totalOverlapArea;
                if(totalArea < minArea)
                {
                    minArea = totalArea;
                    bubble1 = i;
                    bubble2 = j;
                    bubble3 = k;
                }
            }
        }
    }

    std::vector<RouteSelectBubbleInfo> bestLayout;
    bestLayout.push_back(bubbleRects1[bubble1]);
    bestLayout.push_back(bubbleRects2[bubble2]);
    bestLayout.push_back(bubbleRects3[bubble3]);
    return bestLayout;
}

bool RouteSelectBubbleLayoutHelper::NeedCalculateNewLayoutOrNot(const QPoint &centerPoint1,
                                                                const QPoint &centerPoint2,
                                                                const QPoint &centerPoint3)
{
    QRect bubbleRects1 = GetRightBottomBubble(centerPoint1);
    QRect bubbleRects2 = GetRightBottomBubble(centerPoint2);
    QRect bubbleRects3 = GetRightBottomBubble(centerPoint3);

    int totalOutsideArea = GetTotalOutsiddArea(bubbleRects1, bubbleRects2, bubbleRects3, mScreen);
    int totalOverlapArea = GetTotalOverlapArea(bubbleRects1, bubbleRects2, bubbleRects3);
    if((totalOutsideArea + totalOverlapArea) != 0)
    {
        return true;
    }

    return false;
}

void RouteSelectBubbleLayoutHelper::SetBubbleWidth(int width)
{
    mBubbleWidth = width;
}

void RouteSelectBubbleLayoutHelper::SetBubbleHeight(int height)
{
    mBubbleHeight = height;
}

void RouteSelectBubbleLayoutHelper::SetScreenGeometry(QRect rect)
{
    mScreen = rect;
}

bool RouteSelectBubbleLayoutHelper::IsOutsetScreenArea(const QRect &rect)
{
    int area = GetOutsidearea(rect, mScreen);
    if(area > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

QRect RouteSelectBubbleLayoutHelper::GetRightBottomBubble(const QPoint &centerPoint)
{
    QRect rect(centerPoint.x(), centerPoint.y(),
               mBubbleWidth, mBubbleHeight);
    return rect;
}

QRect RouteSelectBubbleLayoutHelper::GetLeftBottomBubble(const QPoint &centerPoint)
{
    QRect rect(centerPoint.x() - mBubbleWidth + OFFSET_X, centerPoint.y(),
               mBubbleWidth, mBubbleHeight);
    return rect;
}

QRect RouteSelectBubbleLayoutHelper::GetRightTopBubble(const QPoint &centerPoint)
{
    QRect rect(centerPoint.x() - OFFSET_X, centerPoint.y() - mBubbleHeight - OFFSET_Y,
               mBubbleWidth, mBubbleHeight);
    return rect;
}

QRect RouteSelectBubbleLayoutHelper::GetLeftTopBubble(const QPoint &centerPoint)
{
    QRect rect(centerPoint.x() - mBubbleWidth + OFFSET_X, centerPoint.y() - mBubbleHeight - OFFSET_Y,
               mBubbleWidth, mBubbleHeight);
    return rect;
}

int RouteSelectBubbleLayoutHelper::GetTotalOverlapArea(const QRect &rect1,
                                                       const QRect &rect2,
                                                       const QRect &rect3)
{
    int area = GetOverlapArea(rect1, rect2);
    area += GetOverlapArea(rect1, rect3);
    area += GetOverlapArea(rect2, rect3);

    return area;
}

int RouteSelectBubbleLayoutHelper::GetTotalOutsiddArea(const QRect &rect1,
                                                       const QRect &rect2,
                                                       const QRect &rect3,
                                                       const QRect& screen)
{
    int area = GetOutsidearea(rect1, screen);
    area += GetOutsidearea(rect2, screen);
    area += GetOutsidearea(rect3, screen);

    return area;
}

int RouteSelectBubbleLayoutHelper::GetOverlapArea(const QRect &rect1, const QRect &rect2)
{
    QRect rect = rect1.intersected(rect2);
    return rect.width() * rect.height();
}

int RouteSelectBubbleLayoutHelper::GetOutsidearea(const QRect &rect, const QRect &screen)
{
    int overlapArea = GetOverlapArea(rect, screen);
    return (rect.width() * rect.height() - overlapArea);
}

/*! @} */

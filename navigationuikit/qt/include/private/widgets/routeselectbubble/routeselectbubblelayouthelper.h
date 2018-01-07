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
    @file routeselectbubblelayouthelper.h
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

#ifndef _ROUTESELECTBUBBLELAYOUTHELPER_H_
#define _ROUTESELECTBUBBLELAYOUTHELPER_H_

#include <vector>
#include <QRect>

class QWidget;
namespace locationtoolkit
{
class RouteSelectBubbleInfo
{
public:
    typedef enum{
        BT_Invalid = 0,
        BT_LeftTopOff,
        BT_LeftTopOn,
        BT_LeftBottomOff,
        BT_LeftBottomOn,
        BT_RightTopOff,
        BT_RightTopOn,
        BT_RightBottomOff,
        BT_RightBottomOn
    }RouteSelectBubbleType;

public:
    RouteSelectBubbleInfo()
        : type(BT_Invalid)
    {}

    ~RouteSelectBubbleInfo() {}

    RouteSelectBubbleInfo& operator=(const RouteSelectBubbleInfo& info)
    {
        rect = info.rect;
        type = info.type;
    }

public:
    QRect rect;
    RouteSelectBubbleType type;
};

class RouteSelectBubbleLayoutHelper
{
public:
    static const int OFFSET_X = 0;
    static const int OFFSET_Y = 0;

public:
    RouteSelectBubbleLayoutHelper(QWidget* parent);
    virtual ~RouteSelectBubbleLayoutHelper();

public:
    std::vector<RouteSelectBubbleInfo> GetBestLayouts(const QPoint &centerPoint1,
                                                      const QPoint &centerPoint2,
                                                      const QPoint &centerPoint3);
    bool NeedCalculateNewLayoutOrNot(const QPoint &centerPoint1,
                                     const QPoint &centerPoint2,
                                     const QPoint &centerPoint3);
    void SetBubbleWidth(int width);
    void SetBubbleHeight(int height);
    void SetScreenGeometry(QRect rect);
    bool IsOutsetScreenArea(const QRect &rect);
    QRect GetRightBottomBubble(const QPoint& centerPoint);

private:
    QRect GetLeftBottomBubble(const QPoint& centerPoint);
    QRect GetRightTopBubble(const QPoint& centerPoint);
    QRect GetLeftTopBubble(const QPoint& centerPoint);
    std::vector<RouteSelectBubbleInfo> GetFourBubbles(const QPoint& centerPoint);
    int GetTotalOverlapArea(const QRect& rect1, const QRect& rect2, const QRect& rect3);
    int GetTotalOutsiddArea(const QRect& rect1, const QRect& rect2, const QRect& rect3, const QRect& screen);
    int GetOverlapArea(const QRect& rect1, const QRect& rect2);
    int GetOutsidearea(const QRect& rect, const QRect& screen);

private:
    int mBubbleWidth;
    int mBubbleHeight;
    QRect mScreen;
};
}

#endif /* _ROUTESELECTBUBBLELAYOUTHELPER_H_ */

/*! @} */

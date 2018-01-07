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

#include "laneguidancewidgetinternal.h"
#include "laneguidancewidgetimpl.h"
#include "navigationuikitwrapper.h"
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QPainter>

using namespace locationtoolkit;

LaneGuidanceWidgetInternal::LaneGuidanceWidgetInternal(QWidget* parent)
    : QWidget(parent)
    , mLeftImageRotate(NULL)
    , mRightImageRotate(NULL)
    , mLeftImageWidth(0)
    , mLeftImageHeight(0)
    , mCenterImageHeight(0)
    , mRightImageWidth(0)
    , mRightImageHeight(0)
    , mNexgenFont("LG NexGen", LANE_FONT_SIZE)
    , mFm(mNexgenFont)
    , mOneCharWidth(0)
{
    QString appPath = NavigaitonUIKitWrapper::GetResourcePath();
    QString imagePath = appPath + "/images/laneguidance_left.png";
    QPixmap leftImage(imagePath);

    imagePath = appPath + "/images/laneguidance_strip.png";
    mCenterImage = new QPixmap(imagePath);

    imagePath = appPath + "/images/laneguidance_right.png";
    QPixmap rightImage(imagePath);

    QMatrix matrix;
    matrix.rotate(180);
    mLeftImageRotate = new QPixmap(rightImage.transformed(matrix));
    mRightImageRotate = new QPixmap(leftImage.transformed(matrix));

    mLeftImageWidth = mLeftImageRotate->width();
    mLeftImageHeight = mLeftImageRotate->height();
    mCenterImageHeight = mCenterImage->height();
    mRightImageWidth = mRightImageRotate->width();
    mRightImageHeight = mRightImageRotate->height();

    mOneCharWidth = mFm.charWidth(QString("A"), 0);
}

void LaneGuidanceWidgetInternal::DrawLanes(std::vector<LaneData> laneDataList)
{
    mLaneDataList =  laneDataList;
    update();
}

void LaneGuidanceWidgetInternal::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.fillRect(0, 0, width(), height(), QColor(71, 71, 71, 255));
    painter.setFont(mNexgenFont);

    //get font's height
    int fontHeight = mFm.lineSpacing();
    //get y-position, The y-position is used as the baseline of the font.
    int drawTextYPos = (this->rect().height() - fontHeight)/2.0 + fontHeight - mFm.descent();
    int centerTextWidth = mOneCharWidth * mLaneDataList.size();
    int combineImageWidth  = mLeftImageWidth + centerTextWidth + mRightImageWidth;
    int combineImageHeight = mCenterImageHeight;
    int startPaintPointX   = (width() - combineImageWidth) / 2;

    int startDrawTextX = startPaintPointX + mLeftImageWidth;
    for(size_t i = 0; i < mLaneDataList.size(); ++i)
    {
        LaneData laneData = mLaneDataList[i];

        std::vector<QString> highlights = laneData.highlightCharaters;
        if(!highlights.empty())
        {
            painter.setPen(QColor(255, 255, 255, 255));
            for(int i = 0; i < highlights.size(); ++i)
            {
                painter.drawText(startDrawTextX,
                                 drawTextYPos,
                                 highlights[i]);
            }
        }

        std::vector<QString> nonhighlights = laneData.nonhighlightCharaters;
        if(!nonhighlights.empty())
        {
            painter.setPen(QColor(136, 136, 136, 136));

            for(int i = 0; i < nonhighlights.size(); ++i)
            {
                painter.drawText(startDrawTextX,
                                 drawTextYPos,
                                 nonhighlights[i]);
            }
        }

        startDrawTextX += mOneCharWidth;
    }

    painter.end();

    QVector<QPoint> points;
    points.push_back(QPoint(startPaintPointX + mLeftImageWidth, 0));
    points.push_back(QPoint(startPaintPointX + mLeftImageWidth + centerTextWidth, 0));
    points.push_back(QPoint(startPaintPointX + combineImageWidth, combineImageHeight));
    points.push_back(QPoint(startPaintPointX, combineImageHeight));
    setMask(QRegion(QPolygon(points)));
}

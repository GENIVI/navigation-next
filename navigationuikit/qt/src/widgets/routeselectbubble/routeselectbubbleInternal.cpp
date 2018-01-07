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

#include "routeselectbubbleInternal.h"
#include "routeselectbubblelayouthelper.h"
#include "navigationuikitwrapper.h"
#include <QPainter>
#include <QBitmap>
#include <QDir>
#include <QCoreApplication>
#include <QLabel>
#include <QVBoxLayout>

using namespace locationtoolkit;

RouteSelectBubbleInternal::RouteSelectBubbleInternal(QWidget *parent)
    : QPushButton(parent)
    , mBackgroudPixmap(NULL)
    , mLabel(NULL)
    , mSelected(false)
{
    setAutoFillBackground(true);

    LoadBackgroundImages();

    mLabel = new QLabel(this);
    // @todo need move definitions of colors/font size into separate files,
    //       so user of NKUI can customize them.
    mLabel->setStyleSheet("background-color: rgba(255, 255, 255, 0); font: bold 10pt;");
    mLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(mLabel);
}

RouteSelectBubbleInternal::~RouteSelectBubbleInternal()
{
    std::map<RouteSelectBubbleInfo::RouteSelectBubbleType, QPixmap*>::iterator it = mPixmaps.begin();
    for(; it != mPixmaps.end(); ++it)
    {
        QPixmap* pixmap = it->second;
        if(pixmap)
        {
            pixmap->detach();
            delete pixmap;
        }

    }
    mPixmaps.clear();
}

void RouteSelectBubbleInternal::Update(const RouteSelectBubbleInfo &info)
{
    mBubbleInfo = info;
    RouteSelectBubbleInfo::RouteSelectBubbleType type = ConvertBubbleType(info.type, mSelected);
    mBackgroudPixmap = GetBackgroundImage(type);
    Repaint();
}

void RouteSelectBubbleInternal::SetText(const QString &text)
{
    mLabel->setText(text);
}

void RouteSelectBubbleInternal::SetSelected(bool bSelected)
{
    mSelected = bSelected;
    RouteSelectBubbleInfo::RouteSelectBubbleType type = ConvertBubbleType(mBubbleInfo.type, mSelected);
    mBackgroudPixmap = GetBackgroundImage(type);
    Repaint();
}

void RouteSelectBubbleInternal::paintEvent(QPaintEvent *paintEvent)
{
    QPainter painter(this);

    if(mBackgroudPixmap)
    {
        painter.drawPixmap(0, 0, *mBackgroudPixmap);
    }
}

void RouteSelectBubbleInternal::LoadBackgroundImages()
{
    QString background = NavigaitonUIKitWrapper::GetResourcePath() + "/images/navui_bubble_left_up_off.png";
    QPixmap backgroundImage(background);
    QPixmap* leftTopPixmap = new QPixmap(backgroundImage.scaled(backgroundImage.width() / 2,
                                                                backgroundImage.height() / 2));

    mPixmaps[RouteSelectBubbleInfo::BT_LeftTopOff] = leftTopPixmap;

    background = NavigaitonUIKitWrapper::GetResourcePath() + "/images/navui_bubble_left_down_off.png";
    backgroundImage.load(background);
    QPixmap* leftBottomPixmap = new QPixmap(backgroundImage.scaled(backgroundImage.width() / 2,
                                                                   backgroundImage.height() / 2));

    mPixmaps[RouteSelectBubbleInfo::BT_LeftBottomOff] = leftBottomPixmap;

    background = NavigaitonUIKitWrapper::GetResourcePath() + "/images/navui_bubble_right_up_off.png";
    backgroundImage.load(background);
    QPixmap* rightTopPixmap = new QPixmap(backgroundImage.scaled(backgroundImage.width() / 2,
                                                                 backgroundImage.height() / 2));
    mPixmaps[RouteSelectBubbleInfo::BT_RightTopOff] = rightTopPixmap;

    background = NavigaitonUIKitWrapper::GetResourcePath() + "/images/navui_bubble_right_down_off.png";
    backgroundImage.load(background);
    QPixmap* rightBottomPixmap = new QPixmap(backgroundImage.scaled(backgroundImage.width() / 2,
                                                                    backgroundImage.height() / 2));
    mPixmaps[RouteSelectBubbleInfo::BT_RightBottomOff] = rightBottomPixmap;

    background = NavigaitonUIKitWrapper::GetResourcePath() + "/images/navui_bubble_left_up_on.png";
    backgroundImage.load(background);
    QPixmap* leftTopPixmapOn = new QPixmap(backgroundImage.scaled(backgroundImage.width() / 2,
                                                                  backgroundImage.height() / 2));
    mPixmaps[RouteSelectBubbleInfo::BT_LeftTopOn] = leftTopPixmapOn;

    background = NavigaitonUIKitWrapper::GetResourcePath() + "/images/navui_bubble_left_down_on.png";
    backgroundImage.load(background);
    QPixmap* leftBottomPixmapOn = new QPixmap(backgroundImage.scaled(backgroundImage.width() / 2,
                                                                     backgroundImage.height() / 2));
    mPixmaps[RouteSelectBubbleInfo::BT_LeftBottomOn] = leftBottomPixmapOn;

    background = NavigaitonUIKitWrapper::GetResourcePath() + "/images/navui_bubble_right_up_on.png";
    backgroundImage.load(background);
    QPixmap* rightTopPixmapOn = new QPixmap(backgroundImage.scaled(backgroundImage.width() / 2,
                                                                   backgroundImage.height() / 2));
    mPixmaps[RouteSelectBubbleInfo::BT_RightTopOn] = rightTopPixmapOn;

    background = NavigaitonUIKitWrapper::GetResourcePath() + "/images/navui_bubble_right_down_on.png";
    backgroundImage.load(background);
    QPixmap* rightBottomPixmapOn = new QPixmap(backgroundImage.scaled(backgroundImage.width() / 2,
                                                                      backgroundImage.height() / 2));
    mPixmaps[RouteSelectBubbleInfo::BT_RightBottomOn] = rightBottomPixmapOn;
}

RouteSelectBubbleInfo::RouteSelectBubbleType RouteSelectBubbleInternal::ConvertBubbleType(RouteSelectBubbleInfo::RouteSelectBubbleType type, bool selected)
{
    if(!selected)
    {
        return type;
    }

    RouteSelectBubbleInfo::RouteSelectBubbleType convertType = RouteSelectBubbleInfo::BT_Invalid;
    switch (type) {
    case RouteSelectBubbleInfo::BT_LeftTopOff:
    {
        convertType = RouteSelectBubbleInfo::BT_LeftTopOn;
        break;
    }
    case RouteSelectBubbleInfo::BT_LeftBottomOff:
    {
        convertType = RouteSelectBubbleInfo::BT_LeftBottomOn;
        break;
    }
    case RouteSelectBubbleInfo::BT_RightTopOff:
    {
        convertType = RouteSelectBubbleInfo::BT_RightTopOn;
        break;
    }
    case RouteSelectBubbleInfo::BT_RightBottomOff:
    {
        convertType = RouteSelectBubbleInfo::BT_RightBottomOn;
        break;
    }
    default:
        break;
    }

    return convertType;
}

QPixmap *RouteSelectBubbleInternal::GetBackgroundImage(RouteSelectBubbleInfo::RouteSelectBubbleType type)
{
    std::map<RouteSelectBubbleInfo::RouteSelectBubbleType, QPixmap*>::iterator it = mPixmaps.find(type);
    if(it != mPixmaps.end())
    {
        return it->second;
    }

    return NULL;
}

void RouteSelectBubbleInternal::Repaint()
{
    if(mBackgroudPixmap)
    {
        setFixedSize(mBackgroudPixmap->size());
        setMask(mBackgroudPixmap->mask());
    }

    setGeometry(mBubbleInfo.rect);
    update();
}

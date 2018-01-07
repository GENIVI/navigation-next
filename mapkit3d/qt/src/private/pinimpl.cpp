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
 *
 */
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QBuffer>
#include "pinimpl.h"
#include "mapwidgetinternal.h"

using namespace std;
using namespace locationtoolkit;


PinImpl::PinImpl(MapWidgetInternal* mapInternal, nbmap::MapController& mapController, const Coordinates& position,
                 const PinImageInfo& selectedImage, const PinImageInfo& unselectedImage,
                 const RadiusParameters& radiusParameters,
                 QString title, QString subTitle,
                 shared_ptr<BubbleInternal>& bubbleimpl, bool visible, const QString& groupId):
    mPin(NULL),
    mPosition(position),
    mRadiusParameter(radiusParameters),
    mSelectedImageInfo(selectedImage),
    mUnselectedImageInfo(unselectedImage),
    mTitle(title),
    mSubTitle(subTitle),
    mVisible(visible),
    mMapController(mapController),
    mMapInternal(mapInternal),
    mGroupId(groupId)
{
    CreatePin( bubbleimpl );
}

PinImpl::~PinImpl()
{
    mMapController.RemovePin(mPin);
}

/*! Gets the pin's position. */
const Coordinates& PinImpl::GetPosition()
{
    return mPosition;
}

/*! Sets the pin's position. */
void PinImpl::SetPosition(Coordinates& position)
{
    mPosition.latitude = position.latitude;
    mPosition.longitude = position.longitude;
}

/*! Checks weather the pin is visible. */
bool PinImpl::IsVisible()
{
    return mVisible;
}

/*! Sets the visibility. */
void PinImpl::SetVisible(bool visible)
{
    mVisible = visible;
    if( mPin )
    {
        mPin->SetVisible( visible );
    }
}

/*! Selects the pin. */
void PinImpl::SetSelected(bool selected)
{
    mMapController.SelectPin(mPin,selected);
}

void PinImpl::Detach()
{
    mPinId = QString("0");
}

void PinImpl::SetPinId(int id)
{
    mPinId = QString(id);
}

int PinImpl::GetPinId() const
{
    qDebug()<<"mPinId.toInt(): "<<mPinId.toInt();
    return mPinId.toInt();
}

const nbmap::MapPin* PinImpl::GetMapPin() const
{
    return mPin;
}

/*! writes pixmap into bytes in PNG format */
void PinImpl::PixmapToBytes(const QPixmap& pixmap , QByteArray& bytes)
{
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
}

/*! create a pin */
void PinImpl::CreatePin(shared_ptr<BubbleInternal> &bubble)
{
    int8 unselectedCxOffset = (int8)(mUnselectedImageInfo.GetPinAnchor().x());
    int8 unselectedCyOffset = (int8)(mUnselectedImageInfo.GetPinAnchor().y());
    int8 unselectedBxOffset = (int8)(mUnselectedImageInfo.GetBubbleAnchor().x());
    int8 unselectedByOffset = (int8)(mUnselectedImageInfo.GetBubbleAnchor().y());
    int8 selectedCxOffset = (int8)(mSelectedImageInfo.GetPinAnchor().x());
    int8 selectedCyOffset = (int8)(mSelectedImageInfo.GetPinAnchor().y());
    int8 selectedBxOffset = (int8)(mSelectedImageInfo.GetBubbleAnchor().x());
    int8 selectedByOffset = (int8)(mSelectedImageInfo.GetBubbleAnchor().y());

    QByteArray unselectedImageByteArray;
    PixmapToBytes(mUnselectedImageInfo.GetPixmap(), unselectedImageByteArray);
    nbcommon::DataStreamPtr unselectedImage;
    if(!unselectedImageByteArray.isEmpty())
    {
        unselectedImage = nbcommon::DataStream::Create();
        unselectedImage->AppendData((uint8*)unselectedImageByteArray.data(), unselectedImageByteArray.count());
    }

    QByteArray selectedImageByteArray;
    PixmapToBytes(mSelectedImageInfo.GetPixmap(), selectedImageByteArray);
    nbcommon::DataStreamPtr selectedImage;
    if(!selectedImageByteArray.isEmpty())
    {
        selectedImage = nbcommon::DataStream::Create();
        selectedImage->AppendData((uint8*)selectedImageByteArray.data(), selectedImageByteArray.count());
    }

    nbmap::PinType pintype;
    if((!unselectedImageByteArray.isEmpty()) && (!selectedImageByteArray.isEmpty()))
    {
        pintype = nbmap::PT_CUSTOM;
    }
    else
    {
        pintype = nbmap::PT_COMMON;
    }

    float unselectedImageWidth = mUnselectedImageInfo.GetPixmap().width();
    float unselectedImageHeight = mUnselectedImageInfo.GetPixmap().height();
    float selectedImageWidth = mSelectedImageInfo.GetPixmap().width();
    float selectedImageHeight = mSelectedImageInfo.GetPixmap().height();
    uint32 unselectedInteriorColor = mRadiusParameter.GetFillColor();
    uint32 unselectedOutlineColor  = 0;
    uint32 selectedInteriorColor   = mRadiusParameter.GetFillColor();
    uint32 selectedOutlineColor    = 0;
    uint8  circleOutlineWidth      = mRadiusParameter.GetWidth();
    int customLayerPinID = 0;
    float customLayerMargin = 0;
    nbmap::CustomPinInformation* customPinInfo = new nbmap::CustomPinInformation(unselectedCxOffset,
                                                                                         unselectedCyOffset,
                                                                                         unselectedBxOffset,
                                                                                         unselectedByOffset,
                                                                                         selectedCxOffset,
                                                                                         selectedCyOffset,
                                                                                         selectedBxOffset,
                                                                                         selectedByOffset,
                                                                                         unselectedImage,
                                                                                         selectedImage,
                                                                                         unselectedImageWidth,
                                                                                         unselectedImageHeight,
                                                                                         selectedImageWidth,
                                                                                         selectedImageHeight,
                                                                                         unselectedInteriorColor,
                                                                                         unselectedOutlineColor,
                                                                                         selectedInteriorColor,
                                                                                         selectedOutlineColor,
                                                                                         circleOutlineWidth,
                                                                                         customLayerPinID,
                                                                                         customLayerMargin);

    nbmap::CustomPinInformationPtr customPinInformation = nbmap::CustomPinInformationPtr(customPinInfo);
    nbmap::PinParameters<nbmap::BubbleInterface> pinParameters (pintype,
                                                                shared_ptr<nbmap::BubbleInterface>(bubble),
                                                                customPinInformation,
                                                                mPosition.latitude,mPosition.longitude,
                                                                mGroupId.toStdString(),
                                                                mRadiusParameter.GetWidth());
    mPin = mMapController.AddPin( pinParameters );
}

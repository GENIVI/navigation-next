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

    @file pinimpl.h
    @date 08/06/2014
    @defgroup mapkit3d
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
#ifndef __PIN_IMPL_H__
#define __PIN_IMPL_H__

#include <QString>
#include "bubbleinternal.h"
#include "pin.h"
#include "PinLayer.h"
#include "mappin.h"
#include "pinparameters.h"

namespace locationtoolkit
{
class MapWidgetInternal;

class PinImpl : public Pin
{
public:
    PinImpl(MapWidgetInternal* mapInternal, nbmap::MapController& mapController, const Coordinates& position, const PinImageInfo& selectedImage,
              const PinImageInfo& unselectedImage, const RadiusParameters& radiusParameters,
              QString title, QString subTitle, shared_ptr<BubbleInternal>& bubble, bool visible, const QString& groupId);
    ~PinImpl();
public:
    /*! Gets the pin's position. */
    const Coordinates& GetPosition();
    /*! Sets the pin's position. */
    void SetPosition(Coordinates& position);
    /*! Checks weather the pin is visible. */
    bool IsVisible();
    /*! Sets the visibility. */
    void SetVisible(bool visible);
    /*! Selects the pin. */
    void SetSelected(bool selected);
public:
    void Detach();
    void SetPinId(int id);
    int GetPinId() const;
    const nbmap::MapPin* GetMapPin() const;

private:
    /*! create a pin */
    void CreatePin( shared_ptr<BubbleInternal>& bubble );
    /*! writes pixmap into bytes in PNG format */
    void PixmapToBytes(const QPixmap& pixmap , QByteArray& bytes);
private:
    nbmap::MapPin* mPin;

    Coordinates mPosition;
    RadiusParameters mRadiusParameter;
    PinImageInfo mSelectedImageInfo;
    PinImageInfo mUnselectedImageInfo;
    QString mTitle;
    QString mSubTitle;
    bool mVisible;
    QString mPinId;
    QString mGroupId;

    nbmap::MapController& mMapController;
    MapWidgetInternal* mMapInternal;
    QWidget* mBubbleWidget;
};
}
#endif // __PIN_IMPL_H__

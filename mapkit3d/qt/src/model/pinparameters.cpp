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
#include "pinparameters.h"

using namespace locationtoolkit;

/*! PinParameters constructor
 Constructor to use when creating a PinParameters object.
 @param position The geographic position of the pin.
 @param selectedImage The image used when the pin is selected.
 @param unselectedImage The image used when the pin is unselected.
 @param radiusParameters The parameters of the radius.
 @param title The title of the default bubble.
 @param subtitle  The subtitle of the default bubble.
 @param bubble The custom bubble, instead of the default bubble if it is set.
 @param visible The visibility.
 */
PinParameters::PinParameters(Coordinates position, PinImageInfo& selectedImage,
                     PinImageInfo& unselectedImage, RadiusParameters& radiusParameters,
                     QString title, QString subtitle,
                     Bubble* bubble, bool visible, QString groupId)
       :mPosition(position),
        mSelectedImage(selectedImage),
        mUnselectedImage(unselectedImage),
        mRadiusParameters(radiusParameters),
        mVisible(visible),
        mTitle(title),
        mSubtitle(subtitle),
        mBubble(bubble),
        mPinId(0),
        mGroupId(groupId)
{
}

/*! PinParameters constructor
 Default constructor to use when creating a PinParameters object.
 */
PinParameters::PinParameters()
{
    mPinId = 0;
    // TODO Auto-generated constructor stub
    // tilt & subtile & bubble
}

PinParameters::~PinParameters()
{
}

/*! Gets the position.
 @return The position of the pin.
 */
const Coordinates& PinParameters::GetPosition() const
{
    return mPosition;
}

/*! Gets the title.
 @return The title of the default bubble.
 */
const QString& PinParameters::GetTitle() const
{
    return mTitle;
}

/*! Gets the subtitle.
 @return The subtitle of the default bubble.
 */
const QString& PinParameters::GetSubtitle() const
{
    return mSubtitle;
}

/*! Gets the custom bubble. */
Bubble* PinParameters::GetBubble() const
{
    return mBubble;
}

/*! Gets the parameters of the radius. */
const RadiusParameters& PinParameters::GetRadiusParameters() const
{
    return mRadiusParameters;
}

/*! Sets the parameters of the radius.
 @param radiusParameters The RadiusParameters object.
 @return None.
 */
PinParameters& PinParameters::SetRadiusParameters(RadiusParameters& radiusParameters)
{
    mRadiusParameters = radiusParameters;
    return *this;
}

/*! Gets the visibility.
 @deprecated This method has not been supported yet.
 @return True if the pin is to be visible; false if it is not.
 */
bool PinParameters::IsVisible() const
{
    return mVisible;
}

/*! Sets the visibility.
 @deprecated This method has not been supported yet.
 @param visible True to make the pin visible;false to make the pin invisible.
 */
PinParameters& PinParameters::SetVisible(bool visible)
{
    mVisible = visible;
    return *this;
}

/*! Sets the selected image of the pin.
 This image will be displayed when the pin is selected.
 @param selectedImage.
 @return None.
 */
PinParameters& PinParameters::SetSelectedImage(PinImageInfo* selectedImage)
{
    mSelectedImage = *selectedImage;
    return *this;
}

/*! Sets the unselected image of the pin.
 This image will be displayed when the pin is unselected.
 @param unselectedImage.
 @return None.
 */
PinParameters& PinParameters::SetUnselectedImage(PinImageInfo* unselectedImage)
{
    mUnselectedImage = *unselectedImage;
    return *this;
}

/*! Gets the selected image.
 @return See {PinImageInfo}.
 */
const PinImageInfo& PinParameters::GetSelectedImage() const
{
    return mSelectedImage;
}

/*! Gets the unselected image. */
const PinImageInfo& PinParameters::GetUnselectedImage() const
{
    return mUnselectedImage;
}

/*! Sets the position.
 @param position The geographic position of the pin.
 */
PinParameters& PinParameters::SetPosition(Coordinates& position)
{
    mPosition.latitude = position.latitude;
    mPosition.longitude = position.longitude;
    return *this;
}

/*! Sets the title of the default bubble.
 If a custom bubble is set to the pin, this setting will be ignored.
 @param title The title of the default bubble.
 */
PinParameters& PinParameters::SetTitle(const QString& title)
{
    mTitle = title;
    return *this;
}

/*! Sets the subtitle of the default bubble.
 If a custom bubble is set to the pin, this setting will be ignored.
 @param subtitle The subtitle of the default bubble.
 */
PinParameters& PinParameters::SetSubtitle(const QString& subtitle)
{
    mSubtitle = subtitle;
    return *this;
}

/*! Sets a custom bubble attached to this pin instead of the default bubble.
 If the custom bubble is not set, the default bubble will be used.
 @param bubble The custom bubble object.
 */
PinParameters& PinParameters::SetBubble(Bubble* bubble)
{
    mBubble = bubble;
    return *this;
}

/*! set a unique identifier by user */
PinParameters& PinParameters::SetPinId(int id)
{
    mPinId = id;
    return *this;
}

int PinParameters::GetPinId() const
{
    return mPinId;
}

QString PinParameters::GetGroupId() const
{
    return mGroupId;
}

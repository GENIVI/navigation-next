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
    @file pinparameters.h
    @date 08/22/2014
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
#ifndef __PIN_PARAMETERS_H__
#define __PIN_PARAMETERS_H__

#include <QString>
#include <QPixmap>
#include <QPoint>
#include "bubble.h"
#include "float.h"
#include "radiusparameters.h"
#include "coordinate.h"

namespace locationtoolkit
{
/*! PinImageInfo defines information about images which will be used for pins. */
class PinImageInfo
{
public:
    PinImageInfo()
    {
        OffsetX = 50;
        OffsetY = 50;
        mPinAnchor.setX(OffsetX);
        mPinAnchor.setY(OffsetY);
        mBubbleAnchor.setX(OffsetX);
        mBubbleAnchor.setY(OffsetY);
    }

    ~PinImageInfo()
    {
    }

   /*! Gets the pixmap
    @return The pixmap.
    */
    const QPixmap& GetPixmap() const
    {
        return mPixmap;
    }

   /*! Gets the anchor point for the pin.
    @return The anchor point of the pin in the image, {offsetX, offsetY}.
    */
    const QPoint& GetPinAnchor() const
    {
        return mPinAnchor;
    }

   /*! Gets the anchor point for the bubble.
    @return The anchor point of the bubble in the image, {offsetX, offsetY}.
    */
    const QPoint& GetBubbleAnchor() const
    {
        return mBubbleAnchor;
    }

   /*! Sets the pixmap.
    Please note that the pixmap passed in won't be recycled inside.
    The recycle process should be done by users after the pin is added into the map if necessary.
    @param pixmap The pixmap object.
    */
    void SetPixmap(const QPixmap& pixmap)
    {
        mPixmap = pixmap;
    }

   /*! Specify the anchor point for the pin in the image.
    The top-left point of the image is the original point (0, 0).
    @param x Described in the percent of the image width.Range[0, 100].
    @param y Described in the percent of the image height.Range[0, 100].
    */
    void SetPinAnchor(int percentX, int percentY)
    {
        if(percentX >100)
        {
            mPinAnchor.setX(100);
        }
        else if(percentX < 0)
        {
            mPinAnchor.setX(0);
        }
        else
        {
            mPinAnchor.setX(percentX);
        }
        if(percentY >100)
        {
            mPinAnchor.setY(100);
        }
        else if(percentY < 0)
        {
            mPinAnchor.setY(0);
        }
        else
        {
            mPinAnchor.setY(percentY);
        }
    }

    /*! Specify the anchor point for the bubble in the image.
     The top-left point of the image is the original point (0, 0).
     @param x Described in the percent of the image width.Range[0, 100].
     @param y Described in the percent of the image height.Range[0, 100].
     */
    void SetBubbleAnchor(int percentX, int percentY)
    {
        if(percentX >100)
        {
            mBubbleAnchor.setX(100);
        }
        else if(percentX < 0)
        {
            mBubbleAnchor.setX(0);
        }
        else
        {
            mBubbleAnchor.setX(percentX);
        }
        if(percentY >100)
        {
            mBubbleAnchor.setY(100);
        }
        else if(percentY < 0)
        {
            mBubbleAnchor.setY(0);
        }
        else
        {
            mBubbleAnchor.setY(percentY);
        }
    }
private:
    QPixmap mPixmap;
    QPoint mPinAnchor;
    QPoint mBubbleAnchor;
    float OffsetX;
    float OffsetY;
};

class PinParameters
{
public:
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
    PinParameters(Coordinates position,PinImageInfo& selectedImage,
                  PinImageInfo& unselectedImage,RadiusParameters& radiusParameters,
                  QString title, QString subtitle, Bubble* bubble, bool visible, QString groupId = "DefaultGroup");

    /*! PinParameters constructor
     Default constructor to use when creating a PinParameters object.
     */
    PinParameters();
    ~PinParameters();
    /*! Gets the position.
     @return The position of the pin.
     */
    const Coordinates& GetPosition() const;

    /*! Gets the title.
     @return The title of the default bubble.
     */
    const QString& GetTitle() const;

    /*! Gets the subtitle.
     @return The subtitle of the default bubble.
     */
    const QString& GetSubtitle() const;

    /*! Gets the custom bubble.
     @return See {@link Bubble}.
     */
    Bubble* GetBubble() const;

    /*! Gets the parameters of the radius.
     @return See {@link RadiusParameters}.
     */
    const RadiusParameters& GetRadiusParameters() const;

    /*! Sets the parameters of the radius.
     @param radiusParameters The RadiusParameters object. See {@RadiusParameters}.
     @return None.
     */
    PinParameters& SetRadiusParameters(RadiusParameters& radiusParameters);

    /*! Gets the visibility.
     @deprecated This method has not been supported yet.
     @return True if the pin is to be visible; false if it is not.
     */
    bool IsVisible() const;

    /*! Sets the visibility.
     @deprecated This method has not been supported yet.
     @param visible True to make the pin visible;false to make the pin invisible.
     */
    PinParameters& SetVisible(bool visible);

    /*! Sets the selected image of the pin.
     This image will be displayed when the pin is selected.
     @param selectedImage.
     @return this.
     */
    PinParameters& SetSelectedImage(PinImageInfo* selectedImage);

    /*! Sets the unselected image of the pin.
     This image will be displayed when the pin is unselected.
     @param unselectedImage.
     @return this.
     */
    PinParameters& SetUnselectedImage(PinImageInfo* unselectedImage);

    /*! Gets the selected image. */
    const PinImageInfo& GetSelectedImage() const;

    /*! Gets the unselected image. */
    const PinImageInfo& GetUnselectedImage() const;

    /*! Sets the position.
     @param position The geographic position of the pin.
     */
    PinParameters& SetPosition(Coordinates& position);

    /*! Sets the title of the default bubble.
     If a custom bubble is set to the pin, this setting will be ignored.
     @param title The title of the default bubble.
     */
    PinParameters& SetTitle(const QString& title);

    /*! Sets the subtitle of the default bubble.
     If a custom bubble is set to the pin, this setting will be ignored.
     @param subtitle The subtitle of the default bubble.
     */
    PinParameters& SetSubtitle(const QString& subtitle);

    /*! Sets a custom bubble attached to this pin instead of the default bubble.
     If the custom bubble is not set, the default bubble will be used.
     @param bubble The custom bubble object.
     */
    PinParameters& SetBubble(Bubble* bubble);

    /*! set a unique identifier by user */
    PinParameters& SetPinId(int id);
    int GetPinId() const;
    QString GetGroupId() const;

private:
    Coordinates mPosition;
    PinImageInfo mSelectedImage;
    PinImageInfo mUnselectedImage;
    RadiusParameters mRadiusParameters;
    bool mVisible;
    QString mTitle;
    QString mSubtitle;
    Bubble* mBubble;
    int mPinId;
    QString mGroupId;
};
}
#endif // __PIN_PARAMETERS_H__

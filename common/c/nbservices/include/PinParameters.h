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
@file     PinParameters.h
@defgroup nbmap

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems, is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __PINPARAMETERS__
#define __PINPARAMETERS__

#include "paltypes.h"
#include "nbexp.h"
#include "navpublictypes.h"
#include "smartpointer.h"
#include "base.h"
#include "datastream.h"
#include "TileKey.h"
#include "Layer.h"
#include <vector>
#include <string>

namespace nbmap
{

/*! Pin type */
typedef enum
{
    PT_INVALID,         /*!< Invalid type */
    PT_COMMON,          /*!< Common type */
    PT_START_FLAG,      /*!< Start flag */
    PT_END_FLAG,        /*!< End flag */
    PT_SEVERE_INCIDENT, /*!< Severe incident */
    PT_MAJOR_INCIDENT,  /*!< Major incident */
    PT_MINOR_INCIDENT,  /*!< Minor incident */
    PT_CUSTOM           /*!< Custom type */
    // @todo: Add other types of pin here.

} PinType;

/*! Custom information to create a pin of type PT_CUSTOM */
class CustomPinInformation : public Base
{
public:
    CustomPinInformation(){}
    /*! CustomPinInformation constructor */
    CustomPinInformation(int8 unselectedCxOffset,
                         int8 unselectedCyOffset,
                         int8 unselectedBxOffset,
                         int8 unselectedByOffset,
                         int8 selectedCxOffset,
                         int8 selectedCyOffset,
                         int8 selectedBxOffset,
                         int8 selectedByOffset,
                         nbcommon::DataStreamPtr unselectedImage,
                         nbcommon::DataStreamPtr selectedImage,
                         float unselectedImageWidth,
                         float unselectedImageHeight,
                         float selectedImageWidth,
                         float selectedImageHeight,
                         uint32 unselectedInteriorColor = 0x6721D826,
                         uint32 unselectedOutlineColor  = 0,
                         uint32 selectedInteriorColor   = 0x6721D826,
                         uint32 selectedOutlineColor    = 0,
                         uint8  circleOutlineWidth      = 0,
                         int customLayerPinID = 0,
                         float customLayerMargin = 0)
            : m_unselectedCalloutOffsetX(unselectedCxOffset),
              m_unselectedCalloutOffsetY(unselectedCyOffset),
              m_unselectedBubbleOffsetX(unselectedBxOffset),
              m_unselectedBubbleOffsetY(unselectedByOffset),

              m_unselectedImageWidth(unselectedImageWidth),
              m_unselectedImageHeight(unselectedImageHeight),
              m_selectedImageWidth(selectedImageWidth),
              m_selectedImageHeight(selectedImageHeight),

              m_selectedCalloutOffsetX(selectedCxOffset),
              m_selectedCalloutOffsetY(selectedCyOffset),
              m_selectedBubbleOffsetX(selectedBxOffset),
              m_selectedBubbleOffsetY(selectedByOffset),

              m_selectedImage(selectedImage),
              m_unselectedImage(unselectedImage),

              m_unselectedCircleInteriorColor(unselectedInteriorColor),
              m_unselectedCircleOutLineColor(unselectedOutlineColor),

              m_selectedCircleInteriorColor(selectedInteriorColor),
              m_selectedCircleOutLineColor(selectedOutlineColor),

              m_circleOutlineWidth(circleOutlineWidth),

              m_customLayerPinID(customLayerPinID),
              m_customLayerMargin(customLayerMargin)
    {
    }

    /*! CustomPinInformation constructor */
    CustomPinInformation(int8 calloutOffsetX,
                         int8 calloutOffsetY,
                         int8 bubbleOffsetX,
                         int8 bubbleOffsetY,
                         nbcommon::DataStreamPtr selectedImage,
                         nbcommon::DataStreamPtr unselectedImage,
                         float unselectedImageWidth,
                         float unselectedImageHeight,
                         float selectedImageWidth,
                         float selectedImageHeight,
                         uint32 circleInteriorColor = 0x6721D826, // Default value Provided by UX team.
                         uint32 circleOutLineColor = 0,
                         uint8  circleOutlineWidth = 0,
                         int customLayerPinID = 0,
                         float customLayerMargin = 0)
            : m_unselectedCalloutOffsetX(calloutOffsetX),
              m_unselectedCalloutOffsetY(calloutOffsetY),
              m_unselectedBubbleOffsetX (bubbleOffsetX),
              m_unselectedBubbleOffsetY (bubbleOffsetY),

              m_unselectedImageWidth(unselectedImageWidth),
              m_unselectedImageHeight(unselectedImageHeight),
              m_selectedImageWidth(selectedImageWidth),
              m_selectedImageHeight(selectedImageHeight),

              m_selectedCalloutOffsetX(calloutOffsetX),
              m_selectedCalloutOffsetY(calloutOffsetY),
              m_selectedBubbleOffsetX(bubbleOffsetX),
              m_selectedBubbleOffsetY(bubbleOffsetY),

              m_selectedImage(selectedImage),
              m_unselectedImage(unselectedImage),

              m_unselectedCircleInteriorColor(circleInteriorColor),
              m_unselectedCircleOutLineColor(circleOutLineColor),

              m_selectedCircleInteriorColor(circleInteriorColor),
              m_selectedCircleOutLineColor(circleOutLineColor),

              m_circleOutlineWidth(circleOutlineWidth),

              m_customLayerPinID(customLayerPinID),
              m_customLayerMargin(customLayerMargin)
    {
    }

    /*! CustomPinInformation destructor */
    virtual ~CustomPinInformation()
    {
        // Nothing to do here.
    }

    // Copy constructor and assignment operator are not supported.
    CustomPinInformation(const CustomPinInformation& information);
    CustomPinInformation& operator=(const CustomPinInformation& information);

    // Offsets for unselected pin.
    int8 m_unselectedCalloutOffsetX; /*!< Offset x of the callout. The value is from
                                           0 to 100. It is the percent of the image
                                           width. It is used to specify the pin point
                                           in the image. The top left point of the
                                           callout is the orginal point (0, 0). */
    int8 m_unselectedCalloutOffsetY;  /*!< Offset y of the callout. The value is from
                                           0 to 100. It is the percent of the image
                                           height. */
    int8 m_unselectedBubbleOffsetX;   /*!< Offset x of the bubble. The value is 0 to
                                           100, it is a percent of the image width.
                                           The bubble position is an offset related to
                                           the pin point specified by the above two
                                           offsets. */
    int8 m_unselectedBubbleOffsetY;   /*!< Offset y of the bubble. The value is 0 to
                                           100, it is a percent of the image height.
                                           */

    float m_unselectedImageWidth;   // unselected pin width in DPI
    float m_unselectedImageHeight;  // unselected pin height in DPI
    float m_selectedImageWidth;   // selected pin width in DPI
    float m_selectedImageHeight;  // selected pin height in DPI

    // offsets for selected pin.
    int8 m_selectedCalloutOffsetX;
    int8 m_selectedCalloutOffsetY;
    int8 m_selectedBubbleOffsetX;
    int8 m_selectedBubbleOffsetY;

    nbcommon::DataStreamPtr m_selectedImage;    /*!< Custom selected image */
    nbcommon::DataStreamPtr m_unselectedImage;  /*!< Custom unselected image */

    // Color for unselected pin.
    uint32 m_unselectedCircleInteriorColor; /*!< Circle interior color, in RGBA order. */
    uint32 m_unselectedCircleOutLineColor;  /*!< Circle outline color, in RGBA order.  */

    // Color for selected pin.
    uint32 m_selectedCircleInteriorColor;
    uint32 m_selectedCircleOutLineColor;

    uint8  m_circleOutlineWidth;       /*!< Width of circle outline, unit: dp. Value
                                            not change for both selected and
                                            unselected pin. */
    int m_customLayerPinID;
    float m_customLayerMargin;
};

typedef shared_ptr<CustomPinInformation> CustomPinInformationPtr;

/*! Parameters to create a pin */
template <class T>
class PinParameters : public Base
{
public:
    /*! PinParameters constructor */
    PinParameters(PinType type,                                 /*!< Identified the type of pin */
                  shared_ptr<T> bubble,                         /*!< Pin bubble */
                  CustomPinInformationPtr customPinInformation, /*!< Information to create a custom pin. It is
                                                                     used if the pin type is PT_CUSTOM. */
                  double latitude,                              /*!< Latitude */
                  double longitude,                             /*!< Longitude */
                  std::string groupId = "DefaultGroup",
                  uint16 circleRadius = 0);

    /*! PinParameters destructor */
    virtual ~PinParameters();

    /*! PinParameters copy constructor */
    PinParameters(const PinParameters& parameters);

    /*! PinParameters assignment operator */
    PinParameters&
    operator=(const PinParameters& parameters);

    PinType m_type;                                 /*!< Identified the type of pin */
    shared_ptr<std::string> m_pinId;                /*!< An ID associated with the pin. It is always
                                                         used to return the pin ID if this pin is
                                                         added successfully. */
    shared_ptr<std::string> m_groupId;              /*!< Pin Group ID*/
    shared_ptr<T> m_bubble;                         /*!< Pin bubble */
    CustomPinInformationPtr m_customPinInformation; /*!< Information to create a custom pin. It is
                                                         used if the pin type is PT_CUSTOM. */
    double             m_latitude;      /*!< Latitude */
    double             m_longitude;     /*!< Longitude */
    uint16             m_circleRadius;  /*!< Radius of circle, unit: meter. */
};
}

#endif

/*! @} */

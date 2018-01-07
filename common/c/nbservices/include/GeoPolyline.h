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
 @file     geopolyline.h
 */
/*
 (C) Copyright 2013 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef GEOPOLYLINE_H_
#define GEOPOLYLINE_H_

#include "paltypes.h"
#include "GeoPoint.h"
#include "Geographic.h"
#include <vector>
#include <string>
#include "smartpointer.h"
#include "datastream.h"

namespace nbmap
{

/*! Type of GeoPolylineAttribute.
  define which region has what color
*/
class GeoPolylineAttribute
{
public:
    GeoPolylineAttribute(uint32 color, uint32 endIndex)
            : m_color(color),
              m_endIndex(endIndex){}
    virtual ~GeoPolylineAttribute(){}

    uint32 m_color;    /**< Color of Polyline region, in RGBA order. */
    uint32 m_endIndex; /**< the cover region of color. */
};

typedef enum
{
    PT_SOLID_LINE = 0,
    PT_DASH_LINE

} PolylineType;

class GeoPolylineAttributeEx
{
public:
    GeoPolylineAttributeEx(uint32 endIndex, PolylineType type)
              :m_type(type), m_endIndex(endIndex){}
    virtual ~GeoPolylineAttributeEx(){}

    /*! Less comaprison used for std::vector */
    static bool sort(const GeoPolylineAttributeEx* leftValue, const GeoPolylineAttributeEx* rightValue)
    {
        return (leftValue->m_endIndex < rightValue->m_endIndex);
    }
    PolylineType m_type;
    uint32 m_endIndex; /**< the cover region of color. */
};

class GeoPolylineColorAttribute : public GeoPolylineAttributeEx
{
public:
    GeoPolylineColorAttribute(uint32 color, uint32 endIndex)
            : GeoPolylineAttributeEx(endIndex, PT_SOLID_LINE),
              m_color(color){}
    virtual ~GeoPolylineColorAttribute(){}

    uint32 m_color;    /**< Color of Polyline region, in RGBA order. */
};

class GeoPolylinePatternAttribute : public GeoPolylineAttributeEx
{
public:
    GeoPolylinePatternAttribute(uint32 endIndex, uint32 distance, string textureName, nbcommon::DataStreamPtr textureData, int textureWidth, int textureHeight)
            : GeoPolylineAttributeEx(endIndex, PT_DASH_LINE),
              m_distance(distance),
              m_textureName(textureName),
              m_textureData(textureData),
              m_textureWidth(textureWidth),
              m_textureHeight(textureHeight),
              m_color(100){}
    virtual ~GeoPolylinePatternAttribute(){}

    uint32 m_distance;
    string m_textureName;
    nbcommon::DataStreamPtr m_textureData;
    int m_textureWidth;
    int m_textureHeight;
    uint32 m_color;
};

/*! Type of GeoPolyline.
  Polyline graphic class
*/
class GeoPolyline : public Geographic
{
public:
    virtual ~GeoPolyline() {}

    /**
     * @name SetColors - Set polyline color attribute.
     * @param attrs - define which region has what color
     */
    virtual void SetColors(shared_ptr<const vector<GeoPolylineAttributeEx*> > attrs)=0;

    /**
     * @name SetWidth - Set polyline width.
     * @param width - unit is pixel
     */
    virtual void SetWidth(uint32 width) = 0;

    // Refer to Geographic for description.
    virtual void SetSelected(bool isSelected)=0;

    // Some default values used by Polyline.
    static const uint32 POLYLINE_UNHIGHLIGHTED_COLOR = 0x838b8b00  ; // in RGBA
    static const uint32 POLYLINE_DEFAULT_WIDTH       = 10;

protected:
    GeoPolyline(const std::string& id);
};

/*! Type of polyline cap. */
typedef enum _PolylineCapType
{
    PCT_Arrow ,
    PCT_Round,
    PCT_Join,
    PCT_Invalid,
} PolylineCapType;

/*! Cap of polyline. */
class GeoPolylineCap
{
public:

    virtual ~GeoPolylineCap(){}

    /*! Get Type of this PolylineCap.*/
    PolylineCapType GetCapType();

    /*! Creates an ArrowCap for polyline.

      @return shared pointer of crated ArrowCap.
    */
    static shared_ptr<GeoPolylineCap>
    CreateArrowCap(uint32 shapeWidth,  /*!< Width of arrow (in pixels)  */
                   uint32 shapeHeight  /*!< Height of arrow (in pixels) */
                   );

    /*! Creates an RoundCap for polyline.

      @return shared pointer of created RoundCap.
    */
    static shared_ptr<GeoPolylineCap>
    CreateRoundCap(uint32 radius /*!< radius of cirle (in pixels) */
                   );

protected:
    GeoPolylineCap(PolylineCapType type);

private:
    PolylineCapType m_type;
};

typedef shared_ptr<GeoPolylineCap> GeoPolylineCapPtr;

/*!  Option used when creating polyline. */
class GeoPolylineOption
{
public:
    GeoPolylineOption(shared_ptr<const vector<GeoPoint> > points,
                      shared_ptr<const vector<GeoPolylineAttributeEx*> > attrs,
                      uint32 width,
                      uint32 unhighlightedColor,
                      int32 zOrder,
                      uint32 outlineColor,
                      uint8 outlineWidth,
                      GeoPolylineCapPtr startCap = GeoPolylineCapPtr(),
                      GeoPolylineCapPtr endCap = GeoPolylineCapPtr()
                      )
            : m_points(points),
              m_attrs(attrs),
              m_startCap(startCap),
              m_endCap(endCap),
              m_width(width),
              m_unhighlightedColor(unhighlightedColor),
              m_zOrder(zOrder),
              m_outlineColor(outlineColor),
              m_outlineWidth(outlineWidth){}

    GeoPolylineOption(const GeoPolylineOption& other);
    GeoPolylineOption()
            : m_width(GeoPolyline::POLYLINE_DEFAULT_WIDTH),
              m_unhighlightedColor(GeoPolyline::POLYLINE_UNHIGHLIGHTED_COLOR) {}

    virtual ~GeoPolylineOption(){}

    shared_ptr<const vector<GeoPoint> > m_points; /*!< Points of this line. */
    shared_ptr<const vector<GeoPolylineAttributeEx*> > m_attrs; /*!< Color attribute of line. */
    GeoPolylineCapPtr m_startCap;
    GeoPolylineCapPtr m_endCap;
    uint32 m_width;                     /*!< Width of this line in pixels. */
    uint32 m_unhighlightedColor;        /*!< unhighlighted Color of this line. */
    int32  m_zOrder;                    /*!< unhighlighted Color of this line. */
    uint32 m_outlineColor;              /*!<Outline color, in r, g, b, a order */
    uint8 m_outlineWidth;               /*!<Outline width, as %a of the road width. */
};

class GeoPolylineOptionOperation
{
public:
    GeoPolylineOptionOperation(const GeoPolylineOption& option):m_option(option){}
    virtual ~GeoPolylineOptionOperation(){};

    //Maybe the polyline contain the multi-style(solid and dash line).
    //The NBM library only handle one style polyline in one time, if the polyline style mixed,
    //we have to cut them.
    virtual bool isNeedCutPolyline(shared_ptr<vector<GeoPolylineOption> >& subPolylineOptions);
    GeoPolylineOption m_option;
};

}

#endif /* GEOPOLYLINE_H_ */

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
    @file polylineparameters.h
    @date 09/12/2014
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

#ifndef __POLYLINE_PARAMETERS_H__
#define __POLYLINE_PARAMETERS_H__

#include <QList>
#include <QVector>
#include <QColor>
#include "segmentattribute.h"

namespace locationtoolkit
{
class Coordinates;

enum CapParameterType
{
    CPT_Nothing,
    CPT_Round,
    CPT_Arrow
};

struct CapParameter
{
    /*! mType:: 0:Nothing, 1:Round, 2:Arrow */
    CapParameterType type;
    int radius;
    int width;
    int length;
};

class PolylineParameters
{
public:
    typedef enum
    {
        SAT_Invalid = 0,
        SAT_ColorSegment,
        SAT_PatternSegment
    }SegmentAttributeType;

public:
    /*! PolylineParameters constructor
     Default Constructor to use when creating a PolylineParameters object.
     */
    PolylineParameters();
    ~PolylineParameters();

    /*! Adds a vertex to the end of the polyline being built.
     @param pos The position of the point.
     */
    PolylineParameters& AddPoints(const Coordinates& pos);

    /*! Sets the list of vertices of the polyline being built.
     @param points The list of points.
     */
    PolylineParameters& SetPoints(const QList<Coordinates>& points);

    /*! Sets the width.
     @param width The width of the polyline in screen pixels.
     */
    PolylineParameters& SetWidth(float width);

    /*! Sets the list of segment attributes of the polyline being built.
     This is a convenient method for displaying traffic on the route.
     @param colors The list of SegmentAttribute.
     */
    PolylineParameters& SetSegmentAttributes(const QList<SegmentAttribute*>& segmentAttributes);

    /*! Sets the unhighlighted color.
     @param color The unhighlighted color.
     */
    PolylineParameters& SetUnhighlightColor(QColor color);

    /*! Sets the z-order of the polyline.
     The zOrder value will affect the draw orders between polylines.
     @param zOrder The zOrder value.
     */
    PolylineParameters& SetZOrder(int zOrder);

    /*! Sets the visibility.
     @param visible True to make this polyline visible.
     */
    PolylineParameters& SetVisible(bool visible);

    /*! Sets the attribute of the start cap.
     @param cap
     */
    PolylineParameters& SetStartCap(const CapParameter& cap);

    /*! Sets the attribute of the end cap.
     @param cap
     */
    PolylineParameters& SetEndCap(const CapParameter& cap);

    /*! Gets the width.
     @return The width of the polyline in screen pixels.
     */
    float GetWidth() const;

    /*! Gets the unhighlighted color.
     @return The unhighlighted color
     */
    QColor GetUnhighlightColor() const;

    /*! Gets the z-order value.
     @return The zOrder value.
     */
    int GetZOrder() const;

    /*! Gets the visibility setting for this Parameters object.
     @return True if the polyline is to be visible, false otherwise.
     */
    bool IsVisible() const;

    /*! Gets a copy list of vertices.
     @return The list of vertices.
     */
    QList<Coordinates>& GetPoints() const;

    /*! Gets a list of segment attributes for this polyline.
     @return The list of SegmentAttribute.
     */
    const QList<SegmentAttribute*>& GetSegmentAttributes() const;

    /*! Gets the attribute of the start cap.
     @return See CapParameter
     */
    const CapParameter& GetStartCap() const;

    /*! Gets the attribute of the end cap.
     @return See {@link CapParameter}.
     */
    const CapParameter& GetEndCap() const;

    /*! Gets the outline color.
     @return The outline color of the polyline in RGBA format.
     */
    QColor GetOutlineColor() const;

    /*! Sets the outline color.
     @param outlineColor The outline color of the polyline in RGBA format.
     @return None.
     */
    PolylineParameters& setOutlineColor(QColor outlineColor);

    /*! Gets the outline width.
     @return The outline width in screen pixels.
     */
    int getOutlineWidth() const;

    /*! Sets the outline width.
     @param outlineWidth The outline width in screen pixels.
     @return None.
     */
    PolylineParameters& setOutlineWidth(int outlineWidth);

    PolylineParameters& SetAttributeType(SegmentAttributeType type);
    SegmentAttributeType GetAttributeType() const;

private:
    QList<Coordinates>* mPoints;
    QList<SegmentAttribute*> mSegmentAttributes;
    QColor mDefaultColor;// default grey color
    float mWidth;
    int mzOrder;
    bool mVisible;
    CapParameter mStartCap;
    CapParameter mEndCap;
    QColor mOutlineColor;   //Outline color, in r, g, b, a order.
    int mOutlineWidth;  //Outline width, as %a of the road width.
    SegmentAttributeType mSegmentAttributeType;
};
}
#endif // __POLYLINE_PARAMETERS_H__

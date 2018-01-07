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

#include "polylineparameters.h"
#include "coordinate.h"

using namespace locationtoolkit;

/*! PolylineParameters constructor
    Default Constructor to use when creating a PolylineParameters object.
 */
PolylineParameters::PolylineParameters()
    : mWidth(0),
      mzOrder(0),
      mVisible(true),
      mStartCap({CPT_Nothing,0,0,0}),
      mEndCap({CPT_Nothing,0,0,0}),
      mOutlineWidth(0)
{
    mPoints = new QList<Coordinates>();
}

PolylineParameters::~PolylineParameters()
{

}


/*! Adds a vertex to the end of the polyline being built.
 @param pos The position of the point.
 */
PolylineParameters& PolylineParameters::AddPoints(const Coordinates& pos)
{
    mPoints->append(pos);
    return *this;
}

/*! Sets the list of vertices of the polyline being built.
 @param points The list of points.
 */
PolylineParameters& PolylineParameters::SetPoints(const QList<Coordinates>& points)
{
    for(int i = 0; i < points.count(); ++i)
    {
        mPoints->append(points.at(i));
    }
    return *this;
}

/*! Sets the width.
 @param width The width of the polyline in screen pixels.
 */
PolylineParameters& PolylineParameters::SetWidth(float width)
{
    mWidth = width;
    return *this;
}

/*! Sets the list of segment attributes of the polyline being built.
 This is a convenient method for displaying traffic on the route.
 @param colors The list of SegmentAttribute.
 */
PolylineParameters& PolylineParameters::SetSegmentAttributes(const QList<SegmentAttribute*>& segmentAttributes)
{
    mSegmentAttributes.clear();
    for(int i = 0; i < segmentAttributes.count(); ++i)
    {
        mSegmentAttributes.append(segmentAttributes.at(i));
    }
    return *this;
}

/*! Sets the unhighlighted color.
 @param color The unhighlighted color.
 */
PolylineParameters& PolylineParameters::SetUnhighlightColor(QColor color)
{
    mDefaultColor = color;
    return *this;
}

/*! Sets the z-order of the polyline.
 The zOrder value will affect the draw orders between polylines.
 @param zOrder The zOrder value.
 */
PolylineParameters& PolylineParameters::SetZOrder(int zOrder)
{
    mzOrder = zOrder;
    return *this;
}

/*! Sets the visibility.
 @param visible True to make this polyline visible.
 */
PolylineParameters& PolylineParameters::SetVisible(bool visible)
{
    mVisible = visible;
    return *this;
}

/*! Sets the attribute of the start cap.
 @param cap
 */
PolylineParameters& PolylineParameters::SetStartCap(const CapParameter& cap)
{
    mStartCap.type = cap.type;
    mStartCap.radius = cap.radius;
    mStartCap.length = cap.length;
    mStartCap.width = cap.width;
    return *this;
}

/*! Sets the attribute of the end cap.
 @param cap
 */
PolylineParameters& PolylineParameters::SetEndCap(const CapParameter& cap)
{
    mEndCap.type = cap.type;
    mEndCap.radius = cap.radius;
    mEndCap.length = cap.length;
    mEndCap.width = cap.width;
    return *this;
}

/*! Gets the width.
 @return The width of the polyline in screen pixels.
 */
float PolylineParameters::GetWidth() const
{
    return mWidth;

}

/*! Gets the unhighlighted color.
 @return The unhighlighted color
 */
QColor PolylineParameters::GetUnhighlightColor() const
{
    return mDefaultColor;
}

/*! Gets the z-order value.
 @return The zOrder value.
 */
int PolylineParameters::GetZOrder() const
{
    return mzOrder;
}

/*! Gets the visibility setting for this Parameters object.
 @return True if the polyline is to be visible, false otherwise.
 */
bool PolylineParameters::IsVisible() const
{
    return mVisible;
}

/*! Gets a copy list of vertices.
 @return The list of vertices.
 */
QList<Coordinates>& PolylineParameters::GetPoints() const
{
    return *mPoints;
}

/*! Gets a list of segment attributes for this polyline.
 @return The list of SegmentAttribute.
 */
const QList<SegmentAttribute*>& PolylineParameters::GetSegmentAttributes() const
{
    return mSegmentAttributes;
}

/*! Gets the attribute of the start cap.
 @return See CapParameter
 */
const CapParameter& PolylineParameters::GetStartCap() const
{
    return mStartCap;
}

/*! Gets the attribute of the end cap.
 @return See {@link CapParameter}.
 */
const CapParameter& PolylineParameters::GetEndCap() const
{
    return mEndCap;
}

/*! Gets the outline color.
 @return The outline color of the polyline in RGBA format.
 */
QColor PolylineParameters::GetOutlineColor() const
{
    return mOutlineColor;
}

/*! Sets the outline color.
 @param outlineColor The outline color of the polyline in RGBA format.
 @return None.
 */
PolylineParameters& PolylineParameters::setOutlineColor(QColor outlineColor)
{
    mOutlineColor = outlineColor;
    return *this;
}

/*! Gets the outline width.
 @return The outline width in screen pixels.
 */
int PolylineParameters::getOutlineWidth() const
{
    return mOutlineWidth;
}

/*! Sets the outline width.
 @param outlineWidth The outline width in screen pixels.
 @return None.
 */
PolylineParameters& PolylineParameters::setOutlineWidth(int outlineWidth)
{
    mOutlineWidth = outlineWidth;
    return *this;
}

PolylineParameters &PolylineParameters::SetAttributeType(SegmentAttributeType type)
{
    mSegmentAttributeType = type;
    return *this;
}

PolylineParameters::SegmentAttributeType PolylineParameters::GetAttributeType() const
{
    return mSegmentAttributeType;
}

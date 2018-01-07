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
#include "polylineimpl.h"
#include "GeoPolyline.h"

using namespace locationtoolkit;
PolylineImpl::PolylineImpl(const PolylineParameters& para, nbmap::GeoPolyline* geopolyline, nbmap::MapController* mapControl)
    : mWidth(para.GetWidth()),
      mColor(para.GetUnhighlightColor()),
      mVisible(para.IsVisible()),
      mSelected(false),
      mZOrder(para.GetZOrder()),
      mGeoPolyline(geopolyline),
      mMapControl(mapControl)
{
    mID = QString::fromStdString( mGeoPolyline->GetId() );
}

PolylineImpl::~PolylineImpl()
{
}

/*! Gets the width.
@return The width of the polyline in screen pixels.
*/
float PolylineImpl::GetWidth()
{
    return mWidth;
}

/*! Sets the width.
@param width The width of the polyline in screen pixels.
@return this.
*/
Polyline& PolylineImpl::SetWidth(float width)
{
    mWidth = width;
    if(mGeoPolyline)
    {
        mGeoPolyline->SetWidth(width);
    }
    return *this;
}

/*! Gets the color.
@return The color in RGBA format.
*/
QColor& PolylineImpl::GetColor()
{
    return mColor;
}

/*! Sets the color.
@param color The color in RGBA format.
@return this.
*/
Polyline& PolylineImpl::SetColor(QColor color)
{
    mColor = color;
    if(mGeoPolyline)
    {
       // mGeoPolyline->SetColors();
    }
    mColors.append(ColorSegment(-1, mColor));
    return *this;
}

/*! Checks whether the polyline is visible.
@return True if the polyline is visible, false otherwise.
*/
bool PolylineImpl::IsVisible()
{
    return mVisible;
}

/*! Sets the visibility.
@param visible True to make this polyline visible; false to make it invisible.
@return None.
*/
void PolylineImpl::SetVisible(bool visible)
{
    mVisible = visible;
    if(mGeoPolyline)
    {
        mMapControl->SetPolylineVisible(mGeoPolyline,visible);
    }
}

/*! Sets the list of colored segments to the polyline.
@param colors
*/
Polyline& PolylineImpl::SetColors(const QList<ColorSegment>& colors)
{
    mColors = colors;
    return *this;
}

/*! Gets the list of colored segments from the polyline.
@return See {@link ColorSegment}.
*/
QList<ColorSegment>& PolylineImpl::GetColors()
{
    return mColors;
}

/*! Checks whether the polyline is selected.
@return True if the polyline is selected, false otherwise.
*/
bool PolylineImpl::IsSelected()
{
    return mSelected;
}

/*! Selects the polyline.
@param selected True to make this polyline selected;false to make it unselected.
@return None.
*/
void PolylineImpl::SetSelected(bool selected)
{
    mSelected = selected;
    if(mGeoPolyline)
    {
        mMapControl->SetPolylineSelected(mGeoPolyline, selected);
    }
}

/*! Gets the zOrder value.
@return the zOrder value.
*/
int PolylineImpl::GetZOrder()
{
    return mZOrder;
}

/*! Sets the zOrder value.
The zOrder value will affect the draw orders between polylines.
@param zOrder The zOrder value.
@return None.
*/
void PolylineImpl::SetZOrder(int zOrder)
{
    mZOrder = zOrder;
    if(mGeoPolyline)
    {
        mMapControl->SetPolylineZorder(mGeoPolyline,zOrder);
    }
}

const QString& PolylineImpl::GetID() const
{
    return mID;
}

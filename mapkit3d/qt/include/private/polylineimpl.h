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

    @file polylineimpl.h
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
#ifndef __POLYLINE_IMPL_H__
#define __POLYLINE_IMPL_H__

#include <QList>
#include "polyline.h"
#include "polylineparameters.h"
#include "GeoPolyline.h"
#include "mapcontroller.h"

namespace locationtoolkit
{
class PolylineImpl: public Polyline
{
public:
    PolylineImpl(const PolylineParameters& para, nbmap::GeoPolyline* geopolyline, nbmap::MapController* mapControl);
    ~PolylineImpl();

public:
    /*! Gets the width.
    @return The width of the polyline in screen pixels.
    */
    float GetWidth();

    /*! Sets the width.
    @param width The width of the polyline in screen pixels.
    @return None.
    */
    Polyline& SetWidth(float width);

    /*! Gets the color.
    @return The color in RGBA format.
    */
    QColor& GetColor();

    /*! Sets the color.
    @param color The color in RGBA format.
    @return None.
    */
    Polyline& SetColor(QColor color);

    /*! Checks whether the polyline is visible.
    @return True if the polyline is visible, false otherwise.
    */
    bool IsVisible();

    /*! Sets the visibility.
    @param visible True to make this polyline visible; false to make it invisible.
    @return None.
    */
    void SetVisible(bool visible);

    /*! Sets the list of colored segments to the polyline.
    @param colors
    */
    Polyline& SetColors(const QList<ColorSegment>& colors);

    /*! Gets the list of colored segments from the polyline.
    @return
    */
    QList<ColorSegment>& GetColors();

    /*! Checks whether the polyline is selected.
    @return True if the polyline is selected, false otherwise.
    */
    bool IsSelected();

    /*! Selects the polyline.
    @param selected True to make this polyline selected;false to make it unselected.
    @return None.
    */
    void SetSelected(bool selected);

    /*! Gets the zOrder value.
    @return the zOrder value.
    */
    int GetZOrder();

    /*! Sets the zOrder value.
    The zOrder value will affect the draw orders between polylines.
    @param zOrder The zOrder value.
    @return None.
    */
    void SetZOrder(int zOrder);

    /*! Gets the polyline's id.
    @return id of the polyline.
    */
    virtual const QString& GetID() const;

private:
    float mWidth;
    QColor mColor;
    QList<ColorSegment> mColors;
    bool mVisible;
    bool mSelected;
    int mZOrder;
    nbmap::GeoPolyline* mGeoPolyline;
    nbmap::MapController* mMapControl;
    QString mID;
};
}
#endif // __POLYLINE_IMPL_H__

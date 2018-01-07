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
    @file polyline.h
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
#ifndef __POLYLINE_H__
#define __POLYLINE_H__

#include <QList>
#include "colorsegment.h"

namespace locationtoolkit
{
class Polyline
{
public:
    Polyline(){}
    virtual ~Polyline(){}

    /*! Gets the width.
    @return The width of the polyline in screen pixels.
    */
    virtual float GetWidth() = 0;

    /*! Sets the width.
    @param width The width of the polyline in screen pixels.
    @return None.
    */
    virtual Polyline& SetWidth(float width) = 0;

    /*! Gets the color.
    @return The color in RGBA format.
    */
    virtual QColor& GetColor() = 0;

    /*! Sets the color.
    @param color The color in RGBA format.
    @return None.
    */
    virtual Polyline& SetColor(QColor color) = 0;

    /*! Checks whether the polyline is visible.
    @return True if the polyline is visible, false otherwise.
    */
    virtual bool IsVisible() = 0;

    /*! Sets the visibility.
     @param visible True to make this polyline visible; false to make it invisible.
     @return None.
    */
    virtual void SetVisible(bool visible) = 0;

    /*! Sets the list of colored segments to the polyline.
     @param colors See {@link ColorSegment}.
    */
    virtual Polyline& SetColors(const QList<ColorSegment>& colors) = 0;

    /*! Gets the list of colored segments from the polyline.
     @return
    */
    virtual QList<ColorSegment>& GetColors() = 0;

    /*! Checks whether the polyline is selected.
     @return True if the polyline is selected, false otherwise.
    */
    virtual bool IsSelected() = 0;

    /*! Selects the polyline.
     @param selected True to make this polyline selected;false to make it unselected.
     @return None.
    */
    virtual void SetSelected(bool selected) = 0;

    /*! Gets the zOrder value.
    @return the zOrder value.
    */
    virtual int GetZOrder() = 0;

    /*! Sets the zOrder value.
    The zOrder value will affect the draw orders between polylines.
    @param zOrder The zOrder value.
    @return None.
    */
    virtual void SetZOrder(int zOrder) = 0;

    /*! Gets the polyline's id.
    @return id of the polyline.
    */
    virtual const QString& GetID() const = 0;
};
}
#endif // __POLYLINE_H__

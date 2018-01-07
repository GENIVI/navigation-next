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

    @file     ltknkuitypeimpl.h
    @date     12/09/2014
    @defgroup navigationuikit sample
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef LTKNKUITYPEIMPL_H
#define LTKNKUITYPEIMPL_H

#include "nkuitypes.h"

namespace locationtoolkit{
class MapWidget;
class PolylineParameters;
class Polyline;
class SegmentAttribute;
}

class LTKNKUIPolylineImpl: public locationtoolkit::LTKNKUIPolyline
{
public:
    /*! route unhighlight color for day, 0X807D7DFF */
    static const unsigned int routeUnhightlightDay   = 0X807D7DFF;
    /*! route unhighlight color for night, 0x434343FF */
    static const unsigned int routeUnhightlightNight = 0x434343FF;
    /*! route navigation color, 0x0069BCFF */
    static const unsigned int routeNavigation        = 0x0069BCFF;
    /*! sign of the polylines in route */
    static const int PT_SOLID_LINE_ZORDER         = 1;
    static const int PT_DASH_LINE_ZORDER          = 2;
    /*! (traffic polyline width)/(solid polyline width) = 0.826/3.472 */
    static const float TRAFFICE_LINE_WIDTH_RATE;

    /*! (unhighlight polyline width)/(highlight polyline width) = 2.498/3.472 */
    static const float ROUTE_UNHIGTHLIGHT_WIDTH_RATE;

    /*! zOrder for polyline when selected and unselected */
    static const int LowwerOrder = 0;
    static const int HigherOrder = 100;

    static const int POLYLINE_WIDTH  = 12;
    static const int POLYLINE_BORDER_WIDTH = 2;
    static const int AVATAR_REFERENCE_BOTTOM_GAP = 120;
    static const char* POLYLINE_HIGHLIGHT_COLOR;
    static const char* POLYLINE_UNHIGHLIGHT_COLOR;
    static const char* POLYLINE_BORDER_COLOR;
    static const char* MANEUVER_COLOR;

    enum PolylineType
    {
        PT_SOLID = 0,
        PT_DASH,
        PT_MAX,
    };

public:
    LTKNKUIPolylineImpl(locationtoolkit::MapWidget* map);
    LTKNKUIPolylineImpl(locationtoolkit::Polyline* polyline);
    ~LTKNKUIPolylineImpl();

    virtual void SetVisible(bool visible);
    virtual void SetSelected(bool selected);
    virtual void SetZOrder(int zOrder);
    virtual void SetNavMode(locationtoolkit::NavigationMode navMode);
    virtual bool IsVisible() const;
    virtual bool IsSeleced() const;
    bool Init(const locationtoolkit::LTKNKUIPolylineParameter& parameter);
    virtual int  GetZOrder() const;
    virtual locationtoolkit::NavigationMode GetNavMode() const;
    virtual void SetDayNightMode(bool isDayMode);
    virtual void UpdateTrafficPolyline(const locationtoolkit::LTKNKUIPolylineParameter* parameter);
    virtual const QString& GetID();
    void Remove();
    bool ContainPolyline(locationtoolkit::Polyline* polyline);
private:
    locationtoolkit::Polyline* mPolyline;
    locationtoolkit::Polyline* mTrafficPolylineGreen;
    locationtoolkit::Polyline* mTrafficPolylineOthers;
    bool mSelected;
    bool mVisible;
    int mZOrder;
    locationtoolkit::NavigationMode mNavMode;
    bool mIsDayMode;
    float mLineWidth;
    locationtoolkit::MapWidget* mMapView;
};

#endif // LTKNKUITYPEIMPL_H

/*! @} */

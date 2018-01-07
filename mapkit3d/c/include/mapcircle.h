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
 @file     mapcircle.h
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

#ifndef MAPCIRCLE_H_
#define MAPCIRCLE_H_

#include "mapcontroller.h"

namespace nbmap
{

/*! a simple wrapper class to control circle
 */
class MapCircle
{
    public:
        MapCircle(MapController& controller, int id, double lat, double lon, float radius, int fillColor, int strokeColor, int zOrder, bool visible);
        ~MapCircle();

    public:
        /**
         * @return ID.
         */
        int GetId() const;
        /**
         * @name SetCenter - set the center of the circle.
         * @param lat - the latitude of the center
         * @param lon - the longtitude of the center
         * @return None.
         */
        void SetCenter(double lat, double lon);

        /**
         * @name GetCenter - get the center of the circle.
         * @param lat - output, the latitude of the center
         * @param lon - output, the longtitude of the center
         * @return None.
         */
        void GetCenter(double &lat, double &lon)const;

        /**
         * @name SetRadius - set the radius of the circle.
         * @param radius - the radius of the center, in pixels
         * @return None.
         */
        void SetRadius(float radius);

        /**
         * @name GetRadius - get the radius of the circle.
         * @return radius, in pixels.
         */
        float GetRadius()const;

        /**
         * @name SetFillColor - set the fill color of the circle.
         * @param fillColor - the component format is RGBA.
         * @return None.
         */
        void SetFillColor(int fillColor);

        /**
         * @name GetFillColor - get the fill color of the circle.
         * @return FillColor, in RGBA.
         */
        int GetFillColor()const;

        /**
         * @name SetStokeColor - set the stroke color of the circle.
         * @param strokeColor - the component format is RGBA.
         * @return None.
         */
        void SetStrokeColor(int strokeColor);

        /**
         * @name GetStrokeColor - get the stroke color of the circle.
         * @return strokeColor, in RGBA.
         */
        int GetStrokeColor()const;

        /**
         * @name SetZOrder - set the zOrder of the circle.
         * @param zOrder - the zOrder value.
         * @return None.
         */
        void SetZOrder(int zOrder);

        /**
         * @name GetZOrder - get the zOrder of the circle.
         * @return the zOrder value.
         */
        int GetZOrder()const;

        /**
         * @name SetVisible - set the visibility of the circle.
         * @param visible - true if the circle is visible; false if it is invisible.
         * @return None.
         */
        void SetVisible(bool visible);

        /**
         * @name IsVisible - get the visibility of the circle.
         * @return True if the circle is visible; false if it is invisible.
         */
        bool IsVisible()const;

        /**
         * @name Remove - remove the circle.
         * @return None.
         */
        void Remove();

        /**
         * @name IsRemoved - check if the circel is removed.
         * @return True if the circle is removed; false if not.
         */
        bool IsRemoved()const;

    private:
        int m_id;
        double m_lat;
        double m_lon;
        float m_radius;
        int m_fillColor;
        int m_strokeColor;
        int m_zOrder;
        bool m_visible;
        bool m_removed;
        MapController &m_mapController;
};

}

#endif /* MAPPIN_H_ */

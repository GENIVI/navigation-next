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
 @file     maprect2d.h
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

#ifndef MAPRECT2D_H_
#define MAPRECT2D_H_

#include "mapcontroller.h"

namespace nbmap
{

/*! a simple wrapper class to control rect2d
 */
class MapRect2d
{
    public:
        MapRect2d(MapController& controller, int id, double lat, double lon, float heading, float width, float height, bool visible);
        ~MapRect2d();

    public:
        /**
         * @return ID.
         */
        int GetId() const;

        /**
         * @name Update - set the center and heading angle of the rect2d.
         * @param lat - the latitude of the center
         * @param lon - the longtitude of the center
         * @param heading - the heading angle, in degree
         * @return None.
         */
        void Update(double lat, double lon, float heading);

        /**
         * @name SetSize - set the size of the rect2d.
         * @param width - the width of the rect2d, in pixels
         * @param height - the height of the rect2d, in pixels
         * @return None.
         */
        void SetSize(float width, float height);

        /**
         * @name SetVisible - set the visibility of the rect2d.
         * @param visible - true if the rect2d is visible; false if it is invisible.
         * @return None.
         */
        void SetVisible(bool visible);

        /**
         * @name Remove - remove the rect2d.
         * @return None.
         */
        void Remove();

        /**
         * @name GetCenter - get the center of the rect2d.
         * @param lat - output, the latitude of the center
         * @param lon - output, the longtitude of the center
         * @return None.
         */
        void GetCenter(double &lat, double &lon)const;

        /**
         * @name GetHeading - get the heading angle of the rect2d.
         * @return heading, in degrees.
         */

        float GetHeading()const;
        /**
         * @name GetSize - get the size of the rect2d.
         * @param width - output, in pixels
         * @param height - output, in pixels
         * @return None.
         */
        void GetSize(float &width, float &height)const;

        /**
         * @name IsVisible - get the visibility of the rect2d.
         * @return True if the rect2d is visible; false if it is invisible.
         */
        bool IsVisible()const;

        /**
         * @name IsRemoved - check if the rect2d is removed.
         * @return True if the rect2d is removed; false if not.
         */
        bool IsRemoved()const;

    private:
        int m_id;
        double m_lat;
        double m_lon;
        float m_heading;
        float m_width;
        float m_height;
        bool m_visible;
        bool m_removed;
        MapController &m_mapController;
};

}

#endif /* MAPRECT2D_H_ */

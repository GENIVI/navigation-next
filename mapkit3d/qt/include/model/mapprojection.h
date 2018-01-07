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
    @file mapprojection.h
    @date 11/11/2014
    @addtogroup mapkit3d
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
---------------------------------------------------------------------------*/

/*! @{ */

#ifndef MAPPROJECTION_H
#define MAPPROJECTION_H

#include <QtCore>

namespace locationtoolkit
{

/**
 * Defines a mapping between widget coordinate and coordinates in the map's view.
 * Passing invalid coordinate to this object may
 * result in undefined behavior.
 *
 * This class should not be instantiated directly, instead, obtained via projection on MapWidget.
 */
class MapProjection
{
public:
    virtual ~MapProjection() {}

    /**
     * Gets the geographic location that corresponds to a screen location.
     *
     * @param x,y Point on the screen in screen pixels.
     * @param lat,lon geographical coordinate corresponding to the point on the screen
     * @return whether x and y are valid.
     */
    virtual bool FromScreenPosition(int x, int y, double& lat, double& lon) const = 0;

    /**
     * Gets the screen location that corresponds to a geographical coordinate .
     *
     * @param lat,lon geographical coordinate in the world.
     * @param x,y Point on the screen in pixels corresponding to the geographical coordinate
     * @return whether lat and lon are valid.
     */
    virtual bool ToScreenPosition(double lat, double lon, int& x, int& y) const = 0;

    /**
     * Gets the calculated length(in meters) per pixel based on the current camera.
     *
     * @return Meters per one pixel.
     */
    virtual qreal MetersPerPixel() const = 0;
};
}
#endif // MAPPROJECTION_H

/*! @} */

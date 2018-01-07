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

    @file navvisibleprocessor.h
    @date 12/23/2011

*/
/*
    (C) Copyright 2012 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef NBSERVICE_VISIBLE_PROCESSOR_H
#define NBSERVICE_VISIBLE_PROCESSOR_H

#include "base.h"
#include "TileKey.h"
#include <set>
extern "C"
{
#include "nbnavigation.h"
#include "spatialvector.h"
}

namespace nbmap
{

typedef struct
{
    uint32 cameraHeightMeters;               /*!< */
    uint32 cameraToAvatarHorizontalMeters;   /*!< */
    uint32 avatarToHorizonHorizontalMeters;  /*!< */
    uint32 horizontalFieldOfViewDegrees;     /*!< */
    uint32 avatarToMapBottomPixel;           /*!< */
    uint32 mapFieldWidthPixel;
    uint32 mapFieldHeightPixel;
} NB_CameraProjectionParameters;

/*! Class to help calculating visible tiles.

    Use NB_CameraProjectionParameters to initiallize a camera, call 'GetVisibleQuadrangle' to calculate an area of sight view.
    or call 'GetVisibleTiles' to get all visible tiles of a specific zoom level.
*/
class VisibleProcessor
    : public Base
{
public:
    VisibleProcessor(const NB_CameraProjectionParameters& configuration);
    VisibleProcessor();

    /*! Reset projection settings

        @return None
    */
    void
    UpdateProjectionParameters(const NB_CameraProjectionParameters& configuration   /*!< the new configuration */
                               );

    /*! Set the position base to which visible quadrangle is calculated.

        Camera parameters are calculate base a position on XY plan. This position usually the same
        with the avatar, so that the camera will follow the avatar during navigation. Sometimes, look ahead
        for example, the position may be a street cross on the map, then the camera will focus on that point.

        @return None

        @see GetVisibleQuadrangle
        @see GetVisibleTiles
    */
    void
    SetBasePosition(double latitude,                                                /*!< latitude */
                    double longitude,                                               /*!< longitude */
                    double heading                                                  /*!< angle between camera heading and north. deg */
                    );

    /*! Get visible quadrangle according to current settings.

        In perspective view, an area on XY plan can be seen on screen.
        This function is used for calculateing the area.

        @return None

        @see SetBasePosition
    */
    void
    GetVisibleQuadrangle(NB_Vector quadrangle[4]                                    /*!< the 4 mercator vertex of visible area in the order of bottom left, top left, top right and bottom right. */
                         );

    /*! Get visible tile indecis according to current settings.

        GetVisibleQuadrangle is called in this function.
        This function will calculate all tiles in the area which GetVisibleQuadrangle returned.

        @return None

        @see GetVisibleQuadrangle
        @see SetBasePosition
    */
    void
    GetVisibleTiles(NB_EnhancedCityModelBoundingBoxParameters* ecmBoundingBoxes,    /*!< ecm bounding boxes */
                    int zoomLevel,                                                  /*!< zoom level */
                    std::set<TileKey>& visibleTiles,                                /*!< all visible tiles */
                    std::set<TileKey>& tilesInECM                                   /*!< tiles in ECM bounding box */
                    );

private:
    double      m_latitude;                                                         /*!< base position latitude */
    double      m_longitude;                                                        /*!< base position longitude */
    double      m_heading;                                                          /*!< base position heading */

    double      m_nearDistance;                                                     /*!< near plane distance from the base position */
    double      m_farDistance;                                                      /*!< far plane distance from the base position */
    double      m_viewNearWidth;                                                    /*!< near plane width */
    double      m_viewFarWidth;                                                     /*!< far plane width */
};
}

#endif

/*! @} */

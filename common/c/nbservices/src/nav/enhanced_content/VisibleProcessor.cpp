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

    @file navvisibleprocessor.cpp
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

#include "VisibleProcessor.h"
#include "palmath.h"

#include <list>

extern "C"
{
#include "transformutility.h"
#include "nbnavigationprivate.h"
#include "nbcontextprotected.h"
#include "nbenhancedvectormapprocessorprivate.h"
}

#include "nbenhancedvectormapprivatetypes.h"

static const double s_extendMeters = 50.;

namespace nbmap
{

VisibleProcessor::VisibleProcessor(const NB_CameraProjectionParameters& config)
    : m_latitude(INVALID_LATLON)
    , m_longitude(INVALID_LATLON)
    , m_heading(0.0)
{
    UpdateProjectionParameters(config);
}

VisibleProcessor::VisibleProcessor()
{
}

/*! see header file for description */
void
VisibleProcessor::UpdateProjectionParameters(const NB_CameraProjectionParameters& configuration)
{
    m_nearDistance = 0.0;
    m_farDistance = 0.0;
    m_viewNearWidth = 0.0;
    m_viewFarWidth = 0.0;

    // validate config parameters
    if (configuration.cameraHeightMeters == 0 || configuration.avatarToMapBottomPixel > configuration.mapFieldHeightPixel)
    {
        return;
    }

    double verticalFieldOfViewDegrees = (double)configuration.horizontalFieldOfViewDegrees * (double)configuration.mapFieldHeightPixel / (double)configuration.mapFieldWidthPixel;
    double nearAngle = nsl_atan((double)configuration.cameraToAvatarHorizontalMeters / (double)configuration.cameraHeightMeters)
        - TO_RAD(verticalFieldOfViewDegrees * (double)configuration.avatarToMapBottomPixel / (double)configuration.mapFieldHeightPixel);

    m_nearDistance = configuration.cameraHeightMeters * nsl_tan(nearAngle) - configuration.cameraToAvatarHorizontalMeters;

    m_viewNearWidth = configuration.cameraHeightMeters / nsl_cos(nearAngle) * nsl_tan(TO_RAD(configuration.horizontalFieldOfViewDegrees) / 2.0);

    double horizonAngle = nsl_atan((double)(configuration.avatarToHorizonHorizontalMeters + configuration.cameraToAvatarHorizontalMeters) / (double)configuration.cameraHeightMeters);
    double farAngle = nearAngle + TO_RAD(verticalFieldOfViewDegrees);
    if (horizonAngle < farAngle)
    {
        farAngle = horizonAngle;
        m_farDistance = configuration.avatarToHorizonHorizontalMeters;
    }
    else
    {
        m_farDistance = configuration.cameraHeightMeters * nsl_tan(farAngle) - configuration.cameraToAvatarHorizontalMeters;
    }
    m_viewFarWidth = configuration.cameraHeightMeters / nsl_cos(farAngle) * nsl_tan(TO_RAD(configuration.horizontalFieldOfViewDegrees) / 2.0);

    // we calculate a little larger area, so we can start to load tile earlier
    m_nearDistance -= s_extendMeters;
    m_farDistance += s_extendMeters;
    m_viewNearWidth += s_extendMeters;
    m_viewFarWidth += s_extendMeters;
}

/*! see header file for description */
void
VisibleProcessor::SetBasePosition(double latitude, double longitude, double heading)
{
    m_latitude = latitude;
    m_longitude = longitude;
    m_heading = heading;
}

/*! see header file for description */
void
VisibleProcessor::GetVisibleQuadrangle(NB_Vector quadrangle[4])
{
    if (m_latitude == INVALID_LATLON || m_longitude == INVALID_LATLON)
    {
        return;
    }
    NB_Vector basePosition = {0};
    // Convert to mercator. Don't use vector_from_lat_lon()!
    NB_SpatialConvertLatLongToMercator(m_latitude, m_longitude, &basePosition.x, &basePosition.y);

    double referenceRadiusMeters = nsl_cos(TO_RAD(m_latitude)) * RADIUS_EARTH_METERS;
    double rotateSine            = nsl_sin(TO_RAD(m_heading));
    double rotateCosine          = nsl_cos(TO_RAD(m_heading));

    // calculate near plane offset from the basePosition.
    NB_Vector nearOffset;
    vector_set(&nearOffset,
               m_nearDistance * rotateSine/ referenceRadiusMeters,
               m_nearDistance * rotateCosine / referenceRadiusMeters,
               0.0);

    NB_Vector startPosition;
    vector_add(&startPosition, &basePosition, &nearOffset);

    // calculate far plane offset from the basePosition.
    NB_Vector farOffset;
    vector_set(&farOffset,
               m_farDistance * rotateSine / referenceRadiusMeters,
               m_farDistance * rotateCosine / referenceRadiusMeters,
               0.0);

    NB_Vector endPosition;
    vector_add(&endPosition, &basePosition, &farOffset);

    // calculate normal direction. (this normal is pointing at right)
    NB_Vector unitZ = {0};
    vector_set(&unitZ, 0.0, 0.0, 1.0);
    NB_Vector sightDirection;
    vector_subtract(&sightDirection, &endPosition, &startPosition);
    NB_Vector normal;
    vector_cross_product(&normal, &sightDirection, &unitZ);

    NB_Vector normalNear;
    vector_normalize_to_length(&normalNear, &normal, m_viewNearWidth / referenceRadiusMeters);
    NB_Vector normalFar;
    vector_normalize_to_length(&normalFar, &normal, m_viewFarWidth / referenceRadiusMeters);

    vector_subtract(&quadrangle[0], &startPosition, &normalNear);    // bottom left
    vector_subtract(&quadrangle[1], &endPosition, &normalFar);       // top left
    vector_add( &quadrangle[2], &endPosition, &normalFar);           // top right
    vector_add( &quadrangle[3], &startPosition, &normalNear);        // bottom right
}

void
VisibleProcessor::GetVisibleTiles(NB_EnhancedCityModelBoundingBoxParameters* ecmBoundingBoxes, int zoomLevel, std::set<TileKey>& visibleTiles, std::set<TileKey>& tilesInECM)
{
    if (m_latitude == INVALID_LATLON || m_longitude == INVALID_LATLON)
    {
        return;
    }
    // Get visible area
    NB_Vector visibleQuad[4] = {0};
    GetVisibleQuadrangle(visibleQuad);
    int verticesCount = 4;

    // Get ECM bounding box array for the visible area.
    uint32 boundingBoxesCount = 0;
    uint32* boundingBoxIndexes = NULL;
    NB_GetECMBoundingBoxes(visibleQuad, verticesCount, ecmBoundingBoxes, &boundingBoxesCount, &boundingBoxIndexes);

    // use sutherland_hodgman_polygon_clip to calculate tiles in the visible area
    {
        NB_Vector buffer1[20] = {0};
        NB_Vector buffer2[20] = {0};

        int verticesCount = 4;
        // Clip against two planes
        int minTileX = -1;
        int maxTileX = -1;
        // Find the x tile extents
        for (int n = 0; n < verticesCount; ++n)
        {
            int tx = 0;
            NB_SpatialConvertMercatorToTile(visibleQuad[n].x, visibleQuad[n].y, zoomLevel, &tx, NULL);
            minTileX = minTileX == -1 ? tx : MIN(tx, minTileX);
            maxTileX = maxTileX == -1 ? tx : MAX(tx, maxTileX);
        }

        for (int tileX = minTileX; tileX <= maxTileX; ++tileX)
        {
            buffer1[0] = visibleQuad[0];
            buffer1[1] = visibleQuad[1];
            buffer1[2] = visibleQuad[2];
            buffer1[3] = visibleQuad[3];
            verticesCount = 4;

            // clip against the minimum x
            struct ortho_clip_data ocdata = {0};
            ocdata.clipflags = ORTHO_CLIP_MIN | ORTHO_CLIP_X;
            NB_SpatialConvertTileToMercator(tileX, 0, zoomLevel, 0, 0, &ocdata.val, NULL); // ty is a don't-care
            int retval = sutherland_hodgman_polygon_clip(buffer1, verticesCount, buffer2, 20, ortho_clip, ortho_intersect, &ocdata);
            // if visible vertex less than 2
            if (retval < 2)
            {
                continue;
            }
            verticesCount = retval;

            // clip against the maximum x
            ocdata.clipflags = ORTHO_CLIP_MAX | ORTHO_CLIP_X;
            NB_SpatialConvertTileToMercator(tileX, 0,zoomLevel, 1.0, 0, &ocdata.val, NULL); // ty is a don't-care
            retval = sutherland_hodgman_polygon_clip(buffer2, verticesCount, buffer1, 20, ortho_clip, ortho_intersect, &ocdata);
            if (retval < 2)
            {
                continue;
            }
            verticesCount = retval;

            // Get the min and max y tile values of the resulting polygon
            int minTileY = -1;
            int maxTileY = -1;
            for (int n = 0; n < verticesCount; n++)
            {
                int ty = 0;
                NB_SpatialConvertMercatorToTile(buffer1[n].x, buffer1[n].y, zoomLevel, NULL, &ty);
                minTileY = minTileY == -1 ? ty : MIN(ty, minTileY);
                maxTileY = maxTileY == -1 ? ty : MAX(ty, maxTileY);
            }

            for (int tileY = minTileY; tileY <= maxTileY; ++tileY)
            {
                // check if tile in ECM area
                bool bInECM = FALSE;
                double minLatitude, maxLatitude, minLongitude, maxLongitude;
                double latitude1, latitude2, longitude1, longitude2;
                double mercatorX, mercatorY;
                NB_SpatialConvertTileToMercator(tileX, tileY, zoomLevel, 0, 0, &mercatorX, &mercatorY);
                NB_SpatialConvertMercatorToLatLong(mercatorX, mercatorY, &latitude1, &longitude1);
                NB_SpatialConvertTileToMercator(tileX, tileY, zoomLevel, 1, 1, &mercatorX, &mercatorY);
                NB_SpatialConvertMercatorToLatLong(mercatorX, mercatorY, &latitude2, &longitude2);
                minLatitude = MIN(latitude1, latitude2);
                maxLatitude = MAX(latitude1, latitude2);
                minLongitude = MIN(longitude1, longitude2);
                maxLongitude = MAX(longitude1, longitude2);

                for (uint32 i = 0; i < boundingBoxesCount; ++i)
                {
                    // check if tile intersects bounding box
                    NB_CityBoundingBox* pBoundingBox = &ecmBoundingBoxes->ecmBoundingBoxes[boundingBoxIndexes[i]];
                    latitude1 = MIN(pBoundingBox->point1.latitude, pBoundingBox->point2.latitude);
                    latitude2 = MAX(pBoundingBox->point1.latitude, pBoundingBox->point2.latitude);
                    longitude1 = MIN(pBoundingBox->point1.longitude, pBoundingBox->point2.longitude);
                    longitude2 = MAX(pBoundingBox->point1.longitude, pBoundingBox->point2.longitude);

                    latitude1 = MAX(minLatitude, latitude1);
                    latitude2 = MIN(maxLatitude, latitude2);
                    longitude1 = MAX(minLongitude, longitude1);
                    longitude2 = MIN(maxLongitude, longitude2);
                    if (latitude1 < latitude2 && longitude1 < longitude2)
                    {
                        bInECM = TRUE;
                        break;
                    }
                }

                // add to result set
                if (bInECM)
                {
                    tilesInECM.insert(TileKey(tileX, tileY, zoomLevel));
                }
                else
                {
                    visibleTiles.insert(TileKey(tileX, tileY, zoomLevel));
                }
            }
        }
    }

    // clear
    if (boundingBoxIndexes)
    {
        nsl_free(boundingBoxIndexes);
    }
}

}

/*! @} */

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

    @file prefetchprocessor.cpp
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

#include "PrefetchProcessor.h"
#include <list>


#include "palmath.h"

extern "C"
{
#include "nbspatial.h"
#include "spatialvector.h"
#include "transformutility.h"
#include "datautil.h"
#include "nbnavigationprivate.h"
#include "nbnavigationstateprivate.h"
#include "nbcontextprotected.h"
#include "nbpointiteration.h"
#include "nbenhancedvectormapprocessorprivate.h"
#include "nbqalog.h"
}

#include "nbenhancedvectormapprivatetypes.h"
#include "paldebuglog.h"

namespace nbmap
{
    /*! see header file for description */
    PrefetchProcessor::PrefetchProcessor(const PrefecthConfiguration& configuration)
        : m_configuration(configuration)
        , m_tolerance(1.2)
    {
    }

    PrefetchProcessor::PrefetchProcessor()
        : m_tolerance(1.2)
    {

    }

    /*! see header file for description */
    PrefetchProcessor::~PrefetchProcessor()
    {
    }

    /*! see header file for description */
    NB_Error
    PrefetchProcessor::ResetConfiguration(const PrefecthConfiguration& configuration)
    {
        m_configuration = configuration;
        return NE_OK;
    }

    void
    PrefetchProcessor::Prefetch(shared_ptr<vector<pair<double, double> > > polylineList,
                double prefetchExtensionLengthMeters, double prefetchExtensionWidthMeters, float zoomLevel,
                std::vector<nbmap::TileKeyPtr>& tilesToPrefetch)
    {
        vector<pair<double, double> >::const_iterator iter = polylineList->begin();
        vector<pair<double, double> >::const_iterator end = polylineList->end();
        int counter = 0;
        double startLat = 0;
        double startLon = 0;
        double endLat = 0;
        double endLon = 0;
        for(; iter != end; ++iter)
        {
            startLat = iter->first;
            startLon = iter->second;
            if(iter + 1 == end)
            {
                break;
            }
            endLat = (iter + 1)->first;
            endLon = (iter + 1)->second;

            NB_Vector startPoint = {0};
            NB_Vector endPoint = {0};
            NB_SpatialConvertLatLongToMercator(startLat, startLon, &startPoint.x, &startPoint.y);
            NB_SpatialConvertLatLongToMercator(endLat, endLon, &endPoint.x, &endPoint.y);

            //From here enter into the core logic
            //segment vector from start point to end point
            NB_Vector segmentVector;
            vector_subtract(&segmentVector, &endPoint, &startPoint);

            // Can't prefetch if the segment is zero length
            if (vector_magnitude(&segmentVector) == 0)
            {
                return;
            }

            double prefetchLength = vector_distance(&startPoint, &endPoint);
            double referenceCosLatitudeRadius = nsl_cos(TO_RAD(startLat)) * RADIUS_EARTH_METERS;
            double prefetchLengthMercator = NB_SpatialMetersToMercator(prefetchLength, referenceCosLatitudeRadius);
            // prefetch extension length in mercator coordinate
            double prefetchExtensionLengthMercator = NB_SpatialMetersToMercator(prefetchExtensionLengthMeters, referenceCosLatitudeRadius);
            // prefetch extension width in mercator coordinate
            double prefetchExtensionWidthMercator = NB_SpatialMetersToMercator(prefetchExtensionWidthMeters * m_tolerance, referenceCosLatitudeRadius);

            // Extend segment by prefetchExtensionLengthMercator
            NB_Vector temp, start, end;
            start = startPoint;
            vector_normalize_to_length(&temp, &segmentVector, prefetchLengthMercator + prefetchExtensionLengthMercator);
            vector_add(&end, &startPoint, &temp);

            // Extend the segment on both side, to get a box around the segment
            NB_Vector startLeftPoint, startRightPoint, endLeftPoint, endRightPoint;
            calculate_segment_normals(&start, &end, prefetchExtensionWidthMercator, &startLeftPoint, &startRightPoint, &endLeftPoint, &endRightPoint, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL);

            // Set up array of vertices for clipping (being sure to use convex ordering)
            NB_Vector buffer1[20] = {{0}};
            buffer1[0] = startLeftPoint;
            buffer1[1] = startRightPoint;
            buffer1[2] = endRightPoint;
            buffer1[3] = endLeftPoint;
            NB_Vector buffer2[20] = {{0}};

            int verticesCount = 4;
            // Clip against two planes
            int minTileX = -1;
            int maxTileX = -1;
            // Find the x tile extents
            for (int n = 0; n < verticesCount; ++n)
            {
                int tx = 0;
                NB_SpatialConvertMercatorToTile(buffer1[n].x, buffer1[n].y, (int)zoomLevel, &tx, NULL);
                minTileX = minTileX == -1 ? tx : MIN(tx, minTileX);
                maxTileX = maxTileX == -1 ? tx : MAX(tx, maxTileX);
            }

            for (int tileX = minTileX; tileX <= maxTileX; ++tileX)
            {
                // Set up array of vertices for clipping (being sure to use convex ordering)
                buffer1[0] = startLeftPoint;
                buffer1[1] = startRightPoint;
                buffer1[2] = endRightPoint;
                buffer1[3] = endLeftPoint;
                verticesCount = 4;

                // clip against the minimum x
                struct ortho_clip_data ocdata = {0};
                ocdata.clipflags = ORTHO_CLIP_MIN | ORTHO_CLIP_X;
                NB_SpatialConvertTileToMercator(tileX, 0, (int)zoomLevel, 0, 0, &ocdata.val, NULL); // ty is a don't-care
                int retval = sutherland_hodgman_polygon_clip(buffer1, verticesCount, buffer2, 20, ortho_clip, ortho_intersect, &ocdata);
                if (retval < 2)
                {
                    continue;
                }
                verticesCount = retval;

                // clip against the maximum x
                ocdata.clipflags = ORTHO_CLIP_MAX | ORTHO_CLIP_X;
                NB_SpatialConvertTileToMercator(tileX, 0, (int)zoomLevel, 1.0, 0, &ocdata.val, NULL); // ty is a don't-care
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
                    NB_SpatialConvertMercatorToTile(buffer1[n].x, buffer1[n].y, (int)zoomLevel, NULL, &ty);
                    minTileY = minTileY == -1 ? ty : MIN(ty, minTileY);
                    maxTileY = maxTileY == -1 ? ty : MAX(ty, maxTileY);
                }

                for (int tileY = minTileY; tileY <= maxTileY; ++tileY)
                {
                    tilesToPrefetch.push_back(nbmap::TileKeyPtr(new nbmap::TileKey(tileX, tileY, (int)zoomLevel)));
                    DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("tileX[%d], tileY[%d], zoomLevel[%lf]", tileX, tileY, zoomLevel));
                    counter++;
                }
            }
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("total prefetch counter[%d]", counter));
        }
    }

    void
    PrefetchProcessor::Prefetch(float zoomLevel,
            std::vector<Point2d64>& _frustum,
            std::vector<nbmap::TileKeyPtr>& tilesToPrefetch)
    {
        NB_Vector frustum[4];
        for (size_t i = 0; i < 4; ++i)
        {
            frustum[i].x = _frustum[i].x;
            frustum[i].y = _frustum[i].y;
        }
        Point minTile(0x7FFFFFFF, 0x7FFFFFFF);
        Point maxTile;

        //Get rectangular area based on the frustum
        GetMinMaxTile(frustum[0].x, frustum[0].y, (int)zoomLevel, minTile, maxTile);
        GetMinMaxTile(frustum[1].x, frustum[1].y, (int)zoomLevel, minTile, maxTile);
        GetMinMaxTile(frustum[2].x, frustum[2].y, (int)zoomLevel, minTile, maxTile);
        GetMinMaxTile(frustum[3].x, frustum[3].y, (int)zoomLevel, minTile, maxTile);

        bool isFrustumOverlapping = CheckFrustumOverlapping(frustum);

        /*
          Now go through the rectangular area and check which tile actually intersects with the frustum.
          Additional check is added to ensure the generated TileKey is valid.
         */
         int maxValue = static_cast<int>(nsl_pow(2, zoomLevel));
         for (int x = minTile.x; x <= maxTile.x; ++x)
         {
            for (int y = minTile.y; y <= maxTile.y; ++y)
            {
                // Convert tile corners to mercator coordinates
                NB_Vector tileCorners[4] = {{0}};
                ConvertTileToMercator(TileKey(x, y, (int)zoomLevel), tileCorners);

                // Check if the tile intersects with the frustum
                if (vector_polygons_intersect(tileCorners, 4, TRUE, frustum, 4, FALSE))
                {
                    if (isFrustumOverlapping)
                    {
                        // Wrap around logic
                        int xTile(x);
                        int yTile(y);

                        while (xTile < 0)
                        {
                            xTile += maxValue;
                        }
                        while (yTile < 0)
                        {
                            yTile += maxValue;
                        }
                        while (xTile >= maxValue)
                        {
                            xTile -= maxValue;
                        }
                        while (yTile >= maxValue)
                        {
                            yTile -= maxValue;
                        }
                        tilesToPrefetch.push_back(nbmap::TileKeyPtr(new nbmap::TileKey(xTile, yTile, (int)zoomLevel)));
                    }
                    else
                    {
                        // Only add it if it is in range
                        if ((x >= 0) && (x < maxValue) && (y >= 0) && (y < maxValue))
                        {
                            tilesToPrefetch.push_back(nbmap::TileKeyPtr(new nbmap::TileKey(x, y, (int)zoomLevel)));
                        }
                    }
                }
            }
        }
    }

    bool PrefetchProcessor::CheckFrustumOverlapping(NB_Vector frustum[4])
    {
        for (int i = 0; i < 4; ++i)
        {
            // For now just check the east/west boundary. I don't think we need to check
            if ((frustum[i].x < -PI) || (frustum[i].x > PI))
            {
                return true;
            }
        }

        return false;
    }

    /*! Convert a tile key to mercator coordinates */
    void
    PrefetchProcessor::ConvertTileToMercator(TileKey key,               /*!< Tile key to convert */
                                 NB_Vector tileCorners[4]   /*!< On return the tile corners in mercator coordinates */
                                 )
    {
        // Convert top-left and bottom-right corners
        NB_SpatialConvertTileToMercator(key.m_x, key.m_y, key.m_zoomLevel, 0, 0, &tileCorners[0].x, &tileCorners[0].y);
        NB_SpatialConvertTileToMercator(key.m_x, key.m_y, key.m_zoomLevel, 1.0, 1.0, &tileCorners[2].x, &tileCorners[2].y);

        // Set top-right and bottom-left corners
        tileCorners[1].x = tileCorners[2].x;
        tileCorners[1].y = tileCorners[0].y;
        tileCorners[3].x = tileCorners[0].x;
        tileCorners[3].y = tileCorners[2].y;
    }

    /*! Helper function to calculate the minimum/maximum tile index based on a given mercator coordinate.

        @see UpdateTilesFrustum
     */
    void
    PrefetchProcessor::GetMinMaxTile(double x,          /*!< x coordinate in mercator */
                         double y,          /*!< y coordinate in mercator */
                         int zoomLevel,
                         Point& minTile,    /*!< In/Out: minimum tile coordinate */
                         Point& maxTile     /*!< In/Out: maximum tile coordinate */
                         )
    {
        // Convert the mercator coordinate into a tile index and then update the minimum/maximum tile index.
        Point tempTile;
        NB_SpatialConvertMercatorToTile(x, y, zoomLevel, &(tempTile.x), &(tempTile.y));

        minTile.x = min(minTile.x, tempTile.x);
        minTile.y = min(minTile.y, tempTile.y);
        maxTile.x = max(maxTile.x, tempTile.x);
        maxTile.y = max(maxTile.y, tempTile.y);
    }

    NB_Error
    PrefetchProcessor::Update(NB_Navigation* navigation,                                                   /*!< navigation instance */
           NB_EnhancedCityModelBoundingBoxParameters* ecmBoundingBoxes,                 /*!< ecm bounding boxes */
           uint32 zoomLevel,                                                            /*!< reference zoom level */
           std::set<nbmap::TileKey>& tilesToPrefetch,                                   /*!< returns tiles to prefetch */
           std::set<nbmap::TileKey>& tilesInECM                                         /*!< returns tiles to prefetch, but in ECM area */
           ){return NE_OK;};
}

/*! @} */

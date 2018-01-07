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

    @file prefetchprocessor.h
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

#ifndef NBSERVICE_PREFETCH_PROCESSOR_H
#define NBSERVICE_PREFETCH_PROCESSOR_H

#include "nberror.h"
#include "paltypes.h"
#include "base.h"
#include "TileKey.h"
#include <set>
#include <vector>
#include "nbenhancedvectormaptypes.h"
#include "graphics.h"

extern "C"
{
#include "nbspatial.h"
#include "nbnavigation.h"
}

using namespace nb;

namespace nbmap
{

/*! prefetch configurations */
typedef struct
{
    double  prefetchWindow[ENHANCED_VECTOR_MAP_PREFETCH_LEVEL_COUNT];
    double  prefetchMinWindow[ENHANCED_VECTOR_MAP_PREFETCH_LEVEL_COUNT];
    double  prefetchExtensionLengthMeters;
    double  prefetchExtensionWidthMeters;
} PrefecthConfiguration;

typedef struct _Point2d64
{
    double x;        /*!< x */
    double y;        /*!< y */
}Point2d64;

class PrefetchProcessor
    : public Base
{
public:
    PrefetchProcessor(const PrefecthConfiguration& configuration);
    PrefetchProcessor();
    ~PrefetchProcessor();

    void Prefetch(shared_ptr<vector<pair<double, double> > > polylineList,
            double prefetchExtensionLengthMeters, double prefetchExtensionWidthMeters, float zoomLevel,
            std::vector<nbmap::TileKeyPtr>& tilesToPrefetch);

    void Prefetch(float zoomLevel,
            std::vector<Point2d64>& _frustum,
            std::vector<nbmap::TileKeyPtr>& tilesToPrefetch);
    /*! set a new prefetch configuration */
    NB_Error
    ResetConfiguration(const PrefecthConfiguration& configuration                       /*!< new configuration parameters */
                       );

    //Keep this function just make the build success.
    //Later, maybe we involve the ECM content in prefetch feature.
    //we will re-implement this function.
    NB_Error
    Update(NB_Navigation* navigation,                                                   /*!< navigation instance */
           NB_EnhancedCityModelBoundingBoxParameters* ecmBoundingBoxes,                 /*!< ecm bounding boxes */
           uint32 zoomLevel,                                                            /*!< reference zoom level */
           std::set<nbmap::TileKey>& tilesToPrefetch,                                   /*!< returns tiles to prefetch */
           std::set<nbmap::TileKey>& tilesInECM                                         /*!< returns tiles to prefetch, but in ECM area */
           );

private:
    void GetMinMaxTile(double x, double y, int zoomLevel, Point& minTile, Point& maxTile);
    void ConvertTileToMercator(TileKey key, NB_Vector tileCorners[4]);
    bool CheckFrustumOverlapping(NB_Vector frustum[4]);

    PrefecthConfiguration   m_configuration;                                            /*!< prefetch configurations */
    double                  m_tolerance;                                                /*!< we will prefetch a larger area, tolerance initiallized to 1.2 */
};

}

#endif

/*! @} */

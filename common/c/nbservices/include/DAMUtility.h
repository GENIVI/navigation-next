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
  @file     DAMUtility.h
  @defgroup nbmap
*/
/*
  (C) Copyright 2013 by TeleCommunication Systems, Inc.

  The information contained herein is confidential, proprietary
  to TeleCommunication Systems, Inc., and considered a trade secret as
  defined in section 499C of the penal code of the State of
  California. Use of this information by anyone other than
  authorized employees of TeleCommunication Systems, is granted only
  under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.

  ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef DAMUTILITY_H
#define DAMUTILITY_H

extern "C"
{
#include "nbcontext.h"
}

#include "TileKey.h"
#include <bitset>
#include <vector>

namespace nbmap
{

/*! DAM verify index each index corresponding to the bit in DAM*/
enum DamIndex
{
    DI_Map_Partial = 0,
    DI_Map_Full,
    DI_Route_Partial,
    DI_Route_Full,
    DI_Geocode_Partial,
    DI_Geocode_Full,
    DI_Poi_Partial,
    DI_Poi_Full,
    DI_Realtime_Traffic_Partial,
    DI_Realtime_Traffic_Full,
    DI_Traffic_Incident_Partial,
    DI_Traffic_Incident_Full,
    DI_TotalCount
};

/*! DAM verify return value. */
enum NB_DamValue
{
    NDV_False = 0,   /*!< All indeices are false. */
    NDV_True,        /*!< All indeices are true. */
    NDV_Partial      /*!< Partial indeices are true and partial are false. */
};

class DAMUtility
{
public:

    DAMUtility() {}
    virtual ~DAMUtility() {}

    /*! This function will check the DAM tile which is locate at lat and lon
        by the damIndices.

    @return bool.
    */
    virtual bool IsDataAvailable(double latitude,    /*!< Latitude of the place to be verified.*/
                                 double longitude,   /*!< Latitude of the place to be verified.*/
                                 std::bitset<DI_TotalCount> damIndices   /*!< Combination of one or more DamIndex values. */
                                ) = 0;

    /*! This function will check the DAM tiles which are locate in the bounding box
        by the damIndices.

    @return NB_DamValue.
    */
    virtual NB_DamValue IsDataAvailable(double latitudeTopLeft,                        /*!< Left top latitude of the bounding box.*/
                                        double longitudeTopLeft,                       /*!< Left top longitude of the bounding box.*/
                                        double latitudeBottomRight,                    /*!< Right bottom latitude of the bounding box.*/
                                        double longitudeBottomRight,                   /*!< Right bottom longitude of the bounding box.*/
                                        std::bitset<DI_TotalCount> damIndices   /*!< Combination of one or more DamIndex values. */
                                       ) = 0;

    /*! This function will check the tiles by the damIndices.

    @return NB_DamValue.
    */
    virtual NB_DamValue IsDataAvailable(const std::vector<TileKeyPtr >& tileKeys,            /*!< Tile collection used to be checked. */
                                        std::bitset<DI_TotalCount> damIndices   /*!< Combination of one or more DamIndex values. */
                                       ) = 0;
};

typedef shared_ptr<DAMUtility> DAMUtilityPtr;

}; //namespace nbmap
#endif /* DAMUTILITY_H */

/*! @} */

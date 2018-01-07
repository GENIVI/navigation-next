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
   @file        DAMUtilityImpl.h
   @defgroup    nbmap

*/
/*
   (C) Copyright 2013 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */
#ifndef DAMUTILITYIMPL_H
#define DAMUTILITYIMPL_H

extern "C"
{
#include "palclock.h"
}

#include "NBPng.h"
#include "DAMUtility.h"
#include "TileKey.h"
#include "AsyncCallback.h"
#include "Tile.h"
#include "LayerManager.h"
#include "LamProcessor.h"
#include "UnifiedLayerListener.h"

namespace nbmap
{

const static int DAMCONTAINER_CAPACITY = 4;

/*! DAM cache in memory. */
struct DAMCacheEntry
{
    TileKeyPtr tileKey;        /*!< TileKey of DAM. */
    time_t     lastAccessTime; /*!< Last Access Time of this DAM. */
    PNG_Data*  decodeData;     /*!< Data struct contains decoded DAM.  */

    DAMCacheEntry()
        : lastAccessTime(0),
          decodeData(NULL)
    {
    }
};

class DamTileRequestCallback;

/*! DAM utility */
class DAMUtilityImpl : public DAMUtility,
                       public UnifiedLayerListener
{
public:
    DAMUtilityImpl(NB_Context* context, LayerManagerPtr layerManager);
    virtual ~DAMUtilityImpl();

    // See discription in DataAvailabilityMatrixInterface.h
    virtual bool IsDataAvailable(double latitude, double longitude, std::bitset<DI_TotalCount> damIndices);


    virtual NB_DamValue IsDataAvailable(double latitudeTopLeft,                        /*!< Left top latitude of the bounding box.*/
                                        double longitudeTopLeft,                       /*!< Left top longitude of the bounding box.*/
                                        double latitudeBottomRight,                    /*!< Right bottom latitude of the bounding box.*/
                                        double longitudeBottomRight,                   /*!< Right bottom longitude of the bounding box.*/
                                        std::bitset<DI_TotalCount> damIndices          /*!< Combination of one or more DamIndex values. */
                                       );

    virtual NB_DamValue IsDataAvailable(const std::vector<TileKeyPtr>& tileKeys,            /*!< Tile collection used to be checked. */
                                        std::bitset<DI_TotalCount> damIndices   /*!< Combination of one or more DamIndex values. */
                                       );

    /*! Store the dam tile decode data to memory cache.

     */
    void StoreDamToCache(TileKeyPtr tileKey, TilePtr tile);

    NB_Context* GetContext();

    // Inherited from UnifiedLayerListener
    virtual void UnifiedLayerUpdated();

private:
    // Private member functions .........................................................

    /*! This function will check the DAM tiles which are locate in the bounding box
        with point(x, y) of tile.

    @return NB_DamValue.
    */
    NB_DamValue IsDataAvailable(int xTopLeft,                                  /*!< Left top point of the bounding box.*/
                                int yTopLeft,                                  /*!< Left top point of the bounding box.*/
                                int xBottomRight,                              /*!< Right bottom point of the bounding box.*/
                                int yBottomRight,                              /*!< Right bottom point of the bounding box.*/
                                std::bitset<DI_TotalCount> damIndices          /*!< Combination of one or more DamIndex values. */
                               );

     /*! Convert lat and long to Tile's point(x, y). */
    void ConvertLatLongToTile(double latitude,  double longitude, int& xTile,  int& yTile);

    bool CheckDamIndex(int x, int y, int index);

    /*! Get reference zoom level of DAM from server. */
    int GetDAMReferenceZoomLevel();

    /*! Get DAM tile first check whether in memory cache,
     *  if not, need to get from persistent cache.
     */
    DAMCacheEntry* GetDamTileFromCache(TileKeyPtr tileKey);

    // Private member fields ............................................................
    shared_ptr<DamTileRequestCallback> m_pDamTileRequestcallback;    /*!< Dam tile request callback  */
    DAMCacheEntry m_damCacheEntry[DAMCONTAINER_CAPACITY];            /*!< Memory cache for decoded DAM tiles */
    LayerManagerPtr m_layerManager;                                  /*!< Pointer to layer manager */
    LamProcessor* m_lamProcessor;                                    /*!< Pointer to LamProcessor */
};

typedef shared_ptr<DAMUtilityImpl> DamUtilityImplPtr;

/*!Special callback used by Dam tile request. */
class DamTileRequestCallback : public AsyncCallbackWithRequest<TileKeyPtr, TilePtr>
{
public:
    DamTileRequestCallback(DAMUtilityImpl* damUtility);
    virtual ~DamTileRequestCallback() {}

private:
    /* See AsyncCallback.h for description */
    virtual void Success(TileKeyPtr request, TilePtr response);
    virtual void Error(TileKeyPtr request, NB_Error error);

    DAMUtilityImpl* m_damUtility;
};

typedef shared_ptr<DamTileRequestCallback> DamTileRequestCallbackPtr;

}; // namespace nbmap

#endif /* DAMUTILITYIMPL_H */

/*! @} */

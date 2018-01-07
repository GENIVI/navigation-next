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
   @file        LayerAvailabilityMatrixLayer.h
   @defgroup    nbmap

   Description: LayerAvailabilityMatrixLayer is a special layer which will
   download and parse Layer Availability Matrix (LAM)  tiles. Other layers
   that need LAM optimization will ask this layer for help to decide if a tile
   is needed to download or not.
*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */
#ifndef _LAYERAVAILABILITYMATRIXLAYER_H_
#define _LAYERAVAILABILITYMATRIXLAYER_H_

#include "UnifiedLayer.h"
#include <vector>
extern "C"
{
#include "NBPng.h"
#include "palclock.h"
}

namespace nbmap
{

#define  LAMCONTAINER_CAPACITY                          8

/*! Status of cached entry of LAM. */
typedef enum _LAM_ENTRY_STATE
{
    NB_LAM_State_Free = 0,      /*!< Free state, initialized, but not in use. */
    NB_LAM_State_Undergoing,    /*!< Undergoing state, being processed now. */
    NB_LAM_State_Ready,         /*!< Ready state, LAM info is parsed, can be used directly. */
} LamEntryState;

class UnifiedLayerWithLAM;


/*! TileRequestEntry is used to represent a tile request that needs LAM optimization. */
class TileRequestEntry
{
public:
    TileRequestEntry(TileKeyPtr tileKey,
                     TileKeyPtr convertedTileKey,
                     UnifiedLayerWithLAM* layer,
                     AsyncTileRequestWithRequestPtr callback,
                     uint32 priority,
                     shared_ptr<bool> valid);
    virtual ~TileRequestEntry();

    void StartRealDownload() const;
    void ReportError(NB_Error error) const;
    uint32 GetLayerAvailabilityMatrixIndex() const;
    TileKeyPtr GetDataTileKey() const;
    shared_ptr<string> GetTileDataType() const;
    bool IsLayerAvaiable() const;

private:

    TileKeyPtr                     m_tileKey;
    TileKeyPtr                     m_dataTileKey;
    UnifiedLayerWithLAM*           m_layer;    /*!< Layer to request a tile */
    AsyncTileRequestWithRequestPtr m_callback;
    uint32                         m_priority;
    shared_ptr<bool>               m_isValid;  /*!< Flag to show if this request is valid.*/
};

typedef shared_ptr<TileRequestEntry> TileRequestEntryPtr;

/*! LAM cache in memory. */
class LamCacheEntry
{
public:
    LamCacheEntry();
    ~LamCacheEntry();
    void Reset();

    TileKey                  m_tileKey; /*!< TileKey of LAM. */
    time_t                   m_lastAccessTime; /*!< Last Access Time of this LAM. */
    LamEntryState            m_state;   /*!< LAM cache state.  */
    PNG_Data*                m_decodedData; /*!< Data struct contains decoded LAM.  */
    vector<TileRequestEntry> m_requests;
};


class LamTileRequestCallback;
class LamProcessor;
/*! */
class LayerAvailabilityMatrixLayer : public UnifiedLayer
{
public:
    LayerAvailabilityMatrixLayer(TileManagerPtr tileManager,
                                 uint32 layerIdDigital,
                                 NB_Context* context);
    virtual ~LayerAvailabilityMatrixLayer();

    // Refer to UnifiedLayer for description.
    virtual void GetTiles(vector<TileKeyPtr> tileKey,    /*!< Tile key to request */
                          AsyncTileRequestWithRequestPtr callback,   /*!< Callback of tile request */
                          uint32 priority        /*!< Priority of tile request */
                          );

    void TileRequestSuccess(TileKeyPtr request, TilePtr response);
    void TileRequestError(TileKeyPtr request, NB_Error error);
    void GetTileWithLamOptimized(const TileRequestEntry& request);
    virtual std::string className() const { return "LayerAvailablityMatrixLayer"; }

private:

    // Private member functions ............................................................

    TileKey ConvertTileKeyToLamKey(TileKeyPtr tileKey);

    /*! Finds an cache entry for LAM tile.

      This function will try to find a usable Cache based on lamKey, if no free entry is
      found, it will try to reuse old ones.

      @return pointer of found entry, or NULL if failed.
    */
    LamCacheEntry*
    FindUsableLamCacheEntry(const TileKey& lamKey   /*!< Tile Key to be searched. */
                            );



    /*! Process cached tile requests which has been sotred in LamCacheEntry.

      This function will walk through all tile requests mapped to the same `lamTileKey`,
      move them to the tile requests queue identified by m_dummyTileKey, and then start to
      download them.

      @return None.
    */
    void
    ProcessRequestsInCacheEntry(LamCacheEntry* entry   /*!< Tile key of LAM. */
                                );

    bool IsRequestDownloadable(PNG_Data* decodedData,
                               const TileRequestEntry& request);

    // Private member fields ............................................................

    LamCacheEntry m_pLamCache[LAMCONTAINER_CAPACITY]; /*!< Memory cache for decoded LAM tiles,
                                                           Number of cache should be limit to a
                                                           fixed small value, thus an array is
                                                           enough. */
    map<TileKey, LamCacheEntry*> m_dirtyEntries;      /*!< Collection of dirty entries, ie,
                                                           the Caches who's status is READY
                                                           or UnderGoing. */
    shared_ptr<LamTileRequestCallback> m_pLamTileRequestCallback;

    LamProcessor* m_lamProcessor;
};

typedef shared_ptr<LayerAvailabilityMatrixLayer> LamLayerPtr;


/*! Special callback used by LayerAvailabilityMatrixLayer. */
class LamTileRequestCallback : public AsyncCallbackWithRequest<TileKeyPtr, TilePtr>,
                               public Base
{
public:
    LamTileRequestCallback(LayerAvailabilityMatrixLayer* lamLayer);
    virtual ~LamTileRequestCallback();

    void Reset();

    /* See AsyncCallback.h for description */
    virtual void Success(TileKeyPtr request, TilePtr response);
    virtual void Error(TileKeyPtr request, NB_Error error);

private:
    LayerAvailabilityMatrixLayer* m_pLamLayer;
};

typedef shared_ptr<LamTileRequestCallback> LamTileRequestCallbackPtr;

}

#endif /* _LAYERAVAILABILITYMATRIXLAYER_H_ */

/*! @} */

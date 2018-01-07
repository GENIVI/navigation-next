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

    @file       OnboardTileManager.h

    Class OnboardTileManager inherits from TileManager interface. An
    OnboardTileManager object is used to request NBM tiles by using NCDB library
    without any network connections.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef ONBOARDTILEMANAGER_H
#define ONBOARDTILEMANAGER_H

#include "useonboard.h"

/*!
    @addtogroup nbmap
    @{
*/

extern "C"
{
    #include "nbcontext.h"
}

#include "workerqueuemanager.h"
#include "commoncomponentconfiguration.h"
#include "NBMTile.h"
#include "TileImpl.h"
#include "TileManager.h"
#include "TileType.h"
#include "UnifiedTileManager.h"

#include <vector>
#include <deque>
#include <map>
#include <string>

using namespace Ncdb;
using namespace std;
using namespace nbcommon;

namespace nbmap
{

// Constants ....................................................................................
static const char ID_PLACEHOLDER[] = "$lid";  // @todo: what placeholder will be used for "LayerID"

const int LAYER_IDS[] = {NBMTile::LAYER_ID_ROAD,
                         NBMTile::LAYER_ID_AREA,
                         NBMTile::LAYER_ID_UNTEXTUREONLY,
                         NBMTile::LAYER_ID_LANDMARK,
                         NBMTile::LAYER_ID_ROAD_AREA,
                         NBMTile::COMMON_MATERIAL_DAY,
                         NBMTile::COMMON_MATERIAL_NIGHT,
                         NBMTile::LAYER_NBM_LAM_PNG};
static const int LAM_INDEX_ID = 7;

// Types ........................................................................................
const string CACHE_TYPE[] = {
    "BMAT",
    "NTHM",
    "DTHM",
    "STHM",
    "PBMAT",
    "DVR",
    "DVA",
    "B3D",
    "LM3D",
    "B2D",
    "BR",
    "LBL",
    "POI"
};

const NBMTile::FileTypesEnum NBMFILE_TYPE[] = {
    NBMTile::NBM_MBASEBMP,
    NBMTile::NBM_MNIGHTTHEME,
    NBMTile::NBM_MDAYTHEME,
    NBMTile::NBM_MSATELLITETHEME,
    NBMTile::NBM_MPOI,
    NBMTile::NBM_DVR,
    NBMTile::NBM_DVA,
    NBMTile::NBM_3DUT,
    NBMTile::NBM_3DLM,
    NBMTile::NBM_B2D,
    NBMTile::NBM_RAST,
    NBMTile::NBM_LBLT,
    NBMTile::NBM_POIS
};
class TileManagerTask;             /* See source file for description */

typedef shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > TileCallback;

/*! Tile Data, used for pending and requested tile vectors, contain all related with single tile data */
class OnboardTileData: public Base
{
public:
    // Public functions .........................................................................

    /*! OnboardTileData constructor */
    OnboardTileData():
                x(0),
                y(0),
                z(0),
                mode(0),
                result(NE_BADDATA),
                priority(0)
    { };

    /*! OnboardTileData destructor */
    ~OnboardTileData() { };

    bool operator<(const OnboardTileData& b) const
    {
        return (priority > b.priority);
    }

    bool operator==(const OnboardTileData& b) const
    {
        return (x == b.x && y == b.y && z == b.z && mode == b.mode);
    }

    unsigned int                          x;        /*!< x coordinate */
    unsigned int                          y;        /*!< y coordinate */
    unsigned int                          z;        /*!< Zoom level */
    unsigned int                          mode;     /*!< 0 is day, otherwise is night */
    NB_Error                              result;   /*!< Request result */
    shared_ptr<TileImpl>                  tile;     /*!< Tile object with ID and PNG data */
    TileCallback                          callback; /*!< User provided callback */
    shared_ptr<std::map<std::string, std::string> > templateParameters;    /*!< String map provided by user */
    uint32                                priority;
};

/*! Tile manager for getting tiles by NCDB library from local database */
class OnboardTileManager : public UnifiedTileManager
{
public:
    // Public functions .........................................................................

    /*! OnboardTileManager constructor */
    OnboardTileManager();

    /*! OnboardTileManager destructor */
    virtual ~OnboardTileManager();

    /*! Initialize an OnboardTileManager object
        This function should be called before user requests or cancels tiles.
        @return NE_OK if success
    */
    virtual NB_Error Initialize(NB_Context* context,
                                shared_ptr<protocol::UrlArgsTemplate> urlArgsTemplate,
                                bool compressed,
                                uint32 retryDelay,
                                uint32 maximumTileRequestCount,
                                UnifiedTileTypePtr tileType,
                                TileLayerInfoPtr info,
                                shared_ptr<string> clientGuid
                                );

    virtual void SetTileConverter(shared_ptr<TileConverter> tileConverter);

    /* See description in TileManager.h */
    virtual void GetTile(shared_ptr<std::map<std::string, std::string> > templateParameters, uint32 priority);
    virtual void GetTile(shared_ptr<std::map<std::string, std::string> > templateParameters,
                             shared_ptr<AsyncCallbackWithRequest<shared_ptr<std::map<std::string, std::string> >, TilePtr> > callback,
                             uint32 priority,
                             bool fastLoadOnly = false);
    virtual void RemoveTile(shared_ptr<std::map<std::string, std::string> > templateParameters);
    virtual void RemoveAllTiles();
    virtual NB_Error SetCommonParameterMap(shared_ptr<std::map<std::string, std::string> > commonParameters);
    virtual void UpdateTileLayerInfo(TileLayerInfoPtr info);
    virtual std::string str() const;

private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    OnboardTileManager(const OnboardTileManager& tileManager);
    OnboardTileManager& operator=(const OnboardTileManager& tileManager);

    /* See source file for description */
    bool IsInitialized() const;
    void Reset();
    void Clear();

    NB_Error ExtractTileInfo(shared_ptr<std::map<std::string, std::string> > templateParameters,
                             unsigned int* x,
                             unsigned int* y,
                             unsigned int* z,
                             unsigned int* mode);

    void SetSession(shared_ptr<Ncdb::Session> session);
    bool FindEntry(std::vector<OnboardTileData>& vector, OnboardTileData& entry, bool remove);

    NB_Error SheduleTileRequest(const OnboardTileData& information);

    void NcdbTileRequest();
    void SendCallback();
    void NcdbRequestCompleted();
    bool ProcessNextPendingTile();

    shared_ptr<std::string> GetContentId(shared_ptr<std::map<string, string> > templateParameters);

    bool FindFailedTile(OnboardTileData& failedTile, bool addToVector);
    NBMTile::NbmFileTypes GetNbmFileTypeByType(shared_ptr<std::string> type);

private:
    NB_Context*                     m_context;             /*!< Pointer to current context */
    PAL_Instance*                   m_palInstance;         /*!< Pointer to current PAL */

    shared_ptr<Ncdb::NBMTile>              m_tiler;        /*!< Pointer NCDB object, responsible for building NBM tiles */

    shared_ptr<WorkerQueueManager>         m_manager;      /*!< Pointer to Manager, responsible for getting NCDB session and event/worker queues*/
    shared_ptr<WorkerTaskQueue>            m_workQueue;    /*!< Pointer to worker task queue */
    shared_ptr<EventTaskQueue>             m_eventQueue;   /*!< Pointer to event task queue */
    TileManagerTask*                       m_workerTask;   /*!< Pointer to task, excecutes in worker thread (NCDB request) */
    TileManagerTask*                       m_eventTask;    /*!< Pointer to task, excecutes in event thread (User callbacks) */

    //int                                m_layerID;                 /*!< Layer ID for tiles to be extracted */

    std::vector<OnboardTileData>       m_pendingTiles;            /*!< Vector of pending tiles */
    OnboardTileData                    m_requestedTile;           /*!< Vector of tiles being requested */

    bool                               m_requestIsInProgress;     /*!< Is request in progress */

    TileTypePtr                        m_tileType;
    NBMTile::NbmFileTypes              m_nbmFileType;                /*!< Saved tile type, provided by LayerManager */

    std::deque<OnboardTileData>        m_failedTiles;             /*!< Failed tiles shouldn't be requested again */
    TileLayerInfoPtr                   m_tileLayerInfo;           /*!< Common information shared by tiles of same layer.*/
};

typedef shared_ptr<OnboardTileManager > OnboardTileManagerPtr;

}  // namespace nbmap

/*! @} */

#endif  // ONBOARDTILEMANAGER_H

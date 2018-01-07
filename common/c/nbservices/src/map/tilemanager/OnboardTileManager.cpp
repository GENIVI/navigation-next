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

    @file       OnboardNBMTileManager.cpp

    See header file for description.
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

#include "useonboard.h"

extern "C"
{
    #include "nbcontextprotected.h"
    #include "nbspatial.h"
    #include "palfile.h"
}

#include "datastreamimplementation.h"
#include "OnboardTileManager.h"
#include "StringUtility.h"
#include "pallock.h"
#include "palclock.h"
#include <algorithm>

/*! @{ */

using namespace nbmap;

// Local constants
static const int  TILE_DATA_FOUND = 7;                          /*!< 111 mask */
static const int  BUFFER_SIZE = 4096;
static const int  FAILED_TILES_COUNT = 50;

static const unsigned short NBM_VERSION_MAJOR = 24;
static const unsigned short NBM_VERSION_MINOR = 0;

// following strings used in 'templateParameter' map to define requested tile coordinates
static const char X_PLACEHOLDER[] = "$x";
static const char Y_PLACEHOLDER[] = "$y";
static const char Z_PLACEHOLDER[] = "$z";
static const char TOD_PLACEHOLDER[] = "$tod";
static const char TOD_NIGHT[] = "night";

///////////////////////////////////////////
// class TileManagerTask;
////////////////////////////////////////////
typedef void (OnboardTileManager::*TaskFunction)(void);

/* Task provided into task queues, used just to invoke OnboardTileManager method in choosen queue (event/worker) */
class nbmap::TileManagerTask : public Task
{
public:
    // Public functions .........................................................................

    /*! TileManagerTask constructor */
    TileManagerTask(OnboardTileManager* handler, TaskFunction func, PAL_Instance* pal)
                         :m_handler(handler),
                          m_func(func),
                          m_invokedCount(0)
    {
        PAL_LockCreate(pal, &m_lock);
    }

    /*! TileManagerTask destructor */
    ~TileManagerTask()
    {
        PAL_LockDestroy(m_lock);
    }

    void UnsetHandler()
    {
        PAL_LockLock(m_lock);
        m_handler = NULL;
        PAL_LockUnlock(m_lock);
    }

    void IncreaseInvoked()
    {
        PAL_LockLock(m_lock);
        ++m_invokedCount;
        PAL_LockUnlock(m_lock);
    }

    /*! Method, invocked then task is executed

        @return nothing
    */
    virtual void Execute(void)
    {
        PAL_LockLock(m_lock);
        if(m_handler)
        {
            (m_handler->*m_func)();
        }

        DecreaseInvoked();
        // only destory task after hander was reset
        if(m_invokedCount <= 0 && !m_handler)
        {
            PAL_LockUnlock(m_lock);
            delete this;
            return;
        }

        PAL_LockUnlock(m_lock);
    }

private:
    void DecreaseInvoked()
    {
        --m_invokedCount;
    }

private:
    OnboardTileManager*   m_handler;    /*!< Pointer to OnboardTileManager */
    TaskFunction          m_func;       /*!< Pointer to OnboardTileManager's method */
    PAL_Lock*             m_lock;
    int                   m_invokedCount;
};

// OnboardTileManager Public functions .........................................................

/* See header file for description */
OnboardTileManager::OnboardTileManager() : UnifiedTileManager(),
                                                 m_context(NULL),
                                                 m_palInstance(NULL),
                                                 m_workerTask(NULL),
                                                 m_eventTask(NULL),
                                                 m_requestIsInProgress(false),
                                                 m_tileType(TileTypePtr()),
                                                 m_nbmFileType(NBMTile::NBM_DVR)
{
}

/* See header file for description */
OnboardTileManager::~OnboardTileManager()
{
    Reset();
}

NB_Error OnboardTileManager::Initialize(NB_Context* context,
                                        shared_ptr<protocol::UrlArgsTemplate> /*urlArgsTemplate*/,
                                        bool /*compressed*/,
                                        uint32 /*retryDelay*/,
                                        uint32 /*maximumTileRequestCount*/,
                                        UnifiedTileTypePtr tileType,
                                        TileLayerInfoPtr info,
                                        shared_ptr<string> /*clientGuid*/)
{
    if (!context)
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (IsInitialized())
    {
        return NE_EXIST;
    }

    Reset();

    // Initialize the members of this object.
    m_context = context;
    m_tileType = tileType;
    m_nbmFileType = GetNbmFileTypeByType(tileType->GetDataType());
    m_tileLayerInfo = info;
    m_palInstance = NB_ContextGetPal(context);
    m_workerTask = new TileManagerTask(this, &OnboardTileManager::NcdbTileRequest, m_palInstance);
    m_eventTask =  new TileManagerTask(this, &OnboardTileManager::NcdbRequestCompleted, m_palInstance);

    m_manager = CommonComponentConfiguration::GetWorkerQueueManagerComponent(m_context);

    if (m_manager)
    {
        m_workQueue = m_manager->RetrieveCommonWorkerTaskQueue();
    }

    m_eventQueue = shared_ptr<EventTaskQueue>(new EventTaskQueue(m_palInstance));

    CommonComponentConfiguration::GetCommonNBMTile(m_context, m_tiler);

    return NE_OK;
}

void OnboardTileManager::SetTileConverter(shared_ptr<TileConverter> /*tileConverter*/)
{
}


/* See description in TileManager.h */
void
OnboardTileManager::GetTile(shared_ptr<map<string, string> > templateParameters, uint32 priority)
{
    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return;
    }

    GetTile(templateParameters,
            shared_ptr<AsyncCallbackWithRequest<shared_ptr<std::map<std::string, std::string> >, TilePtr> >(),
            priority);
}

/* See description in TileManager.h */
void
OnboardTileManager::GetTile(shared_ptr<map<string, string> > templateParameters,
                            shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > callback,
                            uint32 priority, bool fastLoadOnly)
{
    NB_Error error = NE_OK;
    OnboardTileData currentTileData;

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return;
    }

    if (fastLoadOnly)
    {
        // fastLoadOnly means that no request should be scheduled
        // return here because if tile is in the cache - it's was returned above
        if (callback)
        {
            callback->Error(templateParameters, NE_NOENT);
        }
        return;
    }

    // check - is template parameters contain all requried data
    error = ExtractTileInfo(templateParameters, &currentTileData.x, &currentTileData.y, &currentTileData.z, &currentTileData.mode);
    if (error != NE_OK)
    {
        return;
    }

    currentTileData.result = NE_BADDATA;
    currentTileData.templateParameters = templateParameters;
    currentTileData.priority = priority;
    currentTileData.callback = callback;
/*TODO:need to check
    // Firstly, we check - is this tile already requsted
    if (currentTileData.x == m_requestedTile.x &&
        currentTileData.y == m_requestedTile.y &&
        currentTileData.z == m_requestedTile.z)
    {
        // already requested
        return;
    }
*/
    if (!m_requestIsInProgress) //if no outstanding request we can start new one
    {
        SheduleTileRequest(currentTileData);
        return;
    }

    if (FindFailedTile(currentTileData, false))
    {
        // Tile cannot be downloaded due to NCDB error, so we invokes callback with standard error code.
        if (callback)
        {
            callback->Error(templateParameters, NE_BADDATA);
        }
        return;
    }

    //add tile to pending vector
    FindEntry(m_pendingTiles, currentTileData, true);
    m_pendingTiles.push_back(currentTileData);

    return;
}

/* See description in TileManager.h */
void
OnboardTileManager::RemoveTile(shared_ptr<map<string, string> > templateParameters)
{
    OnboardTileData currentTileData;

    // Check if this object is initialized.
    if (IsInitialized())
    {
        if (NE_OK == ExtractTileInfo(templateParameters, &currentTileData.x, &currentTileData.y, &currentTileData.z, &currentTileData.mode))
        {
            // remove from pending
            FindEntry(m_pendingTiles, currentTileData, true);
        }
    }
}

/* See description in TileManager.h */
void
OnboardTileManager::RemoveAllTiles()
{
    // Check if this object is initialized.
    if (IsInitialized())
    {
        // Clear all pending tiles
        m_pendingTiles.clear();
    }
}

/* See description in TileManager.h */
NB_Error
OnboardTileManager::SetCommonParameterMap(shared_ptr<std::map<std::string, std::string> > /*commonParameters*/)
{
    // We should improve this method in the future
    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    //not needed for onboard

    return NE_OK;
}

void OnboardTileManager::UpdateTileLayerInfo(TileLayerInfoPtr info)
{
    m_tileLayerInfo = info;
}

std::string OnboardTileManager::str() const
{
    std::string s = "(OnboardTM)";
    return s;
}

// OnboardTileManager Private functions ........................................................

/*! Check if this OnboardTileManager object is initialized

    This function should be called in all public functions of class OnboardTileManager.

    @return Flag if this OnboardTileManager object is initialized
*/
bool
OnboardTileManager::IsInitialized() const
{
    if (m_context && m_palInstance && m_workQueue && m_eventQueue)
    {
        return true;
    }

    return false;
}

/*! Reset an OnboardTileManager object to be uninitialized

    Clear all members in this OnboardTileManager object.

    @return None
*/
void
OnboardTileManager::Reset()
{
    // Clear all pending requests.
    m_pendingTiles.clear();
    m_failedTiles.clear();

    if(m_workerTask)
    {
        m_workerTask->UnsetHandler();
        m_workerTask = NULL;
    }
    if(m_eventTask)
    {
        m_eventTask->UnsetHandler();
        m_eventTask = NULL;
    }

    // Clear all members in this OnboardTileManager object.
    m_context = NULL;
    m_palInstance = NULL;

    m_requestIsInProgress = false;

}


/*! Extract tile coordinates from template parameters map

    @return NE_OK if all coordinates extracted, NE_INVAL otherwise
*/
NB_Error
OnboardTileManager::ExtractTileInfo(shared_ptr<std::map<std::string, std::string> > templateParameters,  /*!< Parameters map passed by user */
                                       unsigned int* x,  /*!< out: x coordinate */
                                       unsigned int* y,  /*!< out: y coordinate  */
                                       unsigned int* z,  /*!< out: zoom level  */
                                       unsigned int* mode/*!< out: day/night mode  */
                                       )
{
    int result = 0;

    if (!x || !y || !z)
    {
        return NE_INVAL;
    }

    map<string, string>::const_iterator iterator = templateParameters->begin();
    map<string, string>::const_iterator end = templateParameters->end();

    for ( ; iterator != end; ++iterator)
    {
        const string& key = iterator->first;
        const string& value = iterator->second;

        if (key.compare(X_PLACEHOLDER) == 0)
        {
            *x = nsl_atoi(value.c_str());
            result |= 1 << 0;
        }
        else if (key.compare(Y_PLACEHOLDER) == 0)
        {
            *y = nsl_atoi(value.c_str());
            result |= 1 << 1;
        }
        else if (key.compare(Z_PLACEHOLDER) == 0)
        {
            *z = nsl_atoi(value.c_str());
            result |= 1 << 2;
        }
        else if (key.compare(TOD_PLACEHOLDER) == 0)
        {
            if(value.compare(TOD_NIGHT) == 0)
            {
                *mode = 1;
            }
        }
    }

    if (result == TILE_DATA_FOUND)
    {
        return NE_OK;
    }
    else
    {
        return NE_INVAL;
    }
}

/*! Find entry in passed vector

    @return true if entry found, false otherwise
*/
bool OnboardTileManager::FindEntry(std::vector<OnboardTileData>& vector,    /*!< vector to search in */
                                      OnboardTileData& entry,                  /*!< entry to search */
                                      bool remove                                /*!< should be found entry deleted */
                                     )
{
    std::vector<OnboardTileData>::iterator iterator = vector.begin();
    std::vector<OnboardTileData>::const_iterator end =      vector.end();

    for ( ; iterator != end; ++iterator)
    {
        if ((*iterator).x == entry.x &&
            (*iterator).y == entry.y &&
            (*iterator).z == entry.z &&
            (*iterator).mode == entry.mode)
        {
            if (remove)
            {
                vector.erase(iterator);
            }
            return true;
        }
    }

    return false;
}

/*! Shedules new request to NCDB into worker queue

    @return NE_OK if request succesfully sheduled, NE_INVAL otherwise
*/
NB_Error
OnboardTileManager::SheduleTileRequest(const OnboardTileData& information  /*!< pointer to tiles data array */
                                          )
{
    m_requestedTile = information;
    m_requestIsInProgress = true;

    m_workerTask->IncreaseInvoked();
    m_workQueue->AddTask(m_workerTask);

    return NE_OK;
}

/*! Function performs NCDB request

    Perform all NCDB related operations and scheduling user callbacks inokation
    NOTE: always called in worker queue

    @return nothing
*/
void
OnboardTileManager::NcdbTileRequest()
{
    ReturnCode ncdbRet = NCDB_OK;

    if (!m_tiler)
    {
        NB_Error res = CommonComponentConfiguration::GetCommonNBMTile(m_context, m_tiler);
        if(res == NE_AGAIN)
        {
            m_workerTask->IncreaseInvoked();
            m_workQueue->AddTask(m_workerTask);
            return;
        }
        else if (res != NE_OK)
        {
            // something goes really wrong
            SendCallback();
            return;
        }
    }

    // setting parameters for Ncdb
    m_tiler->setNBMVersion(NBM_VERSION_MAJOR, NBM_VERSION_MINOR);
    m_tiler->SetExonym(true);
    m_tiler->EnableLOD(true);
    m_tiler->setTile(m_requestedTile.x, m_requestedTile.y, m_requestedTile.z);

    if((NBMTile::NBM_RAST==m_nbmFileType) && (0!=m_requestedTile.mode))
    {
        //night mode
        ncdbRet = m_tiler->extract(NBMTile::NBM_RAST_NIGHT);
    }
    else
    {
        ncdbRet = m_tiler->extract(m_nbmFileType);
    }
//    TEST_LOGGING("OnboardTileManager::NcdbRequest result: %u", ncdbRet);

    if (ncdbRet == NCDB_OK)
    {
        const char* buffer = NULL;
        int bufferLength = 0;

        m_tiler->getTileBuffer(buffer, bufferLength);
        if (buffer != NULL && bufferLength > 0)
        {
            DataStreamPtr dataStream = DataStreamPtr(new DataStreamImplementation());
            shared_ptr<std::string> contentId = GetContentId(m_requestedTile.templateParameters);

            dataStream->AppendData(reinterpret_cast<const uint8*>(buffer), bufferLength);

            m_requestedTile.tile = shared_ptr<TileImpl>(new TileImpl(contentId, m_tileLayerInfo));
            m_requestedTile.tile->SetData(dataStream);
            m_requestedTile.result = NE_OK;
        }
    }

    SendCallback();
}

/*! Sheduling user callback invokation for all requested tiles at once

    @return nothing
*/
void
OnboardTileManager::SendCallback()
{
    m_eventQueue->AddTask(m_eventTask);
}

/*! User callback invokation

    Invokes user callbacks for all requested tiles at once
    NOTE: always called in event queue

    @return nothing
*/
void
OnboardTileManager::NcdbRequestCompleted()
{
    if (m_requestedTile.result == NE_OK)
    {
        if (m_requestedTile.callback)
        {
            m_requestedTile.callback->Success(m_requestedTile.templateParameters, m_requestedTile.tile);
        }
    }
    else
    {
        FindFailedTile(m_requestedTile, true);
        
        if (m_requestedTile.callback)
        {
            m_requestedTile.callback->Error(m_requestedTile.templateParameters, m_requestedTile.result);
        }
    }

    m_requestedTile.templateParameters.reset();
    m_requestedTile.callback.reset();
    m_requestedTile.tile.reset();

    m_requestIsInProgress = false;

    while(!ProcessNextPendingTile());
}


/*! Processing pending tiles

    Sheduling next request if there is any pending tiles

    @return true if here is no any pending tiles to be processed
*/
bool
OnboardTileManager::ProcessNextPendingTile()
{
    bool retStatus = true;
    NB_Error result = NE_OK;

    if (m_pendingTiles.size() == 0)
    {
        return true;
    }

    std::sort(m_pendingTiles.begin(), m_pendingTiles.end());

    OnboardTileData currentTileData = m_pendingTiles.back();
    m_pendingTiles.pop_back();

    result = SheduleTileRequest(currentTileData);

    if (NE_OK != result)
    {
        if (currentTileData.callback)
        {
            currentTileData.callback->Error(currentTileData.templateParameters, NE_NET);
        }
        retStatus = false;
    }

    return retStatus;
}

shared_ptr<std::string>
OnboardTileManager::GetContentId(shared_ptr<std::map<string, string> > templateParameters)
{
    if (!m_tileType)
    {
        return shared_ptr<std::string>();
    }

    shared_ptr<std::string> contentId;
    // Get content ID template.
    std::string strContentID("");
    shared_ptr<std::string> contentIdTemplate = m_tileType->GetContentIDTemplate();
    if(contentIdTemplate)
    {
        strContentID = *contentIdTemplate;
    }

    // Traverse the common parameter map and replace key with value in content ID template.
    if (templateParameters)
    {
        map<string, string>::const_iterator iterator = templateParameters->begin();
        map<string, string>::const_iterator end = templateParameters->end();

        for (; iterator != end; ++iterator)
        {
            const string& key = iterator->first;
            const string& value = iterator->second;

            StringUtility::ReplaceString(strContentID, key, value);
        }
    }

    contentId.reset(new std::string(strContentID));
    return contentId;
}


bool
OnboardTileManager::FindFailedTile(OnboardTileData& failedTile, bool addToVector)
{
    int size = m_failedTiles.size();
    bool found = false;

    for (int i = size - 1; i >= 0; i--)
    {
        if (m_failedTiles.at(i) == failedTile)
        {
            found = true;
            break;
        }
    }

    if (!found && addToVector) // we should add new tile only if it not already is in the vector
    {
        if (size >= FAILED_TILES_COUNT)
        {
            m_failedTiles.pop_front();
        }

        m_failedTiles.push_back(failedTile);
    }

    return found;
}

NBMTile::NbmFileTypes OnboardTileManager::GetNbmFileTypeByType(shared_ptr<std::string> type)
{
    NBMTile::NbmFileTypes nbmFileType = NBMTile::NBM_DVR;
    string strType = *type;
    bool found = false;

    // two passes: first pass is exact match, second pass is substring match
    // reason: PBMAT will erroneously substring match BMAT

    for(unsigned int i = 0; i < sizeof(CACHE_TYPE)/sizeof(CACHE_TYPE[0]); i++)
    {
        if(CACHE_TYPE[i] == strType)
        {
            found = true;
            nbmFileType = NBMFILE_TYPE[i];
            break;
        }
    }
    if (found) return nbmFileType;

    for(unsigned int i = 0; i < sizeof(CACHE_TYPE)/sizeof(CACHE_TYPE[0]); i++)
    {
        if(strType.find(CACHE_TYPE[i]) != string::npos)
        {
            found = true;
            nbmFileType = NBMFILE_TYPE[i];
            break;
        }
    }
//    if (!found) qDebug() << "ONBOARD OnboardTileManager::GetNbmFileTypeByType(" << type->c_str() << ") not found";
    return nbmFileType;
}

/*! @} */

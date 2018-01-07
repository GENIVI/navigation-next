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

    @file     ncdbobjectsmaanger.cpp
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "useonboard.h"

#include "ncdbobjectsmanager.h"
#include "commoncomponentconfiguration.h"

#include "nbcontextprotected.h"
#include "DataBaseProperties.h"

// Enable QT Debug
//#define DNCDB

#ifdef DNCDB
#include <QDebug>
#include <pthread.h>
#endif DNCDB

extern "C"{
    #include "palfile.h"
}

// Max length for NCDB session path
static const int   MAXIMUM_SESSION_LENGTH   = 256;
static const int   NCDB_OBJECT_REQUEST_TASK_PRIORITY = WORKER_TASK_MAX_USER_PRIORITY;
static const int   NCDB_OBJECT_REQUEST_TASK_OPENSESSION = 20;
static bool killed = true;

using namespace Ncdb;
using namespace nbcommon;
//-----------------------------------------------------------
// class Task for Session initializing
typedef void (NcdbObjectsManager::*TaskFunction)();

class nbcommon::SessionTask: public Task
{
public:
    SessionTask(NcdbObjectsManager* manager, TaskFunction func)
                         :m_manager(manager), m_func(func)
    {}
    ~SessionTask(){}
    virtual void Execute(void){if (!killed){(m_manager->*m_func)();} delete this;}

private:
    NcdbObjectsManager*  m_manager;
    TaskFunction         m_func;
};
//------------------------------------------------------


// Public methods -----------------------------------------------------------------------
NcdbObjectsManager::NcdbObjectsManager(NB_Context* context):
                             m_context(context),
                             m_lock(NULL),
                             m_skipSessionCheck(false)
{
    PAL_LockCreate(NB_ContextGetPal(m_context), &m_lock);
    PAL_LockLock(m_lock);
    killed = false;
    PAL_LockUnlock(m_lock);

    m_queueManager = CommonComponentConfiguration::GetWorkerQueueManagerComponent(m_context);
    m_thread     = m_queueManager->RetrieveCommonWorkerTaskQueue();

    m_sessionRequested = false;
    m_sessionFailed= false;
    m_loadingMapdata = false;
}

NcdbObjectsManager::~NcdbObjectsManager()
{
    PAL_LockLock(m_lock);
    killed = true;
    PAL_LockUnlock(m_lock);

    PAL_LockDestroy(m_lock);
}

void
NcdbObjectsManager::Initialize()
{
    m_skipSessionCheck = true;

    shared_ptr<Ncdb::NBMTile>           nbmTile;
    GetNBMTile(nbmTile);

    m_skipSessionCheck = false;
}

void NcdbObjectsManager::AddMapdataPath(std::string path)
{
    m_mapdataPath.push_back(path);
}

void NcdbObjectsManager::AddWorldMapdataPath(std::string path)
{
    m_worldMapdataPath.push_back(path);
}

NB_Context*
NcdbObjectsManager::GetContext()
{
    return m_context;
}

void NcdbObjectsManager::BeginLoadMapdata()
{
    m_loadingMapdata = true;
}

void NcdbObjectsManager::EndLoadMapdata()
{
    RequestSession();
    m_loadingMapdata = false;
}

NB_Error
NcdbObjectsManager::GetNBMTile(shared_ptr<Ncdb::NBMTile>& nbmTile)
{
    if (m_sessionFailed || m_loadingMapdata)
    {
        return NE_INVAL;
    }

    if (!m_skipSessionCheck && m_sessionRequested)
    {
        return NE_AGAIN;
    }

    if (m_nbmTile)
    {
        nbmTile = m_nbmTile;
        return NE_OK;
    }

    SessionTask* workerTask = new SessionTask(this, &NcdbObjectsManager::CreateNBMTile);

    if (!workerTask)
    {
        return NE_NOMEM;
    }

    if (PAL_Ok != m_thread->AddTask(workerTask, NCDB_OBJECT_REQUEST_TASK_PRIORITY))
    {
        delete workerTask;
        return NE_TASK_QUEUE_FAILED;
    }

    return NE_AGAIN;
}

void
NcdbObjectsManager::CreateNBMTile()
{
    if (m_session && !m_nbmTile && !m_loadingMapdata)
    {
        PAL_LockLock(m_lock);
        m_nbmTile = shared_ptr<Ncdb::NBMTile>(new NBMTile(*(m_session), false));
        PAL_LockUnlock(m_lock);
    }
}

void
NcdbObjectsManager::CreateSession()
{
    if (!m_session)
    {
#ifdef DNCDB
        qWarning() << "NCDB" << pthread_self() << "Enter NcdbObjecsManager::CreateSession";
#endif
        PAL_LockLock(m_lock);

        m_session = shared_ptr<Session>(CreateMapSession(), &SessionDeleter);
        m_sessionRequested = false;

        if (!m_session)
        {
            m_sessionFailed = true;
        }

        PAL_LockUnlock(m_lock);
#ifdef DNCDB
        qWarning() << "NCDB" << pthread_self() << " Exit NcdbObjecsManager::CreateSession";
#endif
    }
}

void
NcdbObjectsManager::RequestSession()
{
    if (!killed && !m_session)
    {
        SessionTask* sessionTask = new SessionTask(this, &NcdbObjectsManager::CreateSession);
        if (!sessionTask)
        {
            m_sessionRequested = false;
        }
        else if (PAL_Ok != m_thread->AddTask(sessionTask, NCDB_OBJECT_REQUEST_TASK_OPENSESSION))
        {
            delete sessionTask;
            m_sessionRequested = false;
        }
        else
        {
#ifdef DNCDB
            qWarning() << "NCDB" << pthread_self() << " NcdbObjecsManager::RequestSession";
#endif
            m_sessionRequested = true;
        }
    }
}

Session*
NcdbObjectsManager::CreateMapSession()
{
    Ncdb::Session* session = new Session();
    session->SetInitOptions(SESSION_INIT_DEFER_NBM|SESSION_INIT_DEFER_TRAFFIC);
    for(size_t i=0; i<m_mapdataPath.size(); ++i)
    {
        const char* sessionPath = m_mapdataPath[i].c_str();
#ifdef DNCDB
        qWarning() << "NCDB" << pthread_self() << " Begin " << sessionPath;
#endif
        if (session->Add(sessionPath) != NCDB_OK)
        {
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NcdbObjectsManager, failed to load mapdata from %s", sessionPath);
        }
#ifdef DNCDB
        qWarning() << "NCDB" << pthread_self() << " End " << sessionPath;
#endif
    }

    for(size_t i=0; i<m_worldMapdataPath.size(); ++i)
    {
        const char* sessionPath = m_worldMapdataPath[i].c_str();
#ifdef DNCDB
        qWarning() << "NCDB" << pthread_self() << " Begin " << sessionPath;
#endif
        if (session->OpenWorld(sessionPath) != NCDB_OK)
        {
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NcdbObjectsManager, failed to load world mapdata from %s", sessionPath);
        }
#ifdef DNCDB
        qWarning() << "NCDB" << pthread_self() << " End " << sessionPath;
#endif
    }

    if(!session->IsOpened()){
        delete session;
        session = NULL;
    }

    return session;
}

void
NcdbObjectsManager::SessionDeleter(void* pointer)
{
    Session* session = static_cast<Session*>(pointer);
    if (session)
    {
        session->Close();
        delete session;
    }
}


const char*
NcdbObjectsManager::GetMapVersionString()
{
    if (!m_session)
        return "";

    int nMaps = m_session->GetNumMaps();

    for (int i = 0; i < nMaps; i++) {
        const Ncdb::DatabaseProperties * db = m_session->GetDatabaseProperties(i);
        if (db) {
           if (i != 0) m_mapVersion.append(":");
           m_mapVersion.append(db->MapBuildId());
        }
    }

    return m_mapVersion.c_str();
}

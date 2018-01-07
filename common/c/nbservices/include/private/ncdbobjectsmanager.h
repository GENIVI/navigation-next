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

@file     ncdbobjectsmanager.h

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

#ifndef NCDBOBJECTSMANAGER_H
#define NCDBOBJECTSMANAGER_H

// CCC headers
extern "C"{
    #include "pallock.h"
}
#include "nbcontext.h"
#include "workerqueuemanager.h"

// NCDB headers
#include "Session.h"
#include "NBMTile.h"

namespace nbcommon {

//-----------------------------------------------------
// NcdbObjectsManager Class
// Used for NCDB Session & objects managing - creating, owning, retrieveing them to user
//-----------------------------------------------------
class SessionTask;

class NcdbObjectsManager: public Base
{
    friend class SessionTask;

public:
    NcdbObjectsManager(NB_Context* context);
    virtual ~NcdbObjectsManager();

    /*!< Can be invoked to pre-create all NCDB objects, not needed to be invoked before using other methods>*/
    void Initialize();

    void AddMapdataPath(std::string path);
    void AddWorldMapdataPath(std::string path);

    // NCDB NBMTile getters, can be called in any thread, it guarantees objects will be created in worker thread
    NB_Error GetNBMTile(shared_ptr<Ncdb::NBMTile>& nbmTile);

    NB_Context* GetContext();

    shared_ptr<Ncdb::Session> GetNcdbSession() const
    {
        return m_session;
    }

    bool IsSessionRequested() const
    {
        return m_sessionRequested;
    }

    void BeginLoadMapdata();
    void EndLoadMapdata();

    const char* GetMapVersionString();

private:
    // Session related methods
    Ncdb::Session* CreateMapSession();
    void CreateSession();
    void RequestSession();
    static void SessionDeleter(void* pointer); // helper method, deleter passed into shared_ptr

    void CreateNBMTile();
private:
    NB_Context* m_context;
    PAL_Lock*   m_lock;

    shared_ptr<WorkerQueueManager>     m_queueManager;
    shared_ptr<WorkerTaskQueue>        m_thread;

    bool  m_sessionRequested;
    bool  m_sessionFailed;
    bool  m_skipSessionCheck;

    shared_ptr<Ncdb::Session> m_session;

    shared_ptr<Ncdb::NBMTile>          m_nbmTile;

    std::vector<std::string>    m_mapdataPath;
    std::vector<std::string>    m_worldMapdataPath;
    bool    m_loadingMapdata;
    std::string m_mapVersion;
};

}; // namespace nbcommon

#endif   // NCDBOBJECTSMANAGER_H

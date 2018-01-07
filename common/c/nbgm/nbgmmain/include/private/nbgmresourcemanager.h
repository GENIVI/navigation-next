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

    @file nbgmresourcemanager.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBGM_RESOURCE_MANAGER_H_
#define _NBGM_RESOURCE_MANAGER_H_

#include "nbgmcontext.h"
#include "nbgmresourcemanagerconfig.h"
#include "nbgmtaskqueue.h"

struct NBGM_NBMDataLoadResult;
struct NBGM_NBMDataLoadInfo;
class NBGM_NBMData;

typedef NBRE_String NBGM_NBMDataId;

class NBGM_IResourceListener
{
public:
    ~NBGM_IResourceListener() {}

public:
    /// In call back thread
    virtual void OnResponseNBMData(const NBRE_Vector<NBGM_NBMDataLoadResult>& result) = 0;
    virtual void OnUpdateNBMData(const NBRE_Vector<NBGM_NBMDataLoadResult>& result) = 0;
};

class NBGM_ResourceManager
{
    friend class NBGM_SyncTask;
public:
    explicit NBGM_ResourceManager(const NBGM_ResourceManagerConfig& config, PAL_Instance* palInstance);
    ~NBGM_ResourceManager();

public:
    void RequestNBMData(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tileInfo, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& currentTaskQueue);
    void RequestBINData(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tileInfo, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& currentTaskQueue);
    void RequestUpdateNBMData(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tileInfo, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& currentTaskQueue);
    void ReleaseData(const NBGM_NBMDataId& nbmDataId);
    void ReleaseData(const NBRE_Vector<NBGM_NBMDataId>& nbmDataId);
    const NBGM_ResourceManagerConfig& GetConfig() { return mConfig; }
    NBGM_ResourceContext& GetResourceContext() { return mResourceContext; }
    const NBGM_ResourceContext& GetResourceContext() const { return mResourceContext; }
    const shared_ptr<WorkerTaskQueue>& GetLoadingTaskQueue() { return mLoadingThread; }

protected:
    void RecieveNBMDataFromTask(const NBRE_Vector<NBGM_NBMDataLoadResult>& result, NBGM_IResourceListener* listener);
    void RecieveUpdateNBMDataFromTask(const NBRE_Vector<NBGM_NBMDataLoadResult>& result, NBGM_IResourceListener* listener);

private:
    void Initialize();

private:
    typedef NBRE_Map<NBGM_NBMDataId, shared_ptr<NBGM_NBMData> > NBMDataMap;

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_ResourceManager);

private:
    NBGM_ResourceManagerConfig      mConfig;
    shared_ptr<WorkerTaskQueue>     mLoadingThread;
    NBGM_ResourceContext            mResourceContext;
    NBMDataMap                      mNBMDataMap;
};

#endif
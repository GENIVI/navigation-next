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
@file         OnboardContentManagerImpl.cpp
@defgroup     onboardcontentmanager
*/
/*
(C) Copyright 2014 by TeleCommunications Systems, Inc.

The information contained herein is confidential, proprietary to
TeleCommunication Systems, Inc., and considered a trade secret as defined
in section 499C of the penal code of the State of California. Use of this
information by anyone other than authorized employees of TeleCommunication
Systems is granted only under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#include "OnboardContentManagerImpl.h"
#include "ContentManagerTask.h"
#include "ContentManagerAdaptor.h"
#include <functional>
#include <algorithm>

extern "C"
{
#include "nbcontextprotected.h"
}

using namespace nbcommon;

#define TASK_EXECUTE(task)      \
    if (task)                   \
    {                           \
        task->Execute();        \
        return NE_OK;           \
    }                           \
    else                        \
    {                           \
        return NE_NOMEM;        \
    }

const uint32 DEFAULT_SCREEN_WIDTH  = 1280;
const uint32 DEAAULT_SCREEN_HEIGHT = 720;

//Functors for download status listener
class DownloadStatusChangedFunctor
{
public:
    DownloadStatusChangedFunctor(string regionalId,
                                 NB_EnhancedContentDataType dataType,
                                 RegionDownloadStatus status)
        : m_regionalId(regionalId),
          m_dataType(dataType),
          m_status(status)
    {
    }
    virtual ~DownloadStatusChangedFunctor() {}
    void operator() (DownloadStatusListener* listener)
    {
        if (listener)
        {
            listener->OnStatusChanged(m_regionalId, m_dataType, m_status);
        }
    }

    NB_EnhancedContentDataType      m_dataType;     /*!< Current downloading dataType.*/
    string                          m_regionalId;   /*!< Current downloading region Id.*/
    RegionDownloadStatus            m_status;       /*!< Current downloading status. */
};

class DownloadErrorFunctor
{
public:
    DownloadErrorFunctor(string regionalId,
                         NB_Error error)
        : m_regionalId(regionalId),
          m_error(error)
    {
    }
    virtual ~DownloadErrorFunctor() {}
    void operator() (DownloadStatusListener* listener)
    {
        if (listener)
        {
            listener->OnError(m_regionalId, m_error);
        }
    }
    string       m_regionalId;
    NB_Error     m_error;
};

class DownloadProgressFunctor
{
public:
    DownloadProgressFunctor(string regionalId,
                            uint32 percent)
        : m_regionalId(regionalId),
          m_percent(percent)
    {
    }
    virtual ~DownloadProgressFunctor() {}
    void operator() (DownloadStatusListener* listener)
    {
        if (listener)
        {
            listener->OnProgress(m_regionalId, m_percent);
        }
    }
    string       m_regionalId;
    uint32       m_percent;
};


RegionalInformation::RegionalInformation()
    :m_regionalID(""),
     m_regionalName(""),
     m_downloadSize(0),
     m_uncompressedSize(0),
     m_downloadedProgress(0),
     m_downloadedSize(0),
     m_version(""),
     m_updateAvailable(false),
     m_isInUse(false),
     m_downloadStatus(RDS_NotStart),
     m_errorCode(NE_OK)
{
}

RegionalInformation::RegionalInformation(uint64      downloadSize,
                                         uint64      uncompressedSize,
                                         uint32      downloadedProgress,
                                         uint64      downloadedSize,
                                         nb_boolean  updateAvailable,
                                         nb_boolean  isInUse,
                                         RegionDownloadStatus downloadStatus,
                                         NB_Error    errorCode,
                                         const std::string& regionalID,
                                         const std::string& regionalName,
                                         const std::string& version)
    :m_regionalID(regionalID),
     m_regionalName(regionalName),
     m_downloadSize(downloadSize),
     m_uncompressedSize(uncompressedSize),
     m_downloadedProgress(downloadedProgress),
     m_downloadedSize(downloadedSize),
     m_version(version),
     m_updateAvailable(updateAvailable),
     m_isInUse(false),
     m_downloadStatus(downloadStatus),
     m_errorCode(errorCode)
{
}

OnboardContentManagerConfig::OnboardContentManagerConfig()
     :m_screenWidth(DEFAULT_SCREEN_WIDTH),
      m_screenHeight(DEAAULT_SCREEN_HEIGHT),
      m_screenResolution(DEFAULT_SCREEN_WIDTH*DEAAULT_SCREEN_HEIGHT),
      m_country(""),
      m_language(""),
      m_nbgmVersion(""),
      m_mapDataStoragePath(""),
      m_persistentMetadataPath(""),
      m_clientGuid(""),
      m_supportedTypes(NB_ECDT_MAP_NONE)
{
}


OnboardContentManagerConfig::OnboardContentManagerConfig(uint32 screenWidth,
                                                         uint32 screenHeight,
                                                         uint32 screenResolution,
                                                         const std::string& country,
                                                         const std::string& language,
                                                         const std::string& nbgmVersion,
                                                         const std::string& mapDataStoragePath,
                                                         const std::string& persistentMetadataPath,
                                                         const std::string& clientGuid,
                                                         NB_SupportedMapType supportedTypes)
     :m_screenWidth(screenWidth),
      m_screenHeight(screenHeight),
      m_screenResolution(screenResolution),
      m_country(country),
      m_language(language),
      m_nbgmVersion(nbgmVersion),
      m_mapDataStoragePath(mapDataStoragePath),
      m_persistentMetadataPath(persistentMetadataPath),
      m_clientGuid(clientGuid),
      m_supportedTypes(supportedTypes)
{
}

RegionalMapDataListenerParameter::RegionalMapDataListenerParameter(
                                    NB_Error error,
                                    const std::vector<RegionalInformationPtr>& regions,
                                    void* userData)
    :m_error(error),
     m_availableRegions(regions),
     m_userData(userData)
{
}

DownloadStatusListenerParameter::DownloadStatusListenerParameter(NB_Error error,
                                    uint32 percent,
                                    NB_EnhancedContentDataType dataType,
                                    const std::string& regionalId,
                                    RegionDownloadStatus status,
                                    void* userData)
    :m_error(error),
     m_percent(percent),
     m_dataType(dataType),
     m_regionalId(regionalId),
     m_status(status),
     m_userData(userData)
{
}

shared_ptr<OnboardContentManager> OnboardContentManager::CreateOnboardContentManager(
    NB_Context* context,
    OnboardContentManagerConfigPtr config)
{
    OnboardContentManagerImplPtr contentManagerImpl(new OnboardContentManagerImpl(context, config));
    return (shared_ptr<OnboardContentManager>)contentManagerImpl;
}

OnboardContentManagerImpl::OnboardContentManagerImpl(NB_Context* context,
                                                     OnboardContentManagerConfigPtr config)
    : m_context(context),
      m_regionalId(""),
      m_downloadType(NB_ECD0_OnDemand),
      m_isValid(new bool(true))
{
    m_contentManagerAdaptor.reset(new ContentManagerAdaptor(context, config));
    if (m_contentManagerAdaptor)
    {
        m_contentManagerAdaptor->RegisterDownloadStatusListener(this);
        m_contentManagerAdaptor->RegisterRegionalMapDataListener(this);
    }
}

OnboardContentManagerImpl::~OnboardContentManagerImpl()
{
    m_context = NULL;
    m_isValid.reset();
    m_contentManagerAdaptor->UnregisterDownloadStatusListener();
    m_contentManagerAdaptor->UnregisterRegionalMapDataListener();
    m_contentManagerAdaptor.reset();
    m_regionalMapDataListeners.clear();
    m_downloadstatusListeners.clear();
}

NB_Error OnboardContentManagerImpl::RegisterRegionalMapDataListener(
    RegionalMapDataListener* availableRegionListener)
{
    if (!availableRegionListener)
    {
        return NE_INVAL;
    }
    pair<set<RegionalMapDataListener*>::iterator, bool> result =
            m_regionalMapDataListeners.insert(availableRegionListener);
    return NE_EXIST;
}

void OnboardContentManagerImpl::UnregisterRegionalMapDataListener(
    RegionalMapDataListener* availableRegionListener)
{
    m_regionalMapDataListeners.erase(availableRegionListener);
}

NB_Error OnboardContentManagerImpl::RegisterDownloadStatusListener(
    DownloadStatusListener* downloadStatusListener)
{
    if (!downloadStatusListener)
    {
        return NE_INVAL;
    }
    pair<set<DownloadStatusListener*>::iterator, bool> result =
            m_downloadstatusListeners.insert(downloadStatusListener);
    return NE_OK;
}

void OnboardContentManagerImpl::UnregisterDownloadStatusListener(
    DownloadStatusListener* downloadStatusListener)
{
    m_downloadstatusListeners.erase(downloadStatusListener);
}

NB_Error OnboardContentManagerImpl::CheckAvaliableRegions()
{
    ContentManagerTask<OnboardContentManagerImpl,void>* task =
        new ContentManagerTask<OnboardContentManagerImpl, void> (
                NB_ContextGetPal(m_context),
                this,
                (void*)this,
                &OnboardContentManagerImpl::CCCThread_CheckAvaliableRegions,
                m_isValid);
    TASK_EXECUTE(task);
}

NB_Error OnboardContentManagerImpl::RequestRegionalData(const std::string& regionalId)
{
    m_regionalId = regionalId;
    ContentManagerTask<OnboardContentManagerImpl,void>* task =
        new ContentManagerTask<OnboardContentManagerImpl, void> (
                NB_ContextGetPal(m_context),
                this,
                (void*)this,
                &OnboardContentManagerImpl::CCCThread_RequestRegionalData,
                m_isValid);
    TASK_EXECUTE(task);
}

NB_Error OnboardContentManagerImpl::RemoveRegionalData(const std::string& regionalId)
{
    m_regionalId = regionalId;
    ContentManagerTask<OnboardContentManagerImpl,void>* task =
        new ContentManagerTask<OnboardContentManagerImpl, void> (
                NB_ContextGetPal(m_context),
                this,
                (void*)this,
                &OnboardContentManagerImpl::CCCThread_RemoveRegionalData,
                m_isValid);
    TASK_EXECUTE(task);
}

NB_Error OnboardContentManagerImpl::PauseDownload(const std::string& regionalId,
                                                  NB_EnhancedContentDownloadType downloadType)
{
    m_regionalId = regionalId;
    m_downloadType = downloadType;
    ContentManagerTask<OnboardContentManagerImpl,void>* task =
        new ContentManagerTask<OnboardContentManagerImpl, void> (
                NB_ContextGetPal(m_context),
                this,
                (void*)this,
                &OnboardContentManagerImpl::CCCThread_PauseDownload,
                m_isValid);
    TASK_EXECUTE(task);
}

NB_Error OnboardContentManagerImpl::ResumeDownload(const std::string& regionalId,
                                                   NB_EnhancedContentDownloadType downloadType)
{
    m_regionalId = regionalId;
    m_downloadType = downloadType;
    ContentManagerTask<OnboardContentManagerImpl,void>* task =
        new ContentManagerTask<OnboardContentManagerImpl, void> (
                NB_ContextGetPal(m_context),
                this,
                (void*)this,
                &OnboardContentManagerImpl::CCCThread_ResumeDownload,
                m_isValid);
    TASK_EXECUTE(task);
}

NB_Error OnboardContentManagerImpl::CancelDownload(const std::string& regionalId)
{
    m_regionalId = regionalId;
    ContentManagerTask<OnboardContentManagerImpl,void>* task =
        new ContentManagerTask<OnboardContentManagerImpl, void> (
                NB_ContextGetPal(m_context),
                this,
                (void*)this,
                &OnboardContentManagerImpl::CCCThread_CancelDownload,
                m_isValid);
    TASK_EXECUTE(task);
}

//Thread switching functions for public interfaces
void OnboardContentManagerImpl::CCCThread_CheckAvaliableRegions(void* userData)
{
    OnboardContentManagerImpl* pThis = static_cast<OnboardContentManagerImpl*>(userData);
    if (pThis && pThis->m_contentManagerAdaptor)
    {
        pThis->m_contentManagerAdaptor->CheckAvaliableRegions();
    }
}
void OnboardContentManagerImpl::CCCThread_RequestRegionalData(void* userData)
{
    OnboardContentManagerImpl* pThis = static_cast<OnboardContentManagerImpl*>(userData);
    if (pThis && pThis->m_contentManagerAdaptor)
    {
        pThis->m_contentManagerAdaptor->RequestRegionalData(pThis->m_regionalId);
    }
}
void OnboardContentManagerImpl::CCCThread_RemoveRegionalData(void* userData)
{
    OnboardContentManagerImpl* pThis = static_cast<OnboardContentManagerImpl*>(userData);
    if (pThis && pThis->m_contentManagerAdaptor)
    {
        pThis->m_contentManagerAdaptor->RemoveRegionalData(pThis->m_regionalId);
    }
}
void OnboardContentManagerImpl::CCCThread_PauseDownload(void* userData)
{
    OnboardContentManagerImpl* pThis = static_cast<OnboardContentManagerImpl*>(userData);
    if (pThis && pThis->m_contentManagerAdaptor)
    {
        pThis->m_contentManagerAdaptor->PauseDownload(pThis->m_regionalId, pThis->m_downloadType);
    }
}
void OnboardContentManagerImpl::CCCThread_ResumeDownload(void* userData)
{
    OnboardContentManagerImpl* pThis = static_cast<OnboardContentManagerImpl*>(userData);
    if (pThis && pThis->m_contentManagerAdaptor)
    {
        pThis->m_contentManagerAdaptor->ResumeDownload(pThis->m_regionalId, pThis->m_downloadType);
    }
}
void OnboardContentManagerImpl::CCCThread_CancelDownload(void* userData)
{
    OnboardContentManagerImpl* pThis = static_cast<OnboardContentManagerImpl*>(userData);
    if (pThis && pThis->m_contentManagerAdaptor)
    {
        pThis->m_contentManagerAdaptor->CancelDownload(pThis->m_regionalId);
    }
}

/*! Listeners inherited from RegionalMapDataListener */
void OnboardContentManagerImpl::OnError(NB_Error error)
{
    vector<RegionalInformationPtr> regions;
    RegionalMapDataListenerParameter* mapDataListenerParameter =
        new RegionalMapDataListenerParameter(error,regions,this);
    ContentManagerTask<OnboardContentManagerImpl,void>* task =
        new ContentManagerTask<OnboardContentManagerImpl, void> (
                NB_ContextGetPal(m_context),
                this,
                (void*)mapDataListenerParameter,
                &OnboardContentManagerImpl::UIThread_RegionalMapDataOnError,
                m_isValid,
                true);
    if (task)
    {
        task->Execute();
    }
}

void OnboardContentManagerImpl::OnAvailableRegions(const std::vector<RegionalInformationPtr>& regions)
{
    RegionalMapDataListenerParameter* mapDataListenerParameter =
        new RegionalMapDataListenerParameter(NE_OK,regions,this);
    ContentManagerTask<OnboardContentManagerImpl,void>* task =
        new ContentManagerTask<OnboardContentManagerImpl, void> (
                NB_ContextGetPal(m_context),
                this,
                (void*)mapDataListenerParameter,
                &OnboardContentManagerImpl::UIThread_RegionalMapDataOnAvailableRegions,
                m_isValid,
                true);
    if (task)
    {
        task->Execute();
    }
}

/*! Listeners inherited from RegionalMapDataListener */
void OnboardContentManagerImpl::OnStatusChanged(const std::string& regionalId,
                                                NB_EnhancedContentDataType dataType,
                                                RegionDownloadStatus newStatus)
{
    DownloadStatusListenerParameter* downloadListenerParameter =
        new DownloadStatusListenerParameter(NE_OK,
                                            0,
                                            dataType,
                                            regionalId,
                                            newStatus,
                                            this);
    ContentManagerTask<OnboardContentManagerImpl,void>* task =
        new ContentManagerTask<OnboardContentManagerImpl, void> (
                NB_ContextGetPal(m_context),
                this,
                (void*)downloadListenerParameter,
                &OnboardContentManagerImpl::UIThread_DownloadStatusOnStatusChanged,
                m_isValid,
                true);
    if (task)
    {
        task->Execute();
    }
}

void OnboardContentManagerImpl::OnError(const std::string& regionalId, NB_Error error)
{
    DownloadStatusListenerParameter* downloadListenerParameter =
        new DownloadStatusListenerParameter(error,
                                            0,
                                            NB_ECDT_None,
                                            regionalId,
                                            RDS_NotStart,
                                            this);
    ContentManagerTask<OnboardContentManagerImpl,void>* task =
        new ContentManagerTask<OnboardContentManagerImpl, void> (
                NB_ContextGetPal(m_context),
                this,
                (void*)downloadListenerParameter,
                &OnboardContentManagerImpl::UIThread_DownloadStatusOnError,
                m_isValid,
                true);
    if (task)
    {
        task->Execute();
    }
}

void OnboardContentManagerImpl::OnProgress(const std::string& regionalId, int percentage)
{
    DownloadStatusListenerParameter* downloadListenerParameter =
        new DownloadStatusListenerParameter(NE_OK,
                                            percentage,
                                            NB_ECDT_None,
                                            regionalId,
                                            RDS_NotStart,
                                            this);
    ContentManagerTask<OnboardContentManagerImpl,void>* task =
        new ContentManagerTask<OnboardContentManagerImpl, void> (
                NB_ContextGetPal(m_context),
                this,
                (void*)downloadListenerParameter,
                &OnboardContentManagerImpl::UIThread_DownloadStatusOnProgress,
                m_isValid,
                true);
    if (task)
    {
        task->Execute();
    }
}

//thread switching for listeners
void OnboardContentManagerImpl::UIThread_RegionalMapDataOnError(void* userData)
{
    RegionalMapDataListenerParameter* parameter =
        static_cast<RegionalMapDataListenerParameter*>(userData);
    OnboardContentManagerImpl* pThis =
        parameter ?
        static_cast<OnboardContentManagerImpl*>(parameter->m_userData) : NULL;
    if (pThis)
    {
        for_each(m_regionalMapDataListeners.begin(),
                 m_regionalMapDataListeners.end(),
                 bind2nd(mem_fun(&RegionalMapDataListener::OnError), parameter->m_error));
    }
    delete parameter;
}

void OnboardContentManagerImpl::UIThread_RegionalMapDataOnAvailableRegions(void* userData)
{
    RegionalMapDataListenerParameter* parameter =
        static_cast<RegionalMapDataListenerParameter*>(userData);
    OnboardContentManagerImpl* pThis =
        parameter ?
        static_cast<OnboardContentManagerImpl*>(parameter->m_userData) : NULL;
    if (pThis)
    {
        for_each(m_regionalMapDataListeners.begin(),
                 m_regionalMapDataListeners.end(),
                 bind2nd(mem_fun(&RegionalMapDataListener::OnAvailableRegions),
                                 parameter->m_availableRegions));
    }
    delete parameter;
}

void OnboardContentManagerImpl::UIThread_DownloadStatusOnStatusChanged(void* userData)
{
    DownloadStatusListenerParameter* parameter =
        static_cast<DownloadStatusListenerParameter*>(userData);
    OnboardContentManagerImpl* pThis =
        parameter ?
        static_cast<OnboardContentManagerImpl*>(parameter->m_userData) : NULL;
    if (pThis)
    {
        for_each(m_downloadstatusListeners.begin(),
                 m_downloadstatusListeners.end(),
                 DownloadStatusChangedFunctor(parameter->m_regionalId,
                                              parameter->m_dataType,
                                              parameter->m_status));
    }
    delete parameter;
}

void OnboardContentManagerImpl::UIThread_DownloadStatusOnError(void* userData)
{
    DownloadStatusListenerParameter* parameter =
        static_cast<DownloadStatusListenerParameter*>(userData);

    OnboardContentManagerImpl* pThis =
        parameter ?
        static_cast<OnboardContentManagerImpl*>(parameter->m_userData) : NULL;
    if (pThis)
    {
        for_each(m_downloadstatusListeners.begin(),
                 m_downloadstatusListeners.end(),
                 DownloadErrorFunctor(parameter->m_regionalId,
                                      parameter->m_error));
    }
    delete parameter;
}

void OnboardContentManagerImpl::UIThread_DownloadStatusOnProgress(void* userData)
{
    DownloadStatusListenerParameter* parameter =
        static_cast<DownloadStatusListenerParameter*>(userData);
    OnboardContentManagerImpl* pThis =
        parameter ?
        static_cast<OnboardContentManagerImpl*>(parameter->m_userData) : NULL;
    if (pThis)
    {
        for_each(m_downloadstatusListeners.begin(),
                 m_downloadstatusListeners.end(),
                 DownloadProgressFunctor(parameter->m_regionalId,
                                         parameter->m_percent));
    }
    delete parameter;
}
/*! @} */

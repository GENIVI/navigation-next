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
@file         OnboardContentManagerImpl.h
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
#ifndef __NBSERVICES__ONBOARDCONTENTMANAGERIMPL_H__
#define __NBSERVICES__ONBOARDCONTENTMANAGERIMPL_H__

#include "OnboardContentManager.h"
#include <set>

namespace nbcommon
{
class ContentManagerAdaptor;
/*! Definition of RegionalMapDataListenerParameter
 *  This class used for the thread switching of RegionalMapDataListener
 */
class RegionalMapDataListenerParameter
{
public:
     /*! Constructor.
      *
      *  @param error           Error code.
      *  @param regions         Available regions.
      *  @param userData        User data used for callback
      */
    RegionalMapDataListenerParameter(NB_Error error,
                                    const std::vector<RegionalInformationPtr>& regions,
                                    void* userData);

    NB_Error                             m_error;              /*!< error code. */
    std::vector<RegionalInformationPtr>  m_availableRegions;    /*!< available regions */
    void*                                m_userData;            /*!< User data used for callback */
};
typedef shared_ptr<RegionalMapDataListenerParameter> RegionalMapDataListenerParameterPtr;

/*! Definition of DownloadStatusListenerParameter
 *  This class used for the thread switching of DownloadStatusListener
 */
class DownloadStatusListenerParameter
{
public:
     /*! Constructor.
      *
      *  @param error           Error code.
      *  @param percent         Progress in percent. Value from 0 - 100.
      *  @param dateType        Current downloading dataType.
      *  @param regionalId      Current downloading region id.
      *  @param status          Current downloading status.
      *  @param userData        User data used for callback
      */
    DownloadStatusListenerParameter(NB_Error error,
                                    uint32 percent,
                                    NB_EnhancedContentDataType dataType,
                                    const std::string& regionalId,
                                    RegionDownloadStatus status,
                                    void* userData);

    NB_Error                        m_error;        /*!< Error code. */
    uint32                          m_percent;      /*!< Progress in percent. Value from 0 - 100.*/
    NB_EnhancedContentDataType      m_dataType;     /*!< Current downloading dataType.*/
    std::string                     m_regionalId;   /*!< Current downloading region Id.*/
    RegionDownloadStatus            m_status;       /*!< Current downloading status. */
    void*                           m_userData;     /*!< User data used for callback */
};
typedef shared_ptr<DownloadStatusListenerParameter> DownloadStatusListenerParameterPtr;

/*! Definition of the class OnboardContentManagerImpl
    The class handles threadings and communicates with UI using listeners
 */
class OnboardContentManagerImpl : public OnboardContentManager,
                                         RegionalMapDataListener,
                                         DownloadStatusListener
{
public:
    using RegionalMapDataListener::OnError;
    using DownloadStatusListener::OnError;

    /*! Constructor
     *
     *  @param context      NB_Context instance.
     *  @param config       OnboardContentManagerConfig instance.
     */
    OnboardContentManagerImpl(NB_Context* context, OnboardContentManagerConfigPtr config);
    /*! Destructor
     */
    ~OnboardContentManagerImpl();

    /*! Below are functions inherited from OnboardContentManager.  */
    NB_Error RegisterRegionalMapDataListener(RegionalMapDataListener* availableRegionListener);
    void UnregisterRegionalMapDataListener(RegionalMapDataListener* availableRegionListener);
    NB_Error RegisterDownloadStatusListener(DownloadStatusListener* downloadStatusListener);
    void UnregisterDownloadStatusListener(DownloadStatusListener* downloadStatusListener);
    NB_Error CheckAvaliableRegions();
    NB_Error RequestRegionalData(const std::string& regionalId);
    NB_Error RemoveRegionalData(const std::string& regionalId);
    NB_Error PauseDownload(const std::string& regionalId,
                           NB_EnhancedContentDownloadType downloadType = NB_ECD0_OnDemand);
    NB_Error ResumeDownload(const std::string& regionalId,
                            NB_EnhancedContentDownloadType downloadType = NB_ECD0_OnDemand);
    NB_Error CancelDownload(const std::string& regionalId);

    /*! Listeners inherited from RegionalMapDataListener */
    void OnError(NB_Error error);
    void OnAvailableRegions(const std::vector<RegionalInformationPtr>& regions);

    /*! Listeners inherited from RegionalMapDataListener */
     void OnStatusChanged(const std::string& regionalId,
                          NB_EnhancedContentDataType dataType,
                          RegionDownloadStatus newStatus);
    void OnError(const std::string& regionalId, NB_Error error);
    void OnProgress(const std::string& regionalId, int percentage);

private:
    //thread switching for public interfaces
    void CCCThread_CheckAvaliableRegions(void* userData);
    void CCCThread_RequestRegionalData(void* userData);
    void CCCThread_RemoveRegionalData(void* userData);
    void CCCThread_PauseDownload(void* userData);
    void CCCThread_ResumeDownload(void* userData);
    void CCCThread_CancelDownload(void* userData);

    //thread switching for listeners
    void UIThread_RegionalMapDataOnError(void* userData);
    void UIThread_RegionalMapDataOnAvailableRegions(void* userData);
    void UIThread_DownloadStatusOnStatusChanged(void* userData);
    void UIThread_DownloadStatusOnError(void* userData);
    void UIThread_DownloadStatusOnProgress(void* userData);

    NB_Context*                          m_context;                  /*!< pointer to NB_Context instance */
    std::string                          m_regionalId;               /*!< current region id */
    NB_EnhancedContentDownloadType       m_downloadType;             /*!< download type of current region */
    std::set<RegionalMapDataListener*>   m_regionalMapDataListeners; /*!< stores listeners */
    std::set<DownloadStatusListener*>    m_downloadstatusListeners;  /*!< stores listeners */
    shared_ptr<ContentManagerAdaptor>    m_contentManagerAdaptor;    /*!< pointer to ContentManagerAdaptor instance */
    shared_ptr<bool>                     m_isValid;                  /*!< sign the instance is avaliable or not. */
};
typedef shared_ptr<OnboardContentManagerImpl> OnboardContentManagerImplPtr;
}
#endif
/*! @} */
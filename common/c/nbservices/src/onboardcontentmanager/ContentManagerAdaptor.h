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
@file         ContentManagerAdaptor.h
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
#ifndef __NBSERVICES__CONTENTMANAGERADAPTOR_H__
#define __NBSERVICES__CONTENTMANAGERADAPTOR_H__

#include "OnboardContentManager.h"
extern "C"
{
#include "nbenhancedcontentmanager.h"
}

namespace nbcommon
{

/*! Definition of the class ContentManagerAdaptor.
 *  The class bridges the OnboardContentManager(cpp) and ECM(c language).
 */
class ContentManagerAdaptor
{
public:
    /*! Constructor
     *
     *  @param context      NB_Context instance.
     *  @param config       OnboardContentManagerConfig instance.
     */
    ContentManagerAdaptor(NB_Context* context, OnboardContentManagerConfigPtr config);
    /*! Destructor
     *  Destory the enhancedContentManager instance and release resource.
     */
    ~ContentManagerAdaptor();

    /*see OnboardContentManager.h*/
    NB_Error RegisterRegionalMapDataListener(RegionalMapDataListener* availableRegionListener);
    void UnregisterRegionalMapDataListener();
    NB_Error RegisterDownloadStatusListener(DownloadStatusListener* downloadStatusListener);
    void UnregisterDownloadStatusListener();
    NB_Error CheckAvaliableRegions();
    NB_Error RequestRegionalData(const std::string& regionalId);
    NB_Error RemoveRegionalData(const std::string& regionalId);
    NB_Error PauseDownload(const std::string& regionalId,
                           NB_EnhancedContentDownloadType downloadType = NB_ECD0_OnDemand);
    NB_Error ResumeDownload(const std::string& regionalId,
                            NB_EnhancedContentDownloadType downloadType = NB_ECD0_OnDemand);
    NB_Error CancelDownload(const std::string& regionalId);

private:
    /*! Enhanced content manager synchronization call back
     *  see nbenhancedcontentmanager.h
     *  we only use the callback event here, do not use the call back data.
     */
    static void EnhancedContentSynchronizationCallback(NB_EnhancedContentManager* manager,
                                                       NB_Error result,
                                                       NB_RequestStatus status,
                                                       uint32 percent,
                                                       uint32 addedCityCount,
                                                       const NB_EnhancedContentCityData* addedCities,
                                                       uint32 updatedCityCount,
                                                       const NB_EnhancedContentCityData* updatedCities,
                                                       uint32 deletedCityCount,
                                                       const NB_EnhancedContentCityData* deletedCities,
                                                       void* userData);
    /*! Enhanced content manager download call back
     *  see nbenhancedcontentmanager.h
     */
    static void EnhancedContentDownloadCallback(NB_EnhancedContentManager* manager,
                                                NB_Error result,
                                                uint32 percent,
                                                NB_EnhancedContentDataType dataType,
                                                const char* datasetId,
                                                void* userData);

    /*! CreateEnhancedContentManager
     */
    NB_Error CreateEnhancedContentManager();

    /*! Create enhanced content manager configuration
     *
     *  @param config  created enhanced content manager configuration.
     *  @return error code.
     */
    NB_Error CreateEnhancedContentManagerConfig(NB_EnhancedContentManagerConfiguration* config);

    /*! Get available regions form ECM
     *
     *  @return available regions.
     */
    std::vector<RegionalInformationPtr> GetAvailableRegions();

    /*! list the file path of the main NCDB configuration file for each map region installed on the client
     *
     *  @return error code.
     */
    NB_Error UpdateRegionalConfiguration();

    /*! remove the suffix of file name.
     *
     *  @param filePath - file path.
     *  @return file name without suffix.
     */
    char* GetFileNameSansExtenstion(char* filePath);

    /*! write the content to the config file
     *
     *  @param filecontent - content which will write into the file.
     *  @return error code.
     */
    NB_Error SaveRegionalConfiguration(const string& fileConten);

    NB_Context*                          m_context;                 /*!< pointer to NB_Context instance */
    OnboardContentManagerConfigPtr       m_contentManagerConfig;    /*!< pointer to OnboardContentManagerConfigPtr instance */
    NB_EnhancedContentManager*           m_contentManager;          /*!< pointer to NB_EnhancedContentManager instance */
    std::string                          m_regionalId;              /*!< current regionalId */
    RegionalMapDataListener*             m_regionalMapDataListener; /*!< regional map data listener */
    DownloadStatusListener*              m_downloadstatusListener;  /*!< download status listener */
};
typedef shared_ptr<ContentManagerAdaptor> ContentManagerAdaptorPtr;
}
#endif
/*! @} */

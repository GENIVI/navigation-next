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
@file         OnboardContentManager.h
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
#ifndef __NBSERVICES__CONTENTMANAGER_H__
#define __NBSERVICES__CONTENTMANAGER_H__

#include "nbcontext.h"
#include "smartpointer.h"
#include "nbenhancedcontenttypes.h"
#include <string>
#include <vector>
extern "C"
{
#include "paltypes.h"
#include "nberror.h"
}

namespace nbcommon
{
/*! Definition of the download status */
enum RegionDownloadStatus
{
    RDS_NotStart = 0,            /*!< Download not start */
    RDS_Downloading,             /*!< Download in progress */
    RDS_Paused,                  /*!< Download paused. user pauses the download */
    RDS_Compelete,               /*!< Download completed and files are available */
    RDS_Failed,                  /*!< Download failed */
    RDS_Removed,                 /*!< Downloaded data has been removed */
};

/*! Definition of the regional information */
//@TODO: this class is similar to NB_EnhancedContentCityData.they will be merged in the future.
class RegionalInformation
{
public:
    /*! dafault constructor
     */
    RegionalInformation();
    /*! constructor
     *
     *  @param downloadSize - size of the data for region.
     *  @param uncompressedSize - uncompressed data size of the region.
     *  @param downloadedProgress - download progress of regin, value should be 0~100.
     *  @param downloadedSize - downloaded size.
     *  @param updateAvailable - if update is available or not.
     *  @param isInUse - a flag shows whether the region is in use.
     *  @param downloadStatus - download status of the region.
     *  @param errorCode - error code.
     *  @param regionalID - identifition of region.
     *  @param regionalName - name of the region.
     *  @param version - version of the region.
     */
    RegionalInformation(uint64      downloadSize,
                        uint64      uncompressedSize,
                        uint32      downloadedProgress,
                        uint64      downloadedSize,
                        nb_boolean  updateAvailable,
                        nb_boolean  isInUse,
                        RegionDownloadStatus downloadStatus,
                        NB_Error    errorCode,
                        const std::string& regionalID,
                        const std::string& regionalName,
                        const std::string& version);

    std::string m_regionalID;              /*!< identifition of region */
    std::string m_regionalName;            /*!< name of the region */
    uint64      m_downloadSize;            /*!< size of the data for region */
    uint64      m_uncompressedSize;        /*!< uncompressed data size of the region */
    uint32      m_downloadedProgress;      /*!< download progress of regin, value should be 0~100 */
    uint64      m_downloadedSize;          /*!< downloaded size */
    std::string m_version;                 /*!< version of the region */
    nb_boolean  m_updateAvailable;         /*!< if update is available, if value is true,
                                               downloadSize, uncompressedSize and version should be the new values */
    nb_boolean  m_isInUse;                 /*!< a flag shows whether the region is in use */
    RegionDownloadStatus m_downloadStatus; /*!< download status of the region */
    NB_Error             m_errorCode;      /*!< error code */
};
typedef shared_ptr<RegionalInformation> RegionalInformationPtr;

/*! RegionalMapDataListener
    Notify the available regions and update available regions
*/
class RegionalMapDataListener
{
public:
    virtual ~RegionalMapDataListener(){};

    /*! Notify the available regional update checking failed.
     *
     *  @param error - error code of the failure which will show the reason.
     *  @return none
     */
    virtual void OnError(NB_Error error) = 0;

    /*! Notify regions are available.
     *
     *  @param regions -  map of the regions are available.
     *  @return none
     */
    virtual void OnAvailableRegions(const std::vector<RegionalInformationPtr>& regions) = 0;
};
typedef shared_ptr<RegionalMapDataListener> RegionalMapDataListenerPtr;

/*! DownloadStatusListener
    Notify the download status.
*/
class DownloadStatusListener
{
public:
    virtual ~DownloadStatusListener(){};

    /*! Notify the changed status.
     *
     *  @param regionalId - Id of the downloaded region.
     *  @param dataType   - data type
     *  @param newStatus  - new download status.
     *  @return none
     */
    virtual void OnStatusChanged(const std::string& regionalId,
                                 NB_EnhancedContentDataType dataType,
                                 RegionDownloadStatus newStatus) = 0;

    /*! Notify the download is failed.
     *
     *  @param regionalId - Id of the downloaded region.
     *  @param error - error code of the failure.
     *  @return none
     */
    virtual void OnError(const std::string& regionalId, NB_Error error) = 0;

    /*! Notify the download progress.
     *
     *  @param regionalId - Id of the downloaded region.
     *  @param percentage - progress in percent (range: 0~100).
     *  @return none
     */
    virtual void OnProgress(const std::string& regionalId, int percentage) = 0;
};
typedef shared_ptr<DownloadStatusListener> DownloadStatusListenerPtr;

/*! ContentManagerConfiguration
*/
class OnboardContentManagerConfig
{
public:
    /*! dafault constructor
     */
    OnboardContentManagerConfig();
    /*! constructor
     *
     *  @param screenWidth              The screen width of the device, specified in pixels.
     *  @param screenHeight             The screen height of the device, specified in pixels.
     *  @param screenResolution         The resolution of the screen, specified in DPI.
     *  @param country                  Country.
     *  @param language                 Language.
     *  @param nbgmVersion              NBGM version.
     *  @param mapDataStoragePath       Path to Save the Map Data.
     *  @param persistentMetadataPath   Persistent metadata pat
     *  @param clientGuid               Client guid to be sent with http request
     *  @param supportedTypes           Types of map this client can handle
     */
    OnboardContentManagerConfig(uint32 screenWidth,
                                uint32 screenHeight,
                                uint32 screenResolution,
                                const std::string& country,
                                const std::string& language,
                                const std::string& nbgmVersion,
                                const std::string& mapDataStoragePath,
                                const std::string& persistentMetadataPath,
                                const std::string& clientGuid,
                                NB_SupportedMapType supportedTypes);

    uint32 m_screenWidth;                         /*!< The screen width of the device, specified in pixels.*/
    uint32 m_screenHeight;                        /*!< The screen height of the device, specified in pixels.*/
    uint32 m_screenResolution;                    /*!< The resolution of the screen, specified in DPI */
    std::string m_country;                        /*!< Country */
    std::string m_language;                       /*!< Language */
    std::string m_nbgmVersion;                    /*!< NBGM version */
    std::string m_mapDataStoragePath;             /*!< Path to Save the Map Data */
    std::string m_persistentMetadataPath;         /*!< Persistent metadata path */
    std::string m_clientGuid;                     /*!< Client guid to be sent with http requests */
    NB_SupportedMapType m_supportedTypes;         /*!< Types of map this client can handle. */
};
typedef shared_ptr<OnboardContentManagerConfig> OnboardContentManagerConfigPtr;

/*! class OnboardContentManager.
    interface of the OnboardContentManager.
*/
class OnboardContentManager
{
public:
    virtual ~OnboardContentManager(){};

    /*! Create OnboardContentManager.
     *
     *  @param context - pointer of nbContext object.
     *  @param config -  configuration of OnboardContentManager.
     *  @return shared pointer of ContentManager object
     */
    static shared_ptr<OnboardContentManager> CreateOnboardContentManager(NB_Context* context,
                                                                         OnboardContentManagerConfigPtr config);

    /*! register RegionalMapDataListener
     *
     *  @param availableRegionListener - pointer of RegionalMapDataListener object.
     *  @return error code
     */
    virtual NB_Error RegisterRegionalMapDataListener(RegionalMapDataListener* availableRegionListener) = 0;

    /*! unregister RegionalMapDataListener
     *
     *  @param availableRegionListener - pointer of RegionalMapDataListener object.
     */
    virtual void UnregisterRegionalMapDataListener(RegionalMapDataListener* availableRegionListener) = 0;

    /*! register DownloadStatusListener
     *
     *  @param downloadStatusListener - pointer of DownloadStatusListener object.
     *  @return error code
     */
    virtual NB_Error RegisterDownloadStatusListener(DownloadStatusListener* downloadStatusListener) = 0;

    /*! unregister RegionalMapDataListener
     *
     *  @param downloadStatusListener - pointer of DownloadStatusListener object.
     */
    virtual void UnregisterDownloadStatusListener(DownloadStatusListener* downloadStatusListener) = 0;

    /*! Get all available regions. regions will be provide via AvailableRegionsListener.
     *
     *  @return error code.
     */
    virtual NB_Error CheckAvaliableRegions() = 0;

    /*! add regional id to download list. download will started once you add a available id to the list.
     *
     *  @param regionalId - regional id which you want to download.
     *  @param downloadStatusListener - shared pointer of DownloadStatusListener object.
     *  @return error code.
     */
    virtual NB_Error RequestRegionalData(const std::string& regionalId) = 0;

    /*! remove regional data from device.
     *
     *  @param regionalId - regional id which you want to remove.
     *  @return error code.
     */
    virtual NB_Error RemoveRegionalData(const std::string& regionalId) = 0;

    /*! pause the specific download.
     *
     *  @param  regionalId - id of the region you want to pause the download.
                             set it to "" will pause all pending downloads.
     *  @return error code.
     */
    virtual NB_Error PauseDownload(const std::string& regionalId,
                                   NB_EnhancedContentDownloadType downloadType = NB_ECD0_OnDemand) = 0;
    /*! resume the specific download.
     *
     *  @param  regionalId - id of the region you want to resume the download.
                             set it to "" will resume all pending downloads.
     *  @return error code.
     */
    virtual NB_Error ResumeDownload(const std::string& regionalId,
                                    NB_EnhancedContentDownloadType downloadType = NB_ECD0_OnDemand) = 0;
    /*! cancel the specific download.
     *
     *  @param  regionalId - id of the region you want to cancel the download.
                             set it to "" will cancel all pending downloads.
     *  @return error code.
     */
    virtual NB_Error CancelDownload(const std::string& regionalId) = 0;
};
}
#endif
/*! @} */



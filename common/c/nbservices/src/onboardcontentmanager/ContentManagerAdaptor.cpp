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

#include "ContentManagerAdaptor.h"
#include <fstream>
#include <sstream>
extern "C"
{
#include "nbenhancedcontentmanager.h"
#include "palfile.h"
#include "nbcontextprotected.h"
}

using namespace nbcommon;

#define TEXTURE_PATH                           "Textures"

#define ECM_FILE_FORMAT                        "BIN"
#define MJO_FILE_FORMAT                        "BIN"
#define SAR_FILE_FORMAT                        "PNG"
#define SPD_FILE_FORMAT                        "PNG"
#define TEXTURE_FILE_FORMAT                    "PNG"
#define COMPRESSED_TEXTURE_FILE_FORMAT         "zip"
#define MAX_FILE_PATH                          256

static char* REGION_CONFIG_FILE_LIST_FILENAME = "main_MultiMapList_WC.txt";
static char* MAP_CONFIG_FILE_SUBPATH          = "maps";
static char* MAP_CONFIG_FILE_NAME             = "main_NA.ini";

ContentManagerAdaptor::ContentManagerAdaptor(NB_Context* context, OnboardContentManagerConfigPtr config)
    :m_context(context),
     m_contentManagerConfig(config),
     m_contentManager(NULL),
     m_regionalId(""),
     m_downloadstatusListener(NULL),
     m_regionalMapDataListener(NULL)
{
    CreateEnhancedContentManager();
}

ContentManagerAdaptor::~ContentManagerAdaptor()
{
    NB_EnhancedContentManagerDestroy(m_contentManager);
    m_contentManager = NULL;
}

NB_Error ContentManagerAdaptor::RegisterRegionalMapDataListener(RegionalMapDataListener* availableRegionListener)
{
    m_regionalMapDataListener = availableRegionListener;
    return NE_OK;
}

void ContentManagerAdaptor::UnregisterRegionalMapDataListener()
{
    m_regionalMapDataListener = NULL;
}

NB_Error ContentManagerAdaptor::RegisterDownloadStatusListener(DownloadStatusListener* downloadStatusListener)
{
    m_downloadstatusListener = downloadStatusListener;
    return NE_OK;
}

void ContentManagerAdaptor::UnregisterDownloadStatusListener()
{
    m_downloadstatusListener = NULL;
}

NB_Error ContentManagerAdaptor::CheckAvaliableRegions()
{
    return NB_EnhancedContentManagerStartMetadataSynchronization(m_contentManager);
}

NB_Error ContentManagerAdaptor::RequestRegionalData(const std::string& regionalId)
{
    NB_Error result = NE_OK;
    result = NB_ContentManagerSelectMapRegion(m_contentManager, regionalId.c_str(), TRUE);
    result != NE_OK? result : NB_EnhancedContentManagerStartManifestSynchronization(m_contentManager);
    return result;
}

NB_Error ContentManagerAdaptor::RemoveRegionalData(const std::string& regionalId)
{
    return NB_ContentManagerRemoveRegion(m_contentManager, regionalId.c_str());
}

NB_Error ContentManagerAdaptor::PauseDownload(const std::string& regionalId,
                        NB_EnhancedContentDownloadType downloadType)
{
    return NB_EnhancedContentManagerPauseRegionDownload(m_contentManager, regionalId.c_str(), downloadType);
}

NB_Error ContentManagerAdaptor::ResumeDownload(const std::string& regionalId,
                        NB_EnhancedContentDownloadType downloadType)
{
    return NB_EnhancedContentManagerResumeRegionDownload(m_contentManager, regionalId.c_str(), downloadType);
}

NB_Error ContentManagerAdaptor::CancelDownload(const std::string& regionalId)
{
    return NB_ContentManagerSelectMapRegion(m_contentManager, regionalId.c_str(), FALSE);
}

void ContentManagerAdaptor::EnhancedContentSynchronizationCallback(NB_EnhancedContentManager* manager,
                                                       NB_Error result,
                                                       NB_RequestStatus status,
                                                       uint32 percent,
                                                       uint32 addedCityCount,
                                                       const NB_EnhancedContentCityData* addedCities,
                                                       uint32 updatedCityCount,
                                                       const NB_EnhancedContentCityData* updatedCities,
                                                       uint32 deletedCityCount,
                                                       const NB_EnhancedContentCityData* deletedCities,
                                                       void* userData)
{
    if (status == NB_NetworkRequestStatus_Progress)
    {
        return;
    }
    ContentManagerAdaptor* pThis = static_cast<ContentManagerAdaptor*>(userData);
    if (pThis && pThis->m_regionalMapDataListener)
    {
        if (status == NB_NetworkRequestStatus_Success && result == NE_OK)
        {
            pThis->m_regionalMapDataListener->OnAvailableRegions(pThis->GetAvailableRegions());
        }
        else
        {
            pThis->m_regionalMapDataListener->OnError(result);
        }
    }
}

void ContentManagerAdaptor::EnhancedContentDownloadCallback(NB_EnhancedContentManager* manager,
                                                NB_Error result,
                                                uint32 percent,
                                                NB_EnhancedContentDataType dataType,
                                                const char* datasetId,
                                                void* userData)
{
    ContentManagerAdaptor* pThis = static_cast<ContentManagerAdaptor*>(userData);
    if (pThis && pThis->m_downloadstatusListener)
    {
        string regionID = datasetId ? datasetId : "";
        if (result != NE_OK)
        {
            pThis->m_downloadstatusListener->OnError(regionID, result);
            pThis->m_downloadstatusListener->OnStatusChanged(regionID, dataType, RDS_Failed);
        }
        else if(percent < 100)
        {
            pThis->m_downloadstatusListener->OnProgress(regionID, percent);
        }
        else
        {
            pThis->m_downloadstatusListener->OnStatusChanged(regionID, dataType, RDS_Compelete);
            //list the  region config file to main_MultiMapList_WC.txt
            pThis->UpdateRegionalConfiguration();
        }
    }
}

NB_Error ContentManagerAdaptor::CreateEnhancedContentManager()
{
    NB_Error result = NE_OK;

    NB_EnhancedContentSynchronizationCallback synchronizationCallback = { EnhancedContentSynchronizationCallback, this };
    NB_EnhancedContentDownloadCallback downloadCallback = { EnhancedContentDownloadCallback, this };

    NB_EnhancedContentManagerConfiguration config = { 0 };
    CreateEnhancedContentManagerConfig(&config);

    result = NB_EnhancedContentManagerCreate(m_context, &config, &synchronizationCallback, &downloadCallback, &m_contentManager);
    return result;
}

NB_Error ContentManagerAdaptor::CreateEnhancedContentManagerConfig(NB_EnhancedContentManagerConfiguration* config)
{
    config->screenWidth  = m_contentManagerConfig->m_screenWidth;
    config->screenHeight = m_contentManagerConfig->m_screenHeight;
    config->screenResolution = m_contentManagerConfig->m_screenResolution;
    config->supportedTypes = m_contentManagerConfig->m_supportedTypes;

    config->maxRealisticSignsCacheSize = 100;
    config->maxMotorwayJunctionsCacheSize = 100;
    config->maxSpeedLimitImagesCacheSize = 100;
    config->maxSpecialRegionsCacheSize = 100;

    config->enhancedCityModelEnabled = FALSE;
    config->motorwayJunctionObjectsEnabled = FALSE;
    config->realisticSignsEnabled = FALSE;
    config->speedLimitsSignEnabled = FALSE;
    config->specialRegionsEnabled = FALSE;
    config->mapRegionsEnabled = TRUE;

    nsl_strlcpy(config->country, m_contentManagerConfig->m_country.c_str(), sizeof(config->country));
    nsl_strlcpy(config->language, m_contentManagerConfig->m_language.c_str(), sizeof(config->language));
    nsl_strlcpy(config->nbgmVersion, m_contentManagerConfig->m_nbgmVersion.c_str(), sizeof(config->nbgmVersion));
    nsl_strlcpy(config->clientGuid, m_contentManagerConfig->m_clientGuid.c_str(), sizeof(config->clientGuid));
    nsl_strlcpy(config->persistentMetadataPath, m_contentManagerConfig->m_persistentMetadataPath.c_str(), sizeof(config->persistentMetadataPath));
    nsl_strlcpy(config->mapRegionsPath, m_contentManagerConfig->m_mapDataStoragePath.c_str(), sizeof(config->mapRegionsPath));

    nsl_strlcpy(config->texturesPath,
                m_contentManagerConfig->m_persistentMetadataPath.c_str(),
                sizeof(config->texturesPath));

    PAL_FileAppendPath(NB_ContextGetPal(m_context), config->texturesPath, sizeof(config->texturesPath), TEXTURE_PATH);

    nsl_strcpy(config->enhancedCityModelFileFormat, ECM_FILE_FORMAT);
    nsl_strcpy(config->motorwayJunctionObjectFileFormat, MJO_FILE_FORMAT);
    nsl_strcpy(config->realisticSignsFileFormat, SAR_FILE_FORMAT);
    nsl_strcpy(config->speedLimitsSignFileFormat, SPD_FILE_FORMAT);
    nsl_strcpy(config->textureFileFormat, TEXTURE_FILE_FORMAT);
    nsl_strcpy(config->compressedTextureFileFormat, COMPRESSED_TEXTURE_FILE_FORMAT);
    return NE_OK;
}

vector<RegionalInformationPtr> ContentManagerAdaptor::GetAvailableRegions()
{
    vector<RegionalInformationPtr> regions;
    uint32 count = 0;
    uint32 n = 0;
    NB_ContentEntryData* regionArray = NULL;
    NB_ContentEntryData* regionPointer = NULL;
    NB_Error result = NE_OK;

    result = NB_ContentManagerGetRegionList(m_contentManager, &count, &regionArray);
    if (result == NE_OK)
    {
        regionPointer = regionArray;
        for (n = 0; n < count; ++n)
        {
            RegionalInformationPtr region(new RegionalInformation);
            region->m_regionalID = regionPointer->cityId;
            region->m_regionalName = regionPointer->displayName;
            region->m_uncompressedSize = regionPointer->cityModelDataSize;
            region->m_version = regionPointer->version;
            region->m_downloadedSize = regionPointer->downloadedCityModelDataSize;
            region->m_isInUse = regionPointer->updateStatus == NB_ECUS_UpdateStatusUsed ? true : false;

            //TODO: more values will be set later if necessary.
            regions.push_back(region);
            regionPointer++;
        }
    }
    nsl_free(regionArray);
    regionArray = NULL;
    return regions;
}

NB_Error ContentManagerAdaptor::UpdateRegionalConfiguration()
{
    int32 count = 0;
    char** fileIdArray = NULL;
    stringstream fileContent;
    char itemfilePath[MAX_FILE_PATH];
    if (NB_ContentManagerGetExistMapRegionFileIds(m_contentManager, &count, &fileIdArray) == NE_OK)
    {
        for (int32 i = 0; i < count; ++i)
        {
            if (!fileIdArray[i])
            {
                 continue;
            }
            nsl_memset(itemfilePath, 0, MAX_FILE_PATH);
            nsl_strcpy(itemfilePath, GetFileNameSansExtenstion(fileIdArray[i]));
            // eg. CA/maps/main_NA.ini
            PAL_FileAppendPath(NB_ContextGetPal(m_context), itemfilePath, MAX_FILE_PATH, MAP_CONFIG_FILE_SUBPATH);
            PAL_FileAppendPath(NB_ContextGetPal(m_context), itemfilePath, MAX_FILE_PATH, MAP_CONFIG_FILE_NAME);
            fileContent << itemfilePath << endl;
            nsl_free(fileIdArray[i]);
        }
    }
    nsl_free(fileIdArray);

    return SaveRegionalConfiguration(fileContent.str());
}

char* ContentManagerAdaptor::GetFileNameSansExtenstion(char* filePath)
{
    // Remove the suffix of file format. For example ".PNG" or ".BIN".
    static char fileId[MAX_ENHANCED_CONTENT_CITY_DISPLAY_LENGTH] = {0};
    nsl_strlcpy(fileId, filePath, MAX_ENHANCED_CONTENT_CITY_DISPLAY_LENGTH);
    char* suffix = nsl_strrchr(fileId, '.');
    if (suffix)
    {
        *suffix = '\0';
    }
    return fileId;
}

NB_Error ContentManagerAdaptor::SaveRegionalConfiguration(const string& fileContent)
{
    if (fileContent.empty())
    {
        return NE_OK;
    }
    char filePath[MAX_FILE_PATH] = {0};
    nsl_strlcpy(filePath,
                m_contentManagerConfig->m_mapDataStoragePath.c_str(),
                MAX_FILE_PATH);
    PAL_FileAppendPath(NB_ContextGetPal(m_context), filePath,
                        MAX_FILE_PATH, REGION_CONFIG_FILE_LIST_FILENAME);

    ofstream stream(filePath, ofstream::out);
    if (stream.good())
    {
        stream << fileContent;
        return NE_OK;
    }
    return NE_FSYS;
}
/*! @} */

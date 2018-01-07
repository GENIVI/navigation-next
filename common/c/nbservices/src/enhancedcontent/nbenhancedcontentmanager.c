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

    @file     nbenchancedcontentmanager.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "nbcontextprotected.h"
#include "nbenhancedcontentmanagerprivate.h"
#include "nbenhancedcontentmanagerprotected.h"
#include "nbenhancedcontentdatasource.h"
#include "nbenhancedcontentdownloadmanager.h"
#include "nbmetadatatypes.h"
#include "palfile.h"

/*! @{ */

// Constants .....................................................................................


// Local types ...................................................................................

/*! Enhanced content manager object

    @see NB_EnhancedContentManagerCreate
*/
struct NB_EnhancedContentManager
{
    NB_Context*                                 context;                            /*!< NB_Context object */

    NB_EnhancedContentDataSource*               dataSource;                         /*!< NB_EnhancedContentDataSource object */
    NB_EnhancedContentDownloadManager*          downloadManager;                    /*!< NB_EnhancedContentDownloadManager object */

    NB_EnhancedContentManagerConfiguration      configuration;                      /*!< configuration */
    NB_EnhancedContentSynchronizationCallback   synchronizationCallback;            /*!< Synchronization callback */
    NB_EnhancedContentDownloadCallback          downloadCallback;                   /*!< Download callback */

    NB_EnhancedContentDownloadType              allowableDownloadType;              /*!< Allowable download type for city model */
    nb_boolean                                  manifestSynchronizationAllowable;   /*!< Is manifest synchronization allowable */
};


// Local functions ...............................................................................

static NB_Error CreateDataSource(NB_EnhancedContentManager* pThis);
static NB_Error CreateDownloadManager(NB_EnhancedContentManager* pThis);
static nb_boolean IsValidState(NB_EnhancedContentManager* pThis);

NB_DEF NB_Error NB_EnhancedContentManagerSelectEntry(NB_EnhancedContentManager* manager,
                                                     const char* id,
                                                     nb_boolean selected,
                                                     NB_EnhancedContentDataType dataType);

NB_DEF NB_Error NB_EnhancedContentManagerRemoveEntry(NB_EnhancedContentManager* manager,
                                                     const char* id,
                                                     NB_EnhancedContentDataType dataType);

NB_DEF NB_Error NB_EnhancedContentManagerGetEntryList(NB_EnhancedContentManager* manager,
                                                     uint32* count,
                                                     NB_EnhancedContentDataType dataType,
                                                     NB_ContentEntryData** entries);

NB_DEF NB_Error NB_EnhancedContentManagerCalculateDownloadPercentage(
                    NB_EnhancedContentManager* manager,
                    uint32 percent,
                    const NB_EnhancedContentIdentifier* item,
                    uint32* percentOut);
// Public Functions ..............................................................................

/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerCreate(
    NB_Context* context,
    const NB_EnhancedContentManagerConfiguration* configuration,
    NB_EnhancedContentSynchronizationCallback* synchronizationCallback,
    NB_EnhancedContentDownloadCallback* downloadCallback,
    NB_EnhancedContentManager** manager
    )
{
    NB_EnhancedContentManager* pThis = NULL;
    NB_Error result = NE_OK;

    if (!context || !configuration || !manager)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(NB_EnhancedContentManager));
    if (!pThis)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(NB_EnhancedContentManager));

    pThis->context = context;
    pThis->configuration = *configuration;
    if (synchronizationCallback)
    {
        pThis->synchronizationCallback = *synchronizationCallback;
    }
    if (downloadCallback)
    {
        pThis->downloadCallback = *downloadCallback;
    }

    result = result ? result : CreateDataSource(pThis);
    result = result ? result : CreateDownloadManager(pThis);

    if (result != NE_OK)
    {
        NB_EnhancedContentManagerDestroy(pThis);
        return result;
    }

    pThis->allowableDownloadType = NB_ECDO_All;
    pThis->manifestSynchronizationAllowable = TRUE;

    *manager = pThis;

    return NE_OK;
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerDestroy(NB_EnhancedContentManager* manager)
{
    if (!manager)
    {
        return NE_INVAL;
    }

    if (manager->downloadManager)
    {
        NB_EnhancedContentDownloadManagerDestroy(manager->downloadManager);
        manager->downloadManager = NULL;
    }

    if (manager->dataSource)
    {
        NB_EnhancedContentDataSourceDestroy(manager->dataSource);
        manager->dataSource = NULL;
    }

    nsl_free(manager);

    return NE_OK;
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerStartMetadataSynchronization(NB_EnhancedContentManager* manager)
{
    NB_Error result = NE_OK;

    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    if (NB_EnhancedContentDataSourceIsMetadataSynchronizeInProgress(manager->dataSource))
    {
        return NE_BUSY;
    }

    if (NB_EnhancedContentDataSourceIsManifestSynchronizeInProgress(manager->dataSource))
    {
        return NE_BUSY;
    }

    result = NB_EnhancedContentManagerCancelDataDownload(manager);
    if (result != NE_OK)
    {
        return result;
    }

    return NB_EnhancedContentDataSourceStartMetadataSynchronize(manager->dataSource);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerSetLocation(NB_EnhancedContentManager* manager, NB_LatitudeLongitude* location)
{
    if (!IsValidState(manager) || !location)
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentDataSourceSetLocation(manager->dataSource, location);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerSelectCity(NB_EnhancedContentManager* manager, const char* cityId, nb_boolean selected)
{
    return NB_EnhancedContentManagerSelectEntry(manager, cityId, selected, NB_ECDT_CityModel);
}

/* See header file for description. */
NB_DEF NB_Error
NB_ContentManagerSelectMapRegion(NB_EnhancedContentManager* manager, const char* regionId, nb_boolean selected)
{
    return NB_EnhancedContentManagerSelectEntry(manager, regionId, selected, NB_ECDT_MapRegions);
}

/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerRemoveCity(NB_EnhancedContentManager* manager, const char* cityId)
{
    return NB_EnhancedContentManagerRemoveEntry(manager, cityId, NB_ECDT_CityModel);
}

/* See header file for description. */
NB_DEC NB_Error
NB_ContentManagerRemoveRegion(NB_EnhancedContentManager* manager, const char* regionId)
{
    return NB_EnhancedContentManagerRemoveEntry(manager, regionId, NB_ECDT_MapRegions);
}

/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerStartManifestSynchronization(NB_EnhancedContentManager* manager)
{
    NB_Error result = NE_OK;

    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    if (NB_EnhancedContentDataSourceIsMetadataSynchronizeInProgress(manager->dataSource))
    {
        return NE_BUSY;
    }

    if (NB_EnhancedContentDataSourceIsManifestSynchronizeInProgress(manager->dataSource))
    {
        return NE_BUSY;
    }

    if (manager->manifestSynchronizationAllowable)
    {
        result = NB_EnhancedContentManagerCancelDataDownload(manager);
        if (result != NE_OK)
        {
            return result;
        }

        result = NB_EnhancedContentDataSourceStartManifestSynchronize(manager->dataSource);
    }
    else
    {
        // Download next dataset when manifest synchronization isn't allowable.
        result = NE_NOENT;
    }

    if (result == NE_NOENT)
    {
        result = NB_EnhancedContentManagerDownloadNextDataset(manager);

        // If result is NE_OK start downloading.
        // If result is NE_NOENT no need to download.
        // If result is NE_BUSY downloading is pending.
    }

    return result;
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerSetManifestSynchronizationAllowable(NB_EnhancedContentManager* manager, nb_boolean manifestSynchronizationAllowable)
{
    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    manager->manifestSynchronizationAllowable = manifestSynchronizationAllowable;

    if (manifestSynchronizationAllowable)
    {
        return NE_OK;
    }

    // Argument 'manifestSynchronizationAllowable' is FALSE
    return NB_EnhancedContentDataSourceCancelManifestSynchronize(manager->dataSource);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerPauseDownload(NB_EnhancedContentManager* manager, NB_EnhancedContentDownloadType downloadType)
{
    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentDownloadManagerPauseDownload(manager->downloadManager, downloadType);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerResumeDownload(NB_EnhancedContentManager* manager, NB_EnhancedContentDownloadType downloadType)
{
    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentDownloadManagerResumeDownload(manager->downloadManager, downloadType);
}

/* Refer to header file for description. */
NB_DEC NB_Error
NB_EnhancedContentManagerDisableDownloadLevel(NB_EnhancedContentManager* manager,
                                             NB_EnhancedContentDownloadType downloadType,
                                             NB_EnhancedContentDownloadLevel level)
{
    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }
    return NB_EnhancedContentDownloadManagerDisableLevel(manager->downloadManager,
                                                         downloadType, level);
}

/* Refer to header file for description. */
NB_DEC NB_Error
NB_EnhancedContentManagerEnableDownloadLevel(NB_EnhancedContentManager* manager,
                                            NB_EnhancedContentDownloadType downloadType,
                                            NB_EnhancedContentDownloadLevel level)
{
    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }
    return NB_EnhancedContentDownloadManagerEnableLevel(manager->downloadManager,
                                                        downloadType, level);
}

/* Refer to header file for description. */
NB_DEC nb_boolean
NB_EnhancedContentManagerIsDownloadLevelEnabled(NB_EnhancedContentManager* manager,
                                                NB_EnhancedContentDownloadType downloadType,
                                                NB_EnhancedContentDownloadLevel level)
{
    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }
    return NB_EnhancedContentDownloadManagerIsLevelEnabled(manager->downloadManager,
                                                           downloadType, level);
}

/* See header file for description. */
NB_DEF nb_boolean
NB_EnhancedContentManagerIsMetadataSynchronizationInProgress(NB_EnhancedContentManager* manager)
{
    if (!IsValidState(manager))
    {
        return FALSE;
    }

    return NB_EnhancedContentDataSourceIsMetadataSynchronizeInProgress(manager->dataSource);
}

/* See header file for description. */
NB_DEF nb_boolean
NB_EnhancedContentManagerIsManifestSynchronizationInProgress(NB_EnhancedContentManager* manager)
{
    if (!IsValidState(manager))
    {
        return FALSE;
    }

    return NB_EnhancedContentDataSourceIsManifestSynchronizeInProgress(manager->dataSource);
}


/* See header file for description. */
NB_DEF nb_boolean
NB_EnhancedContentManagerIsDownloadInProgress(NB_EnhancedContentManager* manager, NB_EnhancedContentDownloadType downloadType)
{
    if (!IsValidState(manager))
    {
        return FALSE;
    }

    return NB_EnhancedContentDownloadManagerIsPending(manager->downloadManager, downloadType);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerCancelDataDownload(NB_EnhancedContentManager* manager)
{
    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentDownloadManagerCancelRequests(manager->downloadManager, NB_ECDO_All, NB_ECDT_All);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerRemoveAllStoredData(NB_EnhancedContentManager* manager)
{
    NB_Error result = NE_OK;

    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    result = NB_EnhancedContentDownloadManagerMasterClear(manager->downloadManager);
    if (result != NE_OK)
    {
        return result;
    }

    return NB_EnhancedContentDataSourceMasterClear(manager->dataSource);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerGetStorageStatistics(NB_EnhancedContentManager* manager, uint64* totalSize, uint64* downloadedSize, uint32* totalCommonTexturesSize, uint32* downloadedCommonTexturesSize, uint32* downloadedPercentage)
{
    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentDataSourceGetStorageStatistics(
        manager->dataSource,
        totalSize,
        downloadedSize,
        totalCommonTexturesSize,
        downloadedCommonTexturesSize,
        downloadedPercentage);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerGetCityDownloadStatus(NB_EnhancedContentManager* manager, NB_EnhancedContentDownloadStatus* downloadStatus)
{
    NB_Error result = NE_OK;

    if (!IsValidState(manager) || !downloadStatus)
    {
        return NE_INVAL;
    }

    if (NB_EnhancedContentDownloadManagerIsPaused(manager->downloadManager, NB_ECD0_Preload))
    {
        *downloadStatus = NB_ECDS_DownloadPaused;
    }
    else if (NB_EnhancedContentDownloadManagerIsPending(manager->downloadManager, NB_ECD0_Preload))
    {
        *downloadStatus = NB_ECDS_DownloadInProgress;
    }
    else
    {
         NB_EnhancedContentIdentifier* identifier =
                 NB_EnhancedContentIdentifierCreate(NB_ECDT_None, NULL, NULL, NULL);
        if (!identifier)
        {
            return NE_NOMEM;
        }

        result = NB_EnhancedContentDataSourceGetNextDatasetToUpdate(manager->dataSource, (nb_boolean) (manager->allowableDownloadType & NB_ECD0_Preload), identifier);
        NB_EnhancedContentIdentifierDestroy(identifier);

        if (result == NE_OK)
        {
            *downloadStatus = NB_ECDS_DownloadNotStarted;
        }
        else if (result == NE_NOENT)
        {
            result = NE_OK;
            *downloadStatus = NB_ECDS_DownloadComplete;
        }
    }

    return result;
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerGetCityList(NB_EnhancedContentManager* manager, uint32* cityCount, NB_EnhancedContentCityData** cities)
{
    return NB_EnhancedContentManagerGetEntryList(manager, cityCount, NB_ECDT_CityModel, cities);
}

/* See header file for description. */
NB_DEF NB_Error
NB_ContentManagerGetRegionList(NB_EnhancedContentManager* manager, uint32* count, NB_ContentEntryData** regions)
{
    return NB_EnhancedContentManagerGetEntryList(manager, count, NB_ECDT_MapRegions, regions);
}

/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerGetCitySummaryDisplay(NB_EnhancedContentManager* manager, const char** citySummaryDisplay)
{
    if (!IsValidState(manager) || !citySummaryDisplay)
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentDataSourceGetCitySummaryDisplay(manager->dataSource, citySummaryDisplay);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerSetAllowableDownload(NB_EnhancedContentManager* manager, NB_EnhancedContentDownloadType downloadType)
{
    NB_Error result = NE_OK;

    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    manager->allowableDownloadType = downloadType;

    // @todo: Should cancel requests of city textures?
    switch (downloadType)
    {
        case NB_ECDO_None:
        {
            result = NB_EnhancedContentDownloadManagerCancelRequests(manager->downloadManager, NB_ECDO_All, NB_ECDT_CityModel);

            break;
        }
        case NB_ECD0_OnDemand:
        {
            result = NB_EnhancedContentDownloadManagerCancelRequests(manager->downloadManager, NB_ECD0_Preload, NB_ECDT_CityModel);

            break;
        }
        case NB_ECD0_Preload:
        {
            result = NB_EnhancedContentDownloadManagerCancelRequests(manager->downloadManager, NB_ECD0_OnDemand, NB_ECDT_CityModel);

            break;
        }
        case NB_ECDO_All:
        default:
        {
            break;
        }
    }

    return result;
}

/* See header file for description. */
NB_DEF NB_Error NB_EnhancedContentManagerGetNextCityToUpdate(NB_EnhancedContentManager* manager, uint32 maxCityIdSize, char* cityId)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentIdentifier* identifier = NULL;

    if (!IsValidState(manager) || !cityId)
    {
        return NE_INVAL;
    }

    identifier = NB_EnhancedContentIdentifierCreate(NB_ECDT_None, NULL, NULL, NULL);
    if (!identifier)
    {
        return NE_NOMEM;
    }

    result = NB_EnhancedContentDataSourceGetNextCityToUpdate(manager->dataSource, identifier);
    if (result != NE_OK)
    {
        NB_EnhancedContentIdentifierDestroy(identifier);
        return result;
    }

    if (maxCityIdSize < nsl_strlen(identifier->dataset) + 1)
    {
        NB_EnhancedContentIdentifierDestroy(identifier);
        return NE_RANGE;
    }

    nsl_memset(cityId, 0, maxCityIdSize);
    nsl_strcpy(cityId, identifier->dataset);

    NB_EnhancedContentIdentifierDestroy(identifier);

    return result;
}

/* See header file for description. */
NB_DEF nb_boolean
NB_EnhancedContentManagerIsCommonTextureAvailable(NB_EnhancedContentManager* manager)
{
    if (!IsValidState(manager))
    {
        return FALSE;
    }

    return NB_EnhancedContentManagerIsDataAvailable(
        manager,
        NB_ECDT_CommonTexture,
        NB_EnhancedContentDataSourceGetCommonTexturesDatasetId(manager->dataSource),
        NULL);
}


/* See header file for description. */
NB_DEF nb_boolean
NB_EnhancedContentManagerIsMotorwayJunctionTextureAvailable(NB_EnhancedContentManager* manager)
{
    if (!IsValidState(manager))
    {
        return FALSE;
    }

    return NB_EnhancedContentManagerIsDataAvailable(
        manager,
        NB_ECDT_SpecificTexture,
        NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(manager->dataSource),
        NULL);
}


// Functions declared in "nbenhancedcontentmanagerprotected.h" ...................................

/* See header file for description. */
nb_boolean
NB_EnhancedContentManagerIsDataAvailable(NB_EnhancedContentManager* manager, NB_EnhancedContentDataType dataType, const char* dataset, const char* dataId)
{
    NB_EnhancedContentIdentifier* identifier = NULL;
    nb_boolean available = FALSE;

    if (!IsValidState(manager))
    {
        return FALSE;
    }

    identifier = NB_EnhancedContentIdentifierCreate(dataType, dataset, dataId, NULL);
    if (identifier)
    {
        switch (dataType)
        {
            case NB_ECDT_MotorwayJunctionObject:
            case NB_ECDT_RealisticSign:
            case NB_ECDT_SpeedLimitsSign:
            case NB_ECDT_SpecialRegions:
                available = NB_EnhancedContentDownloadManagerIsDataItemAvailable(manager->downloadManager, identifier);
                break;
            default:
                available = NB_EnhancedContentDataSourceIsDataItemAvailableForUse(manager->dataSource, identifier) &&
                    NB_EnhancedContentDownloadManagerIsDataItemAvailable(manager->downloadManager, identifier);
                break;
        }
        NB_EnhancedContentIdentifierDestroy(identifier);
    }

    return available;
}


/* See header file for description. */
NB_DEF nb_boolean
NB_EnhancedContentManagerIsDataAvailableWithDependencies(NB_EnhancedContentManager* manager, NB_EnhancedContentDataType dataType, const char* dataset, const char* dataId)
{
    nb_boolean available = FALSE;

    if (!IsValidState(manager) || !dataset)
    {
        return FALSE;
    }

    // Check for data item and dependencies based on type
    switch (dataType)
    {
        case NB_ECDT_CityModel:
            // Check for common textures, city textures, and data item
            available = NB_EnhancedContentManagerIsCommonTextureAvailable(manager)
                && NB_EnhancedContentManagerIsCityTextureAvailable(manager, dataset)
                && NB_EnhancedContentManagerIsDataAvailable(manager, NB_ECDT_CityModel, dataset, dataId);
            break;
        case NB_ECDT_MotorwayJunctionObject:
            // Check for MJO texture and data item
            available = NB_EnhancedContentManagerIsMotorwayJunctionTextureAvailable(manager)
                && NB_EnhancedContentManagerIsDataAvailable(manager, NB_ECDT_MotorwayJunctionObject, dataset, dataId);
            break;
        case NB_ECDT_RealisticSign:
        case NB_ECDT_SpeedLimitsSign:
        case NB_ECDT_SpecialRegions:
        default:
            // Check for individual item, no dependencies
            available = NB_EnhancedContentManagerIsDataAvailable(manager, dataType, dataset, dataId);
            break;
    }

    return available;
}


/* See header file for description. */
NB_DEF nb_boolean
NB_EnhancedContentManagerIsCityTextureAvailable(NB_EnhancedContentManager* manager, const char* cityId)
{
    return NB_EnhancedContentManagerIsDataAvailable(
        manager,
        NB_ECDT_SpecificTexture,
        cityId,
        NULL);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerGetCityBoundingBox(NB_EnhancedContentManager* manager, const char* cityId, NB_CityBoundingBox* boundingBox)
{
    if (!IsValidState(manager) || !cityId || !boundingBox)
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentDataSourceGetCityBoundingBox(manager->dataSource, cityId, boundingBox);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerGetCityTileBoundingBoxes(NB_EnhancedContentManager* manager, const char* cityId, uint32* cityBoundingBoxesCount, NB_CityBoundingBox** cityBoundingBoxes)
{
    if (!IsValidState(manager) || !cityId || !cityBoundingBoxesCount || !cityBoundingBoxes)
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentDataSourceGetCityTileBoundingBoxes(manager->dataSource, cityId, cityBoundingBoxesCount, cityBoundingBoxes);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerGetCityProjectionParameters(NB_EnhancedContentManager* manager, const char* cityId, NB_EnhancedContentProjectionType* projectionType, NB_EnhancedContentProjectionDatum* datum, NB_LatitudeLongitude* origin, double* scaleFactor, double* falseNorthing, double* falseEasting, double* zOffset)
{
    if (!IsValidState(manager) || !cityId || !projectionType || !datum || !origin || !scaleFactor || !falseNorthing || !falseEasting || !zOffset)
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentDataSourceGetCityProjectionParameters(manager->dataSource, cityId, projectionType, datum, origin, scaleFactor, falseNorthing, falseEasting, zOffset);
}

/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerGetDataItem(NB_EnhancedContentManager* manager,
                                     NB_EnhancedContentDataType dataType,
                                     const char* dataset, const char* dataId,
                                     const char* fileVersion, nb_boolean allowDownload,
                                     uint32* dataSize, uint8** data)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentIdentifier* identifier = NULL;

    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    if ((dataType == NB_ECDT_MotorwayJunctionObject || dataType == NB_ECDT_RealisticSign ||
        dataType == NB_ECDT_SpeedLimitsSign || dataType == NB_ECDT_SpecialRegions) && fileVersion)
    {
        result = NB_EnhancedContentManagerCheckFileVersion(manager, dataType, dataset, dataId, fileVersion);
        if (result != NE_OK && result != NE_NOENT)
        {
            return result;
        }
    }

    identifier = NB_EnhancedContentIdentifierCreate(dataType, dataset, dataId, fileVersion);
    if (identifier)
    {
        result = NB_EnhancedContentDownloadManagerGetDataItem(manager->downloadManager, identifier, dataSize, data);
        NB_EnhancedContentIdentifierDestroy(identifier);
        if (allowDownload && result == NE_NOENT)
        {
            result = NB_EnhancedContentManagerRequestDataDownload(manager, dataType, dataset, dataId, fileVersion);
            if (result == NE_OK)
            {
                // Request was successful, but let caller know item not yet available.
                result = NE_NOENT;
            }
        }
    }
    else
    {
        result = NE_INVAL;
    }

    return result;
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerGetDataItemPath(NB_EnhancedContentManager* manager, NB_EnhancedContentDataType dataType, const char* dataset, const char* dataId, nb_boolean zipped, uint32 dataItemPathSize, char* dataItemPath)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentIdentifier* identifier = NULL;

    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    identifier = NB_EnhancedContentIdentifierCreate(dataType, dataset, dataId, NULL);
    if (identifier)
    {
        result = NB_EnhancedContentDataSourceBuildItemFilePath(manager->dataSource, identifier, zipped, dataItemPathSize, dataItemPath);
        NB_EnhancedContentIdentifierDestroy(identifier);
    }
    else
    {
        result = NE_INVAL;
    }

    return result;
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerRequestDataDownload(NB_EnhancedContentManager* manager, NB_EnhancedContentDataType dataType,const char* dataset, const char* dataId, const char* fileVersion)
{
    NB_Error result = NE_OK;
    const char* motorwayJunctionObjectsDatasetId = NULL;
    NB_EnhancedContentIdentifier* identifier = NULL;

    if (!IsValidState(manager) && !dataset)
    {
        return NE_INVAL;
    }

    motorwayJunctionObjectsDatasetId = NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(manager->dataSource);
    if (!(manager->allowableDownloadType & NB_ECD0_OnDemand) &&     // On demand download isn't allowable
        (dataType == NB_ECDT_CityModel ||                           // Type of city model
         (dataType == NB_ECDT_SpecificTexture && nsl_strcmp(motorwayJunctionObjectsDatasetId, dataset) != 0)))  // Type of city textures
    {
        return NE_INVAL;
    }

    if (NB_EnhancedContentDataSourceIsMetadataSynchronizeInProgress(manager->dataSource))
    {
        return NE_BUSY;
    }

    if (NB_EnhancedContentDataSourceIsManifestSynchronizeInProgress(manager->dataSource))
    {
        return NE_BUSY;
    }

    identifier = NB_EnhancedContentIdentifierCreate(dataType, dataset, dataId, fileVersion);
    if (identifier)
    {
        result = NB_EnhancedContentDownloadManagerRequestData(manager->downloadManager, identifier, fileVersion, NB_ECD0_OnDemand);
        NB_EnhancedContentIdentifierDestroy(identifier);
    }
    else
    {
        result = NE_INVAL;
    }

    return result;
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerClearOnDemandRequests(NB_EnhancedContentManager* manager, NB_EnhancedContentDataType dataType)
{
    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentDownloadManagerCancelRequests(manager->downloadManager, NB_ECD0_OnDemand, dataType);
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerCheckManifestVersion(NB_EnhancedContentManager* manager, NB_EnhancedContentDataType dataType, const char* dataset, const char* manifestVersion)
{
    NB_Error result = NE_OK;

    if (!IsValidState(manager) || !dataset || !manifestVersion)
    {
        return NE_INVAL;
    }

    result = NB_EnhancedContentDataSourceUpdateManifestVersion(manager->dataSource, dataType, dataset, manifestVersion);
    if (result == NE_OK)
    {
        result = result ? result : NE_NOENT;
    }
    else if (result == NE_EXIST)
    {
        result = NE_OK;
    }

    return result;
}


/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerCheckFileVersion(NB_EnhancedContentManager* manager,
                                          NB_EnhancedContentDataType dataType,
                                          const char* dataset,
                                          const char* dataId,
                                          const char* fileVersion)
{
    NB_Error result = NE_OK;

    if (!IsValidState(manager) || !dataset || !dataId || !fileVersion)
    {
        return NE_INVAL;
    }

    result = NB_EnhancedContentDataSourceCheckFileVersion(manager->dataSource, dataType, dataset, dataId, fileVersion);
    if (result == NE_NOENT)
    {
        result = NB_EnhancedContentManagerRemoveData(manager, dataType, dataset, dataId,
                                                     fileVersion);
        result = result ? result : NE_NOENT;
    }

    return result;
}

/* See header file for description. */
NB_DEF const char*
NB_EnhancedContentManagerGetMotorwayJunctionObjectsDatasetId(NB_EnhancedContentManager* manager)
{
    if (!IsValidState(manager))
    {
        return NULL;
    }

    return NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(manager->dataSource);
}


/* See header file for description. */
NB_DEF const char*
NB_EnhancedContentManagerGetRealisticSignsDatasetId(NB_EnhancedContentManager* manager)
{
    if (!IsValidState(manager))
    {
        return NULL;
    }

    return NB_EnhancedContentDataSourceGetRealisticSignsDatasetId(manager->dataSource);
}

/* See header file for description. */
NB_DEF const char*
NB_EnhancedContentManagerGetSpeedLimitsSignDatasetId(NB_EnhancedContentManager* manager)
{
    if (!IsValidState(manager))
    {
        return NULL;
    }
    return NB_EnhancedContentDataSourceGetSpeedLimitsSignDatasetId(manager->dataSource);
}

/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerGetDataItemSize(NB_EnhancedContentManager* manager,
                                         NB_EnhancedContentDataType dataType,
                                         const char* dataset,
                                         const char* dataId,
                                         uint32* dataSize)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentIdentifier* identifier = NULL;

    if (!IsValidState(manager) || !dataSize)
    {
        return NE_INVAL;
    }

    identifier = NB_EnhancedContentIdentifierCreate(dataType, dataset, dataId, NULL);
    if (identifier)
    {
        result = NB_EnhancedContentDataSourceGetDataItemSize(manager->dataSource, identifier, dataSize);
        NB_EnhancedContentIdentifierDestroy(identifier);
    }
    else
    {
        result = NE_INVAL;
    }

    return result;
}

/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerUpdateDataItemDownloadStatus(NB_EnhancedContentManager* manager,
                                                      NB_EnhancedContentDataType dataType,
                                                      const char* dataset,
                                                      const char* dataId,
                                                      NB_EnhancedContentUpdateStatus updateStatus,
                                                      NB_EnhancedContentDownloadStatus downloadStatus,
                                                      uint32 downloadedSize
                                                      )
{
    NB_Error result = NE_OK;
    NB_EnhancedContentIdentifier* identifier = NULL;

    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    identifier = NB_EnhancedContentIdentifierCreate(dataType, dataset, dataId, NULL);
    if (identifier)
    {
        result = NB_EnhancedContentDataSourceUpdateDataItemDownloadStatus(manager->dataSource, identifier, updateStatus, downloadStatus, downloadedSize);
        NB_EnhancedContentIdentifierDestroy(identifier);
    }
    else
    {
        result = NE_INVAL;
    }

    return result;
}

/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerRemoveData(NB_EnhancedContentManager* manager,
                                    NB_EnhancedContentDataType dataType,
                                    const char* dataset,
                                    const char* dataId,
                                    const char* fileVersion)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentIdentifier* identifier = NULL;

    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    identifier = NB_EnhancedContentIdentifierCreate(dataType, dataset, dataId, fileVersion);
    if (identifier)
    {
        // result = result ? result : NB_EnhancedContentDownloadManagerCancelRequestData(manager->downloadManager, NB_ECDO_All, identifier);
        result = result ? result : NB_EnhancedContentDownloadManagerRemoveData(manager->downloadManager, identifier, FALSE);
        NB_EnhancedContentIdentifierDestroy(identifier);

        // Ignore NE_NOENT error.
        result = result == NE_NOENT ? NE_OK : result;
    }
    else
    {
        result = NE_INVAL;
    }

    return result;
}

/* See header file for description. */
NB_DEF NB_Error
NB_EnhancedContentManagerDownloadNextDataset(NB_EnhancedContentManager* manager)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentDownloadType downloadType = NB_ECDO_None;
    const char* motorwayJunctionObjectsDatasetId = NULL;
    NB_EnhancedContentIdentifier* identifier = NULL;
    uint32 fileSize = 0;

    if (NB_EnhancedContentDataSourceIsMetadataSynchronizeInProgress(manager->dataSource))
    {
        return NE_BUSY;
    }

    if (NB_EnhancedContentDataSourceIsManifestSynchronizeInProgress(manager->dataSource))
    {
        return NE_BUSY;
    }

    if (NB_EnhancedContentDownloadManagerIsPaused(manager->downloadManager, NB_ECDO_All))
    {
        return NE_NOENT;
    }

    if (NB_EnhancedContentDownloadManagerIsPending(manager->downloadManager, NB_ECDO_All))
    {
        return NE_BUSY;
    }

    identifier = NB_EnhancedContentIdentifierCreate(NB_ECDT_None, NULL, NULL, NULL);
    if (!identifier)
    {
        return NE_NOMEM;
    }

    //check if the next entry is available
    while(result == NE_OK)
    {
        result = NB_EnhancedContentDataSourceGetNextDatasetToUpdate(manager->dataSource,
                        (nb_boolean) (manager->allowableDownloadType & NB_ECD0_Preload),
                        identifier);
        if (identifier)
        {
            /* get the file size which will be used to check if the disk free space is enough
               and downloaded size for break transmission */
            NB_EnhancedContentDataSourceGetDataItemSize(manager->dataSource, identifier, &fileSize);
            /*Check if disk free space is enough for the region, if no ,return the error.
                As the file size is the original size, so 2*file Size is larger than
                original size + zipped size.*/
            if (PAL_FileGetFreeSpace(NB_ContextGetPal(manager->context)) > 2 * fileSize)
            {
                break;
            }
            else if (manager->downloadCallback.callback)
            {
                manager->downloadCallback.callback(manager, NE_FSYS_NOSPACE, 0, identifier->type,
                                                   identifier->dataset,manager->downloadCallback.callbackData);
            }
        }
    }
    if (result != NE_OK)
    {
        NB_EnhancedContentIdentifierDestroy(identifier);
        return result;
    }

    motorwayJunctionObjectsDatasetId = NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(manager->dataSource);
    if (identifier->type == NB_ECDT_CommonTexture ||
        (identifier->type == NB_ECDT_SpecificTexture && nsl_strcmp(motorwayJunctionObjectsDatasetId, identifier->dataset) == 0))
    {
        downloadType = NB_ECD0_OnDemand;
    }
    else
    {
        downloadType = NB_ECD0_Preload;
    }

    if (NB_EnhancedContentDownloadManagerIsPaused(manager->downloadManager, downloadType))
    {
        NB_EnhancedContentIdentifierDestroy(identifier);
        return NE_NOENT;
    }

    if (NB_EnhancedContentDownloadManagerIsPending(manager->downloadManager, downloadType))
    {
        NB_EnhancedContentIdentifierDestroy(identifier);
        return NE_BUSY;
    }

    if (!(manager->allowableDownloadType & downloadType) &&         // Download type isn't allowable
        (identifier->type == NB_ECDT_CityModel ||                   // Type of city model
         (identifier->type == NB_ECDT_SpecificTexture && nsl_strcmp(motorwayJunctionObjectsDatasetId, identifier->dataset) != 0)))  // Type of city textures
    {
        NB_EnhancedContentIdentifierDestroy(identifier);
        return NE_INVAL;
    }

    // Request whole dataset here except MJO and SAR. File version is only used for MJOs and SARs.
    result = NB_EnhancedContentDownloadManagerRequestData(manager->downloadManager, identifier, NULL, downloadType);
    if (result != NE_OK)
    {
        NB_EnhancedContentIdentifierDestroy(identifier);
        return result;
    }

    // If preload isn't paused return downloading percent.
    if (manager->downloadCallback.callback && !NB_EnhancedContentDownloadManagerIsPaused(manager->downloadManager, NB_ECD0_Preload))
    {
        uint64 totalSize = 0;
        uint64 downloadedSize = 0;
        uint32 totalCommonTexturesSize = 0;
        uint32 downloadedCommonTexturesSize = 0;
        uint32 downloadedPercentage = 0;

        result = NB_EnhancedContentManagerGetStorageStatistics(manager, &totalSize, &downloadedSize, &totalCommonTexturesSize, &downloadedCommonTexturesSize, &downloadedPercentage);
        if (result == NE_OK)
        {
            manager->downloadCallback.callback(manager, NE_OK, downloadedPercentage, identifier->type, identifier->dataset, manager->downloadCallback.callbackData);
        }
    }

    NB_EnhancedContentIdentifierDestroy(identifier);

    return result;
}

/* See header file for description. */
NB_DEF nb_boolean
NB_EnhancedContentManagerIsCityAvailable(NB_EnhancedContentManager* manager,
                                         const char* cityId)
{
    if (!IsValidState(manager) || !cityId)
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentManagerIsDataAvailableWithDependencies(manager, NB_ECDT_CityModel, cityId, NULL);
}

/* See header file for description. */
NB_DEF nb_boolean
NB_EnhancedContentManagerIsCitySelected(NB_EnhancedContentManager* manager,
                                        const char* cityId)
{
    if (!IsValidState(manager) || !cityId)
    {
        return NE_INVAL;
    }

    return NB_EnhancedContentDataSourceIsDatasetSelected(manager->dataSource, NB_ECDT_CityModel, cityId);
}

/* See header file for description. */
NB_DEF NB_EnhancedContentDownloadType
NB_EnhancedContentManagerGetAllowableDownload(NB_EnhancedContentManager* manager)
{
    if (! manager)
    {
        return NB_ECDO_None;
    }
    
    return manager->allowableDownloadType;
}


// Functions declared in "nbenhancedcontentmanagerprivate.h" ............................................................

/* See header for description */
NB_EnhancedContentIdentifier*
NB_EnhancedContentIdentifierCreate(NB_EnhancedContentDataType type, const char* dataset,
                                   const char* itemId, const char* fileVersion)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentIdentifier* identifier = nsl_malloc(sizeof(NB_EnhancedContentIdentifier));
    if (! identifier)
    {
        return NULL;
    }

    nsl_memset(identifier, 0, sizeof(NB_EnhancedContentIdentifier));

    result = NB_EnhancedContentIdentifierSet(identifier, type, dataset, itemId, fileVersion);
    if (result != NE_OK)
    {
        NB_EnhancedContentIdentifierDestroy(identifier);
        identifier = NULL;
    }

    return identifier;
}

/* See header for description */
void
NB_EnhancedContentIdentifierDestroy(NB_EnhancedContentIdentifier* identifier)
{
    if (! identifier)
    {
        return;
    }
    if (identifier->dataset)
    {
        nsl_free(identifier->dataset);
    }
    if (identifier->itemId)
    {
        nsl_free(identifier->itemId);
    }
    if (identifier->fileVersion)
    {
        nsl_free(identifier->fileVersion);
    }
    nsl_free(identifier);
}

/* See header for description */
NB_Error
NB_EnhancedContentIdentifierSet(NB_EnhancedContentIdentifier* identifier,
                                NB_EnhancedContentDataType type, const char* dataset,
                                const char* itemId, const char* fileVersion)
{
    if (! identifier)
    {
        return NE_INVAL;
    }

    identifier->type = type;

    if (identifier->dataset != dataset)
    {
        if (identifier->dataset)
        {
            nsl_free(identifier->dataset);
            identifier->dataset = NULL;
        }

        if (dataset)
        {
            identifier->dataset = nsl_strdup(dataset);
            if (! identifier->dataset)
            {
                return NE_NOMEM;
            }
        }
    }

    if (identifier->itemId != itemId)
    {
        if (identifier->itemId)
        {
            nsl_free(identifier->itemId);
            identifier->itemId = NULL;
        }

        if (itemId)
        {
            identifier->itemId = nsl_strdup(itemId);
            if (! identifier->itemId)
            {
                return NE_NOMEM;
            }
        }
    }

    if (identifier->fileVersion != fileVersion)
    {
        if (identifier->fileVersion)
        {
            nsl_free(identifier->fileVersion);
            identifier->fileVersion = NULL;
        }

        if (fileVersion)
        {
            identifier->fileVersion = nsl_strdup(fileVersion);
            if (! identifier->fileVersion)
            {
                return NE_NOMEM;
            }
        }
    }

    return NE_OK;
}

/* Refer to header for description */
nb_boolean
NB_EnhancedContentManagerDownloadIsWaitingForMetadata(NB_EnhancedContentManager* manager)
{
    return NB_EnhancedContentDownloadManagerIsWaitingMetadata(manager->downloadManager);
}


// Local functions for Data Source .............................................................

static void
DataSourceMetadataSynchronizeCallback(
    NB_EnhancedContentDataSource* dataSource,
    NB_Error result,
    NB_RequestStatus status,
    uint32 percent,
    CSL_Vector* deletedDatasetReplies,
    CSL_Vector* addedCities,
    CSL_Vector* updatedCities,
    CSL_Vector* deletedCities,
    void* userData)
{
    int n = 0;
    int length = 0;
    int addedCityCount = 0;
    int updatedCityCount = 0;
    int deletedCityCount = 0;
    NB_MetadataSourceReply* reply = NULL;
    NB_EnhancedContentCityData* addedCityArray = NULL;
    NB_EnhancedContentCityData* updatedCityArray = NULL;
    NB_EnhancedContentCityData* deletedCityArray = NULL;
    NB_EnhancedContentManager* manager = (NB_EnhancedContentManager*) userData;

    if (result == NE_OK && deletedDatasetReplies)
    {
        length = CSL_VectorGetLength(deletedDatasetReplies);
        for (n = 0; n < length; ++n)
        {
            reply = (NB_MetadataSourceReply*) CSL_VectorGetPointer(deletedDatasetReplies, n);
            result = NB_EnhancedContentManagerRemoveData(manager, reply->contentType,
                                                         reply->datasetId, NULL, NULL);
            if (result != NE_OK)
            {
                break;
            }
        }
    }


    if (manager->synchronizationCallback.callback)
    {
        if (result == NE_OK && status == NB_NetworkRequestStatus_Success)
        {
            if (addedCities)
            {
                addedCityCount = CSL_VectorGetLength(addedCities);
                if (addedCityCount > 0)
                {
                    addedCityArray = (NB_EnhancedContentCityData*)CSL_VectorGetPointer(addedCities, 0);
                }
            }

            if (updatedCities)
            {
                updatedCityCount = CSL_VectorGetLength(updatedCities);
                if (updatedCityCount > 0)
                {
                    updatedCityArray = (NB_EnhancedContentCityData*)CSL_VectorGetPointer(updatedCities, 0);
                }
            }

            if (deletedCities)
            {
                deletedCityCount = CSL_VectorGetLength(deletedCities);
                if (deletedCityCount > 0)
                {
                    deletedCityArray = (NB_EnhancedContentCityData*)CSL_VectorGetPointer(deletedCities, 0);
                }
            }
        }

        manager->synchronizationCallback.callback(manager, result, status, percent, (uint32) addedCityCount, addedCityArray, (uint32) updatedCityCount, updatedCityArray, (uint32) deletedCityCount, deletedCityArray, manager->synchronizationCallback.callbackData);

    }

    if (NB_EnhancedContentDownloadManagerIsWaitingMetadata(manager->downloadManager) &&
        percent == 100)
        // Some download is paused to wait metadata.
    {
        NB_EnhancedContentDownloadManagerSetWaitingMetadata(manager->downloadManager, FALSE);
    }

}

static void
DataSourceManifestSynchronizeCallback(NB_EnhancedContentDataSource* dataSource,
                                      NB_Error result,
                                      NB_RequestStatus status,
                                      NB_EnhancedContentDataType contentType,
                                      const char* datasetId,
                                      CSL_Vector* removedManifestReplies,
                                      void* userData)
{
    NB_EnhancedContentManager* manager = (NB_EnhancedContentManager*) userData;
    if (!manager)
    {
        return;
    }

    // Remove data of removed manifest replies.
    if ((contentType != NB_ECDT_None) &&
        (datasetId) && (nsl_strlen(datasetId) > 0) &&
        removedManifestReplies)
    {
        int n = 0;
        int length = CSL_VectorGetLength(removedManifestReplies);

        for (n = 0; n < length; ++n)
        {
            NB_MetadataManifestReply* reply = (NB_MetadataManifestReply*) CSL_VectorGetPointer(removedManifestReplies,
                                                                                               n);

            if (!reply)
            {
                continue;
            }

            // Ignore the returned error.
            NB_EnhancedContentManagerRemoveData(manager,
                                                contentType,
                                                datasetId,
                                                reply->fileId,
                                                reply->fileVersion);
        }
    }

    if (result == NE_OK)
    {
        switch (status)
        {
            case NB_NetworkRequestStatus_Success:
            {
                result = NB_EnhancedContentManagerDownloadNextDataset(manager);

                // If result is NE_OK start downloading.
                // If result is NE_NOENT no need to download.
                // If result is NE_BUSY downloading is pending.
                if (result == NE_OK || result == NE_BUSY)
                {
                    // Don't callback here. Callback when download complete.
                    return;
                }
                else if (result == NE_NOENT)
                {
                    result = NE_OK;
                    if (manager->downloadCallback.callback)
                    {
                        manager->downloadCallback.callback(manager, NE_OK, 100,
                                                            NB_ECDT_None, datasetId,
                                                            manager->downloadCallback.callbackData);
                        return;
                    }
                }

                break;
            }
            case NB_NetworkRequestStatus_Failed:
            {
                result = NESERVER_FAILED;
                break;
            }
            case NB_NetworkRequestStatus_TimedOut:
            {
                result = NESERVER_TIMEDOUT;
                break;
            }
            default:
            {
                // TODO: Should response canceled and progress.
                return;
            }
        }
    }

    if (result != NE_OK && manager->downloadCallback.callback)
    {
        manager->downloadCallback.callback(manager, result, 0, NB_ECDT_None, NULL, manager->downloadCallback.callbackData);
    }
}

/* Create data source instance

@return NB_OK if success
*/
static NB_Error
CreateDataSource(NB_EnhancedContentManager* pThis)
{
    NB_EnhancedContentDataSourceMetadataSynchronizeCallback metadataCallback = { 0 };
    NB_EnhancedContentDataSourceManifestSynchronizeCallback manifestCallback = { 0 };

    metadataCallback.callback = DataSourceMetadataSynchronizeCallback;
    metadataCallback.callbackData = pThis;

    manifestCallback.callback = DataSourceManifestSynchronizeCallback;
    manifestCallback.callbackData = pThis;

    return NB_EnhancedContentDataSourceCreate(
                pThis->context,
                &pThis->configuration,
                metadataCallback,
                manifestCallback,
                &pThis->dataSource);
}


// Local functions for Download Manager ........................................................

static void
DownloadManagerCallback(NB_EnhancedContentDownloadManager* downloadManager,
                        NB_Error result,
                        uint32 percent,
                        const NB_EnhancedContentIdentifier* item,
                        void* userData)
{
    NB_EnhancedContentManager* manager = (NB_EnhancedContentManager*) userData;
    uint32 dataSize = 0;
    uint32 downloadPercentage = 0;
    NB_Error calcResult = NE_OK;

    if (!manager || !item || !item->dataset)
    {
        return;
    }

    if (result != NE_OK && manager->downloadCallback.callback)
    {
        manager->downloadCallback.callback(manager, result, 0, NB_ECDT_None, NULL,
                                           manager->downloadCallback.callbackData);
        NB_EnhancedContentManagerDownloadNextDataset(manager);
        return;
    }

    if (result == NE_OK)
    {
        calcResult = NB_EnhancedContentManagerCalculateDownloadPercentage(
                        manager, percent, item, &downloadPercentage);
        if (percent == 100)
        {
            if (NB_EnhancedContentManagerGetDataItemSize(manager, item->type, item->dataset,
                                                                item->itemId, &dataSize) == NE_OK)
            {
                result = NB_EnhancedContentManagerUpdateDataItemDownloadStatus(manager, item->type,
                                item->dataset, item->itemId, NB_ECUS_UpdateStatusUsed,
                                NB_ECDS_DownloadComplete, dataSize);
            }
            result = NB_EnhancedContentManagerDownloadNextDataset(manager);
        }
        if (manager->downloadCallback.callback)
        {
            if (result == NE_BUSY || calcResult == NE_OK)
            {
                manager->downloadCallback.callback(manager, NE_OK, downloadPercentage,
                                                   item->type, item->dataset,
                                                   manager->downloadCallback.callbackData);
            }
            else if (result == NE_NOENT)
            {
                manager->downloadCallback.callback(manager, NE_OK, 100, NB_ECDT_None, NULL,
                                                   manager->downloadCallback.callbackData);
            }
        }
    }
}

/* Create download manager instance

@return NB_OK if success
*/
static NB_Error
CreateDownloadManager(NB_EnhancedContentManager* pThis)
{
    NB_EnhancedContentDownloadManagerCallbackData downloadCallback =  { 0 };
    downloadCallback.callback = DownloadManagerCallback;
    downloadCallback.userData = pThis;

    return NB_EnhancedContentDownloadManagerCreate(
        pThis->context,
        &pThis->configuration,
        pThis->dataSource,
        &downloadCallback,
        &pThis->downloadManager);
}


// Local functions .............................................................................

/* Check if enhanced content manager instance is valid

@return NB_OK if success
*/
static nb_boolean
IsValidState(NB_EnhancedContentManager* pThis)
{
    return (nb_boolean)(pThis && pThis->dataSource && pThis->downloadManager);
}

NB_DEF NB_Error
NB_EnhancedContentManagerSelectEntry(NB_EnhancedContentManager* manager, const char* id,
                                     nb_boolean selected, NB_EnhancedContentDataType dataType)
{
    NB_Error result = NE_OK;
    nb_boolean inProgress = FALSE;
    NB_EnhancedContentIdentifier* identifier = NULL;

    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    result = NB_EnhancedContentDataSourceSelectEntry(manager->dataSource, id, selected, dataType);
    if (result != NE_OK)
    {
        return result;
    }

    // If selected is FALSE cancel city textures and city model/region download or return.
    if (selected)
    {
        return result;
    }

    inProgress = NB_EnhancedContentManagerIsDownloadInProgress(manager, NB_ECD0_Preload);

    if (dataType == NB_ECDT_CityModel)
    {
        identifier = NB_EnhancedContentIdentifierCreate(NB_ECDT_SpecificTexture, id, NULL, NULL);
        if (!identifier)
        {
            return NE_INVAL;
        }
        // Cancel city textures download
        result = NB_EnhancedContentDownloadManagerCancelRequestData(manager->downloadManager,
                                                                    NB_ECDO_All,
                                                                    identifier);
        if (result != NE_OK)
        {
            NB_EnhancedContentIdentifierDestroy(identifier);
            return result;
        }
        result = NB_EnhancedContentIdentifierSet(identifier, dataType, id, NULL, NULL);
        if (result != NE_OK)
        {
            NB_EnhancedContentIdentifierDestroy(identifier);
            return result;
        }
    }
    else
    {
        identifier = NB_EnhancedContentIdentifierCreate(dataType, id, NULL, NULL);
        if (!identifier)
        {
            return NE_INVAL;
        }
    }

    // Cancel download
    result = NB_EnhancedContentDownloadManagerCancelRequestData(manager->downloadManager,
                                                                NB_ECDO_All, identifier);
    NB_EnhancedContentIdentifierDestroy(identifier);
    if (result != NE_OK)
    {
        return result;
    }

    // Try to download next dataset if preload is in progress before removing entry.
    if (inProgress)
    {
        result = NB_EnhancedContentManagerDownloadNextDataset(manager);
        if (result == NE_BUSY)
        {
            // Ignore NE_BUSY error.
            result = NE_OK;
        }
        else if (result == NE_NOENT)
        {
            result = NE_OK;
            if (manager->downloadCallback.callback)
            {
                manager->downloadCallback.callback(manager, NE_OK, 100, NB_ECDT_None, NULL,
                                                   manager->downloadCallback.callbackData);
            }
        }
    }

    return result;
}

NB_DEF NB_Error
NB_EnhancedContentManagerRemoveEntry(NB_EnhancedContentManager* manager, const char* id,
                                     NB_EnhancedContentDataType dataType)
{
    NB_Error result = NE_OK;
    nb_boolean inProgress = FALSE;

    if (!IsValidState(manager))
    {
        return NE_INVAL;
    }

    // Reset city download status in database.
    NB_EnhancedContentDataSourceRemoveEntry(manager->dataSource, id, dataType);

    if (result != NE_OK)
    {
        return result;
    }

    inProgress = NB_EnhancedContentManagerIsDownloadInProgress(manager, NB_ECD0_Preload);

    if (dataType == NB_ECDT_CityModel)
    {
        // Cancel city textures download and remove persistent city textures
        result = NB_EnhancedContentManagerRemoveData(manager, NB_ECDT_SpecificTexture, id,
                                                     NULL, NULL);
        if (result != NE_OK)
        {
            return result;
        }
    }
    // Cancel city model/region download and remove persistent city model
    result = NB_EnhancedContentManagerRemoveData(manager, dataType, id, NULL, NULL);
    if (result != NE_OK)
    {
        return result;
    }

    // Try to download next dataset if preload is in progress before removing city.
    if (inProgress)
    {
        result = NB_EnhancedContentManagerDownloadNextDataset(manager);
        if (result == NE_BUSY)
        {
            // Ignore NE_BUSY error.
            result = NE_OK;
        }
        else if (result == NE_NOENT)
        {
            result = NE_OK;
            if (manager->downloadCallback.callback)
            {
                manager->downloadCallback.callback(manager, NE_OK, 100, NB_ECDT_None, NULL,
                                                   manager->downloadCallback.callbackData);
            }
        }
    }

    return result;
}

NB_DEF NB_Error
NB_EnhancedContentManagerGetEntryList(NB_EnhancedContentManager* manager, uint32* count,
                                      NB_EnhancedContentDataType dataType, NB_ContentEntryData** entries)
{
    NB_Error result = NE_OK;
    int length = 0;
    NB_ContentEntryData* entry = NULL;
    NB_ContentEntryData* entryArray = NULL;
    CSL_Vector* vectorEntries = NULL;

    if (!IsValidState(manager) || !count || !entries)
    {
        return NE_INVAL;
    }

    vectorEntries = CSL_VectorAlloc(sizeof(NB_ContentEntryData));
    if (!vectorEntries)
    {
        return NE_NOMEM;
    }

    result = NB_EnhancedContentDataSourceGetAvailableEntries(manager->dataSource,
                                                             dataType,
                                                             vectorEntries);

    if (result != NE_OK)
    {
        CSL_VectorDealloc(vectorEntries);
        return result;
    }

    length = CSL_VectorGetLength(vectorEntries);
    if (length <= 0)
    {
        CSL_VectorDealloc(vectorEntries);
        *count = 0;
        return NE_OK;
    }

    entryArray = nsl_malloc(length * sizeof(NB_ContentEntryData));
    if (!entryArray)
    {
        CSL_VectorDealloc(vectorEntries);
        return NE_NOMEM;
    }
    nsl_memset(entryArray, 0, length * sizeof(NB_ContentEntryData));

    entry = CSL_VectorGetPointer(vectorEntries, 0);
    nsl_memcpy(entryArray, entry, length * sizeof(NB_ContentEntryData));
    CSL_VectorDealloc(vectorEntries);

    *count = length;
    *entries = entryArray;

    return NE_OK;
}

/* See header file for description. */
NB_DEC NB_Error
NB_ContentManagerGetExistMapRegionFileIds(NB_EnhancedContentManager* manager,
                                          int32* count,
                                          char*** fileIds )
{
    NB_Error result = NE_OK;

    if (!IsValidState(manager) || !count || !fileIds)
    {
        return NE_INVAL;
    }
    result = NB_EnhancedContentDataSourceGetExistMapRegionFileIds(manager->dataSource,
                                                                  count,
                                                                  fileIds);

    if (result != NE_OK)
    {
        while (*count >= 0)
        {
            nsl_free(*fileIds[--(*count)]);
        }
        nsl_free(*fileIds);
        return result;
    }

    return NE_OK;
}

NB_DEF NB_Error NB_EnhancedContentManagerCalculateDownloadPercentage(
    NB_EnhancedContentManager* manager,
    uint32 percent,
    const NB_EnhancedContentIdentifier* item,
    uint32* percentOut)
{
    NB_Error result = NE_OK;
    uint64 totalSize = 0;
    uint64 downloadedSize = 0;
    uint32 totalCommonTexturesSize = 0;
    uint32 downloadedCommonTexturesSize = 0;
    if (!item || !manager)
    {
        return NE_INVAL;
    }
    switch (item->type)
    {
        case NB_ECDT_MapRegions:
        case NB_ECDT_MapRegionsCommon:
        {
            result = NB_EnhancedContentDataSourceCalculateRigonDownloadPercentage(
                        manager->dataSource,
                        percent, item, percentOut);
            break;
        }
        default:
        {
            if (percent != 100)
            {
                return NE_NOSUPPORT;
            }
            result = NB_EnhancedContentManagerGetStorageStatistics(manager, &totalSize,
                                    &downloadedSize, &totalCommonTexturesSize,
                                    &downloadedCommonTexturesSize, percentOut);
            break;
        }
    }
    return result;
}

NB_DEC NB_Error NB_EnhancedContentManagerPauseRegionDownload(NB_EnhancedContentManager* manager,
                                                             const char* regionId,
                                                             NB_EnhancedContentDownloadType downloadType)
{
    NB_ContentEntryData region = {0};
    NB_EnhancedContentIdentifier* identifier = NULL;
    NB_Error result = NE_OK;
    if (!IsValidState(manager) || !regionId)
    {
        return NE_INVAL;
    }
    result = NB_EnhancedContentDataSourceGetRegionInformation(manager->dataSource, regionId, &region);
    if (result == NE_OK)
    {
        identifier = NB_EnhancedContentIdentifierCreate(NB_ECDT_MapRegions, region.cityId, NULL, region.version);
        if (!identifier)
        {
            return NE_NOMEM;
        }
        result = NB_EnhancedContentDownloadManagerPauseItemDownload(manager->downloadManager,
                                                                    identifier,
                                                                    downloadType);
    }
    return result;
}

NB_DEC NB_Error NB_EnhancedContentManagerResumeRegionDownload(NB_EnhancedContentManager* manager,
                                                              const char* regionId,
                                                              NB_EnhancedContentDownloadType downloadType)
{
    NB_ContentEntryData region = {0};
    NB_EnhancedContentIdentifier* identifier = NULL;
    NB_Error result = NE_OK;
    if (!IsValidState(manager) || !regionId)
    {
        return NE_INVAL;
    }
    result = NB_EnhancedContentDataSourceGetRegionInformation(manager->dataSource, regionId, &region);
    if (result == NE_OK)
    {
        identifier = NB_EnhancedContentIdentifierCreate(NB_ECDT_MapRegions, region.cityId, NULL, region.version);
        if (!identifier)
        {
            return NE_NOMEM;
        }
        result = NB_EnhancedContentDownloadManagerResumeItemDownload(manager->downloadManager,
                                                                     identifier,
                                                                     downloadType);
    }
    return result;
}

/*! @} */

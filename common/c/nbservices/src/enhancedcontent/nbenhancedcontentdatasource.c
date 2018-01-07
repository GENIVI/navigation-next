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
 
 @file     nbenhancedcontentdatasource.c
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

#include "nbenhancedcontentdatasource.h"
#include "nbmetadatahandler.h"
#include "nbmetadatainformationprivate.h"
#include "nbmanifesthandler.h"
#include "nbmetadatadatabaseoperator.h"
#include "nbcontextprotected.h"
#include "transformutility.h"
#include "palfile.h"

/*! @{ */

// Constants .....................................................................................

/*! Database filename */
static const char METADATA_DATABASE_FILENAME[] = "metadata.dat";


/*! Url template parameters */
static const char URL_TEMPLATE_PARAMETER_ITEM_ID[]          = "$id";
static const char URL_TEMPLATE_PARAMETER_DATASET[]          = "$ds";
static const char URL_TEMPLATE_PARAMETER_FORMAT[]           = "$fmt";
static const char URL_TEMPLATE_PARAMETER_NBGM_VERSION[]     = "$fmtv";
static const char URL_TEMPLATE_PARAMETER_MANIFEST_VERSION[] = "$mv";

// Port used for all requests
static const uint16 HTTP_DEFAULT_PORT = 80;


// Local types ...................................................................................

/*! Configuration used for enhanced content data source */
typedef struct
{
    uint32 screenWidth;                                                                  /*!< The screen width of the device, specified in pixels.*/
    uint32 screenHeight;                                                                 /*!< The screen height of the device, specified in pixels.*/
    uint32 screenResolution;                                                             /*!< The resolution of the screen, specified in DPI */

    nb_boolean enhancedCityModelEnabled;                                                 /*!< Enable Enhanced City Model (ECM) */
    nb_boolean motorwayJunctionObjectsEnabled;                                           /*!< Enable Motorway Junction Objects (MJO) */
    nb_boolean realisticSignsEnabled;                                                    /*!< Enable realistic signs (SAR) */
    nb_boolean speedLimitsSignEnabled;                                                   /*!< Enable speed limits sign. */
    nb_boolean specialRegionsEnabled;                                                    /*!< Enable special regions. */
    nb_boolean mapRegionsEnabled;                                                        /*!< Enable map regions. */

    NB_SupportedMapType supportedTypes;                                                  /*!< Types of map this client can handle. */
    char country[MAX_ENHANCED_CONTENT_CONFIG_ITEM_LENGTH + 1];                           /*!< Country */
    char language[MAX_ENHANCED_CONTENT_CONFIG_ITEM_LENGTH + 1];                          /*!< Language */
    char nbgmVersion[MAX_ENHANCED_CONTENT_CONFIG_ITEM_LENGTH + 1];                       /*!< NBGM version */

    char persistentMetadataPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                   /*!< Persistent metadata path */

    char enhancedCityModelFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];         /*!< ECM file format */
    char motorwayJunctionObjectFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];    /*!< MJO file format */
    char realisticSignsFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];            /*!< SAR file format */
    char speedLimitsSignFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];           /*!< Speed limit sign file format */
    char textureFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];                   /*!< Textures file format */
    char compressedTextureFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];         /*!< Textures compressed file format */
    char specialRegionFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];             /*!< Speical region image file format */

} NB_EnhancedContentDataSourceConfiguration;

/*! The enhanced content data source */
struct NB_EnhancedContentDataSource
{
    NB_Context*                                                 context;                            /*!< NB_Context object */

    NB_MetadataHandler*                                         metadataHandler;                    /*!< NB_MetadataHandler object */
    NB_ManifestHandler*                                         manifestHandler;                    /*!< NB_ManifestHandler object */
    NB_MetadataDatabaseOperator*                                dbOperator;                         /*!< NB_MetadataDatabaseOperator object */
    NB_EnhancedContentDataSourceConfiguration                   configuration;                      /*!< configuration */
    NB_EnhancedContentDataSourceMetadataSynchronizeCallback     metadataSynchronizeCallback;        /*!< Metadata synchronization callback */
    NB_EnhancedContentDataSourceManifestSynchronizeCallback     manifestSynchronizeCallback;        /*!< Manifest synchronization callback */

    CSL_Vector*                                                 manifestQueryCities;                /*!< Vector of cities for manifest query */
    CSL_Vector*                                                 manifestQueryRegions;               /*!< Vector of regions for manifest query */

    char*                                                       citySummaryDisplay;                 /*!< Display text of available city model content
                                                                                                     Only returned if city models were requested. */

    NB_LatitudeLongitude                                        currentLocation;                    /*!< Current location */

    char commonTexturesDatasetId[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];                     /*!< Cached dataset id for common textures */
    char motorwayJunctionObjectsDatasetId[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];            /*!< Cached dataset id for motorway junction objects */
    char realisticSignsDatasetId[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];                     /*!< Cached dataset id for realistic signs */
    char speedLimitsSignDatasetId[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];                    /*!< Cached dataset id for speed limits sign */
    char specialRegionsDatasetId[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];                     /*!< Cached dataset id for special regions*/
    char regionCommonDatasetId[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];                       /*!< Cached dataset id for region common */
};


// Local Functions ...............................................................................

static NB_Error StartManifestRequest(NB_EnhancedContentDataSource* instance);

static void RequestMetadataCallback(void* handler, NB_RequestStatus status, NB_Error result, nb_boolean up, int percent, void* userData);
static void RequestManifestCallback(void* handler, NB_RequestStatus status, NB_Error result, nb_boolean up, int percent, void* userData);

static NB_Error ReplaceString(char* source, uint32 maxSourceSize, const char* sub, const char* replaced);
static NB_Error GetFullFilename(NB_EnhancedContentDataSource* dataSource, const char* path, const char* filename, uint32 maxFullFilenameSize, char* fullFilename);
static NB_Error GetDatasetIdByContentType(NB_MetadataDatabaseOperator* dbOperator, NB_EnhancedContentDataType contentType, uint32 maxDatasetIdSize, char* datasetId);
static NB_Error ConvertMetadataSourceReplyToEnhancedContentCityData(const NB_MetadataSourceReply* reply, NB_EnhancedContentCityData* city);
static NB_Error UpdateManifestReplies(NB_MetadataDatabaseOperator* dbOperator, NB_MetadataSourceReply* metadataToUpdate, CSL_Vector* replies, CSL_Vector** removedReplies);

static int CompareManifestFileId(const NB_MetadataManifestReply* reply1, const NB_MetadataManifestReply* reply2);

static int DisplayNameCompareFunction(const void* left, const void* right);


// Public functions ..............................................................................

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceCreate(NB_Context* context,
                                   const NB_EnhancedContentManagerConfiguration* managerConfiguration,
                                   NB_EnhancedContentDataSourceMetadataSynchronizeCallback metadataSynchronizeCallback,
                                   NB_EnhancedContentDataSourceManifestSynchronizeCallback manifestSynchronizeCallback,
                                   NB_EnhancedContentDataSource** instance)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentDataSource* pThis = NULL;
    NB_RequestHandlerCallback callback = {0};
    char databaseFullFilename[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];

    if (!context || !managerConfiguration || !instance)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));
    pThis->context = context;

    // Set NB_EnhancedContentDataSourceConfiguration from NB_EnhancedContentManagerConfiguration.
    pThis->configuration.screenWidth = managerConfiguration->screenWidth;
    pThis->configuration.screenHeight = managerConfiguration->screenHeight;
    pThis->configuration.screenResolution = managerConfiguration->screenResolution;

    pThis->configuration.enhancedCityModelEnabled = managerConfiguration->enhancedCityModelEnabled;
    pThis->configuration.motorwayJunctionObjectsEnabled = managerConfiguration->motorwayJunctionObjectsEnabled;
    pThis->configuration.realisticSignsEnabled = managerConfiguration->realisticSignsEnabled;
    pThis->configuration.speedLimitsSignEnabled = managerConfiguration->speedLimitsSignEnabled;
    pThis->configuration.specialRegionsEnabled = managerConfiguration->specialRegionsEnabled;
    pThis->configuration.mapRegionsEnabled = managerConfiguration->mapRegionsEnabled;

    pThis->configuration.supportedTypes  =  managerConfiguration->supportedTypes;

    nsl_strlcpy(pThis->configuration.country, managerConfiguration->country, sizeof(pThis->configuration.country));
    nsl_strlcpy(pThis->configuration.language, managerConfiguration->language, sizeof(pThis->configuration.language));
    nsl_strlcpy(pThis->configuration.nbgmVersion, managerConfiguration->nbgmVersion, sizeof(pThis->configuration.nbgmVersion));
    nsl_strlcpy(pThis->configuration.persistentMetadataPath, managerConfiguration->persistentMetadataPath, sizeof(pThis->configuration.persistentMetadataPath));
    nsl_strlcpy(pThis->configuration.enhancedCityModelFileFormat, managerConfiguration->enhancedCityModelFileFormat, sizeof(pThis->configuration.enhancedCityModelFileFormat));
    nsl_strlcpy(pThis->configuration.motorwayJunctionObjectFileFormat, managerConfiguration->motorwayJunctionObjectFileFormat, sizeof(pThis->configuration.motorwayJunctionObjectFileFormat));
    nsl_strlcpy(pThis->configuration.realisticSignsFileFormat, managerConfiguration->realisticSignsFileFormat, sizeof(pThis->configuration.realisticSignsFileFormat));

    nsl_strlcpy(pThis->configuration.speedLimitsSignFileFormat,
                managerConfiguration->speedLimitsSignFileFormat,
                sizeof(pThis->configuration.speedLimitsSignFileFormat));

    nsl_strlcpy(pThis->configuration.textureFileFormat, managerConfiguration->textureFileFormat, sizeof(pThis->configuration.textureFileFormat));
    nsl_strlcpy(pThis->configuration.compressedTextureFileFormat, managerConfiguration->compressedTextureFileFormat, sizeof(pThis->configuration.compressedTextureFileFormat));

    pThis->metadataSynchronizeCallback = metadataSynchronizeCallback;
    pThis->manifestSynchronizeCallback = manifestSynchronizeCallback;

    callback.callbackData = pThis;

    // Initialize NB_MetadataHandler.
    callback.callback = RequestMetadataCallback;
    result = NB_MetadataHandlerCreate(context, &callback, &pThis->metadataHandler);
    if (result != NE_OK)
    {
        NB_EnhancedContentDataSourceDestroy(pThis);
        return result;
    }

    // Initialize NB_ManifestHandler.
    callback.callback = RequestManifestCallback;
    result = NB_ManifestHandlerCreate(context, &callback, &pThis->manifestHandler);
    if (result != NE_OK)
    {
        NB_EnhancedContentDataSourceDestroy(pThis);
        return result;
    }

    // Create persistent metadata path.
    if (PAL_FileCreateDirectoryEx(NB_ContextGetPal(context),
                                  managerConfiguration->persistentMetadataPath) != PAL_Ok)
    {
        NB_EnhancedContentDataSourceDestroy(pThis);
        return NE_FSYS;
    }

    // Initialize database connection.
    nsl_memset(databaseFullFilename, 0, MAX_ENHANCED_CONTENT_PATH_LENGTH + 1);
    result = GetFullFilename(pThis,
                            managerConfiguration->persistentMetadataPath,
                            METADATA_DATABASE_FILENAME,
                            MAX_ENHANCED_CONTENT_PATH_LENGTH + 1,
                            databaseFullFilename);
    if (result != NE_OK)
    {
        NB_EnhancedContentDataSourceDestroy(pThis);
        return result;
    }
    result = NB_MetadataDatabaseOperatorCreate(NB_ContextGetPal(context),
                                               databaseFullFilename, &pThis->dbOperator);
    if (result != NE_OK)
    {
        NB_EnhancedContentDataSourceDestroy(pThis);
        return result;
    }

    pThis->manifestQueryCities = CSL_VectorAlloc(sizeof(NB_ManifestQueryCity));
    if (!pThis->manifestQueryCities)
    {
        NB_EnhancedContentDataSourceDestroy(pThis);
        return NE_NOMEM;
    }

    pThis->manifestQueryRegions = CSL_VectorAlloc(sizeof(NB_ManifestQueryCity));
    if (!pThis->manifestQueryRegions)
    {
        NB_EnhancedContentDataSourceDestroy(pThis);
        return NE_NOMEM;
    }

    pThis->currentLocation.latitude = INVALID_LATITUDE;
    pThis->currentLocation.longitude = INVALID_LONGITUDE;

    *instance = pThis;

    // TODO: Should update the downloaded size of dataset like JCC?

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceDestroy(NB_EnhancedContentDataSource* instance)
{
    if (instance)
    {
        if (instance->metadataHandler)
        {
            NB_MetadataHandlerDestroy(instance->metadataHandler);
            instance->metadataHandler = NULL;
        }

        if (instance->manifestHandler)
        {
            NB_ManifestHandlerDestroy(instance->manifestHandler);
            instance->manifestHandler = NULL;
        }

        if (instance->dbOperator)
        {
            NB_MetadataDatabaseOperatorDestroy(instance->dbOperator);
            instance->dbOperator = NULL;
        }

        if (instance->manifestQueryCities)
        {
            CSL_VectorDealloc(instance->manifestQueryCities);
            instance->manifestQueryCities = NULL;
        }

        if (instance->manifestQueryRegions)
        {
            CSL_VectorDealloc(instance->manifestQueryRegions);
            instance->manifestQueryRegions = NULL;
        }

        if (instance->citySummaryDisplay)
        {
            nsl_free(instance->citySummaryDisplay);
            instance->citySummaryDisplay = NULL;
        }

        nsl_free(instance);
        instance = NULL;
    }

    return NE_OK;
}

/* See header file for description */
nb_boolean NB_EnhancedContentDataSourceIsValid(NB_EnhancedContentDataSource* instance)
{
    const char* commonTexturesDatasetId  = NULL;
    const char* realisticSignsDatasetId  = NULL;

    if (!instance)
    {
        return FALSE;
    }

    if (instance->configuration.motorwayJunctionObjectsEnabled ||
        instance->configuration.enhancedCityModelEnabled)
    {
        commonTexturesDatasetId = NB_EnhancedContentDataSourceGetCommonTexturesDatasetId(instance);
        if (!commonTexturesDatasetId || nsl_strlen(commonTexturesDatasetId) == 0)
        {
            return FALSE;
        }
    }

    if (instance->configuration.realisticSignsEnabled)
    {
        realisticSignsDatasetId = NB_EnhancedContentDataSourceGetRealisticSignsDatasetId(instance);
        if (!realisticSignsDatasetId || nsl_strlen(realisticSignsDatasetId) == 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceStartMetadataSynchronize(NB_EnhancedContentDataSource* instance)
{
    NB_Error result = NE_OK;
    NB_MetadataParameters* parameters = NULL;
    NB_EnhancedContentDataType wantedContentType = NB_ECDT_None;
    nb_boolean wantExtenedMap = FALSE;
    nb_boolean wantSharedMap  = FALSE;
    nb_boolean wantUnifiedMap = FALSE;

    if (!instance || !instance->metadataHandler)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(instance->context);

    if (NB_EnhancedContentDataSourceIsMetadataSynchronizeInProgress(instance))
    {
        return NE_BUSY;
    }

    if (instance->configuration.supportedTypes & NB_ECDT_MAP_SHARED)
    {
        wantSharedMap = TRUE;
    }
    if (instance->configuration.supportedTypes & NB_ECDT_MAP_UNIFIED)
    {
        wantUnifiedMap = TRUE;
    }
    if ((instance->configuration.supportedTypes & NB_ECDT_MAP_EXTENDED) ||
        (wantSharedMap == FALSE && wantUnifiedMap == FALSE))
    {
        wantExtenedMap = TRUE;
    }

    result = NB_MetadataParametersCreate(instance->context,
                                         wantExtenedMap,
                                         wantSharedMap,
                                         wantUnifiedMap,
                                         instance->configuration.language,
                                         instance->configuration.screenWidth,
                                         instance->configuration.screenHeight,
                                         instance->configuration.screenResolution,
                                         0, &parameters);
    if (result != NE_OK)
    {
        return result;
    }

    if (instance->configuration.enhancedCityModelEnabled)
    {
        wantedContentType |= NB_ECDT_CityModel | NB_ECDT_CommonTexture | NB_ECDT_SpecificTexture;
    }
    if (instance->configuration.motorwayJunctionObjectsEnabled)
    {
        wantedContentType |= NB_ECDT_MotorwayJunctionObject | NB_ECDT_CommonTexture | NB_ECDT_SpecificTexture;
    }
    if (instance->configuration.realisticSignsEnabled)
    {
        wantedContentType |= NB_ECDT_RealisticSign;
    }
    if (instance->configuration.speedLimitsSignEnabled)
    {
        wantedContentType |= NB_ECDT_SpeedLimitsSign;
    }
    if (instance->configuration.specialRegionsEnabled)
    {
        wantedContentType |= NB_ECDT_SpecialRegions;
    }
    if (instance->configuration.mapRegionsEnabled)
    {
        wantedContentType |= NB_ECDT_MapRegions | NB_ECDT_MapRegionsCommon;
    }

    result = NB_MetadataParametersAddWantedContent(parameters, instance->configuration.country,
                                                   "", wantedContentType);
    if (result != NE_OK)
    {
        NB_MetadataParametersDestroy(parameters);
        return result;
    }

    result = NB_MetadataHandlerStartRequest(instance->metadataHandler, parameters);
    NB_MetadataParametersDestroy(parameters);

    return result;
}

NB_Error ManifestEntriesFromReply(CSL_Vector* manifestEntries, CSL_Vector* replies)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply* reply = NULL;
    NB_ManifestEntry manifestQuery = {0};
    int n = 0;
    int length = CSL_VectorGetLength(replies);
    for (n = 0; n < length; ++n)
    {
        reply = (NB_MetadataSourceReply*) CSL_VectorGetPointer(replies, n);

        if ((!reply) || (reply->manifestState == NB_ECMS_ManifestLatest))
        {
            continue;
        }

        // Add this entries to query manifest.
        nsl_memset(&manifestQuery, 0, sizeof(manifestQuery));

        manifestQuery.metadataId = reply->id;
        nsl_strlcpy(manifestQuery.cityId, reply->datasetId, sizeof(manifestQuery.cityId));

        result = CSL_VectorAppend(manifestEntries, &manifestQuery) ? NE_OK : NE_NOMEM;
        if (result != NE_OK)
        {
            break;
        }
    }
    return result;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceStartManifestSynchronize(NB_EnhancedContentDataSource* instance)
{
    NB_Error result = NE_OK;
    CSL_Vector* vectorCitiesReplies = NULL;
    CSL_Vector* vectorRegionsReplies = NULL;

    if (!instance || !instance->manifestHandler || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(instance->context);

    if (NB_EnhancedContentDataSourceIsManifestSynchronizeInProgress(instance))
    {
        return NE_BUSY;
    }

    // Alloc vector of metadata source replies.
    vectorCitiesReplies = CSL_VectorAlloc(sizeof(NB_MetadataSourceReply));
    vectorRegionsReplies = CSL_VectorAlloc(sizeof(NB_MetadataSourceReply));

    if (vectorCitiesReplies)
    {
        result = NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByContentType(
                    instance->dbOperator,
                    (int) NB_ECDT_CityModel,
                    vectorCitiesReplies);
        result = result != NE_OK ? result:
            ManifestEntriesFromReply(instance->manifestQueryCities, vectorCitiesReplies);
        CSL_VectorDealloc(vectorCitiesReplies);
    }

    if (vectorRegionsReplies)
    {
        result = NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByContentType(
                    instance->dbOperator,
                    (int) NB_ECDT_MapRegions,
                    vectorRegionsReplies);
        result = result != NE_OK ? result:
                ManifestEntriesFromReply(instance->manifestQueryRegions, vectorRegionsReplies);
        CSL_VectorDealloc(vectorRegionsReplies);
    }

    result = StartManifestRequest(instance);

    if (result != NE_OK)
    {
        CSL_VectorRemoveAll(instance->manifestQueryCities);
        CSL_VectorRemoveAll(instance->manifestQueryRegions);
    }

    return result;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceCancelManifestSynchronize(NB_EnhancedContentDataSource* instance)
{
    NB_Error result = NE_OK;

    if (!instance || !instance->manifestHandler)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(instance->context);

    CSL_VectorRemoveAll(instance->manifestQueryCities);

    if (NB_ManifestHandlerIsRequestInProgress(instance->manifestHandler))
    {
        result = NB_ManifestHandlerCancelRequest(instance->manifestHandler);
        if (result != NE_OK)
        {
            return result;
        }
    }

    return result;
}

/* See header file for description */
nb_boolean
NB_EnhancedContentDataSourceIsMetadataSynchronizeInProgress(NB_EnhancedContentDataSource* instance)
{
    if (!instance || !instance->metadataHandler)
    {
        return FALSE;
    }

    return NB_MetadataHandlerIsRequestInProgress(instance->metadataHandler);
}

/* See header file for description */
nb_boolean
NB_EnhancedContentDataSourceIsManifestSynchronizeInProgress(NB_EnhancedContentDataSource* instance)
{
    if (!instance || !instance->manifestHandler)
    {
        return FALSE;
    }

    return NB_ManifestHandlerIsRequestInProgress(instance->manifestHandler);
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetAvailableCities(NB_EnhancedContentDataSource* instance,
                                               CSL_Vector* vectorCities)
{
    return NB_EnhancedContentDataSourceGetAvailableEntries(instance, NB_ECDT_CityModel, vectorCities);
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetSelectedCities(NB_EnhancedContentDataSource* instance,
                                              CSL_Vector* vectorCities)
{
    return NB_EnhancedContentDataSourceGetSelectedEntries(instance,vectorCities,NB_ECDT_CityModel);
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceSelectCity(NB_EnhancedContentDataSource* instance,
                                       const char* cityId,
                                       nb_boolean selected)
{
    return NB_EnhancedContentDataSourceSelectEntry(instance,cityId,selected,NB_ECDT_CityModel);
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceRemoveCity(NB_EnhancedContentDataSource* instance,
                                       const char* cityId)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !cityId || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    NB_EnhancedContentDataSourceRemoveEntry(instance, cityId, NB_ECDT_CityModel);

    // Reset city textures in database
    nsl_memset(&reply, 0, sizeof(reply));
    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, NB_ECDT_SpecificTexture,
                cityId, &reply);
    if (result != NE_OK)
    {
        return result;
    }

    reply.downloadedSize = 0;
    result = NB_MetadataDatabaseOperatorUpdateMetadata(instance->dbOperator, &reply);
    // City textures don't have manifest items

    return result;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetCityInformation(NB_EnhancedContentDataSource* instance,
                                               const char* cityId,
                                               NB_EnhancedContentCityData* city)
{
    if (!instance || !cityId || !city || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    return NB_MetadataDatabaseOperatorGetCityByCityId(instance->dbOperator, cityId, city);
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetAvailableEntries(NB_EnhancedContentDataSource* instance,
                                                NB_EnhancedContentDataType dataType,
                                                CSL_Vector* vectorEntries)
{
    NB_Error result = NE_OK;
    const char* motorwayJunctionObjectsDatasetId = NULL;
    NB_ContentEntryData entry;
    nsl_memset(&entry, 0, sizeof(entry));

    if (!instance || !vectorEntries || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    if(dataType == NB_ECDT_CityModel)
    {
        motorwayJunctionObjectsDatasetId = NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(instance);
        result = NB_MetadataDatabaseOperatorGetAvailableCities(instance->dbOperator,
                                                               motorwayJunctionObjectsDatasetId,
                                                               vectorEntries);
    }
    else if(dataType == NB_ECDT_MapRegions)
    {
         result = NB_MetadataDatabaseOperatorGetAvailableRegions(instance->dbOperator, vectorEntries);
    }

    if (result != NE_OK)
    {
        return result;
    }

    CSL_VectorSort2(vectorEntries, &DisplayNameCompareFunction, &entry);

    return result;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetSelectedEntries(NB_EnhancedContentDataSource* instance,
                                               CSL_Vector* vectorEntries,
                                               NB_EnhancedContentDataType dataType)
{
    if (!instance || !vectorEntries || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    switch (dataType)
    {
    case NB_ECDT_CityModel:
        return NB_MetadataDatabaseOperatorGetCitiesBySelectedOrderByDatasetId(
                    instance->dbOperator,
                    (int) TRUE,
                    vectorEntries);
    case NB_ECDT_MapRegions:
        return NB_MetadataDatabaseOperatorGetRegionsBySelectedOrderByDatasetId(
                    instance->dbOperator,
                    (int) TRUE,
                    vectorEntries);
    default:
        break;
    }

    return NE_NOENT;
}

/* See header file for description */
NB_Error NB_EnhancedContentDataSourceGetExistMapRegionFileIds(NB_EnhancedContentDataSource* instance,
                                                              int32* count,
                                                              char*** fileIds)
{
    return NB_MetadataDatabaseOperatorGetExistMapRegionFileIds(instance->dbOperator,
                                                               count,
                                                               fileIds);
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceSelectEntry(NB_EnhancedContentDataSource* instance,
                                        const char* id,
                                        nb_boolean selected,
                                        NB_EnhancedContentDataType dataType)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !id || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    result = result ? result :
                NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                    instance->dbOperator,
                    dataType,
                    id,
                    &reply);
    result = result ? result :
                NB_MetadataDatabaseOperatorSelectCity(instance->dbOperator, id, selected);

    return result;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceRemoveEntry(NB_EnhancedContentDataSource* instance,
                                        const char* id,
                                        NB_EnhancedContentDataType dataType)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !id || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator,
                dataType,
                id,
                &reply);
    if (result != NE_OK)
    {
        return result;
    }

    // Reset entry in database
    reply.downloadedSize = 0;
    result = NB_MetadataDatabaseOperatorUpdateMetadata(instance->dbOperator, &reply);
    if (result != NE_OK)
    {
        return result;
    }

    result = NB_MetadataDatabaseOperatorUpdateManifestDownloadStatusByMetadataId(
                instance->dbOperator,
                reply.id,
                NB_ECDS_DownloadNotStarted,
                0);
    if (result != NE_OK)
    {
        return result;
    }
    return result;
}

/* See header file for description */
NB_Error NB_EnhancedContentDataSourceGetRegionInformation(NB_EnhancedContentDataSource* instance,
                                                          const char* regionId,
                                                          NB_ContentEntryData* region)
{
    if (!instance || !regionId || !region || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    return NB_MetadataDatabaseOperatorGetRegionById(instance->dbOperator, regionId, region);
}

/* See header file for description */
nb_boolean
NB_EnhancedContentDataSourceIsDatasetSelected(NB_EnhancedContentDataSource* instance,
                                              NB_EnhancedContentDataType type,
                                              const char* datasetId)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !datasetId || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return FALSE;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator,
                type,
                datasetId,
                &reply);
    if (result != NE_OK)
    {
        return FALSE;
    }

    return (nb_boolean) reply.selected;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetDatasetSize(NB_EnhancedContentDataSource* instance,
                                           NB_EnhancedContentDataType type,
                                           const char* datasetId,
                                           uint32* datasetSize)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !datasetId || !datasetSize || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator,
                type,
                datasetId,
                &reply);
    if (result != NE_OK)
    {
        return result;
    }

    *datasetSize = (uint32) reply.datasetSize;

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceBuildItemUrl(NB_EnhancedContentDataSource* instance,
                                         const NB_EnhancedContentIdentifier* dataId,
                                         uint32 maxUrlSize,
                                         char* url)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !dataId || !url || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator,
                dataId->type,
                dataId->dataset,
                &reply);
    if (result != NE_OK)
    {
        return result;
    }

    if (maxUrlSize < nsl_strlen(reply.urlArgsTemplate) + 1)
    {
        return NE_RANGE;
    }

    nsl_memset(url, 0, maxUrlSize);
    nsl_strcpy(url, reply.urlArgsTemplate);

    if (dataId->itemId)
    {
        result = result ? result : ReplaceString(url, maxUrlSize,
                                                 URL_TEMPLATE_PARAMETER_ITEM_ID, dataId->itemId);
    }

    if (dataId->dataset)
    {
        result = result ? result : ReplaceString(url, maxUrlSize,
                                                 URL_TEMPLATE_PARAMETER_DATASET, dataId->dataset);
    }
    result = result ? result : ReplaceString(url, maxUrlSize,
                                             URL_TEMPLATE_PARAMETER_NBGM_VERSION,
                                             instance->configuration.nbgmVersion);
    result = result ? result : ReplaceString(url, maxUrlSize,
                                             URL_TEMPLATE_PARAMETER_MANIFEST_VERSION,
                                             reply.manifestVersion);
    switch (dataId->type)
    {
        case NB_ECDT_CityModel:
        {
            result = result ? result : ReplaceString(url, maxUrlSize,
                                                     URL_TEMPLATE_PARAMETER_FORMAT,
                                                     instance->configuration.enhancedCityModelFileFormat);
            break;
        }
        case NB_ECDT_MotorwayJunctionObject:
        {
            result = result ? result : ReplaceString(url, maxUrlSize,
                                                     URL_TEMPLATE_PARAMETER_FORMAT,
                                                     instance->configuration.motorwayJunctionObjectFileFormat);
            break;
        }
        case NB_ECDT_CommonTexture:
        case NB_ECDT_SpecificTexture:
        case NB_ECDT_MapRegions:
        case NB_ECDT_MapRegionsCommon:
        {
            result = result ? result : ReplaceString(url, maxUrlSize,
                                                     URL_TEMPLATE_PARAMETER_FORMAT,
                                                     instance->configuration.compressedTextureFileFormat);
            break;
        }
        case NB_ECDT_RealisticSign:
        {
            result = result ? result : ReplaceString(url, maxUrlSize,
                                                     URL_TEMPLATE_PARAMETER_FORMAT,
                                                     instance->configuration.realisticSignsFileFormat);
            break;
        }
        case NB_ECDT_SpeedLimitsSign:
        {
            result = result ? result : ReplaceString(url, maxUrlSize,
                                                     URL_TEMPLATE_PARAMETER_FORMAT,
                                                     instance->configuration.speedLimitsSignFileFormat);
            break;
        }
        case NB_ECDT_SpecialRegions:
        {
            result = result ? result : ReplaceString(url, maxUrlSize,
                                                     URL_TEMPLATE_PARAMETER_FORMAT,
                                                     instance->configuration.specialRegionFileFormat);
            break;
        }
        default:
        {
            result = NE_INVAL;
            break;
        }
    }

    return result;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetHostname(NB_EnhancedContentDataSource* instance,
                                        uint32 maxHostnameSize,
                                        char* hostname,
                                        uint16* port)
{
    // There's a base url for each content source in metadata reply. We assume that they are all the same
    // and we always use the first one we fine.

    if (!instance || !hostname || !port || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    // The port is not in the metadata reply. We use the default port
    *port = HTTP_DEFAULT_PORT;

    return NB_MetadataDatabaseOperatorGetUrlBase(instance->dbOperator, maxHostnameSize, hostname);
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceBuildItemFilePath(NB_EnhancedContentDataSource* instance,
                                              const NB_EnhancedContentIdentifier* dataId,
                                              nb_boolean zipped,
                                              uint32 maxFilePathSize,
                                              char* filePath)
{
    NB_Error result = NE_OK;
    char path[MAX_ENHANCED_CONTENT_PATH_LENGTH];

    if (!instance || !dataId || !dataId->dataset || !filePath || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    nsl_memset(path, 0, sizeof(path));
    if( dataId->type != NB_ECDT_MapRegions)
    {
        nsl_strlcpy(path, dataId->dataset, sizeof(path));
    }

    if ((dataId->type == NB_ECDT_CommonTexture || dataId->type == NB_ECDT_SpecificTexture ||
         dataId->type == NB_ECDT_MapRegionsCommon) &&
        zipped && !dataId->itemId)
    {
        nsl_strcat(path, ".");
        nsl_strcat(path, instance->configuration.compressedTextureFileFormat);
    }
    else if (dataId->itemId)
    {
        result = PAL_FileAppendPath(NB_ContextGetPal(instance->context), path,
                                    MAX_ENHANCED_CONTENT_PATH_LENGTH,
                                    dataId->itemId) == PAL_Ok ?  NE_OK : NE_FSYS;
        if (result != NE_OK)
        {
            return result;
        }

        if( dataId->type != NB_ECDT_MapRegions)
        {
            nsl_strcat(path, ".");
        }
        switch (dataId->type)
        {
            case NB_ECDT_CityModel:
            {
                nsl_strcat(path, instance->configuration.enhancedCityModelFileFormat);
                break;
            }
            case NB_ECDT_MotorwayJunctionObject:
            {
                nsl_strcat(path, instance->configuration.motorwayJunctionObjectFileFormat);
                break;
            }
            case NB_ECDT_CommonTexture:
            case NB_ECDT_SpecificTexture:
            {
                nsl_strcat(path, instance->configuration.textureFileFormat);
                break;
            }
            case NB_ECDT_RealisticSign:
            {
                nsl_strcat(path, instance->configuration.realisticSignsFileFormat);
                break;
            }
            case NB_ECDT_SpeedLimitsSign:
            {
                nsl_strcat(path, instance->configuration.speedLimitsSignFileFormat);
                break;
            }
            case NB_ECDT_SpecialRegions:
            {
                nsl_strcat(path, instance->configuration.specialRegionFileFormat);
                break;
            }
            case NB_ECDT_MapRegions:
            case NB_ECDT_MapRegionsCommon:
            {
                break;
            }
            default:
            {
                result = NE_INVAL;
                break;
            }
        }
    }

    if (result == NE_OK)
    {
        if (maxFilePathSize > nsl_strlen(path))
        {
            nsl_memset(filePath, 0, maxFilePathSize);
            nsl_strcpy(filePath, path);
        }
        else
        {
            result = NE_RANGE;
        }
    }

    return result;
}

/* See header file for description */
NB_Error NB_EnhancedContentDataSourceGetItemIdFromFilePath(NB_EnhancedContentDataSource* instance,
                                                           const char* filePath,
                                                           char** itemId)
{
    char* ptr = NULL;
    PAL_Instance* pal = NULL;

    if (!instance || !filePath || !itemId)
    {
        return NE_INVAL;
    }

    pal = NB_ContextGetPal(instance->context);

    if (PAL_FileGetLastPath(pal, filePath, itemId) != PAL_Ok)
    {
        return NE_FSYS;
    }

    // Remove the suffix of file format. For example ".PNG" or ".BIN".B
    ptr = nsl_strrchr(*itemId, '.');
    if (ptr)
    {
        *ptr = '\0';
    }

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetCityBoundingBox(NB_EnhancedContentDataSource* instance,
                                               const char* cityId,
                                               NB_CityBoundingBox* boundingBox)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !cityId || !boundingBox || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator,
                NB_ECDT_CityModel,
                cityId, &reply);
    if (result != NE_OK)
    {
        return result;
    }

    boundingBox->point1.latitude = reply.boundingLatitude1;
    boundingBox->point1.longitude = reply.boundingLongitude1;
    boundingBox->point2.latitude = reply.boundingLatitude2;
    boundingBox->point2.longitude = reply.boundingLongitude2;

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetCityProjectionParameters(NB_EnhancedContentDataSource* instance,
                                                        const char* cityId,
                                                        NB_EnhancedContentProjectionType* projectionType,
                                                        NB_EnhancedContentProjectionDatum* datum,
                                                        NB_LatitudeLongitude* origin,
                                                        double* scaleFactor,
                                                        double* falseNorthing,
                                                        double* falseEasting,
                                                        double* zOffset)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !cityId || !projectionType || !datum || !origin ||
        !scaleFactor || !falseNorthing || !falseEasting || !zOffset ||
        !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator,
                NB_ECDT_CityModel,
                cityId, &reply);
    if (result != NE_OK)
    {
        return result;
    }

    *projectionType = reply.projectionType;
    *datum = reply.datum;
    origin->latitude = reply.originLatitude;
    origin->longitude = reply.originLongitude;
    *scaleFactor = reply.scaleFactor;
    *falseNorthing = reply.falseNorthing;
    *falseEasting = reply.falseEasting;
    *zOffset = reply.zOffset;

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetStorageStatistics(NB_EnhancedContentDataSource* instance,
                                                 uint64* totalSize,
                                                 uint64* downloadedSize,
                                                 uint32* totalCommonTexturesSize,
                                                 uint32* downloadedCommonTexturesSize,
                                                 uint32* downloadedPercentage)
{
    NB_Error result = NE_OK;
    int n = 0;
    int length = 0;
    uint64 selectedCitiesTotalSize = 0;
    uint64 selectedCitiesDownloadedSize = 0;
    uint32 commonTexturesTotalSize = 0;
    uint32 commonTexturesDownloadedSize = 0;
    uint32 mjoTexturesTotalSize = 0;
    uint32 mjoTexturesDownloadedSize = 0;
    const char* commonTexturesDatasetId = NULL;
    const char* motorwayJunctionObjectsDatasetId = NULL;
    NB_EnhancedContentCityData* city = NULL;
    CSL_Vector* cities = NULL;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !totalSize || !downloadedSize || !totalCommonTexturesSize ||
        !downloadedCommonTexturesSize || !downloadedPercentage ||
        !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    cities = CSL_VectorAlloc(sizeof(NB_EnhancedContentCityData));
    if (!cities)
    {
        return NE_NOMEM;
    }

    // Get size of city model and specific texture.
    result = NB_MetadataDatabaseOperatorGetCitiesBySelectedOrderByDatasetId(
                instance->dbOperator, TRUE, cities);
    if (result != NE_OK)
    {
        CSL_VectorDealloc(cities);
        return result;
    }

    length = CSL_VectorGetLength(cities);
    if (length == 0)
    {
        result = NB_MetadataDatabaseOperatorGetRegionsBySelectedOrderByDatasetId(
                    instance->dbOperator, TRUE, cities);
        if (result != NE_OK)
        {
            CSL_VectorDealloc(cities);
            return result;
        }
    }
    length = CSL_VectorGetLength(cities);
    for (n = 0; n < length; ++n)
    {
        city = (NB_EnhancedContentCityData*) CSL_VectorGetPointer(cities, n);

        selectedCitiesTotalSize += city->cityModelDataSize + city->textureDataSize;
        selectedCitiesDownloadedSize += city->downloadedCityModelDataSize + city->downloadedTextureDataSize;
    }

    CSL_VectorDealloc(cities);

    // Get size of common texture.
    commonTexturesDatasetId = NB_EnhancedContentDataSourceGetCommonTexturesDatasetId(instance);
    if (commonTexturesDatasetId != NULL)
    {
        nsl_memset(&reply, 0, sizeof(reply));

        result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                    instance->dbOperator,
                    NB_ECDT_CommonTexture, 
                    commonTexturesDatasetId,
                    &reply);
        if (result != NE_OK)
        {
            return result;
        }

        commonTexturesTotalSize = reply.datasetSize;
        commonTexturesDownloadedSize = reply.downloadedSize;
    }

    // Get size of MJO texture.
    motorwayJunctionObjectsDatasetId = NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(instance);
    if (commonTexturesDatasetId != NULL)
    {
        nsl_memset(&reply, 0, sizeof(reply));

        result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                    instance->dbOperator,
                    NB_ECDT_SpecificTexture,
                    motorwayJunctionObjectsDatasetId,
                    &reply);
        if (result != NE_OK)
        {
            return result;
        }

        mjoTexturesTotalSize = reply.datasetSize;
        mjoTexturesDownloadedSize = reply.downloadedSize;
    }

    *totalSize = selectedCitiesTotalSize + commonTexturesTotalSize + mjoTexturesTotalSize;
    *downloadedSize = selectedCitiesDownloadedSize + commonTexturesDownloadedSize + mjoTexturesDownloadedSize;
    *totalCommonTexturesSize = commonTexturesTotalSize + mjoTexturesTotalSize;;
    *downloadedCommonTexturesSize = commonTexturesDownloadedSize + mjoTexturesDownloadedSize;

    *downloadedPercentage = *totalSize == 0 ? 100 : (uint32) (*downloadedSize * 100 / *totalSize);

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetNextDatasetToUpdate(NB_EnhancedContentDataSource* instance,
                                                   nb_boolean cityEnabled,
                                                   NB_EnhancedContentIdentifier* nextDataset)
{
    NB_Error result = NE_OK;
    const char* commonTexturesDatasetId = NULL;
    const char* motorwayJunctionObjectsDatasetId = NULL;
    const char* regionCommonDatasetId = NULL;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !nextDataset || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    // Check common texture.
    commonTexturesDatasetId = NB_EnhancedContentDataSourceGetCommonTexturesDatasetId(instance);
    if (commonTexturesDatasetId != NULL)
    {
        result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                    instance->dbOperator, NB_ECDT_CommonTexture,
                    commonTexturesDatasetId, &reply);
        if (result != NE_OK)
        {
            return result;
        }

        if (reply.datasetSize != reply.downloadedSize)
        {
            return NB_EnhancedContentIdentifierSet(nextDataset, NB_ECDT_CommonTexture,
                                                   commonTexturesDatasetId, NULL, NULL);
        }
    }

    //Check region-common
    regionCommonDatasetId = NB_EnhancedContentDataSourceGetRegionCommonDatasetId(instance);
    if (regionCommonDatasetId != NULL)
    {
        result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                    instance->dbOperator, NB_ECDT_MapRegionsCommon,
                    regionCommonDatasetId, &reply);
        if (result != NE_OK)
        {
            return result;
        }

        if (reply.datasetSize != reply.downloadedSize)
        {
            return NB_EnhancedContentIdentifierSet(nextDataset, NB_ECDT_MapRegionsCommon,
                                                   regionCommonDatasetId, NULL, NULL);
        }
    }

    // Check MJO texture.
    motorwayJunctionObjectsDatasetId = NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(instance);
    if (motorwayJunctionObjectsDatasetId != NULL)
    {
        result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                    instance->dbOperator, NB_ECDT_SpecificTexture,
                    motorwayJunctionObjectsDatasetId, &reply);
        if (result != NE_OK)
        {
            return result;
        }

        if (reply.datasetSize != reply.downloadedSize)
        {
            return NB_EnhancedContentIdentifierSet(nextDataset, NB_ECDT_SpecificTexture,
                                                   motorwayJunctionObjectsDatasetId, NULL,
                                                   NULL);
        }
    }

    if (! cityEnabled)
    {
        return NE_NOENT;
    }

    if ((result = NB_EnhancedContentDataSourceGetNextCityToUpdate(instance, nextDataset)) == NE_NOENT)
    {
        result = NB_EnhancedContentDataSourceGetNextEntryToUpdate(instance, nextDataset, NB_ECDT_MapRegions);
    }
    return result;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetNextCityToUpdate(NB_EnhancedContentDataSource* instance,
                                                NB_EnhancedContentIdentifier* nextCity)
{
    return NB_EnhancedContentDataSourceGetNextEntryToUpdate(instance,
                                                            nextCity,
                                                            NB_ECDT_CityModel);
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetNextEntryToUpdate(NB_EnhancedContentDataSource* instance,
                                                 NB_EnhancedContentIdentifier* nextEntry,
                                                 NB_EnhancedContentDataType dataType)
{
    NB_Error result = NE_OK;
    boolean isTextureComplete = FALSE;
    boolean isEntryComplete = FALSE;
    boolean isCurrentLocationValid = FALSE;
    int n = 0;
    int length = 0;
    double nextDistance = 0;
    double currentDistance = 0;
    NB_ContentEntryData* nextEntryToUpdate = NULL;
    NB_ContentEntryData* entry = NULL;
    CSL_Vector* entries = NULL;

    if (!instance || !nextEntry || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    // Check cities.
    entries = CSL_VectorAlloc(sizeof(NB_EnhancedContentCityData));
    if (! entries)
    {
        return NE_NOMEM;
    }

    result = NB_EnhancedContentDataSourceGetSelectedEntries(instance, entries, dataType);
    if (result != NE_OK)
    {
        CSL_VectorDealloc(entries);
        return result;
    }

    isCurrentLocationValid = IsLatitudeValid(instance->currentLocation.latitude) &&
                                    IsLongitudeValid(instance->currentLocation.longitude);

    length = CSL_VectorGetLength(entries);
    for (n = 0; n < length; ++n)
    {
        entry = (NB_EnhancedContentCityData*)CSL_VectorGetPointer(entries, n);
        // if downloadstatus is paused, continue;
        if (NB_MetadataDatabaseOperatorCheckDownloadStatusIsPausedByDatasetId(instance->dbOperator, entry->cityId))
        {
            continue;
        }
        isTextureComplete = (boolean)(entry->textureDataSize == entry->downloadedTextureDataSize);
        isEntryComplete = (boolean)(entry->cityModelDataSize == entry->downloadedCityModelDataSize);
        if (!entry->selected || (isTextureComplete && isEntryComplete))
        {
            // TODO: Should check download status for each file of city model in mainifest table?
            continue;
        }

        // Continue to download incomplete city.
        if (entry->downloadedTextureDataSize > 0 || entry->downloadedCityModelDataSize > 0)
        {
            nextEntryToUpdate = entry;
            break;
        }

        if (!nextEntryToUpdate)
        {
            nextEntryToUpdate = entry;
            continue;
        }

        if (!isCurrentLocationValid)
        {
            continue;
        }

        nextDistance = NB_SpatialGetLineOfSightDistance(instance->currentLocation.latitude,
                                                        instance->currentLocation.longitude,
                                                        nextEntryToUpdate->center.latitude,
                                                        nextEntryToUpdate->center.longitude, NULL);
        currentDistance = NB_SpatialGetLineOfSightDistance(instance->currentLocation.latitude,
                                                           instance->currentLocation.longitude,
                                                           entry->center.latitude,
                                                           entry->center.longitude, NULL);
        if (nextDistance > currentDistance)
        {
            nextEntryToUpdate = entry;
        }
    }

    if (!nextEntryToUpdate)
    {
        result = NE_NOENT;
    }
    else if (nextEntryToUpdate->textureDataSize != nextEntryToUpdate->downloadedTextureDataSize &&
             dataType == NB_ECDT_CityModel)
    {
        result = NB_EnhancedContentIdentifierSet(nextEntry, NB_ECDT_SpecificTexture,
                                                 nextEntryToUpdate->cityId, NULL, NULL);
    }
    else if (nextEntryToUpdate->cityModelDataSize != nextEntryToUpdate->downloadedCityModelDataSize)
    {
        result = NB_EnhancedContentIdentifierSet(nextEntry, dataType,
                                                 nextEntryToUpdate->cityId, NULL, NULL);
    }
    else
    {
        result = NE_NOENT;
    }

    CSL_VectorDealloc(entries);

    return result;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetDataItemSize(NB_EnhancedContentDataSource* instance,
                                            const NB_EnhancedContentIdentifier* dataId,
                                            uint32* dataSize)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply metadataReply = {0};
    NB_MetadataManifestReply manifestReply = {0};

    if (!instance || !dataId || !dataId->dataset || !dataSize || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, dataId->type, dataId->dataset, &metadataReply);
    if (result != NE_OK)
    {
        return result;
    }

    if (!dataId->itemId)
    {
        *dataSize = metadataReply.datasetSize;
        return NE_OK;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataManifestReplyByMetadataIdAndFileId(
                instance->dbOperator, metadataReply.id, dataId->itemId, &manifestReply);
    if (result != NE_OK)
    {
        return result;
    }

    *dataSize = manifestReply.fileSize;

    return NE_OK;
}

/* See header file for description */
NB_Error NB_EnhancedContentDataSourceGetDataItemSizeInfo(NB_EnhancedContentDataSource* instance,
                                                         const NB_EnhancedContentIdentifier* dataId,
                                                         uint32* dataSize,
                                                         uint32* downloadedSize)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply metadataReply = {0};
    NB_MetadataManifestReply manifestReply = {0};

    if (!instance || !dataId || !dataId->dataset ||
        !dataSize || !downloadedSize ||
        !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, dataId->type, dataId->dataset, &metadataReply);
    if (result != NE_OK)
    {
        return result;
    }

    if (!dataId->itemId)
    {
        *dataSize = metadataReply.datasetSize;
        *downloadedSize = metadataReply.downloadedSize;
        return NE_OK;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataManifestReplyByMetadataIdAndFileId(
                instance->dbOperator, metadataReply.id, dataId->itemId, &manifestReply);
    if (result != NE_OK)
    {
        return result;
    }

    *dataSize = manifestReply.fileSize;
    *downloadedSize = manifestReply.downloadedSize;

    return NE_OK;
}

/* See header file for description */
NB_Error NB_EnhancedContentDataSourceUpdateDataItemDownloadStatus(NB_EnhancedContentDataSource* instance,
                                                                  const NB_EnhancedContentIdentifier* dataId,
                                                                  NB_EnhancedContentUpdateStatus updateStatus,
                                                                  NB_EnhancedContentDownloadStatus downloadStatus,
                                                                  uint32 downloadedSize)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply metadataReply = {0};
    NB_MetadataManifestReply manifestReply = {0};

    if (!instance || !dataId || !dataId->dataset || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, dataId->type, dataId->dataset, &metadataReply);
    if (result != NE_OK)
    {
        return result;
    }

    if (!dataId->itemId)
    {
        metadataReply.updateStatus = updateStatus;
        metadataReply.downloadedSize = downloadedSize;
        return NB_MetadataDatabaseOperatorUpdateMetadata(instance->dbOperator, &metadataReply);
    }

    result = NB_MetadataDatabaseOperatorGetMetadataManifestReplyByMetadataIdAndFileId(
                instance->dbOperator, metadataReply.id, dataId->itemId, &manifestReply);
    if (result != NE_OK)
    {
        return result;
    }

    metadataReply.updateStatus = updateStatus;
    manifestReply.downloadStatus = downloadStatus;
    if (downloadStatus != NB_ECDS_DownloadPaused)
    {
       metadataReply.downloadedSize += downloadedSize - manifestReply.downloadedSize;
    }
    manifestReply.downloadedSize = downloadedSize;

    result = result ? result : NB_MetadataDatabaseOperatorUpdateMetadata(instance->dbOperator, &metadataReply);
    result = result ? result : NB_MetadataDatabaseOperatorUpdateManifest(instance->dbOperator, &manifestReply);

    return result;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetDataItemIDs(NB_EnhancedContentDataSource* instance,
                                           nb_boolean nonDownloadedItemsOnly,
                                           NB_EnhancedContentDataType type,
                                           const char* dataset,
                                           CSL_Vector* vectorDataIds)
{
    NB_Error result = NE_OK;
    int n = 0;
    int length = 0;
    NB_EnhancedContentIdentifier* dataId = NULL;
    NB_MetadataManifestReply* manifestReply = NULL;
    CSL_Vector* vectorManifestReplies = NULL;
    NB_MetadataSourceReply metadataReply = {0};

    if (!instance || !dataset || !vectorDataIds || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, type, dataset, &metadataReply);
    if (result != NE_OK)
    {
        return result;
    }

    if (nonDownloadedItemsOnly && metadataReply.datasetSize == metadataReply.downloadedSize)
    {
        return NE_NOENT;
    }

    if (type != NB_ECDT_CityModel && type != NB_ECDT_MapRegions)
    {
        dataId = NB_EnhancedContentIdentifierCreate(type, dataset, NULL, NULL);
        if (!dataId)
        {
            return NE_NOMEM;
        }

        result = CSL_VectorAppend(vectorDataIds, dataId) ? NE_OK : NE_NOMEM;

        if (result == NE_OK)
        {
            // TODO: Find another method to fix memory leak.
            nsl_free(dataId);
        }
        else
        {
            NB_EnhancedContentIdentifierDestroy(dataId);
        }

        return result;
    }

    vectorManifestReplies = CSL_VectorAlloc(sizeof(NB_MetadataManifestReply));
    if (!vectorManifestReplies)
    {
        return NE_NOMEM;
    }

    if (nonDownloadedItemsOnly)
    {
        result = NB_MetadataDatabaseOperatorGetMetadataManifestRepliesByMetadataIdAndDownloadStatus(
                    instance->dbOperator, metadataReply.id,
                    NB_ECDS_DownloadNotStarted, vectorManifestReplies);
    }
    else
    {
        result = NB_MetadataDatabaseOperatorGetMetadataManifestRepliesByMetadataId(
                    instance->dbOperator, metadataReply.id, vectorManifestReplies);
    }

    if (result != NE_OK)
    {
        CSL_VectorDealloc(vectorManifestReplies);
        return result;
    }

    length = CSL_VectorGetLength(vectorManifestReplies);
    if (length <= 0)
    {
        CSL_VectorDealloc(vectorManifestReplies);
        return NE_NOENT;
    }

    for (n = 0; n < length; ++n)
    {
        manifestReply = (NB_MetadataManifestReply*)CSL_VectorGetPointer(vectorManifestReplies, n);

        dataId = NB_EnhancedContentIdentifierCreate(type, dataset, manifestReply->fileId,
                                                    manifestReply->fileVersion);
        if (!dataId)
        {
            result = NE_NOMEM;
            break;
        }
        result = CSL_VectorAppend(vectorDataIds, dataId) ? NE_OK : NE_NOMEM;
        if (result == NE_OK)
        {
            // TODO: Find another method to fix memory leak.
            nsl_free(dataId);
        }
        else
        {
            NB_EnhancedContentIdentifierDestroy(dataId);
            break;
        }
    }

    CSL_VectorDealloc(vectorManifestReplies);

    if (result != NE_OK)
    {
        length = CSL_VectorGetLength(vectorDataIds);
        for (n = 0; n < length; ++n)
        {
            dataId = (NB_EnhancedContentIdentifier*)CSL_VectorGetPointer(vectorDataIds, n);
            NB_EnhancedContentIdentifierSet(dataId, NB_ECDT_None, NULL, NULL, NULL);
        }

        CSL_VectorRemoveAll(vectorDataIds);
    }

    return result;
}

/* See header file for description */
nb_boolean
NB_EnhancedContentDataSourceIsManifestUpdateNeeded(NB_EnhancedContentDataSource* instance,
                                                   const NB_EnhancedContentIdentifier* dataId)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !dataId || !dataId->dataset || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return FALSE;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, dataId->type, dataId->dataset, &reply);
    if (result != NE_OK)
    {
        return FALSE;
    }

    return (nb_boolean) (dataId->type == NB_ECDT_CityModel && reply.manifestState != NB_ECMS_ManifestLatest);
}

/* See header file for description */
nb_boolean
NB_EnhancedContentDataSourceIsDataItemConsistent(NB_EnhancedContentDataSource* instance,
                                                 const NB_EnhancedContentIdentifier* dataId)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply metadataReply = {0};
    NB_MetadataManifestReply manifestReply = {0};

    if (!instance || !dataId || !dataId->dataset || !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return FALSE;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, dataId->type, dataId->dataset, &metadataReply);
    if (result != NE_OK)
    {
        return FALSE;
    }

    if (!dataId->itemId)
    {
        return TRUE;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataManifestReplyByMetadataIdAndFileId(
                instance->dbOperator, metadataReply.id, dataId->itemId, &manifestReply);
    if (result != NE_OK)
    {
        return FALSE;
    }

    return TRUE;
}

/* See header file for description */
nb_boolean
NB_EnhancedContentDataSourceIsDataItemAvailableForUse(NB_EnhancedContentDataSource* instance,
                                                      const NB_EnhancedContentIdentifier* dataId)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply metadataReply = {0};
    NB_MetadataManifestReply manifestReply = {0};

    if (!instance || !dataId || !dataId->dataset)
    {
        return FALSE;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, dataId->type, dataId->dataset, &metadataReply);
    if (result != NE_OK)
    {
        return FALSE;
    }

    if (!dataId->itemId)
    {
        return (nb_boolean) (metadataReply.datasetSize == metadataReply.downloadedSize);
    }

    result = NB_MetadataDatabaseOperatorGetMetadataManifestReplyByMetadataIdAndFileId(
                instance->dbOperator, metadataReply.id, dataId->itemId, &manifestReply);
    if (result != NE_OK)
    {
        return FALSE;
    }

    return (nb_boolean) (manifestReply.downloadStatus == NB_ECDS_DownloadComplete);
}

/* See header file for description */
nb_boolean
NB_EnhancedContentDataSourceIsDataItemEnabledForDownload(NB_EnhancedContentDataSource* instance,
                                                         const NB_EnhancedContentIdentifier* dataId)
{
    NB_Error result = NE_OK;
    const char* motorwayJunctionObjectsDatasetId = NULL;
    NB_EnhancedContentCityData city;
    nsl_memset(&city, 0, sizeof(city));

    if (!instance || !dataId || !dataId->dataset)
    {
        return FALSE;
    }

    if (dataId->type == NB_ECDT_MotorwayJunctionObject || dataId->type == NB_ECDT_CommonTexture ||
        dataId->type == NB_ECDT_RealisticSign || dataId->type == NB_ECDT_SpeedLimitsSign ||
        dataId->type == NB_ECDT_SpecialRegions)
    {
        return TRUE;
    }

    // Check if MJO or MJO texture.
    motorwayJunctionObjectsDatasetId = NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(instance);
    if (motorwayJunctionObjectsDatasetId && nsl_strcmp(dataId->dataset, motorwayJunctionObjectsDatasetId) == 0)
    {
        return TRUE;
    }

    if (dataId->type == NB_ECDT_CityModel || dataId->type == NB_ECDT_SpecificTexture)
    {
        result = NB_MetadataDatabaseOperatorGetCityByCityId(instance->dbOperator, dataId->dataset, &city);

        return city.selected;
    }

    return FALSE;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetCityTileBoundingBoxes(NB_EnhancedContentDataSource* instance,
                                                     const char* cityId,
                                                     uint32* cityBoundingBoxesCount,
                                                     NB_CityBoundingBox** cityBoundingBoxes)
{
    NB_Error result = NE_OK;
    int n = 0;
    int length = 0;
    NB_CityBoundingBox* cityBoundingBoxArray = NULL;
    NB_MetadataManifestReply* manifestReply = NULL;
    CSL_Vector* manifestReplies = NULL;
    NB_MetadataSourceReply metadataReply = {0};

    if (!instance || !cityId || !cityBoundingBoxesCount || !cityBoundingBoxes)
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, NB_ECDT_CityModel, cityId, &metadataReply);
    if (result != NE_OK)
    {
        return result;
    }

    manifestReplies = CSL_VectorAlloc(sizeof(NB_MetadataManifestReply));
    if (!manifestReplies)
    {
        return NE_NOMEM;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataManifestRepliesByMetadataId(
                instance->dbOperator, metadataReply.id, manifestReplies);
    if (result != NE_OK)
    {
        CSL_VectorDealloc(manifestReplies);
        return result;
    }

    length  = CSL_VectorGetLength(manifestReplies);
    if (length <= 0)
    {
        CSL_VectorDealloc(manifestReplies);
        return NE_NOENT;
    }

    cityBoundingBoxArray = nsl_malloc(length * sizeof(NB_CityBoundingBox));
    if (!cityBoundingBoxArray)
    {
        CSL_VectorDealloc(manifestReplies);
        return NE_NOMEM;
    }
    nsl_memset(cityBoundingBoxArray, 0, length * sizeof(NB_CityBoundingBox));

    for (n = 0; n < length; ++n)
    {
        manifestReply = (NB_MetadataManifestReply*)CSL_VectorGetPointer(manifestReplies, n);

        nsl_strlcpy(cityBoundingBoxArray[n].name, manifestReply->fileId, sizeof(cityBoundingBoxArray[n].name));
        cityBoundingBoxArray[n].point1.latitude = manifestReply->boundingLatitude1;
        cityBoundingBoxArray[n].point1.longitude = manifestReply->boundingLongitude1;
        cityBoundingBoxArray[n].point2.latitude = manifestReply->boundingLatitude2;
        cityBoundingBoxArray[n].point2.longitude = manifestReply->boundingLongitude2;
    }

    CSL_VectorDealloc(manifestReplies);

    *cityBoundingBoxesCount = length;
    *cityBoundingBoxes = cityBoundingBoxArray;

    return result;
}

/* See header file for description */
NB_Error NB_EnhancedContentDataSourceGetManifestVersion(NB_EnhancedContentDataSource* instance,
                                                        const NB_EnhancedContentIdentifier* dataId,
                                                        uint32 maxManifestVersionSize,
                                                        char* manifestVersion)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !dataId || !dataId->dataset || !manifestVersion)
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, dataId->type, dataId->dataset, &reply);
    if (result != NE_OK)
    {
        return result;
    }

    if (maxManifestVersionSize < nsl_strlen(reply.manifestVersion) + 1)
    {
        return NE_RANGE;
    }

    nsl_memset(manifestVersion, 0, maxManifestVersionSize);
    nsl_strcpy(manifestVersion, reply.manifestVersion);

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceMasterClear(NB_EnhancedContentDataSource* instance)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    pal = NB_ContextGetPal(instance->context);

    // Clear members.
    CSL_VectorRemoveAll(instance->manifestQueryCities);
    CSL_VectorRemoveAll(instance->manifestQueryRegions);

    if (instance->citySummaryDisplay)
    {
        nsl_free(instance->citySummaryDisplay);
        instance->citySummaryDisplay = NULL;
    }

    instance->currentLocation.latitude = INVALID_LATITUDE;
    instance->currentLocation.longitude = INVALID_LONGITUDE;

    nsl_memset(instance->commonTexturesDatasetId, 0, sizeof(instance->commonTexturesDatasetId));
    nsl_memset(instance->motorwayJunctionObjectsDatasetId, 0, sizeof(instance->motorwayJunctionObjectsDatasetId));
    nsl_memset(instance->realisticSignsDatasetId, 0, sizeof(instance->realisticSignsDatasetId));
    nsl_memset(instance->speedLimitsSignDatasetId, 0, sizeof(instance->speedLimitsSignDatasetId));

    // Cancel metadata and manifest requests.
    if (NB_MetadataHandlerIsRequestInProgress(instance->metadataHandler))
    {
        result = NB_MetadataHandlerCancelRequest(instance->metadataHandler);
        if (result != NE_OK)
        {
            return result;
        }
    }

    if (NB_ManifestHandlerIsRequestInProgress(instance->manifestHandler))
    {
        result = NB_ManifestHandlerCancelRequest(instance->manifestHandler);
        if (result != NE_OK)
        {
            return result;
        }
    }

    result = NB_MetadataDatabaseOperatorMasterClear(instance->dbOperator);
    if (result != NE_OK)
    {
        return result;
    }

    // Remove persistent metadata path.
    if (PAL_FileExists(pal, instance->configuration.persistentMetadataPath) == PAL_Ok)
    {
        result = PAL_FileRemoveDirectory(pal,
                                        instance->configuration.persistentMetadataPath,
                                        TRUE) == PAL_Ok ? NE_OK : NE_FSYS;
        if (result != NE_OK)
        {
            return result;
        }
    }

    return result;
}

/* See header file for description */
const char*
NB_EnhancedContentDataSourceGetCommonTexturesDatasetId(NB_EnhancedContentDataSource* instance)
{
    if (!instance)
    {
        return NULL;
    }

    if (nsl_strlen(instance->commonTexturesDatasetId) == 0)
    {
        if (GetDatasetIdByContentType(instance->dbOperator,
                                    NB_ECDT_CommonTexture,
                                    MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1,
                                    instance->commonTexturesDatasetId) != NE_OK)
        {
            return NULL;
        }
    }

    return instance->commonTexturesDatasetId;
}

/* See header file for description */
const char* NB_EnhancedContentDataSourceGetRegionCommonDatasetId(NB_EnhancedContentDataSource* instance)
{
    if (!instance)
    {
        return NULL;
    }

    if (nsl_strlen(instance->regionCommonDatasetId) == 0)
    {
        if (GetDatasetIdByContentType(instance->dbOperator,
                                    NB_ECDT_MapRegionsCommon,
                                    MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1,
                                    instance->regionCommonDatasetId) != NE_OK)
        {
            return NULL;
        }
    }

    return instance->regionCommonDatasetId;
}

/* See header file for description */
const char*
NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(NB_EnhancedContentDataSource* instance)
{
    if (!instance)
    {
        return NULL;
    }

    if (nsl_strlen(instance->motorwayJunctionObjectsDatasetId) == 0)
    {
        if (GetDatasetIdByContentType(instance->dbOperator,
                                    NB_ECDT_MotorwayJunctionObject,
                                    MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1,
                                    instance->motorwayJunctionObjectsDatasetId) != NE_OK)
        {
            return NULL;
        }
    }

    return instance->motorwayJunctionObjectsDatasetId;
}

/* See header file for description */
const char*
NB_EnhancedContentDataSourceGetRealisticSignsDatasetId(NB_EnhancedContentDataSource* instance)
{
    if (!instance)
    {
        return NULL;
    }

    if (nsl_strlen(instance->realisticSignsDatasetId) == 0)
    {
        if (GetDatasetIdByContentType(instance->dbOperator,
                                    NB_ECDT_RealisticSign,
                                    MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1,
                                    instance->realisticSignsDatasetId) != NE_OK)
        {
            return NULL;
        }
    }

    return instance->realisticSignsDatasetId;
}

/* See header file for description */
const char*
NB_EnhancedContentDataSourceGetSpeedLimitsSignDatasetId(NB_EnhancedContentDataSource* instance)
{
    if (!instance)
    {
        return NULL;
    }

    if (nsl_strlen(instance->speedLimitsSignDatasetId) == 0)
    {
        if (GetDatasetIdByContentType(instance->dbOperator, NB_ECDT_SpeedLimitsSign,
                                      MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1,
                                      instance->speedLimitsSignDatasetId) != NE_OK)
        {
            return NULL;
        }
    }

    return instance->speedLimitsSignDatasetId;
}

const char*
NB_EnhancedContentDataSourceGetSpecialRegionsDatasetId(NB_EnhancedContentDataSource* instance)
{
    if (!instance)
    {
        return NULL;
    }

    if (nsl_strlen(instance->specialRegionsDatasetId) == 0)
    {
        if (GetDatasetIdByContentType(instance->dbOperator, NB_ECDT_SpecialRegions,
                                      MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1,
                                      instance->specialRegionsDatasetId) != NE_OK)
        {
            return NULL;
        }
    }

    return instance->specialRegionsDatasetId;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceGetCitySummaryDisplay(NB_EnhancedContentDataSource* instance,
                                                  const char** citySummaryDisplay)
{
    if (!instance || !instance->citySummaryDisplay || !citySummaryDisplay)
    {
        return NE_INVAL;
    }

    *citySummaryDisplay = instance->citySummaryDisplay;

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceSetLocation(NB_EnhancedContentDataSource* instance,
                                        NB_LatitudeLongitude* location)
{
    if (!instance || !location)
    {
        return NE_INVAL;
    }

    instance->currentLocation = *location;

    return NE_OK;
}

/* See header file for description */
NB_Error NB_EnhancedContentDataSourceRemoveDataItem(NB_EnhancedContentDataSource* instance,
                                                    NB_EnhancedContentDataType type,
                                                    const char* datasetId,
                                                    const char* itemId)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !datasetId || !itemId)
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, type, datasetId, &reply);
    if (result != NE_OK)
    {
        return result;
    }

    result = NB_MetadataDatabaseOperatorDeleteManifestByMetadataIdAndFileId(
                instance->dbOperator, reply.id, itemId);
    if (result != NE_OK)
    {
        return result;
    }

    return result;
}

/* See header file for description */
NB_Error NB_EnhancedContentDataSourceUpdateManifestVersion(NB_EnhancedContentDataSource* instance,
                                                           NB_EnhancedContentDataType type,
                                                           const char* datasetId,
                                                           const char* manifestVersion)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !datasetId || !manifestVersion)
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, type,
                datasetId, &reply);
    if (result != NE_OK)
    {
        return result;
    }

    if (reply.contentType == NB_ECDT_CityModel && reply.manifestState != NB_ECMS_ManifestLatest)
    {
        return NE_NOENT;
    }

    if (nsl_strcmp(reply.manifestVersion, manifestVersion) == 0)
    {
        return NE_EXIST;
    }

    reply.manifestState = NB_ECMS_ManifestNotSynchronized;
    reply.deleted = (int) FALSE;
    // Don't update manifest version here until metadata synchronization replies.

    result = NB_MetadataDatabaseOperatorUpdateMetadata(instance->dbOperator, &reply);
    if (result != NE_OK)
    {
        return result;
    }

    return result;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceCheckFileVersion(NB_EnhancedContentDataSource* instance,
                                             NB_EnhancedContentDataType type,
                                             const char* datasetId,
                                             const char* itemId,
                                             const char* fileVersion)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply metadataReply = {0};
    NB_MetadataManifestReply manifestReply = {0};

    if (!instance || !datasetId || !itemId || !fileVersion)
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, type,
                datasetId, &metadataReply);
    if (result != NE_OK)
    {
        return result;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataManifestReplyByMetadataIdAndFileId(
                instance->dbOperator, metadataReply.id,
                itemId, &manifestReply);
    if (result != NE_OK)
    {
        // If there's no entry of this item return NE_NOENT here.
        return result;
    }

    if (nsl_strcmp(manifestReply.fileVersion, fileVersion) == 0)
    {
        result = NE_OK;
    }
    else
    {
        result = NE_NOENT;
    }

    return result;
}

/* See header file for description */
NB_Error
NB_EnhancedContentDataSourceAddManifestEntry(NB_EnhancedContentDataSource* instance,
                                             NB_EnhancedContentDataType type,
                                             const char* datasetId,
                                             const char* itemId,
                                             const char* fileVersion)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply metadataReply = {0};
    NB_MetadataManifestReply manifestReply = {0};

    if (!instance || !datasetId || !itemId || !fileVersion)
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, 
                type, datasetId, &metadataReply);
    if (result != NE_OK)
    {
        return result;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataManifestReplyByMetadataIdAndFileId(
                instance->dbOperator, metadataReply.id,
                itemId, &manifestReply);
    if (result == NE_OK)
    {
        result = NE_EXIST;
    }
    if (result == NE_NOENT)
    {
        result = NE_OK;

        manifestReply.metadataId = metadataReply.id;
        manifestReply.downloadStatus = NB_ECDS_DownloadNotStarted;
        nsl_strlcpy(manifestReply.fileId, itemId, sizeof(manifestReply.fileId));
        nsl_strcpy(manifestReply.fileVersion, fileVersion);

        result = NB_MetadataDatabaseOperatorInsertManifest(instance->dbOperator, &manifestReply);
    }

    return result;
}


// Local Functions ...............................................................................

/* Start manifest request for one city

 @return NB_Error NE_NOENT if there is no next manifest request
 */
static NB_Error StartManifestRequest(NB_EnhancedContentDataSource* instance)
{
    NB_Error result = NE_OK;
    int lengthCities = 0;
    int lengthRegions = 0;
    NB_ManifestParameters* parameters = NULL;
    NB_ManifestQueryCity* manifestQueryCity = NULL;

    lengthCities = CSL_VectorGetLength(instance->manifestQueryCities);
    lengthRegions = CSL_VectorGetLength(instance->manifestQueryRegions);
    if(lengthCities <= 0 && lengthRegions <= 0)
    {
        return NE_NOENT;
    }
    result = NB_ManifestParametersCreate(instance->context, &parameters);
    if (result != NE_OK)
    {
        return result;
    }

    if(lengthCities > 0)
    {
        manifestQueryCity = (NB_ManifestQueryCity*) CSL_VectorGetPointer(instance->manifestQueryCities,
                                                                         lengthCities - 1);

        result = NB_ManifestParametersAddWantedContent(parameters,
                                                       instance->configuration.country,
                                                       manifestQueryCity->cityId,
                                                       NB_ECDT_CityModel);
    }
    else if(lengthRegions > 0)
    {
        manifestQueryCity = (NB_ManifestQueryCity*) CSL_VectorGetPointer(instance->manifestQueryRegions,
                                                                         lengthRegions - 1);

        result = NB_ManifestParametersAddWantedContent(parameters,
                                                       instance->configuration.country,
                                                       manifestQueryCity->cityId,
                                                       NB_ECDT_MapRegions);
    }
    if (result != NE_OK)
    {
        NB_ManifestParametersDestroy(parameters);
        return result;
    }

    result = NB_ManifestHandlerStartRequest(instance->manifestHandler, parameters);

    NB_ManifestParametersDestroy(parameters);

    return result;
}

/*! Metadata request callback

    @todo Start download manager if download is paused due to 404.
    @return None
 */
static void RequestMetadataCallback(void* handler, NB_RequestStatus status, NB_Error result, nb_boolean up, int percent, void* userData)
{
    uint32 n = 0;
    uint32 length = 0;
    const char* citySummaryDisplay = NULL;
    NB_EnhancedContentDataSource* dataSource = (NB_EnhancedContentDataSource*)userData;
    NB_MetadataInformation* info = NULL;
    NB_MetadataSourceReply* reply = NULL;
    CSL_Vector* replies = NULL;
    CSL_Vector* deletedDatasetReplies = NULL;
    CSL_Vector* addedCities = NULL;
    CSL_Vector* updatedCities = NULL;
    CSL_Vector* deletedCities = NULL;
    NB_MetadataSourceReply dbReply = {0};
    NB_EnhancedContentCityData city;

    nsl_memset(&city, 0, sizeof(city));

    if (!handler || !dataSource || up)
    {
        return;
    }

    if (result == NE_OK && status == NB_NetworkRequestStatus_Success)
    {
        result = result ? result : NB_MetadataHandlerGetInformation(handler, &info);
        result = result ? result : NB_MetadataDatabaseOperatorSetAllMetadataDeleted(dataSource->dbOperator, TRUE);
        result = result ? result : NB_MetadataInformationGetMetadataSourceReplies(info, &replies);
        if (result == NE_OK)
        {
            deletedDatasetReplies = CSL_VectorAlloc(sizeof(NB_MetadataSourceReply));
            addedCities = CSL_VectorAlloc(sizeof(NB_EnhancedContentCityData));
            updatedCities = CSL_VectorAlloc(sizeof(NB_EnhancedContentCityData));
            if (deletedDatasetReplies && addedCities && updatedCities)
            {
                length = CSL_VectorGetLength(replies);
                for (n = 0; n < length; ++n)
                {
                    reply = (NB_MetadataSourceReply*) CSL_VectorGetPointer(replies, n);

                    // Deal with force-delete reply.
                    if (reply->deleted)
                    {
                        continue;
                    }

                    nsl_memset(&dbReply, 0, sizeof(dbReply));
                    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(dataSource->dbOperator, reply->contentType, reply->datasetId, &dbReply);
                    if (result == NE_OK) // Old entry exists, check the manifest version.
                    {
                        if (nsl_strcmp(reply->manifestVersion, dbReply.manifestVersion) == 0)
                        {
                            // Same manifest version, remove DELETE flag of this entry.
                            result = NB_MetadataDatabaseOperatorSetMetadataDeletedById(dataSource->dbOperator, FALSE, dbReply.id);
                        }
                        else
                        {
                            // Different manifest version, update this entry, and append it to
                            // updatedCities if it represents for a city.

                            reply->id = dbReply.id;
                            reply->manifestState = NB_ECMS_ManifestNotSynchronized;
                            reply->selected = dbReply.selected;
                            reply->deleted = (int) FALSE;
                            
                            // Downloaded and failed data sizes are set after manifest request success.
                            reply->downloadedSize = 0;
                            reply->failedSize = 0;

                            // Append updated city for callback.
                            if (reply->contentType == NB_ECDT_CityModel && reply->selected)
                            {
                                nsl_memset(&city, 0, sizeof(city));
                                result = result ? result : ConvertMetadataSourceReplyToEnhancedContentCityData(reply, &city);
                                result = result ? result : CSL_VectorAppend(updatedCities, &city) ? NE_OK : NE_NOMEM;
                            }

                            result = result ? result : NB_MetadataDatabaseOperatorUpdateMetadata(dataSource->dbOperator, reply);
                        }
                    }
                    else if (result == NE_NOENT) // New entry, insert entry to DB, and append
                                                 // city to the addedCities if it is a city.
                    {
                        result = NE_OK;

                        reply->manifestState = NB_ECMS_ManifestNotSynchronized;
                        reply->updateStatus = NB_ECUS_UpdateStatusNew;
                        reply->selected = (int) FALSE;
                        reply->deleted = (int) FALSE;
                        reply->downloadedSize = 0;
                        reply->failedSize = 0;

                        // Append added city for callback.
                        if (reply->contentType == NB_ECDT_CityModel)
                        {
                            // The 'New' status for a city shall remain until the city is acted on or till the next addition of new cities to the list.
                            if (CSL_VectorGetLength(addedCities) <= 0)
                            {
                                result = result ? result : NB_MetadataDatabaseOperatorSetMetadataUpdateStatusByContentType(dataSource->dbOperator, (int) NB_ECUS_UpdateStatusUsed, (int) NB_ECDT_CityModel);
                            }

                            nsl_memset(&city, 0, sizeof(city));
                            result = result ? result : ConvertMetadataSourceReplyToEnhancedContentCityData(reply, &city);
                            result = result ? result : CSL_VectorAppend(addedCities, &city) ? NE_OK : NE_NOMEM;
                        }

                        result = result ? result : NB_MetadataDatabaseOperatorInsertMetadata(dataSource->dbOperator, reply);
                    }

                    if (result != NE_OK)
                    {
                        break;
                    }

                    // Set cached dataset id for common textures, motorway junction objects or realistic signs.
                    switch ((NB_EnhancedContentDataType)reply->contentType)
                    {
                        case NB_ECDT_CommonTexture:
                        {
                            nsl_memset(dataSource->commonTexturesDatasetId, 0,
                                        sizeof(dataSource->commonTexturesDatasetId));
                            nsl_strlcpy(dataSource->commonTexturesDatasetId, reply->datasetId,
                                        sizeof(dataSource->commonTexturesDatasetId));
                            break;
                        }
                        case NB_ECDT_MotorwayJunctionObject:
                        {
                            nsl_memset(dataSource->motorwayJunctionObjectsDatasetId, 0,
                                        sizeof(dataSource->motorwayJunctionObjectsDatasetId));
                            nsl_strlcpy(dataSource->motorwayJunctionObjectsDatasetId, reply->datasetId,
                                        sizeof(dataSource->motorwayJunctionObjectsDatasetId));
                            break;
                        }
                        case NB_ECDT_RealisticSign:
                        {
                            nsl_memset(dataSource->realisticSignsDatasetId, 0,
                                        sizeof(dataSource->realisticSignsDatasetId));
                            nsl_strlcpy(dataSource->realisticSignsDatasetId, reply->datasetId,
                                        sizeof(dataSource->realisticSignsDatasetId));
                            break;
                        }
                        case NB_ECDT_SpeedLimitsSign:
                        {
                            nsl_memset(dataSource->speedLimitsSignDatasetId, 0,
                                        sizeof(dataSource->speedLimitsSignDatasetId));
                            nsl_strlcpy(dataSource->speedLimitsSignDatasetId, reply->datasetId,
                                        sizeof(dataSource->speedLimitsSignDatasetId));
                            break;
                        }
                        case NB_ECDT_SpecialRegions:
                        {
                            nsl_memset(dataSource->specialRegionsDatasetId, 0,
                                        sizeof(dataSource->specialRegionsDatasetId));
                            nsl_strlcpy(dataSource->specialRegionsDatasetId, reply->datasetId,
                                        sizeof(dataSource->specialRegionsDatasetId));
                            break;
                        }
                        case NB_ECDT_MapRegionsCommon:
                        {
                            nsl_memset(dataSource->regionCommonDatasetId, 0,
                                        sizeof(dataSource->regionCommonDatasetId));
                            nsl_strlcpy(dataSource->regionCommonDatasetId, reply->datasetId,
                                        sizeof(dataSource->regionCommonDatasetId));
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                result = NE_NOMEM;
            }

            if (replies)
            {
                CSL_VectorDealloc(replies);
                replies = NULL;
            }
        }

        // Set city summary display and metadata valid.
        if (result == NE_OK)
        {
            result = NB_MetadataInformationGetCitySummaryDisplay(info, &citySummaryDisplay);
            if (result == NE_OK)
            {
                if (dataSource->citySummaryDisplay)
                {
                    nsl_free(dataSource->citySummaryDisplay);
                }
                dataSource->citySummaryDisplay = nsl_strdup(citySummaryDisplay);
                if (!dataSource->citySummaryDisplay)
                {
                    result = NE_NOMEM;
                }
            }
        }

        NB_MetadataInformationDestroy(info);

        // Get metadata source replies which persistent dataset should be deleted.
        if (result == NE_OK)
        {
            result = result ? result : NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByDeleted(dataSource->dbOperator, TRUE, deletedDatasetReplies);
            if (result == NE_OK)
            {
                deletedCities = CSL_VectorAlloc(sizeof(NB_EnhancedContentCityData));
                if (deletedCities)
                {
                    length = CSL_VectorGetLength(deletedDatasetReplies);
                    for (n = 0; n < length; ++n)
                    {
                        reply = (NB_MetadataSourceReply*) CSL_VectorGetPointer(deletedDatasetReplies, n);

                        // Append deleted city for callback.
                        if (reply->contentType == NB_ECDT_CityModel && reply->selected && reply->deleted)
                        {
                            nsl_memset(&city, 0, sizeof(city));
                            result = result ? result : ConvertMetadataSourceReplyToEnhancedContentCityData(reply, &city);
                            result = result ? result : CSL_VectorAppend(deletedCities, &city) ? NE_OK : NE_NOMEM;
                        }

                        result = result ? result : NB_MetadataDatabaseOperatorDeleteManifestByMetadataId(dataSource->dbOperator, reply->id);

                        if (result != NE_OK)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    result = NE_NOMEM;
                }
            }
        }
    }

    if (dataSource->metadataSynchronizeCallback.callback)
    {
        dataSource->metadataSynchronizeCallback.callback(dataSource, result, status, percent, deletedDatasetReplies, addedCities, updatedCities, deletedCities, dataSource->metadataSynchronizeCallback.callbackData);
    }

    if (result == NE_OK && status == NB_NetworkRequestStatus_Success)
    {
        // Ignore the error.
        result = NB_MetadataDatabaseOperatorDeleteMetadataByDeleted(dataSource->dbOperator);
    }

    if (replies)
    {
        CSL_VectorDealloc(replies);
        replies = NULL;
    }

    if (deletedDatasetReplies)
    {
        CSL_VectorDealloc(deletedDatasetReplies);
        deletedDatasetReplies = NULL;
    }

    if (addedCities)
    {
        CSL_VectorDealloc(addedCities);
        addedCities = NULL;
    }

    if (updatedCities)
    {
        CSL_VectorDealloc(updatedCities);
        updatedCities = NULL;
    }

    if (deletedCities)
    {
        CSL_VectorDealloc(deletedCities);
        deletedCities = NULL;
    }
}

/* Manifest request callback

 @return None
 */
static void RequestManifestCallback(void* handler, NB_RequestStatus status, NB_Error result, nb_boolean up, int percent, void* userData)
{
    uint32 lengthCities = 0;
    uint32 lengthRegions = 0;
    int64 metadataId = 0;
    NB_EnhancedContentDataSource* dataSource = (NB_EnhancedContentDataSource*)userData;
    NB_ManifestInformation* info = NULL;
    CSL_Vector* manifestReplies = NULL;
    CSL_Vector* removedReplies = NULL;
    NB_MetadataSourceReply dbMetadataReply = {0};
    NB_ManifestQueryCity manifestQueryCity = {0};

    if (!handler || !dataSource || up || status == NB_NetworkRequestStatus_Progress)
    {
        return;
    }
    if (result == NE_OK && status == NB_NetworkRequestStatus_Success)
    {
        result = NB_ManifestHandlerGetInformation(handler, &info);
        if (result == NE_OK)
        {
            lengthCities = CSL_VectorGetLength(dataSource->manifestQueryCities);
            lengthRegions = CSL_VectorGetLength(dataSource->manifestQueryRegions);
            if (lengthCities <= 0 && lengthRegions <= 0)
            {
                return;
            }

            if (lengthCities > 0)
            {
                CSL_VectorGet(dataSource->manifestQueryCities, lengthCities - 1, &manifestQueryCity);
                metadataId = manifestQueryCity.metadataId;
                CSL_VectorRemove(dataSource->manifestQueryCities, lengthCities - 1);
            }
            else
            {
                CSL_VectorGet(dataSource->manifestQueryRegions, lengthRegions - 1, &manifestQueryCity);
                metadataId = manifestQueryCity.metadataId;
                CSL_VectorRemove(dataSource->manifestQueryRegions, lengthRegions - 1);
            }
            result = (result == NE_OK) ? NB_MetadataDatabaseOperatorGetMetadataSourceReplyById(dataSource->dbOperator, metadataId, &dbMetadataReply) : result;
            result = (result == NE_OK) ? NB_ManifestInformationGetMetadataManifestReplies(info, metadataId, &manifestReplies) : result;
            result = (result == NE_OK) ? UpdateManifestReplies(dataSource->dbOperator, &dbMetadataReply, manifestReplies, &removedReplies) : result;

            CSL_VectorDealloc(manifestReplies);
            manifestReplies = NULL;

            if (result == NE_OK)
            {
                dbMetadataReply.manifestState = NB_ECMS_ManifestLatest;
                result = NB_MetadataDatabaseOperatorUpdateMetadata(dataSource->dbOperator, &dbMetadataReply);
            }
            if (result == NE_OK)
            {
                result = StartManifestRequest(dataSource);
                if ((result == NE_OK || result == NE_NOENT) && dataSource->manifestSynchronizeCallback.callback)
                {
                    // Return progress state to remove invalid manifest replies.
                    dataSource->manifestSynchronizeCallback.callback(dataSource,
                                                                    NE_OK,
                                                                    result == NE_OK ? NB_NetworkRequestStatus_Progress :  NB_NetworkRequestStatus_Success,
                                                                    lengthCities > 0 ? NB_ECDT_CityModel : NB_ECDT_MapRegions,
                                                                    manifestQueryCity.cityId,
                                                                    removedReplies,
                                                                    dataSource->manifestSynchronizeCallback.callbackData);
                }
            }
            NB_ManifestInformationDestroy(info);
        }
    }
    else
    {
        CSL_VectorRemoveAll(dataSource->manifestQueryCities);
        CSL_VectorRemoveAll(dataSource->manifestQueryRegions);

        if (dataSource->manifestSynchronizeCallback.callback)
        {
            dataSource->manifestSynchronizeCallback.callback(dataSource,
                                                             result,
                                                             status,
                                                             lengthCities > 0 ? NB_ECDT_CityModel : NB_ECDT_MapRegions,
                                                             manifestQueryCity.cityId,
                                                             removedReplies,
                                                             dataSource->manifestSynchronizeCallback.callbackData);
        }
    }

    if (removedReplies)
    {
        CSL_VectorDealloc(removedReplies);
        removedReplies = NULL;
    }
}
/*! Replace the first occurrence of substring with replaced string in source string.

 @return NB_OK if success
 */
static NB_Error
ReplaceString(char* source,                                                 /*!< Source string */
              uint32 maxSourceSize,                                         /*!< Maximum size of source string */
              const char* sub,                                              /*!< Substring */
              const char* replaced                                          /*!< Replaced string */
              )
{
    size_t subLength = 0;
    size_t replacedLength = 0;
    char* position = NULL;
    char tmp[MAX_ENHANCED_CONTENT_PATH_LENGTH];

    if (!source || !sub || !replaced)
    {
        return NE_INVAL;
    }

    subLength = nsl_strlen(sub);
    if (subLength == 0)
    {
        return NE_INVAL;
    }

    position = nsl_stristr(source, sub);
    if (!position)
    {
        return NE_OK;
    }

    replacedLength = nsl_strlen(replaced);
    if (maxSourceSize + subLength < nsl_strlen(source) + replacedLength + 1)
    {
        return NE_RANGE;
    }

    if (nsl_strlen(position + subLength) + 1 > MAX_ENHANCED_CONTENT_PATH_LENGTH)
    {
        return NE_RANGE;
    }

    nsl_memset(tmp, 0, MAX_ENHANCED_CONTENT_PATH_LENGTH);
    nsl_strcpy(tmp, position + subLength);

    *position = '\0';
    nsl_strcat(source, replaced);
    nsl_strcat(source, tmp);

    return NE_OK;
}

/*! Get full filename with path and filename

 @return NB_OK if success
 */
static NB_Error
GetFullFilename(NB_EnhancedContentDataSource* dataSource,
                const char* path,                                           /*!< Optional Path */
                const char* filename,                                       /*!< filename */
                uint32 maxFullFilenameSize,                                 /*!< Maximum size of constructed full filename */
                char* fullFilename                                          /*!< Buffer to receive constructed full filename */
                )
{
    uint32 pathLength = 0;
    uint32 length = 0;

    if (!path || !filename || !fullFilename)
    {
        return NE_INVAL;
    }

    pathLength = (uint32)nsl_strlen(path);
    length = (uint32)(pathLength + nsl_strlen(filename) + 2);   // '/'
    if (maxFullFilenameSize < length)
    {
        return NE_RANGE;
    }

    nsl_memset(fullFilename, 0, maxFullFilenameSize);
    if (pathLength != 0)
    {
        nsl_strcpy(fullFilename, path);
        PAL_FileAppendPath(NB_ContextGetPal(dataSource->context), fullFilename, length, filename);
    }
    else
    {
        nsl_strcpy(fullFilename, filename);
    }

    return NE_OK;
}

/*! Get dataset id by content type

 This function is used only for MJO, ASR and common texture
 which each relates to one dataset.

 @return NB_OK if success
 */
static NB_Error
GetDatasetIdByContentType(NB_MetadataDatabaseOperator* dbOperator,          /*!< NB_MetadataDatabaseOperator object */
                          NB_EnhancedContentDataType contentType,           /*!< Content type */
                          uint32 maxDatasetIdSize,                          /*!< Maximum size of constructed dataset id */
                          char* datasetId                                   /*!< Buffer to receive constructed dataset id */
                          )
{
    NB_Error result = NE_OK;
    CSL_Vector* vectorReplies = NULL;
    NB_MetadataSourceReply* reply = NULL;

    if (!dbOperator || !datasetId)
    {
        return NE_INVAL;
    }

    // Alloc vector of metadata source replies.
    vectorReplies = CSL_VectorAlloc(sizeof(NB_MetadataSourceReply));
    if (!vectorReplies)
    {
        return NE_NOMEM;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByContentType(dbOperator, (int) contentType, vectorReplies);
    if (result != NE_OK)
    {
        CSL_VectorDealloc(vectorReplies);
        return result;
    }

    if (CSL_VectorGetLength(vectorReplies) <= 0)
    {
        CSL_VectorDealloc(vectorReplies);
        return NE_NOENT;
    }

    reply = (NB_MetadataSourceReply*) CSL_VectorGetPointer(vectorReplies, 0);

    if (maxDatasetIdSize < nsl_strlen(reply->datasetId) + 1)
    {
        CSL_VectorDealloc(vectorReplies);
        return NE_RANGE;
    }

    nsl_memset(datasetId, 0, maxDatasetIdSize);
    nsl_strcpy(datasetId, reply->datasetId);

    CSL_VectorDealloc(vectorReplies);

    return NE_OK;
}

/*! Convert NB_MetadataSourceReply to NB_EnhancedContentCityData

 TODO: Only support city model now.

 @return NB_OK if success
 */
static NB_Error
ConvertMetadataSourceReplyToEnhancedContentCityData(const NB_MetadataSourceReply* reply, NB_EnhancedContentCityData* city)
{
    if (!reply || reply->contentType != NB_ECDT_CityModel || !city)
    {
        return NE_INVAL;
    }

    nsl_strlcpy(city->cityId, reply->datasetId, sizeof(city->cityId));
    nsl_strlcpy(city->version, reply->manifestVersion, sizeof(city->version));
    nsl_strlcpy(city->displayName, reply->displayName, sizeof(city->displayName));

    city->selected = (nb_boolean) reply->selected;
    city->deleted = (nb_boolean) reply->deleted;

    city->updateStatus = (NB_EnhancedContentUpdateStatus) reply->updateStatus;

    city->cityModelDataSize = reply->datasetSize;
    city->downloadedCityModelDataSize = reply->downloadedSize;
    city->failedCityModelDataSize = reply->failedSize;

    city->box.point1.latitude = reply->boundingLatitude1;
    city->box.point1.longitude = reply->boundingLongitude1;
    city->box.point2.latitude = reply->boundingLatitude2;
    city->box.point2.longitude = reply->boundingLongitude2;
    city->center.latitude = (city->box.point1.latitude + city->box.point2.latitude) / 2.0;
    city->center.longitude = (city->box.point1.longitude + city->box.point2.longitude) / 2.0;

    return NE_OK;
}

/*! Update manifest replies

    This function updates manifest replies in database and returns removed replies
    in parameter 'removedReplies'.

    @return NE_OK if success
*/
NB_Error
UpdateManifestReplies(NB_MetadataDatabaseOperator* dbOperator,  /*!< An instance of database operator */
                      NB_MetadataSourceReply* metadataToUpdate, /*!< A metadata reply need to update manifest */
                      CSL_Vector* replies,                      /*!< Manifest replies to update. It can be set to NULL,
                                                                     if user want to clear manifest replies. */
                      CSL_Vector** removedReplies               /*!< If it is not NULL, on return removed manifest replies
                                                                     from database if success. User need to free this vector
                                                                     by function CSL_VectorDealloc when it is useless. */
                      )
{
    NB_Error error = NE_OK;
    int64 metadataId = 0;
    int metadataDownloadedSize = 0;
    int index = 0;
    int dbIndex = 0;
    int length = 0;
    int dbLength = 0;
    CSL_Vector* dbReplies = NULL;
    CSL_Vector* repliesToRemove = NULL;
    NB_MetadataManifestReply tempReply = {0};

    // Check if parameters are valid.
    if ((!dbOperator) || (!metadataToUpdate))
    {
        return NE_INVAL;
    }

    // Alloc a vector of manifest replies got from database.
    dbReplies = CSL_VectorAlloc(sizeof(NB_MetadataManifestReply));
    if (!dbReplies)
    {
        return NE_NOMEM;
    }

    // Get the metadata ID related with manifest replies need to update.
    metadataId = metadataToUpdate->id;

    // Get manifest replies from database.
    error = NB_MetadataDatabaseOperatorGetMetadataManifestRepliesByMetadataId(dbOperator,
                                                                              metadataId,
                                                                              dbReplies);
    if (error != NE_OK)
    {
        CSL_VectorDealloc(dbReplies);
        dbReplies = NULL;
        return error;
    }

    // Check if parameter 'replies' is empty.
    if (!replies || CSL_VectorGetLength(replies) <= 0)
    {
        // Remove all manifest replies in database.
        error = NB_MetadataDatabaseOperatorDeleteManifestByMetadataId(dbOperator,
                                                                      metadataId);
        if (error != NE_OK)
        {
            CSL_VectorDealloc(dbReplies);
            dbReplies = NULL;
            return error;
        }

        // Set downloaded size of this dataset to 0.
        metadataToUpdate->downloadedSize = 0;

        // Set removed replies to return.
        if (removedReplies)
        {
            *removedReplies = dbReplies;
        }
        else
        {
            CSL_VectorDealloc(dbReplies);
            dbReplies = NULL;
        }

        return NE_OK;
    }

    // Sort two vectors of manifest replies to get difference.
    CSL_VectorSort2(replies, (CSL_VectorCompareFunction) CompareManifestFileId, &tempReply);
    CSL_VectorSort2(dbReplies, (CSL_VectorCompareFunction) CompareManifestFileId, &tempReply);

    // Get length of manifest replies to update.
    length = CSL_VectorGetLength(replies);
    dbLength = CSL_VectorGetLength(dbReplies);

    // Alloc a vector of removed manifest replies to return.
    repliesToRemove = CSL_VectorAlloc(sizeof(NB_MetadataManifestReply));
    if (!repliesToRemove)
    {
        CSL_VectorDealloc(dbReplies);
        dbReplies = NULL;
        return NE_NOMEM;
    }

    // Start a transaction to operate database.
    error = NB_MetadataDatabaseOperatorTransactionBegin(dbOperator);
    if (error != NE_OK)
    {
        CSL_VectorDealloc(repliesToRemove);
        repliesToRemove = NULL;
        CSL_VectorDealloc(dbReplies);
        dbReplies = NULL;
        return error;
    }

    // Iterate both vectors to deal with different manifest replies.
    while ((index < length) && (dbIndex < dbLength))
    {
        int comparedResult = 0;
        NB_MetadataManifestReply* reply = (NB_MetadataManifestReply*) CSL_VectorGetPointer(replies, index);
        NB_MetadataManifestReply* dbReply = (NB_MetadataManifestReply*) CSL_VectorGetPointer(dbReplies, dbIndex);

        // Check if manifest replies are NULL.
        if (!reply)
        {
            ++index;
            continue;
        }

        if (!dbReply)
        {
            ++dbIndex;
            continue;
        }

        /* Compare two replies to increase indexes and itearte both vectors.
           Because both vectors are sorted. */
        comparedResult = CompareManifestFileId(reply, dbReply);
        if (comparedResult < 0)
        {
            // There is no this manifest reply in database. So add it to database.
            reply->metadataId = metadataId;
            reply->downloadedSize = 0;
            error = NB_MetadataDatabaseOperatorInsertManifest(dbOperator,
                                                              reply);
            if (error != NE_OK)
            {
                break;
            }

            ++index;
        }
        else if (comparedResult > 0)
        {
            // This manifest reply need to remove from database.
            error = NB_MetadataDatabaseOperatorDeleteManifestByMetadataIdAndFileId(dbOperator,
                                                                                   metadataId,
                                                                                   dbReply->fileId);
            if (error != NE_OK)
            {
                break;
            }

            // Add this manifest query to vector of replies to remove.
            if (CSL_VectorAppend(repliesToRemove, dbReply) == 0)
            {
                error = NE_NOMEM;
                break;
            }

            ++dbIndex;
        }
        else    // comparedResult == 0
        {
            // Check if file versions are same.
            if (nsl_strcmp(reply->fileVersion, dbReply->fileVersion) != 0)
            {
                // Update this manifest query with latest file version.
                reply->id = dbReply->id;
                reply->metadataId = metadataId;
                reply->downloadStatus = NB_ECDS_DownloadNotStarted;
                reply->downloadedSize = 0;
                error = NB_MetadataDatabaseOperatorUpdateManifest(dbOperator, reply);
                if (error != NE_OK)
                {
                    break;
                }

                // Add this manifest query to vector of replies to remove.
                if (CSL_VectorAppend(repliesToRemove, dbReply) == 0)
                {
                    error = NE_NOMEM;
                    break;
                }
            }
            else
            {
                // Update downloaded size of this dataset.
                metadataDownloadedSize += dbReply->downloadedSize;
            }

            ++index;
            ++dbIndex;
        }
    }

    // Check if there is an error occurred in above iteration.
    if (error != NE_OK)
    {
        // Ignore the returned error.
        NB_MetadataDatabaseOperatorTransactionRollback(dbOperator);

        CSL_VectorDealloc(repliesToRemove);
        repliesToRemove = NULL;
        CSL_VectorDealloc(dbReplies);
        dbReplies = NULL;
        return error;
    }

    // There are some manifest replies need to remove from database.
    for (; dbIndex < dbLength; ++dbIndex)
    {
        NB_MetadataManifestReply* dbManifestReply = (NB_MetadataManifestReply*) CSL_VectorGetPointer(dbReplies, dbIndex);

        // Check if this manifest reply is NULL.
        if (!dbManifestReply)
        {
            continue;
        }

        // This manifest reply need to remove from database.
        error = NB_MetadataDatabaseOperatorDeleteManifestByMetadataIdAndFileId(dbOperator,
                                                                               metadataId,
                                                                               dbManifestReply->fileId);
        if (error != NE_OK)
        {
            break;
        }

        // Add this manifest query to vector of replies to remove.
        if (CSL_VectorAppend(repliesToRemove, dbManifestReply) == 0)
        {
            error = NE_NOMEM;
            break;
        }
    }

    // Dealloc the vector of manifest replies got from database.
    CSL_VectorDealloc(dbReplies);
    dbReplies = NULL;

    // Check if there is an error occurred in above iteration.
    if (error != NE_OK)
    {
        // Ignore the returned error.
        NB_MetadataDatabaseOperatorTransactionRollback(dbOperator);

        CSL_VectorDealloc(repliesToRemove);
        repliesToRemove = NULL;
        return error;
    }

    // There are some manifest replies need to add to database.
    for (; index < length; ++index)
    {
        NB_MetadataManifestReply* manifestReply = (NB_MetadataManifestReply*) CSL_VectorGetPointer(replies, index);

        // Check if this manifest reply is NULL.
        if (!manifestReply)
        {
            continue;
        }

        manifestReply->metadataId = metadataId;
        manifestReply->downloadedSize = 0;
        error = NB_MetadataDatabaseOperatorInsertManifest(dbOperator,
                                                          manifestReply);
        if (error != NE_OK)
        {
            break;
        }
    }

    // Commit the transaction to database.
    if (error == NE_OK)
    {
        error = NB_MetadataDatabaseOperatorTransactionCommit(dbOperator);
    }

    // Check if there is an error occurred in above iteration.
    if (error != NE_OK)
    {
        // Ignore the returned error.
        NB_MetadataDatabaseOperatorTransactionRollback(dbOperator);

        CSL_VectorDealloc(repliesToRemove);
        repliesToRemove = NULL;
        return error;
    }

    // Set downloaded size of this dataset to return.
    metadataToUpdate->downloadedSize = metadataDownloadedSize;

    // Set removed replies to return.
    if (removedReplies)
    {
        *removedReplies = repliesToRemove;
    }
    else
    {
        CSL_VectorDealloc(repliesToRemove);
        repliesToRemove = NULL;
    }

    return NE_OK;
}

/*! Compare two file IDs of manifest replies

    @return -1 if 'reply1' is smaller,
            1 if 'reply2' is smaller,
            0 if both replies are equal.
*/
int
CompareManifestFileId(const NB_MetadataManifestReply* reply1,
                      const NB_MetadataManifestReply* reply2
                      )
{
    // Check if they are same pointers. They maybe both NULL.
    if (reply1 == reply2)
    {
        return 0;
    }

    // Check if one pointer is NULL.
    if (!reply1)
    {
        return -1;
    }

    if (!reply2)
    {
        return 1;
    }

    return nsl_strcmp(reply1->fileId, reply2->fileId);
}

/*! Compares two NB_EnhancedContentCityData's, using the display name to compare.

 @see NB_EnhancedContentCityData
 */
static int
DisplayNameCompareFunction(const void* left, const void* right)
{
    return nsl_strcmp(((const NB_EnhancedContentCityData*) left)->displayName, ((const NB_EnhancedContentCityData*)right)->displayName);
}

NB_Error NB_EnhancedContentDataSourceCalculateRigonDownloadPercentage(
            NB_EnhancedContentDataSource* instance,
            uint32 percent,
            const NB_EnhancedContentIdentifier* item,
            uint32* percentOut)
{
    NB_Error result = NE_OK;
    NB_MetadataSourceReply metadataReply = {0};
    NB_MetadataManifestReply manifestReply = {0};

    if (!instance || !item || !item->dataset || !percentOut ||
        !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
                instance->dbOperator, item->type, item->dataset, &metadataReply);
    if (result != NE_OK)
    {
        return result;
    }

    //if the dataset has no item, the percentOut is the percent
    if (!item->itemId)
    {
        *percentOut = percent;
        return NE_OK;
    }

    result = NB_MetadataDatabaseOperatorGetMetadataManifestReplyByMetadataIdAndFileId(
                instance->dbOperator, metadataReply.id, item->itemId, &manifestReply);
    if (result != NE_OK)
    {
        return result;
    }

    if (metadataReply.datasetSize != 0)
    {
        *percentOut = (uint32)((manifestReply.fileSize * (percent/100.f) +
                                metadataReply.downloadedSize)/
                                metadataReply.datasetSize*100);
    }
    return NE_OK;
}

NB_Error NB_EnhancedContentDataSourceUpdateDataSetDownloadStatus(
    NB_EnhancedContentDataSource* instance,
    const NB_EnhancedContentIdentifier* dataItem,
    NB_EnhancedContentDownloadStatus downloadStatus
    )
{
    if (!instance || !dataItem || !dataItem->dataset ||
        !NB_EnhancedContentDataSourceIsValid(instance))
    {
        return NE_INVAL;
    }
    return NB_MetadataDatabaseOperatorUpdateDownloadStatusByDatasetId(instance->dbOperator,
                                                                    downloadStatus,
                                                                    dataItem->dataset);
}
/*! @} */

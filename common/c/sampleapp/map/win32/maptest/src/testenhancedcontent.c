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

    @file     TestEnhancedContent.h
    @defgroup TestEnhancedContent_h System tests for Enhanced Content functions
*/
/*
    See description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "main.h"
#include "testenhancedcontent.h"
#include "testnetwork.h"
#include "platformutil.h"
#include "paltimer.h"
#include "palfile.h"
#include "nbenhancedcontentmanager.h"

// Local Constants ...........................................................

#define TEST_MAX_PATH_LENGTH                        256

#define TEST_SCREEN_WIDTH                           780
#define TEST_SCREEN_HEIGHT                          585

#define TEST_COUNTRY                                "USA"
#define TEST_NBGM_VERSION                           "1"

#define TEST_ECM_FILE_FORMAT                        "BIN"
#define TEST_MJO_FILE_FORMAT                        "BIN"
#define TEST_SAR_FILE_FORMAT                        "PNG"
#define TEST_SPD_FILE_FORMAT                        "PNG"
#define TEST_TEXTURE_FILE_FORMAT                    "PNG"
#define TEST_COMPRESSED_TEXTURE_FILE_FORMAT         "zip"

// directories for test data
#define CITYMODEL_PATH                              "Citymodels"
#define MJO_PATH                                    "MJO"
#define SAR_PATH                                    "SAR"
#define SPD_PATH                                    "SPD"
#define TEXTURE_PATH                                "Textures"


// Local Functions ...........................................................

static void TestEnhancedContentCreate(void);
static void TestEnhancedContentStartMetadataSynchronization(void);
static void TestEnhancedContentSetManifestSynchronizationAllowable(void);
static void TestEnhancedContentCancelDataDownload(void);
static void TestEnhancedContentUnselectCity(void);
static void TestEnhancedContentSetAllowableDownload(void);
static void TestEnhancedContentStartManifestSynchronizationAndDownload(void);
static void TestEnhancedContentRemoveAllStoredData(void);

static void ContentSynchronizationCallback(NB_EnhancedContentManager* manager,
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
static void ContentDownloadCallback(NB_EnhancedContentManager* manager,
                                    NB_Error result,
                                    uint32 percent,
                                    NB_EnhancedContentDataType dataType,
                                    const char* datasetId,
                                    void* userData);

static void TimerCallbackForSettingManifestSynchronizationAllowable(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static void TimerCallbackForCancelingDataDownload(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static void TimerCallbackForUnselectingCity(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static void TimerCallbackForPausingPreloadDataDownload(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static void TimerCallbackForResumingPreloadDataDownload(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static void TimerCallbackForRemovingAllStoredData(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);


// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;


/*! Add all your test functions here

@return None
*/
void
TestEnhancedContent_AddTests( CU_pSuite pTestSuite, int level )
{
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestEnhancedContentCreate", TestEnhancedContentCreate);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestEnhancedContentStartMetadataSynchronization", TestEnhancedContentStartMetadataSynchronization);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestEnhancedContentSetManifestSynchronizationAllowable", TestEnhancedContentSetManifestSynchronizationAllowable);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestEnhancedContentCancelDataDownload", TestEnhancedContentCancelDataDownload);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestEnhancedContentUnselectCity", TestEnhancedContentUnselectCity);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestEnhancedContentSetAllowableDownload", TestEnhancedContentSetAllowableDownload);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestEnhancedContentStartManifestSynchronizationAndDownload", TestEnhancedContentStartManifestSynchronizationAndDownload);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestEnhancedContentRemoveAllStoredData", TestEnhancedContentRemoveAllStoredData);
}


/*! Add common initialization code here.

@return 0

@see TestDirection_SuiteCleanup
*/
int 
TestEnhancedContent_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestDirection_SuiteSetup
*/
int 
TestEnhancedContent_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
    return 0;
}


/*! Set up enhanced content manager configuration for tests

    @return None. CUnit Asserts get called on failures.
*/
void
SetupEnhancedContentManagerConfiguration(NB_EnhancedContentManagerConfiguration* config)
{
    const char* basePath = NULL;
    char fullPath[TEST_MAX_PATH_LENGTH];

    config->screenWidth = TEST_SCREEN_WIDTH;
    config->screenHeight = TEST_SCREEN_HEIGHT;
    config->screenResolution = TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT;
    config->maxRealisticSignsCacheSize     = 100;
    config->maxMotorwayJunctionsCacheSize  = 100;
    config->maxSpeedLimitImagesCacheSize   = 100;

    config->enhancedCityModelEnabled = TRUE;
    config->motorwayJunctionObjectsEnabled = TRUE;
    config->realisticSignsEnabled = TRUE;
    config->speedLimitsSignEnabled = TRUE;

    nsl_strcpy(config->country, TEST_COUNTRY);
    nsl_strcpy(config->language, Test_OptionsGet()->currentLanguage);
    nsl_strcpy(config->nbgmVersion, TEST_NBGM_VERSION);

    basePath = GetBasePath();
    CU_ASSERT_PTR_NOT_NULL(basePath);
    nsl_strcpy(config->persistentMetadataPath, basePath);

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strcpy(fullPath, basePath);
    nsl_strcat(fullPath, PATH_DELIMITER CITYMODEL_PATH);
    nsl_strcpy(config->enhancedCityModelPath, fullPath);

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strcpy(fullPath, basePath);
    nsl_strcat(fullPath, PATH_DELIMITER MJO_PATH);
    nsl_strcpy(config->motorwayJunctionObjectPath, fullPath);

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strcpy(fullPath, basePath);
    nsl_strcat(fullPath, PATH_DELIMITER TEXTURE_PATH);
    nsl_strcpy(config->texturesPath, fullPath);

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strcpy(fullPath, basePath);
    nsl_strcat(fullPath, PATH_DELIMITER SAR_PATH);
    nsl_strcpy(config->realisticSignsPath, fullPath);

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strcpy(fullPath, basePath);
    nsl_strcat(fullPath, PATH_DELIMITER SPD_PATH);
    nsl_strcpy(config->speedLimitsSignsPath, fullPath);

    nsl_strcpy(config->enhancedCityModelFileFormat, TEST_ECM_FILE_FORMAT);
    nsl_strcpy(config->motorwayJunctionObjectFileFormat, TEST_MJO_FILE_FORMAT);
    nsl_strcpy(config->realisticSignsFileFormat, TEST_SAR_FILE_FORMAT);
    nsl_strcpy(config->speedLimitsSignFileFormat, TEST_SPD_FILE_FORMAT);
    nsl_strcpy(config->textureFileFormat, TEST_TEXTURE_FILE_FORMAT);
    nsl_strcpy(config->compressedTextureFileFormat, TEST_COMPRESSED_TEXTURE_FILE_FORMAT);
}


/*! Test enhanced content manager creation

    @return None. CUnit Asserts get called on failures.
*/
static void 
TestEnhancedContentCreate(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        NB_Error result = NE_OK;
        NB_EnhancedContentManager* manager = NULL;

        NB_EnhancedContentSynchronizationCallback synchronizationCallback = { ContentSynchronizationCallback, 0 };
        NB_EnhancedContentDownloadCallback downloadCallback = { ContentDownloadCallback, 0 };
        NB_EnhancedContentManagerConfiguration config = { 0 };

        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        SetupEnhancedContentManagerConfiguration(&config);

        result = NB_EnhancedContentManagerCreate(context, &config, &synchronizationCallback, &downloadCallback, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(manager);

        result = NB_EnhancedContentManagerRemoveAllStoredData(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_EnhancedContentManagerDestroy(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test enhanced content manager create and initial start sync

    @return None. CUnit Asserts get called on failures.
*/
static void
TestEnhancedContentStartMetadataSynchronization(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        NB_Error result = NE_OK;
        NB_EnhancedContentManager* manager = NULL;
        
        NB_EnhancedContentSynchronizationCallback synchronizationCallback = { ContentSynchronizationCallback, 0 };
        NB_EnhancedContentDownloadCallback downloadCallback = { ContentDownloadCallback, 0 };
        NB_EnhancedContentManagerConfiguration config = { 0 };
        
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        SetupEnhancedContentManagerConfiguration(&config);

        result = NB_EnhancedContentManagerCreate(context, &config, &synchronizationCallback, &downloadCallback, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(manager);

        result = NB_EnhancedContentManagerStartMetadataSynchronization(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }
        }

        result = NB_EnhancedContentManagerDestroy(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test enhanced content manager set manifest synchronization allowable

    @return None. CUnit Asserts get called on failures.
*/
static void
TestEnhancedContentSetManifestSynchronizationAllowable(void)
{
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        NB_Error result = NE_OK;
        NB_EnhancedContentManager* manager = NULL;
        NB_EnhancedContentDownloadStatus downloadStatus = NB_ECDS_DownloadNotStarted;
        NB_EnhancedContentSynchronizationCallback synchronizationCallback = { ContentSynchronizationCallback, 0 };
        NB_EnhancedContentDownloadCallback downloadCallback = { ContentDownloadCallback, 0 };
        NB_EnhancedContentManagerConfiguration config = { 0 };

        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        SetupEnhancedContentManagerConfiguration(&config);

        result = NB_EnhancedContentManagerCreate(context, &config, &synchronizationCallback, &downloadCallback, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(manager);

        // Test download status
        CU_ASSERT(!NB_EnhancedContentManagerIsDownloadInProgress(manager, NB_ECD0_Preload));
        result = NB_EnhancedContentManagerGetCityDownloadStatus(manager, &downloadStatus);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(downloadStatus, NB_ECDS_DownloadNotStarted);

        result = NB_EnhancedContentManagerStartManifestSynchronization(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            CU_ASSERT_EQUAL(PAL_TimerSet(pal, 10000, TimerCallbackForSettingManifestSynchronizationAllowable, manager), PAL_Ok);

            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 600000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }

            CU_ASSERT_EQUAL(PAL_TimerCancel(pal, TimerCallbackForSettingManifestSynchronizationAllowable, manager), PAL_Ok);
        }

        result = NB_EnhancedContentManagerSetManifestSynchronizationAllowable(manager, TRUE);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_EnhancedContentManagerDestroy(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test enhanced content manager cancel data download

@return None. CUnit Asserts get called on failures.
*/
static void
TestEnhancedContentCancelDataDownload(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        NB_Error result = NE_OK;
        NB_EnhancedContentManager* manager = NULL;
        NB_EnhancedContentDownloadStatus downloadStatus = NB_ECDS_DownloadNotStarted;
        NB_EnhancedContentSynchronizationCallback synchronizationCallback = { ContentSynchronizationCallback, 0 };
        NB_EnhancedContentDownloadCallback downloadCallback = { ContentDownloadCallback, 0 };
        NB_EnhancedContentManagerConfiguration config = { 0 };

        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        SetupEnhancedContentManagerConfiguration(&config);

        result = NB_EnhancedContentManagerCreate(context, &config, &synchronizationCallback, &downloadCallback, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(manager);

        // Test selecting cities
        result = NB_EnhancedContentManagerSelectCity(manager, "USA-BOS", TRUE);
        CU_ASSERT_EQUAL(result, NE_OK);
        result = NB_EnhancedContentManagerSelectCity(manager, "USA-LA", TRUE);
        CU_ASSERT_EQUAL(result, NE_OK);

        // Test download status
        CU_ASSERT(!NB_EnhancedContentManagerIsDownloadInProgress(manager, NB_ECD0_Preload));
        result = NB_EnhancedContentManagerGetCityDownloadStatus(manager, &downloadStatus);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(downloadStatus, NB_ECDS_DownloadNotStarted);

        result = NB_EnhancedContentManagerStartManifestSynchronization(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            // Set the timer for canceling all requests.
            CU_ASSERT_EQUAL(PAL_TimerSet(pal, 5000, TimerCallbackForCancelingDataDownload, manager), PAL_Ok);

            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 3600000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }

            CU_ASSERT_EQUAL(PAL_TimerCancel(pal, TimerCallbackForCancelingDataDownload, manager), PAL_Ok);
        }

        result = NB_EnhancedContentManagerDestroy(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test enhanced content manager unselect city

@return None. CUnit Asserts get called on failures.
*/
static void
TestEnhancedContentUnselectCity(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        NB_Error result = NE_OK;
        NB_EnhancedContentManager* manager = NULL;
        NB_EnhancedContentSynchronizationCallback synchronizationCallback = { ContentSynchronizationCallback, 0 };
        NB_EnhancedContentDownloadCallback downloadCallback = { ContentDownloadCallback, 0 };
        NB_EnhancedContentManagerConfiguration config = { 0 };

        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        SetupEnhancedContentManagerConfiguration(&config);

        result = NB_EnhancedContentManagerCreate(context, &config, &synchronizationCallback, &downloadCallback, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(manager);

        // Test selecting cities
        result = NB_EnhancedContentManagerSelectCity(manager, "USA-BOS", TRUE);
        CU_ASSERT_EQUAL(result, NE_OK);

        CU_ASSERT(!NB_EnhancedContentManagerIsDownloadInProgress(manager, NB_ECD0_Preload));

        result = NB_EnhancedContentManagerStartManifestSynchronization(manager);
        CU_ASSERT(result == NE_OK || result == NE_NOENT);

        if (result == NE_OK)
        {
            CU_ASSERT_EQUAL(PAL_TimerSet(pal, 5000, TimerCallbackForUnselectingCity, manager), PAL_Ok);

            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 3600000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }

            CU_ASSERT_EQUAL(PAL_TimerCancel(pal, TimerCallbackForUnselectingCity, manager), PAL_Ok);
        }

        result = NB_EnhancedContentManagerDestroy(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test enhanced content manager set allowable download

@return None. CUnit Asserts get called on failures.
*/
static void
TestEnhancedContentSetAllowableDownload(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        NB_Error result = NE_OK;
        const char* basePath = NULL;
        char fullPath[TEST_MAX_PATH_LENGTH];
        NB_EnhancedContentManager* manager = NULL;
        NB_EnhancedContentSynchronizationCallback synchronizationCallback = { ContentSynchronizationCallback, 0 };
        NB_EnhancedContentDownloadCallback downloadCallback = { ContentDownloadCallback, 0 };
        NB_EnhancedContentManagerConfiguration config = { 0 };

        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        SetupEnhancedContentManagerConfiguration(&config);

        result = NB_EnhancedContentManagerCreate(context, &config, &synchronizationCallback, &downloadCallback, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(manager);

        // Test setting allowable download
        result = NB_EnhancedContentManagerSetAllowableDownload(manager, NB_ECD0_OnDemand);
        CU_ASSERT_EQUAL(result, NE_OK);

        // Test selecting cities
        result = NB_EnhancedContentManagerSelectCity(manager, "USA-BOS", TRUE);
        CU_ASSERT_EQUAL(result, NE_OK);

        CU_ASSERT(!NB_EnhancedContentManagerIsDownloadInProgress(manager, NB_ECD0_Preload));

        result = NB_EnhancedContentManagerStartManifestSynchronization(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 3600000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }
        }

        basePath = GetBasePath();
        CU_ASSERT_PTR_NOT_NULL(basePath);

        // Test with city textures
        nsl_memset(fullPath, 0, sizeof(fullPath));
        nsl_strcpy(fullPath, basePath);
        nsl_strcat(fullPath, PATH_DELIMITER TEXTURE_PATH PATH_DELIMITER "USA-BOS" PATH_DELIMITER "920_FRB.PNG");
        CU_ASSERT_EQUAL(PAL_FileExists(pal, fullPath), PAL_ErrFileNotExist);

        // Test with city model
        nsl_memset(fullPath, 0, sizeof(fullPath));
        nsl_strcpy(fullPath, basePath);
        nsl_strcat(fullPath, PATH_DELIMITER CITYMODEL_PATH PATH_DELIMITER "USA-BOS" PATH_DELIMITER "000_000.BIN");
        CU_ASSERT_EQUAL(PAL_FileExists(pal, fullPath), PAL_ErrFileNotExist);

        result = NB_EnhancedContentManagerSelectCity(manager, "USA-BOS", FALSE);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_EnhancedContentManagerSetAllowableDownload(manager, NB_ECDO_All);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_EnhancedContentManagerDestroy(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test enhanced content manager create and start manifest synchronization and download

@return None. CUnit Asserts get called on failures.
*/
static void
TestEnhancedContentStartManifestSynchronizationAndDownload(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        NB_Error result = NE_OK;
        NB_EnhancedContentManager* manager = NULL;
        NB_EnhancedContentDownloadStatus downloadStatus = NB_ECDS_DownloadNotStarted;
        NB_EnhancedContentSynchronizationCallback synchronizationCallback = { ContentSynchronizationCallback, 0 };
        NB_EnhancedContentDownloadCallback downloadCallback = { ContentDownloadCallback, 0 };
        NB_EnhancedContentManagerConfiguration config = { 0 };
        NB_LatitudeLongitude location = { 34.05, -118.35 };

        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        SetupEnhancedContentManagerConfiguration(&config);

        result = NB_EnhancedContentManagerCreate(context, &config, &synchronizationCallback, &downloadCallback, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(manager);

        // Test selecting cities
        result = NB_EnhancedContentManagerSelectCity(manager, "USA-BOS", TRUE);
        CU_ASSERT_EQUAL(result, NE_OK);
        result = NB_EnhancedContentManagerSelectCity(manager, "USA-LA", TRUE);
        CU_ASSERT_EQUAL(result, NE_OK);

        // Test set location
        NB_EnhancedContentManagerSetLocation(manager, &location);
        CU_ASSERT_EQUAL(result, NE_OK);

        // Test get download status
        CU_ASSERT(!NB_EnhancedContentManagerIsDownloadInProgress(manager, NB_ECD0_Preload));
        result = NB_EnhancedContentManagerGetCityDownloadStatus(manager, &downloadStatus);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_EnhancedContentManagerStartManifestSynchronization(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            CU_ASSERT_EQUAL(PAL_TimerSet(pal, 900000, TimerCallbackForPausingPreloadDataDownload, manager), PAL_Ok);

            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 3600000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }

            CU_ASSERT_EQUAL(PAL_TimerCancel(pal, TimerCallbackForPausingPreloadDataDownload, manager), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_TimerCancel(pal, TimerCallbackForResumingPreloadDataDownload, manager), PAL_Ok);
        }

        result = NB_EnhancedContentManagerDestroy(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test enhanced content manager remove all stored data

@return None. CUnit Asserts get called on failures.
*/
static void
TestEnhancedContentRemoveAllStoredData(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        NB_Error result = NE_OK;
        NB_EnhancedContentManager* manager = NULL;
        NB_EnhancedContentDownloadStatus downloadStatus = NB_ECDS_DownloadNotStarted;
        NB_EnhancedContentSynchronizationCallback synchronizationCallback = { ContentSynchronizationCallback, 0 };
        NB_EnhancedContentDownloadCallback downloadCallback = { ContentDownloadCallback, 0 };
        NB_EnhancedContentManagerConfiguration config = { 0 };

        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        SetupEnhancedContentManagerConfiguration(&config);

        result = NB_EnhancedContentManagerCreate(context, &config, &synchronizationCallback, &downloadCallback, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(manager);

        // Test selecting cities
        result = NB_EnhancedContentManagerSelectCity(manager, "USA-DC", TRUE);
        CU_ASSERT_EQUAL(result, NE_OK);

        // Test download status
        CU_ASSERT(!NB_EnhancedContentManagerIsDownloadInProgress(manager, NB_ECD0_Preload));
        result = NB_EnhancedContentManagerGetCityDownloadStatus(manager, &downloadStatus);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(downloadStatus, NB_ECDS_DownloadNotStarted);

        result = NB_EnhancedContentManagerStartManifestSynchronization(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            // Set the timer for canceling all requests.
            CU_ASSERT_EQUAL(PAL_TimerSet(pal, 120000, TimerCallbackForRemovingAllStoredData, manager), PAL_Ok);

            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 3600000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }

            CU_ASSERT_EQUAL(PAL_TimerCancel(pal, TimerCallbackForRemovingAllStoredData, manager), PAL_Ok);
        }

        result = NB_EnhancedContentManagerDestroy(manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


static void
ContentSynchronizationCallback(NB_EnhancedContentManager* manager,
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
    CU_ASSERT_EQUAL(result, NE_OK);

    if (status == NB_NetworkRequestStatus_Progress)
    {
        return;
    }

    if (status == NB_NetworkRequestStatus_Success && result == NE_OK)
    {
        uint32 cityCount = 0;
        const char* citySummaryDisplay = NULL;
        NB_EnhancedContentCityData* cityArray = NULL;

        // Test getting city list
        result = NB_EnhancedContentManagerGetCityList(manager, &cityCount, &cityArray);
        CU_ASSERT_EQUAL(result, NE_OK);
        if (cityCount > 0)
        {
            CU_ASSERT_PTR_NOT_NULL(cityArray);
            nsl_free(cityArray);
            cityArray = NULL;
        }

        // Test getting city summary display
        result =NB_EnhancedContentManagerGetCitySummaryDisplay(manager, &citySummaryDisplay);
        CU_ASSERT_EQUAL(result, NE_OK);
    }
    else
    {
        LOGOUTPUT(LogLevelLow, ("NB_EnhancedContentStartSynchronization() returned %d", result));
    }

    // Trigger main thread
    SetCallbackCompletedEvent(g_CallbackEvent);
}


static void
ContentDownloadCallback(NB_EnhancedContentManager* manager,
                        NB_Error result,
                        uint32 percent,
                        NB_EnhancedContentDataType dataType,
                        const char* datasetId,
                        void* userData)
{
    NB_EnhancedContentDownloadStatus downloadStatus = NB_ECDS_DownloadNotStarted;

    CU_ASSERT_EQUAL(result, NE_OK);

    if (result != NE_OK || percent == 100)
    {
        CU_ASSERT(NB_EnhancedContentManagerIsCommonTextureAvailable(manager));
        CU_ASSERT(NB_EnhancedContentManagerIsMotorwayJunctionTextureAvailable(manager));
        CU_ASSERT(!NB_EnhancedContentManagerIsDownloadInProgress(manager, NB_ECD0_Preload));
        result = NB_EnhancedContentManagerGetCityDownloadStatus(manager, &downloadStatus);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(downloadStatus, NB_ECDS_DownloadComplete);

        // Test getting storage statistics
        {
            uint64 totalSize = 0;
            uint64 downloadedSize = 0;
            uint32 totalCommonTexturesSize = 0;
            uint32 downloadedCommonTexturesSize = 0;
            uint32 downloadedPercentage = 0;

            result = NB_EnhancedContentManagerGetStorageStatistics(manager, &totalSize, &downloadedSize, &totalCommonTexturesSize, &downloadedCommonTexturesSize, &downloadedPercentage);
            CU_ASSERT_EQUAL(result, NE_OK);
            // Comment this test for NB_EnhancedContentManagerSetAllowableDownload (totalSize != downloadedSize)
//            CU_ASSERT(totalSize == downloadedSize && totalCommonTexturesSize == downloadedCommonTexturesSize && downloadedPercentage == 100);
        }

        // Trigger main thread
        SetCallbackCompletedEvent(g_CallbackEvent);
    }
    else if(NB_EnhancedContentManagerIsDownloadInProgress(manager, NB_ECD0_Preload))
    {
        result = NB_EnhancedContentManagerGetCityDownloadStatus(manager, &downloadStatus);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(downloadStatus, NB_ECDS_DownloadInProgress);
    }
}


/*! Timer callback for canceling data download.
 
@return None
*/
void
TimerCallbackForSettingManifestSynchronizationAllowable(PAL_Instance *pal,
                                                        void *userData,
                                                        PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentManager* manager = (NB_EnhancedContentManager*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    result = NB_EnhancedContentManagerSetManifestSynchronizationAllowable(manager, FALSE);
    CU_ASSERT_EQUAL(result, NE_OK);

    // Cancel data download successfully
    SetCallbackCompletedEvent(g_CallbackEvent);
}


/*! Timer callback for canceling data download.

@return None
*/
void
TimerCallbackForCancelingDataDownload(PAL_Instance *pal,
                                      void *userData,
                                      PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentManager* manager = (NB_EnhancedContentManager*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    result = NB_EnhancedContentManagerCancelDataDownload(manager);
    CU_ASSERT_EQUAL(result, NE_OK);

    // Cancel data download successfully
    SetCallbackCompletedEvent(g_CallbackEvent);
}

/*! Timer callback for unselecting city.

@return None
*/
void
TimerCallbackForUnselectingCity(PAL_Instance *pal,
                                void *userData,
                                PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentManager* manager = (NB_EnhancedContentManager*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    result = NB_EnhancedContentManagerSelectCity(manager, "USA-BOS", FALSE);
    CU_ASSERT_EQUAL(result, NE_OK);

    // Cancel data download successfully
    SetCallbackCompletedEvent(g_CallbackEvent);
}

/*! Timer callback for pausing preloadData download.

@return None
*/
void
TimerCallbackForPausingPreloadDataDownload(PAL_Instance *pal,
                                           void *userData,
                                           PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentManager* manager = (NB_EnhancedContentManager*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    result = NB_EnhancedContentManagerPauseDownload(manager, NB_ECD0_Preload);
    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT_EQUAL(PAL_TimerSet(pal, 10000, TimerCallbackForResumingPreloadDataDownload, manager), PAL_Ok);
}

/*! Timer callback for resuming preloadData download.

@return None
*/
void
TimerCallbackForResumingPreloadDataDownload(PAL_Instance *pal,
                                            void *userData,
                                            PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentManager* manager = (NB_EnhancedContentManager*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    result = NB_EnhancedContentManagerResumeDownload(manager, NB_ECD0_Preload);
    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT_EQUAL(PAL_TimerSet(pal, 900000, TimerCallbackForPausingPreloadDataDownload, manager), PAL_Ok);
}


/*! Timer callback for removing All stored data.

@return None
*/
void
TimerCallbackForRemovingAllStoredData(PAL_Instance *pal,
                                      void *userData,
                                      PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentManager* manager = (NB_EnhancedContentManager*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    result = NB_EnhancedContentManagerRemoveAllStoredData(manager);
    CU_ASSERT_EQUAL(result, NE_OK);

    // Cancel data download successfully
    SetCallbackCompletedEvent(g_CallbackEvent);
}


/*! @} */

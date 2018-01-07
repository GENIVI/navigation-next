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

    @defgroup TestDownload_c Unit Tests for NB Enhanced Content Download

    Unit Tests for NB Enhanced Content Download

    This file contains all unit tests for internal download functions used
    by NB Enhanced Content
*/
/*
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

#include "testdownload.h"
#include "main.h"
#include "platformutil.h"
#include "networkutil.h"
#include "nbenhancedcontentdownloadmanager.h"
#include "palfile.h"
#include "paltimer.h"

#ifdef WINCE
#include "winbase.h"
#endif


// Local Constants ...............................................................................


// Local Types ...................................................................................

/*! Collection of all instances used for the unit tests */
typedef struct
{
    PAL_Instance* pal;
    NB_Context* context;

    NB_EnhancedContentDownloadManager* manager;
    NB_EnhancedContentDataSource* dataSource;

    NB_EnhancedContentManagerConfiguration configuration;

    void* event;

    uint32 expectedCount;
    uint32 downloadedCount;

} Instances;


// Local Functions ...............................................................................

// Test functions
static void TestDownloadCreate(void);
static void TestDownloadSingleItem(void);
static void TestDownloadCancelRequestData(void);
static void TestDownloadCancelRequests(void);
static void TestDownloadCancelAllRequests(void);
static void TestDownloadPriority(void);
static void TestDownloadCityModelAndPause(void);
static void TestDownloadMasterClear(void);

// Helper functions

static nb_boolean CreateInstances(Instances* instances);
static void DestroyInstances(Instances* instances);

static void DownloadCallback(NB_EnhancedContentDownloadManager* manager,
                             NB_Error result,
                             uint32 percent,
                             const NB_EnhancedContentIdentifier* item,
                             void* userData);
static void MetadataCallback(NB_EnhancedContentDataSource* dataSource,
                             NB_Error result,
                             NB_RequestStatus status,
                             uint32 percent,
                             CSL_Vector* deletedDatasetReplies,
                             CSL_Vector* addedCities,
                             CSL_Vector* updatedCities,
                             CSL_Vector* deletedCities,
                             void* userData);
static void ManifestCallback(NB_EnhancedContentDataSource* dataSource,       /*!< Enhanced content data source instance */
                             NB_Error result,                                /*!< Result of manifest synchronize */
                             NB_RequestStatus status,                        /*!< Status of manifest synchronize */
                             NB_EnhancedContentDataType contentType,         /*!< Content type of removed manifest replies */
                             const char* datasetId,                          /*!< IDataset identifier of removed manifest replies */
                             CSL_Vector* removedManifestReplies,             /*!< Vector of NB_MetadataManifestReply. Should delete contained persistent files. */
                             void* userData                                  /*!< User data of NB_EnhancedContentDataSourceManifestSynchronizeCallback */
                             );

static void TimerCallbackForCancelingRequestData(PAL_Instance *pal,
                                                 void *userData,
                                                 PAL_TimerCBReason reason);
static void TimerCallbackForCancelingRequests(PAL_Instance *pal,
                                              void *userData,
                                              PAL_TimerCBReason reason);
static void TimerCallbackForCancelingAllRequests(PAL_Instance *pal,
                                                 void *userData,
                                                 PAL_TimerCBReason reason);
static void TimerCallbackForDownloadPriority(PAL_Instance *pal,
                                             void *userData,
                                             PAL_TimerCBReason reason);
static void TimerCallbackForPausingOrResumingDownload(PAL_Instance *pal,
                                                      void *userData,
                                                      PAL_TimerCBReason reason);


// Public Functions ..............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestDownload_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestDownloadCreate", &TestDownloadCreate);
    CU_add_test(pTestSuite, "TestDownloadSingleItem", &TestDownloadSingleItem);
    CU_add_test(pTestSuite, "TestDownloadCancelRequestData", &TestDownloadCancelRequestData);
    CU_add_test(pTestSuite, "TestDownloadCancelRequests", &TestDownloadCancelRequests);
    CU_add_test(pTestSuite, "TestDownloadCancelAllRequests", &TestDownloadCancelAllRequests);
    CU_add_test(pTestSuite, "TestDownloadPriority", &TestDownloadPriority);
    CU_add_test(pTestSuite, "TestDownloadCityModelAndPause", &TestDownloadCityModelAndPause);
    CU_add_test(pTestSuite, "TestDownloadMasterClear", &TestDownloadMasterClear);
};


/*! Add common initialization code here.

    @return 0

    @see TestNetwork_SuiteCleanup
*/
int
TestDownload_SuiteSetup()
{
    // :KLUDGE: 
    // For some reason we can't create the pal here. Pass in dummy-pal pointer for file 
    // function. This works since the pal pointer is not actually used in the file functions.
    PAL_Instance* dummyPal = (PAL_Instance*)0xAABBCCDD;
    const char* basePath = NULL;

    basePath = GetBasePath(dummyPal);

    if (basePath)
    {
        // Remove all test data
        PAL_FileRemoveDirectory(dummyPal, GetBasePath(), TRUE);
    }

    return 0;
}


/*! Add common cleanup code here.

    @return 0

    @see TestDownload_SuiteSetup
*/
int 
TestDownload_SuiteCleanup()
{
    // :KLUDGE: 
    // For some reason we can't create the pal here. Pass in dummy-pal pointer for file 
    // function. This works since the pal pointer is not actually used in the file functions.
    PAL_Instance* dummyPal = (PAL_Instance*)0xAABBCCDD;
    const char* basePath = NULL;

    basePath = GetBasePath(dummyPal);

    if (basePath)
    {
        // Remove all test data
        PAL_FileRemoveDirectory(dummyPal, GetBasePath(), TRUE);
    }

    return 0;
}


// Local Functions ...............................................................................

/*
    Frank Bielsik says:
    city model data are one by one
    Frank Bielsik says:
    like these...
    Frank Bielsik says:
    Connecting to http://content.qa1.navbuilder.nimlbs.net/USA/citymodel/101223/v1/USA-MHN/019_010.BIN
    Connecting to http://content.qa1.navbuilder.nimlbs.net/USA/citymodel/101223/v1/USA-MHN/019_011.BIN
    Connecting to http://content.qa1.navbuilder.nimlbs.net/USA/citymodel/101223/v1/USA-MHN/019_012.BIN
    Connecting to http://content.qa1.navbuilder.nimlbs.net/USA/citymodel/101223/v1/USA-MHN/019_013.BIN

    Frank Bielsik says:
    SARs/MJOs also one by one..
    Frank Bielsik says:
    the textures (per city, or common, or for MJO) are downloaded as ZIPs
    Frank Bielsik says:
    like these....
    Frank Bielsik says:
    Connecting to http://content.qa1.navbuilder.nimlbs.net/USA/texture/101223/USA-COMMON.zip
    Connecting to http://content.qa1.navbuilder.nimlbs.net/USA/texture/101223/USA-DC.zip
    Connecting to http://content.qa1.navbuilder.nimlbs.net/USA/texture/101223/USA-DET.zip
    Connecting to http://content.qa1.navbuilder.nimlbs.net/USA/texture/101223/USA-KC.zip

    SAR samples:

    http://content.qa1.navbuilder.nimlbs.net/USA/sign/110323/780x585/
        US_CA_28703_SIGN_R1.PNG
        US_CA_10018_SIGN_L1.PNG
        US_CA_30434_SIGN_R1.PNG
        US_CA_04102_SIGN_R.PNG
        US_CA_09680_SIGN_L1.PNG
        US_CA_90090_SIGN_L1.PNG
        US_CO_49620_SIGN_L1.PNG
        US_IL_25039_SIGN_R1R2.PNG
        US_IL_16057_SIGN_R.PNG
        US_CA_28703_SIGN_R1_P.PNG
        US_CA_04102_SIGN_R_P.PNG
        US_CA_28703_SIGN_R1_P.PNG
        US_CA_04102_SIGN_R_P.PNG
        US_IL_16057_SIGN_R_P.PNG
*/


/*! Test create/destroy.

    @return None, but CU-asserts are called on errors.
*/
void 
TestDownloadCreate(void)
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        DestroyInstances(&instances);
    }
}

/*! Test single item request.

    @return None, but CU-asserts are called on errors.
*/
void 
TestDownloadSingleItem(void)
{
    Instances instances = {0};
    const char* basePath = NULL;
    char fullPath[TEST_MAX_PATH_LENGTH];
    uint32 dataSize = 0;
    uint8* data = NULL;

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;

        NB_EnhancedContentIdentifier item = {NB_ECDT_RealisticSign, "USA-SAR", "US_CA_59022_SIGN_L1_P"};
        const NB_EnhancedContentIdentifier* itemArray = &item;

        // Request wrong download type. This should fail.
        result = NB_EnhancedContentDownloadManagerRequestData(instances.manager, itemArray, "AAAAAA", NB_ECDO_None);
        CU_ASSERT_EQUAL(result, NE_INVAL);

        // Request wrong download type. This should fail.
        result = NB_EnhancedContentDownloadManagerRequestData(instances.manager, itemArray, "AAAAAA", NB_ECDO_All);
        CU_ASSERT_EQUAL(result, NE_INVAL);

        // Request on-demand download
        result = NB_EnhancedContentDownloadManagerRequestData(instances.manager, itemArray, "AAAAAA", NB_ECD0_OnDemand);
        CU_ASSERT_EQUAL(result, NE_OK);

        instances.expectedCount = 1;

        CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_OnDemand));
        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_Preload));
        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECDO_All));

        if (! WaitForCallbackCompletedEvent(instances.event, 60000))
        {
            CU_FAIL("Callback event timed out!");
        }

        basePath = GetBasePath();
        CU_ASSERT_PTR_NOT_NULL(basePath);
        nsl_memset(fullPath, 0, sizeof(fullPath));
        nsl_strcpy(fullPath, basePath);
        nsl_strcat(fullPath, PATH_DELIMITER SAR_PATH PATH_DELIMITER "USA-SAR" PATH_DELIMITER "US_CA_59022_SIGN_L1_P.PNG");

        // Check if the file is there. This has to be modified the path changes
        CU_ASSERT_EQUAL(PAL_FileExists(instances.pal, fullPath), PAL_Ok);
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsDataItemAvailable(instances.manager, itemArray));
        result = NB_EnhancedContentDownloadManagerGetDataItem(instances.manager, itemArray, &dataSize, &data);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT(dataSize > 0 && data);
        nsl_free(data);

        // Test to remove the SAR item
        result = NB_EnhancedContentDownloadManagerRemoveData(instances.manager, itemArray, FALSE);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(PAL_FileExists(instances.pal, fullPath), PAL_ErrFileNotExist);
        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsDataItemAvailable(instances.manager, itemArray));
        result = NB_EnhancedContentDownloadManagerGetDataItem(instances.manager, itemArray, &dataSize, &data);
        CU_ASSERT_EQUAL(result, NE_NOENT);

        DestroyInstances(&instances);
    }
}

/*! Test canceling request data.

    @return None, but CU-asserts are called on errors.
*/
void
TestDownloadCancelRequestData(void)
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;

        NB_EnhancedContentIdentifier item = {NB_ECDT_CityModel, "USA-LA", NULL};
        NB_EnhancedContentIdentifier fileItem = {NB_ECDT_CityModel, "USA-LA", "000_000"};
        const NB_EnhancedContentIdentifier* itemArray = NULL;

        itemArray = &item;
        result = NB_EnhancedContentDownloadManagerRequestData(instances.manager, itemArray, NULL, NB_ECD0_Preload);
        CU_ASSERT_EQUAL(result, NE_OK);

        itemArray = &fileItem;
        result = NB_EnhancedContentDownloadManagerRequestData(instances.manager, itemArray, NULL, NB_ECD0_OnDemand);
        CU_ASSERT_EQUAL(result, NE_OK);
        result = NB_EnhancedContentDownloadManagerRequestData(instances.manager, itemArray, NULL, NB_ECD0_Preload);
        CU_ASSERT_EQUAL(result, NE_OK);

        instances.expectedCount = 63;

        // Set the timer for canceling requests.
        CU_ASSERT_EQUAL(PAL_TimerSet(instances.pal, 10000, TimerCallbackForCancelingRequestData, &instances), PAL_Ok);

        CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_OnDemand));
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_Preload));
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECDO_All));

        if (! WaitForCallbackCompletedEvent(instances.event, 1800000))
        {
            CU_FAIL("Callback event timed out!");
        }

        CU_ASSERT_EQUAL(PAL_TimerCancel(instances.pal, TimerCallbackForCancelingRequestData, &instances), PAL_Ok);

        itemArray = &item;
        result = NB_EnhancedContentDownloadManagerRemoveData(instances.manager, itemArray, FALSE);
        CU_ASSERT_EQUAL(result, NE_OK);

        DestroyInstances(&instances);
    }
}

/*! Test canceling requests.

    @return None, but CU-asserts are called on errors.
*/
void
TestDownloadCancelRequests(void)
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;

        NB_EnhancedContentIdentifier item = {NB_ECDT_CityModel, "USA-LA", NULL};
        const NB_EnhancedContentIdentifier* itemArray = &item;

        result = NB_EnhancedContentDownloadManagerRequestData(instances.manager, itemArray, NULL, NB_ECD0_Preload);
        CU_ASSERT_EQUAL(result, NE_OK);

        instances.expectedCount = 63;

        // Set the timer for canceling requests.
        CU_ASSERT_EQUAL(PAL_TimerSet(instances.pal, 30000, TimerCallbackForCancelingRequests, &instances), PAL_Ok);

        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_OnDemand));
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_Preload));
        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECDO_All));

        if (! WaitForCallbackCompletedEvent(instances.event, 1800000))
        {
            CU_FAIL("Callback event timed out!");
        }

        CU_ASSERT_EQUAL(PAL_TimerCancel(instances.pal, TimerCallbackForCancelingRequests, &instances), PAL_Ok);

        result = NB_EnhancedContentDownloadManagerRemoveData(instances.manager, itemArray, FALSE);
        CU_ASSERT_EQUAL(result, NE_OK);

        DestroyInstances(&instances);
    }
}

/*! Test canceling all requests.

    @return None, but CU-asserts are called on errors.
*/
void
TestDownloadCancelAllRequests(void)
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;

        NB_EnhancedContentIdentifier cityModelItem = {NB_ECDT_CityModel, "USA-LA", NULL};
        NB_EnhancedContentIdentifier cityTexturesItem = {NB_ECDT_SpecificTexture, "USA-LA", NULL};
        const NB_EnhancedContentIdentifier* itemArray = NULL;

        itemArray = &cityModelItem;
        result = NB_EnhancedContentDownloadManagerRequestData(instances.manager, itemArray, NULL, NB_ECD0_Preload);
        CU_ASSERT_EQUAL(result, NE_OK);

        itemArray = &cityTexturesItem;
        result = NB_EnhancedContentDownloadManagerRequestData(instances.manager, itemArray, NULL, NB_ECD0_OnDemand);
        CU_ASSERT_EQUAL(result, NE_OK);

        instances.expectedCount = 64;

        // Set the timer for canceling all requests.
        CU_ASSERT_EQUAL(PAL_TimerSet(instances.pal, 1000, TimerCallbackForCancelingAllRequests, &instances), PAL_Ok);

        CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_OnDemand));
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_Preload));
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECDO_All));

        if (! WaitForCallbackCompletedEvent(instances.event, 1800000))
        {
            CU_FAIL("Callback event timed out!");
        }

        CU_ASSERT_EQUAL(PAL_TimerCancel(instances.pal, TimerCallbackForCancelingAllRequests, &instances), PAL_Ok);

        DestroyInstances(&instances);
    }
}

/*! Test download priority (on demand > preload).

    Cannot stop current "preload" downloading just stop queued items sending new "preload" requests.

    @return None, but CU-asserts are called on errors.
*/
void
TestDownloadPriority(void)
{
    Instances instances = {0};
    const char* basePath = NULL;
    char fullPath[TEST_MAX_PATH_LENGTH];
    uint32 dataSize = 0;
    uint8* data = NULL;

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;
        NB_EnhancedContentIdentifier commonTexturesItem = {NB_ECDT_CommonTexture, "USA-COMMON", NULL};
        NB_EnhancedContentIdentifier cityModelItem = {NB_ECDT_CityModel, "USA-LA", NULL};
        NB_EnhancedContentIdentifier cityModelFileItem = {NB_ECDT_CityModel, "USA-LA", "000_000"};
        NB_EnhancedContentIdentifier cityTexturesItem = {NB_ECDT_SpecificTexture, "USA-LA", NULL};
        NB_EnhancedContentIdentifier cityTexturesFileItem = {NB_ECDT_SpecificTexture, "USA-LA", "010_FBD"};
        const NB_EnhancedContentIdentifier* itemArray = NULL;

        // Request a "preload" item couldn't stop if request is started.
        itemArray = &commonTexturesItem;
        result = NB_EnhancedContentDownloadManagerRequestData(instances.manager, itemArray, NULL, NB_ECD0_Preload);
        CU_ASSERT_EQUAL(result, NE_OK);

        // Set the timer for requesting items to test above item couldn't stop.
        CU_ASSERT_EQUAL(PAL_TimerSet(instances.pal, 3000, TimerCallbackForDownloadPriority, &instances), PAL_Ok);

        instances.expectedCount = 65;

        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_OnDemand));
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_Preload));
        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECDO_All));

        if (! WaitForCallbackCompletedEvent(instances.event, 1800000))
        {
            CU_FAIL("Callback event timed out!");
        }

        CU_ASSERT_EQUAL(PAL_TimerCancel(instances.pal, TimerCallbackForDownloadPriority, &instances), PAL_Ok);

        basePath = GetBasePath();
        CU_ASSERT_PTR_NOT_NULL(basePath);

        // Test with common textures
        itemArray = &commonTexturesItem;
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsDataItemAvailable(instances.manager, itemArray));
        result = NB_EnhancedContentDownloadManagerRemoveData(instances.manager, itemArray, FALSE);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsDataItemAvailable(instances.manager, itemArray));
        CU_ASSERT_EQUAL(PAL_FileExists(instances.pal, fullPath), PAL_ErrFileNotExist);

        // Test with city model
        nsl_memset(fullPath, 0, sizeof(fullPath));
        nsl_strcpy(fullPath, basePath);
        nsl_strcat(fullPath, PATH_DELIMITER CITYMODEL_PATH PATH_DELIMITER "USA-LA" PATH_DELIMITER "000_000.BIN");
        CU_ASSERT_EQUAL(PAL_FileExists(instances.pal, fullPath), PAL_Ok);

        itemArray = &cityModelFileItem;
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsDataItemAvailable(instances.manager, itemArray));
        result = NB_EnhancedContentDownloadManagerGetDataItem(instances.manager, itemArray, &dataSize, &data);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT(dataSize > 0 && data);
        nsl_free(data);

        itemArray = &cityModelItem;
        result = NB_EnhancedContentDownloadManagerRemoveData(instances.manager, itemArray, FALSE);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(PAL_FileExists(instances.pal, fullPath), PAL_ErrFileNotExist);

        itemArray = &cityModelFileItem;
        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsDataItemAvailable(instances.manager, itemArray));
        result = NB_EnhancedContentDownloadManagerGetDataItem(instances.manager, itemArray, &dataSize, &data);
        CU_ASSERT_EQUAL(result, NE_NOENT);


        // Test with city textures
        nsl_memset(fullPath, 0, sizeof(fullPath));
        nsl_strcpy(fullPath, basePath);
        nsl_strcat(fullPath, PATH_DELIMITER TEXTURE_PATH PATH_DELIMITER "USA-LA" PATH_DELIMITER "010_FBD.PNG");
        CU_ASSERT_EQUAL(PAL_FileExists(instances.pal, fullPath), PAL_Ok);

        itemArray = &cityTexturesFileItem;
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsDataItemAvailable(instances.manager, itemArray));
        result = NB_EnhancedContentDownloadManagerGetDataItem(instances.manager, itemArray, &dataSize, &data);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT(dataSize > 0 && data);
        if (data)
        {
            nsl_free(data);
        }

        itemArray = &cityTexturesItem;
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsDataItemAvailable(instances.manager, itemArray));
        result = NB_EnhancedContentDownloadManagerRemoveData(instances.manager, itemArray, FALSE);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsDataItemAvailable(instances.manager, itemArray));
        CU_ASSERT_EQUAL(PAL_FileExists(instances.pal, fullPath), PAL_ErrFileNotExist);

        itemArray = &cityTexturesFileItem;
        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsDataItemAvailable(instances.manager, itemArray));
        result = NB_EnhancedContentDownloadManagerGetDataItem(instances.manager, itemArray, &dataSize, &data);
        CU_ASSERT_EQUAL(result, NE_NOENT);

        DestroyInstances(&instances);
    }
}

/*! Test city model request and pause download.

    @return None, but CU-asserts are called on errors.
*/
void
TestDownloadCityModelAndPause(void)
{
    Instances instances = {0};
    const char* basePath = NULL;
    char fullPath[TEST_MAX_PATH_LENGTH];
    uint32 dataSize = 0;
    uint8* data = NULL;

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;

        NB_EnhancedContentIdentifier item = {NB_ECDT_CityModel, "USA-LA", NULL};
        NB_EnhancedContentIdentifier fileItem = {NB_ECDT_CityModel, "USA-LA", "000_000"};
        const NB_EnhancedContentIdentifier* itemArray = &item;

        // Request on-demand download
        result = NB_EnhancedContentDownloadManagerRequestData(instances.manager, itemArray, NULL, NB_ECD0_Preload);
        CU_ASSERT_EQUAL(result, NE_OK);

        instances.expectedCount = 63;

        // Set the timer for pausing or resuming download.
        CU_ASSERT_EQUAL(PAL_TimerSet(instances.pal, 10000, TimerCallbackForPausingOrResumingDownload, &instances), PAL_Ok);

        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_OnDemand));
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECD0_Preload));
        CU_ASSERT(!NB_EnhancedContentDownloadManagerIsPending(instances.manager, NB_ECDO_All));

        if (! WaitForCallbackCompletedEvent(instances.event, 1800000))
        {
            CU_FAIL("Callback event timed out!");
        }

        CU_ASSERT_EQUAL(PAL_TimerCancel(instances.pal, TimerCallbackForPausingOrResumingDownload, &instances), PAL_Ok);

        basePath = GetBasePath();
        CU_ASSERT_PTR_NOT_NULL(basePath);
        nsl_memset(fullPath, 0, sizeof(fullPath));
        nsl_strcpy(fullPath, basePath);
        nsl_strcat(fullPath, PATH_DELIMITER CITYMODEL_PATH PATH_DELIMITER "USA-LA" PATH_DELIMITER "000_000.BIN");
        CU_ASSERT_EQUAL(PAL_FileExists(instances.pal, fullPath), PAL_Ok);

        itemArray = &fileItem;
        CU_ASSERT(NB_EnhancedContentDownloadManagerIsDataItemAvailable(instances.manager, itemArray));
        result = NB_EnhancedContentDownloadManagerGetDataItem(instances.manager, itemArray, &dataSize, &data);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT(dataSize > 0 && data);
        nsl_free(data);

        // Don't delete this dataset for testing master clear.

        DestroyInstances(&instances);
    }
}

/*! Test download master clear.

    @return None, but CU-asserts are called on errors.
*/
void
TestDownloadMasterClear(void)
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;

        result = NB_EnhancedContentDownloadManagerMasterClear(instances.manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        DestroyInstances(&instances);
    }
}


// Helper functions ..................................................................................................

/*! Create PAL, context and download manager.

    Shared by all unit tests.

    @return TRUE on success, FALSE otherwise.
*/
nb_boolean 
CreateInstances(Instances* instances)
{
    NB_Error result = NE_OK;
    uint8 createResult = 0;
    const char* basePath = NULL;
    char fullPath[TEST_MAX_PATH_LENGTH];
    /*
        We only need the metadata callback for the download manager, but not the manifest callback.
    */
    NB_EnhancedContentDataSourceMetadataSynchronizeCallback metadataSynchronizeCallbackData = {0};
    NB_EnhancedContentDataSourceManifestSynchronizeCallback manifestSynchronizeCallbackData = {0};
    NB_EnhancedContentDownloadManagerCallbackData downloadCallbackData = {0};

    metadataSynchronizeCallbackData.callback = &MetadataCallback;
    metadataSynchronizeCallbackData.callbackData = instances;

    manifestSynchronizeCallbackData.callback = &ManifestCallback;
    manifestSynchronizeCallbackData.callbackData = instances;

    // Use the instances-pointer as the user data for our callback. That way we can trigger the event in the callback.
    downloadCallbackData.callback = &DownloadCallback;
    downloadCallbackData.userData = instances;

    nsl_memset(instances, 0, sizeof(*instances));

    // Fill configuration for download manager
    instances->configuration.screenWidth = TEST_SCREEN_WIDTH;
    instances->configuration.screenHeight = TEST_SCREEN_HEIGHT;
    instances->configuration.screenResolution = TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT;
    instances->configuration.maxRealisticSignsCacheSize     = 100;
    instances->configuration.maxMotorwayJunctionsCacheSize  = 100;

    instances->configuration.enhancedCityModelEnabled = TRUE;
    instances->configuration.motorwayJunctionObjectsEnabled = TRUE;
    instances->configuration.realisticSignsEnabled = TRUE;

    nsl_strcpy(instances->configuration.country, TEST_COUNTRY);
    nsl_strcpy(instances->configuration.language, TEST_LANGUAGE);
    nsl_strcpy(instances->configuration.nbgmVersion, TEST_NBGM_VERSION);

    basePath = GetBasePath();
    CU_ASSERT_PTR_NOT_NULL(basePath);
    nsl_strcpy(instances->configuration.persistentMetadataPath, basePath);

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strcpy(fullPath, basePath);
    nsl_strcat(fullPath, PATH_DELIMITER CITYMODEL_PATH);
    nsl_strcpy(instances->configuration.enhancedCityModelPath, fullPath);

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strcpy(fullPath, basePath);
    nsl_strcat(fullPath, PATH_DELIMITER MJO_PATH);
    nsl_strcpy(instances->configuration.motorwayJunctionObjectPath, fullPath);

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strcpy(fullPath, basePath);
    nsl_strcat(fullPath, PATH_DELIMITER TEXTURE_PATH);
    nsl_strcpy(instances->configuration.texturesPath, fullPath);

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strcpy(fullPath, basePath);
    nsl_strcat(fullPath, PATH_DELIMITER SAR_PATH);
    nsl_strcpy(instances->configuration.realisticSignsPath, fullPath);

    nsl_strcpy(instances->configuration.enhancedCityModelFileFormat, TEST_ECM_FILE_FORMAT);
    nsl_strcpy(instances->configuration.motorwayJunctionObjectFileFormat, TEST_MJO_FILE_FORMAT);
    nsl_strcpy(instances->configuration.realisticSignsFileFormat, TEST_SAR_FILE_FORMAT);
    nsl_strcpy(instances->configuration.textureFileFormat, TEST_TEXTURE_FILE_FORMAT);
    nsl_strcpy(instances->configuration.compressedTextureFileFormat, TEST_COMPRESSED_TEXTURE_FILE_FORMAT);
    
    nsl_strcpy(instances->configuration.clientGuid, "0123456789abcdef0123456789abcdef");

    // Create PAL instance and context
    createResult = CreatePalAndContext(&(instances->pal), &(instances->context));
    CU_ASSERT(createResult);
    CU_ASSERT_PTR_NOT_NULL(instances->pal);
    CU_ASSERT_PTR_NOT_NULL(instances->context);

    if (! createResult)
    {
        DestroyInstances(instances);
        return FALSE;
    }

    // Create data source first, this is usually owned by the content manager. For unit tests we just creata a stand alone object.
    result = NB_EnhancedContentDataSourceCreate(instances->context,
                                                &instances->configuration,
                                                metadataSynchronizeCallbackData,
                                                manifestSynchronizeCallbackData,
                                                &(instances->dataSource));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(instances->dataSource);

    if (result != NE_OK)
    {
        DestroyInstances(instances);
        return FALSE;
    }

    // Create download manager
    result = NB_EnhancedContentDownloadManagerCreate(instances->context, 
                                                     &instances->configuration, 
                                                     instances->dataSource, 
                                                     &downloadCallbackData, 
                                                     &(instances->manager));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(instances->manager);

    if (result != NE_OK)
    {
        DestroyInstances(instances);
        return FALSE;
    }

    instances->event = CreateCallbackCompletedEvent();
    CU_ASSERT_PTR_NOT_NULL(instances->event);

    // We need to synchronize the metadata in order to use the download manager
    result = NB_EnhancedContentDataSourceStartMetadataSynchronize(instances->dataSource);
    CU_ASSERT_EQUAL(result, NE_OK);

    // Wait for synchronize to finish
    if (! WaitForCallbackCompletedEvent(instances->event, 30000))
    {
        CU_FAIL("Callback event timed out!");
        DestroyInstances(instances);
        return FALSE;
    }

    result = NB_EnhancedContentDataSourceSelectCity(instances->dataSource, "USA-LA", TRUE);
    CU_ASSERT_EQUAL(result, NE_OK);

    // We need to synchronize the manifest in order to use the download manager
    result = NB_EnhancedContentDataSourceStartManifestSynchronize(instances->dataSource);
    if (result != NE_NOENT)
    {
        CU_ASSERT_EQUAL(result, NE_OK);

        // Wait for synchronize to finish
        if (! WaitForCallbackCompletedEvent(instances->event, 300000))
        {
            CU_FAIL("Callback event timed out!");
            DestroyInstances(instances);
            return FALSE;
        }
    }

    instances->expectedCount = 0;
    instances->downloadedCount = 0;

    return TRUE;
}

/*! Destroy PAL, context and download manager.

    Shared by all unit tests.

    @return None
*/
void 
DestroyInstances(Instances* instances)
{
    NB_Error result = NE_OK;
    
    if (instances->manager)
    {
        result = NB_EnhancedContentDownloadManagerDestroy(instances->manager);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    if (instances->dataSource)
    {
        result = NB_EnhancedContentDataSourceDestroy(instances->dataSource);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    if (instances->context)
    {
        result = NB_ContextDestroy(instances->context);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    if (instances->pal)
    {
        PAL_Destroy(instances->pal);
    }

    if (instances->event)
    {
        DestroyCallbackCompletedEvent(instances->event);
    }
}

/*! Callback for downloading.

    @see NB_EnhancedContentDownloadManagerCallback
*/
static void
DownloadCallback(NB_EnhancedContentDownloadManager* manager,
                 NB_Error result,
                 uint32 percent,
                 const NB_EnhancedContentIdentifier* item,
                 void* userData)
{
    Instances* instances = (Instances*)userData;

    CU_ASSERT_PTR_NOT_NULL(instances);
    CU_ASSERT_EQUAL(result, NE_OK);

    // Currently we only get called once the download completed. If we change the implementation then we have to change this here.
    CU_ASSERT_EQUAL(percent, 100);

    if (instances)
    {
        ++(instances->downloadedCount);

        if (instances->expectedCount == instances->downloadedCount)
        {
            // For now we only download one item. Trigger event once it's done
            SetCallbackCompletedEvent(instances->event);
        }
    }
}

/*! Callback for metadata.

    We need to get the metadata first in order for the URL and file-path for the downloaded items.

    @see NB_EnhancedContentDataSourceMetadataSynchronizeCallbackFunction
*/
void 
MetadataCallback(NB_EnhancedContentDataSource* dataSource,
                 NB_Error result,
                 NB_RequestStatus status,
                 uint32 percent,
                 CSL_Vector* deletedDatasetReplies,
                 CSL_Vector* addedCities,
                 CSL_Vector* updatedCities,
                 CSL_Vector* deletedCities,
                 void* userData)
{
    Instances* instances = (Instances*)userData;

    CU_ASSERT_PTR_NOT_NULL(instances);
    CU_ASSERT_EQUAL(result, NE_OK);

    //  We don't verify any data here. We just need it to complete so that we can download stuff.

    if (percent == 100)
    {
        if (instances)
        {
            // Metadata synchronized successfully
            SetCallbackCompletedEvent(instances->event);
        }
    }
}

/*! Callback for manifest.

    We need to get the manifest first in order for the URL and file-path for the downloaded items of city model.

    @see NB_EnhancedContentDataSourceManifestSynchronizeCallbackFunction
*/
void
ManifestCallback(
                 NB_EnhancedContentDataSource* dataSource,       /*!< Enhanced content data source instance */
                 NB_Error result,                                /*!< Result of manifest synchronize */
                 NB_RequestStatus status,                        /*!< Status of manifest synchronize */
                 NB_EnhancedContentDataType contentType,         /*!< Content type of removed manifest replies */
                 const char* datasetId,                          /*!< IDataset identifier of removed manifest replies */
                 CSL_Vector* removedManifestReplies,             /*!< Vector of NB_MetadataManifestReply. Should delete contained persistent files. */
                 void* userData                                  /*!< User data of NB_EnhancedContentDataSourceManifestSynchronizeCallback */
                 )
{
    Instances* instances = (Instances*)userData;

    CU_ASSERT_PTR_NOT_NULL(instances);
    CU_ASSERT_EQUAL(result, NE_OK);

    //  We don't verify any data here. We just need it to complete so that we can download stuff.

    if (instances)
    {
        // Metadata synchronized successfully
        SetCallbackCompletedEvent(instances->event);
    }
}

/*! Timer callback for canceling request data.

    @return None
*/
void
TimerCallbackForCancelingRequestData(PAL_Instance *pal,
                                     void *userData,
                                     PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    Instances* instances = (Instances*) userData;
    NB_EnhancedContentIdentifier item = {NB_ECDT_CityModel, "USA-LA", NULL};
    const NB_EnhancedContentIdentifier* itemArray = &item;

    if (reason == PTCBR_TimerCanceled || !instances)
    {
        return;
    }

    result = NB_EnhancedContentDownloadManagerCancelRequestData(instances->manager, NB_ECDO_All, itemArray);
    CU_ASSERT(!NB_EnhancedContentDownloadManagerIsPaused(instances->manager, NB_ECDO_All) &&
              !NB_EnhancedContentDownloadManagerIsPending(instances->manager, NB_ECDO_All));

    // Cancel requests successfully
    SetCallbackCompletedEvent(instances->event);
}

/*! Timer callback for canceling requests.

    @return None
*/
void
TimerCallbackForCancelingRequests(PAL_Instance *pal,
                                  void *userData,
                                  PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    Instances* instances = (Instances*) userData;

    if(!instances)
    {
        return;
    }

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    result = NB_EnhancedContentDownloadManagerCancelRequests(instances->manager, NB_ECD0_Preload, NB_ECDT_CityModel);
    CU_ASSERT(!NB_EnhancedContentDownloadManagerIsPaused(instances->manager, NB_ECD0_Preload) &&
              !NB_EnhancedContentDownloadManagerIsPending(instances->manager, NB_ECD0_Preload));

    if (instances)
    {
        // Cancel requests successfully
        SetCallbackCompletedEvent(instances->event);
    }
}

/*! Timer callback for canceling all request.

    @return None
*/
void
TimerCallbackForCancelingAllRequests(PAL_Instance *pal,
                                     void *userData,
                                     PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    Instances* instances = (Instances*) userData;

    if (reason == PTCBR_TimerCanceled || !instances)
    {
        return;
    }

    result = NB_EnhancedContentDownloadManagerCancelRequests(instances->manager, NB_ECD0_Preload, NB_ECDT_All);
    CU_ASSERT(!NB_EnhancedContentDownloadManagerIsPaused(instances->manager, NB_ECDO_All) &&
              !NB_EnhancedContentDownloadManagerIsPending(instances->manager, NB_ECDO_All));

    // Cancel all requests successfully
    SetCallbackCompletedEvent(instances->event);
}

/*! Timer callback for download priority.

    @return None
*/
void
TimerCallbackForDownloadPriority(PAL_Instance *pal,
                                 void *userData,
                                 PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    Instances* instances = (Instances*) userData;
    NB_EnhancedContentIdentifier cityModelItem = {NB_ECDT_CityModel, "USA-LA", NULL};
    NB_EnhancedContentIdentifier cityTexturesItem = {NB_ECDT_SpecificTexture, "USA-LA", NULL};
    const NB_EnhancedContentIdentifier* itemArray = NULL;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    // Request "on demand" download
    itemArray = &cityModelItem;
    result = NB_EnhancedContentDownloadManagerRequestData(instances->manager, itemArray, NULL, NB_ECD0_OnDemand);
    CU_ASSERT_EQUAL(result, NE_OK);

    // Request "preload" download suspended by above "on demand" request
    itemArray = &cityTexturesItem;
    result = NB_EnhancedContentDownloadManagerRequestData(instances->manager, itemArray, NULL, NB_ECD0_Preload);
    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances->manager, NB_ECD0_OnDemand));
    CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances->manager, NB_ECD0_Preload));
    CU_ASSERT(NB_EnhancedContentDownloadManagerIsPending(instances->manager, NB_ECDO_All));    
}

/*! Timer callback for pausing or resuming download.

    @return None
*/
void
TimerCallbackForPausingOrResumingDownload(PAL_Instance *pal,
                                          void *userData,
                                          PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    static uint32 downloadedCount = 0;
    Instances* instances = (Instances*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    if (NB_EnhancedContentDownloadManagerIsPaused(instances->manager, NB_ECDO_All))
    {
        result = NB_EnhancedContentDownloadManagerResumeDownload(instances->manager, NB_ECDO_All);
        CU_ASSERT_EQUAL(result, NE_OK);
    }
    else if (downloadedCount != instances->downloadedCount)
    {
        downloadedCount = instances->downloadedCount;

        result = NB_EnhancedContentDownloadManagerPauseDownload(instances->manager, NB_ECDO_All);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    CU_ASSERT_EQUAL(PAL_TimerSet(instances->pal, 10000, TimerCallbackForPausingOrResumingDownload, instances), PAL_Ok);
}

/*! @} */

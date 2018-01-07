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

*/
/*
    See header

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

#include "testcontentmanager.h"
#include "main.h"
#include "platformutil.h"
#include "networkutil.h"
#include "palfile.h"
#include "paltimer.h"
#include "nbenhancedcontentmanagerprotected.h"
#include "nbenhancedcontentdownloadmanager.h"

#define _DEBUG_LOGGING_ENABLED_
#include "paldebuglog.h"


// Local Constants ...............................................................................

static const char* OUTPUT_FILE           = "TestContentDownload_%d.png";
static int         saved_index           = 0;

// Local Types ...................................................................................

typedef enum _TestType{
    typePlain = 0,        /*!< Plain test. */
    typeErrorHandling,    /*!< Test for error handling. */
    typeSpeedLimitOnly,
    typeDownloadQueues,
} TestType;


/*! Collection of all instances used for the unit tests */
typedef struct
{
    PAL_Instance* pal;
    NB_Context* context;

    NB_EnhancedContentManager* manager;

    NB_EnhancedContentManagerConfiguration configuration;

    TestType type;

    void* event;

} Instances;


// Local Functions ...............................................................................

// Test functions
// Only test protected functions in callback
static void TestManagerStartMetadataSynchronization(void);
static void TestManagerStartManifestSynchronization(void);
static void TestManagerRequestAndClearRequests(void);
static void TestManagerGetDataItem(void);
static void TestManagerCheckManifestVersion(void);
static void TestManagerCheckFileVersion(void);
static void TestManagerErrorHandler(void);
static void TestManagerDownloadQueues(void);
static void TestManagerGetSpeedLimitOnly(void);
// Helper functions

static nb_boolean CreateInstances(Instances* instances);
static void DestroyInstances(Instances* instances);

static void SetupEnhancedContentManagerConfiguration(NB_EnhancedContentManagerConfiguration* config);

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

static void TimerCallbackForCancelingRequests(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);


// Public Functions ..............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestContentManager_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestManagerStartMetadataSynchronization", &TestManagerStartMetadataSynchronization);
    CU_add_test(pTestSuite, "TestManagerStartManifestSynchronization", &TestManagerStartManifestSynchronization);
    CU_add_test(pTestSuite, "TestManagerCheckManifestVersion", &TestManagerCheckManifestVersion);
    CU_add_test(pTestSuite, "TestManagerGetDataItem", &TestManagerGetDataItem);
    CU_add_test(pTestSuite, "TestManagerRequestAndClearRequests", &TestManagerRequestAndClearRequests);
    CU_add_test(pTestSuite, "TestManagerGetSpeedLimitOnly", &TestManagerGetSpeedLimitOnly);
    CU_add_test(pTestSuite, "TestManagerCheckFileVersion", &TestManagerCheckFileVersion);
    CU_add_test(pTestSuite, "TestManagerErrorHandler", &TestManagerErrorHandler);
    CU_add_test(pTestSuite, "TestManagerDownloadQueues", &TestManagerDownloadQueues);
};


/*! Add common initialization code here.

    @return 0
*/
int
TestContentManager_SuiteSetup()
{
    // // :KLUDGE:
    // // For some reason we can't create the pal here. Pass in dummy-pal pointer for file
    // // function. This works since the pal pointer is not actually used in the file functions.
    // PAL_Instance* dummyPal = (PAL_Instance*)0xAABBCCDD;
    // const char* basePath = NULL;

    // basePath = GetBasePath(dummyPal);

    // if (basePath)
    // {
    //     // Remove all test data
    //     PAL_FileRemoveDirectory(dummyPal, GetBasePath(), TRUE);
    // }

    // // test
    // TEST_LOGGING("TEST %s", "test");

    return 0;
}


/*! Add common cleanup code here.

    @return 0
*/
int
TestContentManager_SuiteCleanup()
{
    // // :KLUDGE:
    // // For some reason we can't create the pal here. Pass in dummy-pal pointer for file
    // // function. This works since the pal pointer is not actually used in the file functions.
    // PAL_Instance* dummyPal = (PAL_Instance*)0xAABBCCDD;
    // const char* basePath = NULL;

    // basePath = GetBasePath(dummyPal);

    // if (basePath)
    // {
    //     // Remove all test data
    //     PAL_FileRemoveDirectory(dummyPal, GetBasePath(), TRUE);
    // }

    return 0;
}


// Local Functions ...............................................................................

/*! Write data to a file.

    @return NB_Error
*/
NB_Error SaveToFile(PAL_Instance* pal,  /*!<  pal instance. */
                    void* data,         /*!< Data to be dumped. */
                    uint32 length       /*!< Length of data */
                    )
{

    PAL_Error palError = PAL_Ok;

    PAL_File* file = NULL;

    char filename[256];
    nsl_memset(filename, 0, 256);
    SPRINTF(filename, OUTPUT_FILE, ++saved_index);

    PAL_FileRemove(pal, filename);

    palError = PAL_FileOpen(pal, filename, PFM_Create, &file);

    CU_ASSERT_EQUAL(palError, PAL_Ok);
    if (palError != PAL_Ok)
    {
        return NE_FSYS;
    }

    if (length > 0)
    {
        uint32 bytesWritten = 0;
        uint32 position = 0;

        do {
            palError = PAL_FileWrite(file, (uint8*)data, length, &bytesWritten);
            CU_ASSERT_EQUAL(palError, PAL_Ok);
            if (palError != PAL_Ok)
            {
                return NE_FSYS;
            }

            position += bytesWritten;

            palError = PAL_FileSetPosition(file, PFSO_Start, position);
            CU_ASSERT_EQUAL(palError, PAL_Ok);
            if (palError != PAL_Ok)
            {
                return NE_FSYS;
            }
        } while (bytesWritten < length);
    }

    palError = PAL_FileClose(file);
    CU_ASSERT_EQUAL(palError, PAL_Ok);
    if (palError != PAL_Ok)
    {
        return NE_FSYS;
    }

    return NE_OK;
}

/*! Test protected functions in metadata synchronization callback

    @return None. CUnit Asserts get called on failures.
*/
void
TestManagerStartMetadataSynchronization(void)
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;

        result = NB_EnhancedContentManagerStartMetadataSynchronization(instances.manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            if (WaitForCallbackCompletedEvent(instances.event, 60000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }
        }

        DestroyInstances(&instances);
    }
}

/*! Test protected functions in manifest synchronization callback

    @return None. CUnit Asserts get called on failures.
*/
void
TestManagerStartManifestSynchronization(void)
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;

        // Test selecting cities
        result = NB_EnhancedContentManagerSelectCity(instances.manager, "USA-LA", TRUE);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_EnhancedContentManagerStartManifestSynchronization(instances.manager);
        CU_ASSERT((result == NE_OK) || (result == NE_NOENT));
        if (result == NE_OK)
        {
            if (WaitForCallbackCompletedEvent(instances.event, 3600000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }
        }

        CU_ASSERT(NB_EnhancedContentManagerIsCityAvailable(instances.manager, "USA-LA"));

        DestroyInstances(&instances);
    }
}

/*! Test requesting and clearing requests

    @return None. CUnit Asserts get called on failures.
*/
void
TestManagerRequestAndClearRequests(void)
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;

        result = NB_EnhancedContentManagerRequestDataDownload(instances.manager, NB_ECDT_SpecificTexture, "USA-LA", NULL, NULL);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            // Set the timer for canceling requests.
            CU_ASSERT_EQUAL(PAL_TimerSet(instances.pal, 30000, TimerCallbackForCancelingRequests, &instances), PAL_Ok);

            if (WaitForCallbackCompletedEvent(instances.event, 1800000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }
        }

        CU_ASSERT_EQUAL(PAL_TimerCancel(instances.pal, TimerCallbackForCancelingRequests, &instances), PAL_Ok);

        DestroyInstances(&instances);
    }
}

/*! Test getting data item

    @return None. CUnit Asserts get called on failures.
*/
void
TestManagerGetDataItem(void)
{
    Instances instances           = {0};
    uint32    dataSize            = 0;
    uint8*    dataSAR             = NULL;
    uint8*    dataSpeedLimitsSign = NULL;

    if (CreateInstances(&instances))
    {
        NB_Error result1 = NE_OK;
        NB_Error result2 = NE_OK;
        uint8*    dataCityModel       = NULL;

        // // City model should have been downloaded already.
        result1 = NB_EnhancedContentManagerGetDataItem(instances.manager, NB_ECDT_CityModel,
                                                       "USA-LA", "000_000", NULL, FALSE,
                                                       &dataSize, &dataCityModel);
        CU_ASSERT_EQUAL(result1, NE_OK);
        CU_ASSERT(dataSize > 0 && dataCityModel);
        nsl_free(dataCityModel);

        // Should not check following values: if the interested data is in Cache, it will return
        // with result = OK.
        result1 = NB_EnhancedContentManagerGetDataItem(instances.manager, NB_ECDT_RealisticSign,
                                                       "USA-SAR", "US_CA_59022_SIGN_L1_P",
                                                       "AAAAAA", TRUE, &dataSize, &dataSAR);
        if (result1 == NE_OK)
        {
            CU_ASSERT(dataSize > 0 && dataSAR);
            nsl_free(dataSAR);
        }
        else if (result1 == NE_NOENT)
        {
            CU_ASSERT(dataSize == 0 && ! dataSAR);
        }
        else
        {
            CU_FAIL("Incorrect return value.");
        }

        // 25 is a $id from server, if it does not work anymore, get a new one from server.
        result2 = NB_EnhancedContentManagerGetDataItem(instances.manager,
                                                       NB_ECDT_SpeedLimitsSign,
                                                       "USA-SPEED",
                                                       "25",
                                                       "AAAAAA", TRUE, &dataSize,
                                                       &dataSpeedLimitsSign);
        if (result2 == NE_OK)
        {
            CU_ASSERT(dataSize > 0 && dataSpeedLimitsSign);
            nsl_free(dataSpeedLimitsSign);
        }
        else if (result2 == NE_NOENT)
        {
            CU_ASSERT(dataSize == 0 && ! dataSpeedLimitsSign);
        }
        else
        {
            CU_FAIL("Incorrect return value.");
        }

        if (result1 == NE_NOENT || result2 == NE_NOENT)
        {
            if (WaitForCallbackCompletedEvent(instances.event, 240000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerGetDataItem() failed");
            }
        }

        // Test if realistic sign is available.
        CU_ASSERT(NB_EnhancedContentManagerIsDataAvailable(instances.manager,
                                                           NB_ECDT_RealisticSign,
                                                           "USA-SAR", "US_CA_59022_SIGN_L1_P"));

        dataSize = 0;
        result1 = NB_EnhancedContentManagerGetDataItem(instances.manager, NB_ECDT_RealisticSign,
                                                       "USA-SAR", "US_CA_59022_SIGN_L1_P",
                                                       "AAAAAA", TRUE, &dataSize, &dataSAR);

        CU_ASSERT_EQUAL(result1, NE_OK);
        CU_ASSERT(dataSize != 0 && dataSAR != NULL);
        SaveToFile(instances.pal, dataSAR, dataSize);
        nsl_free(dataSAR);

        // Test if Speed limits sign is available.
        CU_ASSERT(NB_EnhancedContentManagerIsDataAvailable(instances.manager,
                                                           NB_ECDT_SpeedLimitsSign,
                                                           "USA-SPEED", "25"));
        dataSize = 0;
        result2 = NB_EnhancedContentManagerGetDataItem(instances.manager,
                                                      NB_ECDT_SpeedLimitsSign,
                                                      "USA-SPEED",
                                                      "25",
                                                      "AAAAAA", TRUE, &dataSize,
                                                       &dataSpeedLimitsSign);

        CU_ASSERT_EQUAL(result2, NE_OK);

        SaveToFile(instances.pal, dataSpeedLimitsSign, dataSize);
        nsl_free(dataSpeedLimitsSign);

        DestroyInstances(&instances);
    }
}

/*! Test checking manifest version

    @return None. CUnit Asserts get called on failures.
*/
void
TestManagerCheckManifestVersion(void)
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;

        CU_ASSERT(NB_EnhancedContentManagerIsDataAvailable(instances.manager, NB_ECDT_CityModel, "USA-LA", NULL));

        /* Check the manifest version with new version. If manifest versions are not same,
           do not remove data of this dataset. Wait for manifest synchronization return,
           update each manifest by file version. */
        result = NB_EnhancedContentManagerCheckManifestVersion(instances.manager, NB_ECDT_CityModel, "USA-LA", "AAAAAA");
        CU_ASSERT_EQUAL(result, NE_NOENT);

        // Check the manifest version with new version again
        result = NB_EnhancedContentManagerCheckManifestVersion(instances.manager, NB_ECDT_CityModel, "USA-LA", "AAAAAA");
        CU_ASSERT_EQUAL(result, NE_NOENT);

        // Start metadata synchronization
        result = NB_EnhancedContentManagerStartMetadataSynchronization(instances.manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            if (WaitForCallbackCompletedEvent(instances.event, 60000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }
        }

        // Start manifest synchronization
        result = NB_EnhancedContentManagerStartManifestSynchronization(instances.manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            if (WaitForCallbackCompletedEvent(instances.event, 3600000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }
        }

        CU_ASSERT(NB_EnhancedContentManagerIsCityAvailable(instances.manager, "USA-LA"));

        DestroyInstances(&instances);
    }
}

/*! Test checking file version

    @return None. CUnit Asserts get called on failures.
*/
void
TestManagerCheckFileVersion(void)
{
    Instances instances = {0};
    uint32 dataSize = 0;
    uint8* data = NULL;

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;

        CU_ASSERT(NB_EnhancedContentManagerIsDataAvailable(instances.manager,
                                                           NB_ECDT_RealisticSign, "USA-SAR",
                                                           "US_CA_59022_SIGN_L1_P"));

        // Check the file version with latest
        result = NB_EnhancedContentManagerCheckFileVersion(instances.manager,
                                                           NB_ECDT_RealisticSign, "USA-SAR",
                                                           "US_CA_59022_SIGN_L1_P", "AAAAAA");
        CU_ASSERT_EQUAL(result, NE_OK);

        CU_ASSERT(NB_EnhancedContentManagerIsDataAvailable(instances.manager,
                                                           NB_ECDT_RealisticSign, "USA-SAR",
                                                           "US_CA_59022_SIGN_L1_P"));

        // Check the file version with new version
        result = NB_EnhancedContentManagerCheckFileVersion(instances.manager,
                                                           NB_ECDT_RealisticSign, "USA-SAR",
                                                           "US_CA_59022_SIGN_L1_P", "BBBBBB");
        CU_ASSERT_EQUAL(result, NE_NOENT);

        CU_ASSERT(!NB_EnhancedContentManagerIsDataAvailable(instances.manager,
                                                            NB_ECDT_RealisticSign, "USA-SAR",
                                                            "US_CA_59022_SIGN_L1_P"));

        result = NB_EnhancedContentManagerGetDataItem(instances.manager, NB_ECDT_RealisticSign,
                                                      "USA-SAR", "US_CA_59022_SIGN_L1_P",
                                                      "CCCCCC", TRUE, &dataSize, &data);
        CU_ASSERT_EQUAL(result, NE_NOENT);
        CU_ASSERT(dataSize == 0 && !data);

        // Check the file version with latest
        result = NB_EnhancedContentManagerCheckFileVersion(instances.manager,
                                                           NB_ECDT_SpeedLimitsSign, "USA-SPEED",
                                                           "25", "AAAAAA");
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT(NB_EnhancedContentManagerIsDataAvailable(instances.manager,
                                                           NB_ECDT_SpeedLimitsSign, "USA-SPEED",
                                                           "25"));

        // Check the file version with new version
        result = NB_EnhancedContentManagerCheckFileVersion(instances.manager,
                                                           NB_ECDT_SpeedLimitsSign, "USA-SPEED",
                                                           "25", "BBBBBB");
        CU_ASSERT_EQUAL(result, NE_NOENT);

        result = NB_EnhancedContentManagerGetDataItem(instances.manager, NB_ECDT_SpeedLimitsSign,
                                                      "USA-SPEED", "25", "CCCCCC", TRUE,
                                                      &dataSize, &data);
        CU_ASSERT_EQUAL(result, NE_NOENT);
        CU_ASSERT(dataSize == 0 && !data);


        if (result == NE_NOENT)
        {
            if (WaitForCallbackCompletedEvent(instances.event, 120000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }
        }

        // // Test if realistic sign is available
        CU_ASSERT(NB_EnhancedContentManagerIsDataAvailable(instances.manager, NB_ECDT_RealisticSign, "USA-SAR", "US_CA_59022_SIGN_L1_P"));

        DestroyInstances(&instances);
    }
}

/*! Special test to the Error-Handling mechanism.

    @return None.
*/
void  TestManagerErrorHandler()
{
    Instances instances = {0};
    uint32    dataSize  = 0;
    uint8*    data      = NULL;
    NB_Error  result    = NE_OK;

    if (CreateInstances(&instances))
    {
        instances.type = typeErrorHandling;

        result = NB_EnhancedContentManagerGetDataItem(instances.manager,
                                                      NB_ECDT_SpeedLimitsSign,
                                                      "USA-SPEED",
                                                      "AnUniqeIncrroctSignId",
                                                      "AAAAAA", TRUE, &dataSize,
                                                      &data);

        if (result == NE_NOENT)
        {
            WaitForCallbackCompletedEvent(instances.event, 240000);
        }

        DestroyInstances(&instances);
    }
}

// We need to test this.
void TestManagerGetSpeedLimitOnly()
{
    Instances instances = {0};
    uint32    dataSize  = 0;
    uint8*    data      = NULL;
    NB_Error  result    = NE_OK;

    if (CreateInstances(&instances))
    {
        instances.type = typeSpeedLimitOnly;

        // 25 is a $id from server, if it does not work anymore, get a new one from server.
        result = NB_EnhancedContentManagerGetDataItem(instances.manager,
                                                      NB_ECDT_SpeedLimitsSign,
                                                      "USA-SPEED",
                                                      "25",
                                                      "AAAAAA", TRUE, &dataSize,
                                                      &data);
        if (result == NE_OK)
        {
            CU_ASSERT(dataSize > 0 && data);
            nsl_free(data);
        }
        else if (result == NE_NOENT)
        {
            CU_ASSERT(dataSize == 0 && ! data);
        }
        else
        {
            CU_FAIL("Incorrect return value.");
        }

        if (result == NE_NOENT)
        {
            if (WaitForCallbackCompletedEvent(instances.event, 240000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerGetDataItem() failed");
            }
            CU_ASSERT(NB_EnhancedContentManagerIsDataAvailable(instances.manager,
                                                               NB_ECDT_SpeedLimitsSign,
                                                               "USA-SPEED", "25"));
            dataSize = 0;
            result = NB_EnhancedContentManagerGetDataItem(instances.manager,
                                                           NB_ECDT_SpeedLimitsSign,
                                                           "USA-SPEED",
                                                           "25",
                                                           "AAAAAA", TRUE, &dataSize,
                                                           &data);

            CU_ASSERT_EQUAL(result, NE_OK);

            SaveToFile(instances.pal, data, dataSize);
            nsl_free(data);
        }

        DestroyInstances(&instances);
    }
}

void TestManagerDownloadQueues()
{
    Instances instances = {0};
    NB_Error  result    = NE_OK;

    printf("hello");
    if (CreateInstances(&instances))
    {
        uint32 i = 0;
        nb_boolean enabled = FALSE;
        instances.type = typeDownloadQueues;

        // Test disable or enable download queues.

        for (i = 0; i < (uint32)NB_ECDL_All; i++)  // Check flag
        {
            enabled = NB_EnhancedContentManagerIsDownloadLevelEnabled(instances.manager,
                                                                      NB_ECD0_OnDemand,
                                                                      (NB_EnhancedContentDownloadLevel)i);
            CU_ASSERT_EQUAL(enabled, TRUE);

            enabled = NB_EnhancedContentManagerIsDownloadLevelEnabled(instances.manager,
                                                                      NB_ECD0_Preload,
                                                                      (NB_EnhancedContentDownloadLevel)i);
            CU_ASSERT_EQUAL(enabled, TRUE);
        }

        for (i = 0; i < (uint32)NB_ECDL_All; i++)  // Disable
        {
            result = NE_INVAL;
            result = NB_EnhancedContentManagerDisableDownloadLevel(instances.manager,
                                                                   NB_ECD0_OnDemand,
                                                                   (NB_EnhancedContentDownloadLevel)i);
            CU_ASSERT_EQUAL(result, NE_OK);
            result = NB_EnhancedContentManagerDisableDownloadLevel(instances.manager,
                                                                   NB_ECD0_Preload,
                                                                   (NB_EnhancedContentDownloadLevel)i);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        for (i = 0; i < (uint32)NB_ECDL_All; i++)  // Check flag
        {
            enabled = TRUE;
            enabled = NB_EnhancedContentManagerIsDownloadLevelEnabled(instances.manager,
                                                                      NB_ECD0_OnDemand,
                                                                      (NB_EnhancedContentDownloadLevel)i);
            CU_ASSERT_EQUAL(enabled, FALSE);

            enabled = NB_EnhancedContentManagerIsDownloadLevelEnabled(instances.manager,
                                                                      NB_ECD0_Preload,
                                                                      (NB_EnhancedContentDownloadLevel)i);
            CU_ASSERT_EQUAL(enabled, FALSE);
        }

        for (i = 0; i < (uint32)NB_ECDL_All; i++)  // enable
        {
            result = NE_INVAL;
            result = NB_EnhancedContentManagerEnableDownloadLevel(instances.manager,
                                                                  NB_ECD0_OnDemand,
                                                                  (NB_EnhancedContentDownloadLevel)i);
            CU_ASSERT_EQUAL(result, NE_OK);
            result = NB_EnhancedContentManagerEnableDownloadLevel(instances.manager,
                                                                  NB_ECD0_Preload,
                                                                  (NB_EnhancedContentDownloadLevel)i);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        for (i = 0; i < (uint32)NB_ECDL_All; i++)  // Check flag
        {
            enabled = FALSE;
            enabled = NB_EnhancedContentManagerIsDownloadLevelEnabled(instances.manager,
                                                                      NB_ECD0_OnDemand,
                                                                      (NB_EnhancedContentDownloadLevel)i);
            CU_ASSERT_EQUAL(enabled, TRUE);

            enabled = NB_EnhancedContentManagerIsDownloadLevelEnabled(instances.manager,
                                                                      NB_ECD0_Preload,
                                                                      (NB_EnhancedContentDownloadLevel)i);
            CU_ASSERT_EQUAL(enabled, TRUE);
        }


        // // @todo
        // // Test to add requests of different type, then requests are processed based on level of
        // // queue. It works as designed, but difficult to test under CU.
        // // Uncomment following codes if you want to test them and debug.

        // {
        //     uint32    dataSize  = 0;
        //     uint8*    data      = NULL;

        //     NB_EnhancedContentManagerGetDataItem(instances.manager,
        //                                          NB_ECDT_CommonTexture,
        //                                          "USA-SAR",
        //                                          "25",
        //                                          "AAAAAA", TRUE, &dataSize,
        //                                          &data);

        //     NB_EnhancedContentManagerGetDataItem(instances.manager,
        //                                          NB_ECDT_CityModel,
        //                                          "USA-LA", "000_000",
        //                                          "AAAAAA", TRUE, &dataSize,
        //                                          &data);

        //     NB_EnhancedContentManagerGetDataItem(instances.manager,
        //                                          NB_ECDT_CommonTexture,
        //                                          "TestSetID2",
        //                                          "25",
        //                                          "AAAAAA", TRUE, &dataSize,
        //                                          &data);
        //     result = NB_EnhancedContentManagerDisableDownloadLevel(instances.manager,
        //                                                            NB_ECD0_OnDemand,
        //                                                            NB_ECDL_Middle);
        //     CU_ASSERT_EQUAL(result, NE_OK);

        //     result = NB_EnhancedContentManagerGetDataItem(instances.manager,
        //                                                   NB_ECDT_SpeedLimitsSign,
        //                                                   "USA-SPEED",
        //                                                   "25",
        //                                                   "AAAAAA", TRUE, &dataSize,
        //                                                   &data);

        //     if (data) // This "$signid" may have been download before.
        //     {
        //         nsl_free(data);
        //     }

        //     NB_EnhancedContentManagerGetDataItem(instances.manager,
        //                                          NB_ECDT_RealisticSign,
        //                                          "TestSetID2",
        //                                          "25",
        //                                          "AAAAAA", TRUE, &dataSize,
        //                                          &data);

        //     NB_EnhancedContentManagerGetDataItem(instances.manager,
        //                                          NB_ECDT_RealisticSign,
        //                                          "TestSetID2",
        //                                          "25f",
        //                                          "AAAAAA", TRUE, &dataSize,
        //                                          &data);

        //     result = NB_EnhancedContentManagerGetDataItem(instances.manager,
        //                                                   NB_ECDT_SpeedLimitsSign,
        //                                                   "USA-SPEED",
        //                                                   "25-h",
        //                                                   "AAAAAA", TRUE, &dataSize,
        //                                                   &data);

        //     if (result == NE_OK)
        //     {
        //         CU_ASSERT(dataSize > 0 && data);
        //         nsl_free(data);
        //     }
        //     else if (result == NE_NOENT)
        //     {
        //         CU_ASSERT(dataSize == 0 && ! data);
        //     }
        //     else
        //     {
        //         CU_FAIL("Incorrect return value.");
        //     }


        //     if (result == NE_NOENT)
        //     {
        //         WaitForCallbackCompletedEvent(instances.event, 240000);
        //     }
        // }

        DestroyInstances(&instances);
    }
}


// Helper functions ..................................................................................................

/*! Create PAL, context and content manager.

    Shared by all unit tests.

    @return TRUE on success, FALSE otherwise.
*/
nb_boolean
CreateInstances(Instances* instances)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentSynchronizationCallback synchronizationCallback = { ContentSynchronizationCallback, NULL };
    NB_EnhancedContentDownloadCallback downloadCallback = { ContentDownloadCallback, NULL };
    NB_EnhancedContentManagerConfiguration config = { 0 };

    synchronizationCallback.callbackData = instances;
    downloadCallback.callbackData = instances;

    if (!CreatePalAndContext(&instances->pal, &instances->context))
    {
        DestroyInstances(instances);
        return FALSE;
    }

    CU_ASSERT_PTR_NOT_NULL(instances->pal);
    CU_ASSERT_PTR_NOT_NULL(instances->context);

    instances->event = CreateCallbackCompletedEvent();
    if (!instances->event)
    {
        DestroyInstances(instances);
        return FALSE;
    }

    CU_ASSERT_PTR_NOT_NULL(instances->event);

    instances->type = typePlain;

    SetupEnhancedContentManagerConfiguration(&config);

    result = NB_EnhancedContentManagerCreate(instances->context, &config, &synchronizationCallback, &downloadCallback, &instances->manager);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(instances->manager);

    if (result != NE_OK)
    {
        DestroyInstances(instances);
        return FALSE;
    }

    return TRUE;
}

/*! Destroy PAL, context and content manager.

    Shared by all unit tests.

    @return None
*/
void
DestroyInstances(Instances* instances)
{
    NB_Error result = NE_OK;

    if (instances->manager)
    {
        result = NB_EnhancedContentManagerDestroy(instances->manager);
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

    // Enable following flags when we can synchronize metadata from server.
    // At present, I'm using fake database to test SpeedLimitsSign, so other flags are disabled.
    config->enhancedCityModelEnabled = TRUE;
    config->motorwayJunctionObjectsEnabled = TRUE;
    config->realisticSignsEnabled = TRUE;
    config->speedLimitsSignEnabled = TRUE;

    config->supportedTypes = NB_ECDT_MAP_EXTENDED;

    nsl_strcpy(config->country, TEST_COUNTRY);
    nsl_strcpy(config->language, TEST_LANGUAGE);
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
    nsl_strcat(fullPath, PATH_DELIMITER SPEED_LIMITS_IMAGES_PATH);
    nsl_strcpy(config->speedLimitsSignsPath, fullPath);

    nsl_strcpy(config->enhancedCityModelFileFormat, TEST_ECM_FILE_FORMAT);
    nsl_strcpy(config->motorwayJunctionObjectFileFormat, TEST_MJO_FILE_FORMAT);
    nsl_strcpy(config->realisticSignsFileFormat, TEST_SAR_FILE_FORMAT);
    nsl_strcpy(config->textureFileFormat, TEST_TEXTURE_FILE_FORMAT);
    nsl_strcpy(config->compressedTextureFileFormat, TEST_COMPRESSED_TEXTURE_FILE_FORMAT);
    nsl_strcpy(config->speedLimitsSignFileFormat, TEST_SPEED_LIMITS_SIGN_FILE_FORMAT);
}

/*! Enhanced content manager metadata synchronization callback

    @return None. CUnit Asserts get called on failures.
*/
void
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
    Instances* instances = (Instances*)userData;

    CU_ASSERT_PTR_NOT_NULL(instances);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (status == NB_NetworkRequestStatus_Progress)
    {
        return;
    }

    if (status == NB_NetworkRequestStatus_Success && result == NE_OK)
    {
        double scaleFactor = 0;
        double falseNorthing = 0;
        double falseEasting = 0;
        double zOffset = 0;
        NB_EnhancedContentProjectionType projectionType = NB_ECPT_Invalid;
        NB_EnhancedContentProjectionDatum datum = NB_ECPD_Invalid;
        const char* datasetId = NULL;
        NB_CityBoundingBox box = {0};
        NB_LatitudeLongitude location = {0};

        // Test getting city bounding box.
        nsl_memset(&box, 0, sizeof(box));
        result = NB_EnhancedContentManagerGetCityBoundingBox(instances->manager, "USA-LA", &box);
        CU_ASSERT_EQUAL(result, NE_OK);

        // Test getting city projection parameters.
        nsl_memset(&location, 0, sizeof(location));
        result = NB_EnhancedContentManagerGetCityProjectionParameters(instances->manager, "USA-LA", &projectionType, &datum, &location, &scaleFactor, &falseNorthing, &falseEasting, &zOffset);
        CU_ASSERT_EQUAL(result, NE_OK);

        // Test getting motorway junction objects dataset id
        datasetId = NB_EnhancedContentManagerGetMotorwayJunctionObjectsDatasetId(instances->manager);
        CU_ASSERT_EQUAL(nsl_strcmp(datasetId, "USA-MJO"), 0);

        // Test getting realistic signs dataset id
        datasetId = NB_EnhancedContentManagerGetRealisticSignsDatasetId(instances->manager);
        CU_ASSERT_EQUAL(nsl_strcmp(datasetId, "USA-SAR"), 0);

        // Test getting speed limits signs dataset id
        datasetId = NB_EnhancedContentManagerGetSpeedLimitsSignDatasetId(instances->manager);
        CU_ASSERT_EQUAL(nsl_strcmp(datasetId, "USA-SPEED"), 0);
    }
     else
     {
         CU_FAIL("Failed to synchronize metadata, status is not correct.");
     }

    // Metadata synchronized successfully
    if (NB_EnhancedContentManagerDownloadIsWaitingForMetadata(instances->manager) == FALSE)
    {
        SetCallbackCompletedEvent(instances->event);
    }
}

void CheckNormalDownloadResult(Instances* instances)
{
    uint32 cityBoundingBoxesCount = 0;
    NB_CityBoundingBox* cityBoundingBoxes = NULL;
    NB_Error result = NE_OK;

    // Test if city model is available
    CU_ASSERT(NB_EnhancedContentManagerIsDataAvailable(instances->manager, NB_ECDT_CityModel,
                                                       "USA-LA", "000_000"));

    // Test if city model is available with dependencies
    CU_ASSERT(NB_EnhancedContentManagerIsDataAvailableWithDependencies(instances->manager,
                                                                       NB_ECDT_CityModel,
                                                                       "USA-LA", "000_000"));

    // Test if common texture is available
    CU_ASSERT(NB_EnhancedContentManagerIsCommonTextureAvailable(instances->manager));

    // Test if common texture is available
    CU_ASSERT(NB_EnhancedContentManagerIsMotorwayJunctionTextureAvailable(instances->manager));

    // Test if city texture is available
    CU_ASSERT(NB_EnhancedContentManagerIsCityTextureAvailable(instances->manager, "USA-LA"));

    // Test getting city tile bounding boxes
    result = NB_EnhancedContentManagerGetCityTileBoundingBoxes(instances->manager, "USA-LA",
                                                               &cityBoundingBoxesCount,
                                                               &cityBoundingBoxes);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT(cityBoundingBoxesCount > 0);
    nsl_free(cityBoundingBoxes);
}

void CheckAbnormalDownloadResult(Instances* instances)
{
    nb_boolean result = TRUE;
    result = NB_EnhancedContentManagerIsDataAvailable(instances->manager,
                                                          NB_ECDT_SpeedLimitsSign,
                                                          "USA-SPEED",
                                                          "AnUniqeIncrroctSignId"
                                                          );
    CU_ASSERT_NOT_EQUAL(result, TRUE);
}

/*! Enhanced content manager metadata synchronization callback

    @return None. CUnit Asserts get called on failures.
*/
void
ContentDownloadCallback(NB_EnhancedContentManager* manager,
                        NB_Error result,
                        uint32 percent,
                        NB_EnhancedContentDataType dataType,
                        const char* datasetId,
                        void* userData)
{
    Instances* instances = (Instances*)userData;

    CU_ASSERT_PTR_NOT_NULL(instances);

    // Test will take a long time, so report progress to user to make user know what's going on.
    printf("\n\tDownloading: %d...\n", percent);

    if (result != NE_OK || percent == 100)
    {
        switch (instances->type)
        {
            case typePlain:
            {
                CU_ASSERT_EQUAL(result, NE_OK);
                CheckNormalDownloadResult(instances);
                break;
            }
            case typeErrorHandling:
            {
                CU_ASSERT_NOT_EQUAL(result, NE_OK);
                CheckAbnormalDownloadResult(instances);
                break;
            }
            default:
            {
                break;
            }
        }
        // Trigger main thread
        SetCallbackCompletedEvent(instances->event);
    }
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

    if (reason == PTCBR_TimerCanceled || !instances)
    {
        return;
    }

    result = NB_EnhancedContentManagerClearOnDemandRequests(instances->manager, NB_ECDT_All);
    CU_ASSERT_EQUAL(result, NE_OK);

    // Cancel requests successfully
    SetCallbackCompletedEvent(instances->event);
}

/*! @} */

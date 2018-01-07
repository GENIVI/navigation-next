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

    @defgroup TestMetadata_c Unit Tests for Metadata query/reply

    Unit Tests for NB Enhanced Content Metadata query/reply
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

/*! @{ */

#include "testmetadata.h"
#include "main.h"
#include "networkutil.h"
#include "platformutil.h"
#include "palfile.h"
#include "nbqalog.h"
#include "cslqalog.h"
#include "nbcontextprotected.h"

#ifdef WINCE
#include "winbase.h"
#endif

#include "nbmetadataparameters.h"
#include "nbmetadatahandler.h"
#include "nbmanifestparameters.h"
#include "nbmanifesthandler.h"
#include "nbenhancedcontentdatasource.h"

// Local Constants ...............................................................................


// Local Types ...................................................................................

/*! Collection of all instances used for the unit tests */
typedef struct
{
    PAL_Instance* pal;
    NB_Context* context;

    NB_EnhancedContentDataSource* dataSource;

    NB_EnhancedContentManagerConfiguration configuration;

    void* event;

} Instances;

// Variables .....................................................................................

static Instances g_instances = {0};


// Local Functions ...............................................................................

static void TestMetadataRequest(void);
static void TestManifestRequest(void);
static void TestDataSourceCreate(void);
static void TestDataSourceMasterClear(void);
static void TestDataSourceMetadataSynchronize(void);
static void TestDataSourceManifestSynchronize(void);
static void TestQaLoggingEnhancedContentRecords(void);

static void RequestMetadataCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
static void RequestManifestCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
static void SynchronizeMetadataCallback(NB_EnhancedContentDataSource* dataSource,
                                        NB_Error result,
                                        NB_RequestStatus status,
                                        uint32 percent,
                                        CSL_Vector* deletedDatasetReplies,
                                        CSL_Vector* addedCities,
                                        CSL_Vector* updatedCities,
                                        CSL_Vector* deletedCities,
                                        void* userData);
static void SynchronizeManifestCallback(NB_EnhancedContentDataSource* dataSource,       /*!< Enhanced content data source instance */
                                        NB_Error result,                                /*!< Result of manifest synchronize */
                                        NB_RequestStatus status,                        /*!< Status of manifest synchronize */
                                        NB_EnhancedContentDataType contentType,         /*!< Content type of removed manifest replies */
                                        const char* datasetId,                          /*!< IDataset identifier of removed manifest replies */
                                        CSL_Vector* removedManifestReplies,             /*!< Vector of NB_MetadataManifestReply. Should delete contained persistent files. */
                                        void* userData                                  /*!< User data of NB_EnhancedContentDataSourceManifestSynchronizeCallback */
                                        );

static nb_boolean CreateInstances(Instances* instances);
static void DestroyInstances(Instances* instances);


// Public Functions ..............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestMetadata_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestMetadataRequest", &TestMetadataRequest);
    CU_add_test(pTestSuite, "TestManifestRequest", &TestManifestRequest);
    CU_add_test(pTestSuite, "TestDataSourceCreate", &TestDataSourceCreate);
    CU_add_test(pTestSuite, "TestDataSourceMasterClear", &TestDataSourceMasterClear);
    CU_add_test(pTestSuite, "TestDataSourceMetadataSynchronize", &TestDataSourceMetadataSynchronize);
    CU_add_test(pTestSuite, "TestDataSourceManifestSynchronize", &TestDataSourceManifestSynchronize);
    CU_add_test(pTestSuite, "TestQaLoggingEnhancedContentRecords", &TestQaLoggingEnhancedContentRecords);
}


/*! Add common initialization code here.

    @return 0

    @see TestNetwork_SuiteCleanup
*/
int
TestMetadata_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

    @return 0

    @see TestMetadata_SuiteSetup
*/
int
TestMetadata_SuiteCleanup()
{
    return 0;
}


// Local Functions ...............................................................................
#define TO_BOOLEAN(number)       (number?TRUE:FALSE)

/*! Test metadata request by metadata handler.

@return None, but CU-asserts are called on errors.
*/
static void
TestMetadataRequest(void)
{
    NB_Error err = NE_OK;
    NB_MetadataParameters* parameters = NULL;
    NB_MetadataHandler* handler = NULL;
    NB_RequestHandlerCallback callback = {RequestMetadataCallback, &g_instances};

    int i, total = 8;

    /* Loop to test all possible combinations of want-extended-maps,
       want-shared-maps, and want-unified-maps. */

    /* Loop to test all possible combinations of want-extended-maps,
       want-shared-maps, and want-unified-maps. */
    for (i = 0; i < total; i++) {
        if (CreateInstances(&g_instances))
        {
            CU_ASSERT(g_instances.pal != NULL && g_instances.context != NULL);

            err = NB_MetadataParametersCreate(g_instances.context,
                                              TO_BOOLEAN(i & 0x01),
                                              TO_BOOLEAN(i & 0x02),
                                              TO_BOOLEAN(i & 0x04),
                                              TEST_LANGUAGE,
                                              TEST_SCREEN_WIDTH,
                                              TEST_SCREEN_HEIGHT,
                                              TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT,
                                              0, &parameters);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = NB_MetadataParametersCreate(g_instances.context,
                                              TO_BOOLEAN(i & 0x01),
                                              TO_BOOLEAN(i & 0x02),
                                              TO_BOOLEAN(i & 0x04),
                                              TEST_LANGUAGE,
                                              TEST_SCREEN_WIDTH,
                                              TEST_SCREEN_HEIGHT,
                                              TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT,
                                              0, &parameters);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = NB_MetadataParametersAddWantedContent(parameters, TEST_COUNTRY, "", NB_ECDT_All);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = NB_MetadataHandlerCreate(g_instances.context, &callback, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = NB_MetadataHandlerStartRequest(handler, parameters);
            CU_ASSERT_EQUAL(err, NE_OK);

            if (! WaitForCallbackCompletedEvent(g_instances.event, 15000))
            {
                CU_FAIL("Callback event timed out!");
            }

            NB_MetadataHandlerDestroy(handler);
            NB_MetadataParametersDestroy(parameters);
            DestroyInstances(&g_instances);
        }
    }
}

/*! Test manifest request by manifest handler.

@return None, but CU-asserts are called on errors.
*/
static void
TestManifestRequest(void)
{
    NB_Error err = NE_OK;
    NB_ManifestParameters* parameters = NULL;
    NB_ManifestHandler* handler = NULL;
    NB_RequestHandlerCallback callback = {RequestManifestCallback, &g_instances};

    if (CreateInstances(&g_instances))
    {
        CU_ASSERT(g_instances.pal != NULL && g_instances.context != NULL);

        err = NB_ManifestParametersCreate(g_instances.context, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_ManifestParametersAddWantedContent(parameters, TEST_COUNTRY, "USA-LA", NB_ECDT_All);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_ManifestHandlerCreate(g_instances.context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_ManifestHandlerStartRequest(handler, parameters);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (! WaitForCallbackCompletedEvent(g_instances.event, 15000))
        {
            CU_FAIL("Callback event timed out!");
        }

        NB_ManifestHandlerDestroy(handler);
        NB_ManifestParametersDestroy(parameters);

        DestroyInstances(&g_instances);
    }
}

/*! Test create/destroy.

@return None, but CU-asserts are called on errors.
*/
void
TestDataSourceCreate(void)
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        DestroyInstances(&instances);
    }
}

/*! Test metadata synchronize.

@return None, but CU-asserts are called on errors.
*/
static void
TestDataSourceMetadataSynchronize(void)
{
    NB_Error err = NE_OK;

    if (CreateInstances(&g_instances))
    {
        err = NB_EnhancedContentDataSourceMasterClear(g_instances.dataSource);
        CU_ASSERT_EQUAL(err, NE_OK);

        CU_ASSERT(!NB_EnhancedContentDataSourceIsValid(g_instances.dataSource));
        CU_ASSERT(!NB_EnhancedContentDataSourceIsMetadataSynchronizeInProgress(g_instances.dataSource));

        err = NB_EnhancedContentDataSourceStartMetadataSynchronize(g_instances.dataSource);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (! WaitForCallbackCompletedEvent(g_instances.event, 30000))
        {
            CU_FAIL("Callback event timed out!");
        }

        DestroyInstances(&g_instances);
    }
}

/*! Test manifest synchronize.

@return None, but CU-asserts are called on errors.
*/
static void
TestDataSourceManifestSynchronize(void)
{
    NB_Error err = NE_OK;

    if (CreateInstances(&g_instances))
    {
        CU_ASSERT(NB_EnhancedContentDataSourceIsValid(g_instances.dataSource));
        CU_ASSERT(!NB_EnhancedContentDataSourceIsManifestSynchronizeInProgress(g_instances.dataSource));

        err = NB_EnhancedContentDataSourceSelectCity(g_instances.dataSource, "USA-LA", TRUE);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_EnhancedContentDataSourceStartManifestSynchronize(g_instances.dataSource);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (! WaitForCallbackCompletedEvent(g_instances.event, 300000))
        {
            CU_FAIL("Callback event timed out!");
        }

        DestroyInstances(&g_instances);
    }
}

/*! Test master clear.

@return None, but CU-asserts are called on errors.
*/
static void
TestDataSourceMasterClear(void)
{
    NB_Error err = NE_OK;

    if (CreateInstances(&g_instances))
    {
        err = NB_EnhancedContentDataSourceMasterClear(g_instances.dataSource);
        CU_ASSERT_EQUAL(err, NE_OK);

        CU_ASSERT(!NB_EnhancedContentDataSourceIsValid(g_instances.dataSource));

        DestroyInstances(&g_instances);
    }
}

/*! Test qalogging recording of enhanced content.

@return None, but CU-asserts are called on errors.
*/
static void
TestQaLoggingEnhancedContentRecords(void)
{
    if (CreateInstances(&g_instances))
    {
        
        NB_Error err = NE_OK;
        CU_ASSERT(g_instances.pal != NULL && g_instances.context != NULL);

        if (!NB_ContextGetQaLog(g_instances.context))
        {
            CSL_QaLogHeaderData header = {0};
            CSL_QaLog* log = NULL;
            nb_version productVersion = { 4, 9, 15, 10 };

            header.productName = "NBServices";
            header.productVersion = productVersion;
            header.platformId = 0;
            header.mobileDirectoryNumber = GetMobileDirectoryNumber(g_instances.pal);
            header.mobileInformationNumber = GetMobileInformationNumber(g_instances.pal);

            err = CSL_QaLogCreate(g_instances.pal, &header, "TestMetadata.qa", FALSE, &log);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = NB_ContextSetQaLog(g_instances.context, log);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(NB_ContextGetQaLog(g_instances.context));
        }

        NB_QaLogMetadataSourceRequest(g_instances.context, "language", 7, TRUE, 640, 480, 96);
        NB_QaLogWantedContent(g_instances.context, (unsigned char)1, "junctions", "USA");
        NB_QaLogMetadataSourceResponse(g_instances.context, "citySummary", "baseUrl", "mercator", 1);
        NB_QaLogMetadataSourceTemplate(g_instances.context, "city models", "template", 100, "metadataId", "manifestVersion",
                                       32767, "displayName", 33.5, -117.5, 34.5, -118.5, "WGS84", 500000.0, 1000000.0, -33.0, 117.0, 1.0, 0.0);
        NB_QaLogManifestRequest(g_instances.context, 7);
        NB_QaLogManifestResponse(g_instances.context, "datasetId", 100, "firstTileId", 33.5, -117.5, 34.5, -118.5, "lastTileId", 30.0, -117.5, 29.0, -118.5);
        NB_QaLogManifestResponseContent(g_instances.context, "contentFieldId", "contentFileVersion", 33.5, -117.5, 34.5, -118.5);
        NB_QaLogRealisticSignInfo(g_instances.context, "sarId", "sarType", "datasetId", "version", 5);
        NB_QaLogRealisticSignState(g_instances.context, "sarId", (unsigned char)1, 33.5, -117.5);
        NB_QaLogEnhancedContentInfo(g_instances.context, "ecId", (unsigned char)1, "version", "datasetId", 1, 100.0, 2, 200.0,
                                    (unsigned char)2, (unsigned char)1, 500000.0, 1000000.0, -33.0, 117.0, 1.0, 0.0);
        NB_QaLogEnhancedContentDownload(g_instances.context, "ecId", (unsigned char)1, (unsigned char)1, "version", 20, "url");
        NB_QaLogEnhancedContentDownloadResult(g_instances.context, "ecId", (unsigned char)1, (unsigned char)1, "version", 20, (unsigned char)1);
    }

    DestroyInstances(&g_instances);

}


static void
RequestMetadataCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    Instances* instances = (Instances*) userData;

    if (!up)
    {
        if (err != NE_OK)
        {
            // Abort and trigger event
            CU_FAIL("Callback returned error");
            SetCallbackCompletedEvent(instances->event);
            return;
        }

        switch (status)
        {
            case NB_NetworkRequestStatus_Progress:
                // not complete yet... return
                return;

            case NB_NetworkRequestStatus_Success:
            {
                NB_MetadataInformation* info = NULL;
                NB_Error err = NE_OK;
                const char* citySummaryDisplay = NULL;

                err = NB_MetadataHandlerGetInformation(handler, &info);
                CU_ASSERT_EQUAL(err, NE_OK);

                err = NB_MetadataInformationGetCitySummaryDisplay(info, &citySummaryDisplay);
                CU_ASSERT_EQUAL(err, NE_OK);

                // TODO : Add more tests when using NB_MetadataInformation in NB_EnhancedContentDataSource*.

                NB_MetadataInformationDestroy(info);
                break;
            }
            case NB_NetworkRequestStatus_TimedOut:
                CU_FAIL("NB_NetworkRequestStatus_TimedOut");
                break;
            case NB_NetworkRequestStatus_Canceled:
                CU_FAIL("NB_NetworkRequestStatus_Canceled");
                break;
            case NB_NetworkRequestStatus_Failed:
                CU_FAIL("NB_NetworkRequestStatus_Failed");
                break;
        }

        // Trigger main thread
        SetCallbackCompletedEvent(instances->event);
    }
}

static void
RequestManifestCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    Instances* instances = (Instances*) userData;

    if (!up)
    {
        if (err != NE_OK)
        {
            // Abort and trigger event
            CU_FAIL("Callback returned error");
            SetCallbackCompletedEvent(instances->event);
            return;
        }

        switch (status)
        {
            case NB_NetworkRequestStatus_Progress:
                // not complete yet... return
                return;

            case NB_NetworkRequestStatus_Success:
            {
                NB_ManifestInformation* info = NULL;
                NB_Error err = NE_OK;

                err = NB_ManifestHandlerGetInformation(handler, &info);
                CU_ASSERT_EQUAL(err, NE_OK);

                // TODO : Add more tests when using NB_ManifestInformation in NB_EnhancedContentDataSource*.

                NB_ManifestInformationDestroy(info);
                break;
            }
            case NB_NetworkRequestStatus_TimedOut:
                CU_FAIL("NB_NetworkRequestStatus_TimedOut");
                break;
            case NB_NetworkRequestStatus_Canceled:
                CU_FAIL("NB_NetworkRequestStatus_Canceled");
                break;
            case NB_NetworkRequestStatus_Failed:
                CU_FAIL("NB_NetworkRequestStatus_Failed");
                break;
        }

        // Trigger main thread
        SetCallbackCompletedEvent(instances->event);
    }
}

static void
SynchronizeMetadataCallback(NB_EnhancedContentDataSource* dataSource,
                            NB_Error result,
                            NB_RequestStatus status,
                            uint32 percent,
                            CSL_Vector* deletedDatasetReplies,
                            CSL_Vector* addedCities,
                            CSL_Vector* updatedCities,
                            CSL_Vector* deletedCities,
                            void* userData)
{
    Instances* instances = (Instances*) userData;
    int length = 0;
    uint16 port = 0;
    uint32 datasetSize = 0;
    uint64 totalSize = 0;
    uint64 downloadedSize = 0;
    uint32 totalCommonTexturesSize = 0;
    uint32 downloadedCommonTexturesSize = 0;
    uint32 downloadedPercentage = 0;
    double scaleFactor = 0;
    double falseNorthing = 0;
    double falseEasting = 0;
    double zOffset = 0;
    NB_EnhancedContentProjectionType projectionType = NB_ECPT_Invalid;
    NB_EnhancedContentProjectionDatum datum = NB_ECPD_Invalid;
    char url[TEST_MAX_URL_LENGTH];
    char path[TEST_MAX_PATH_LENGTH];
    const char* datasetId = NULL;
    const char* citySummaryDisplay = NULL;
    NB_EnhancedContentIdentifier* dataId = NULL;
    NB_EnhancedContentCityData* city = NULL;
    CSL_Vector* cities = NULL;
    NB_EnhancedContentCityData cityData = {0};
    NB_CityBoundingBox box = {0};
    NB_LatitudeLongitude location = {0};

    char* TEST_PART_OF_SAR_URL = "/780x585/US_CA_59022_SIGN_L1_P.PNG";

    if (result != NE_OK)
    {
        // Abort and trigger event
        CU_FAIL("Callback returned error");
        SetCallbackCompletedEvent(instances->event);
        return;
    }

    switch (status)
    {
        case NB_NetworkRequestStatus_Progress:
            // not complete yet... return
            return;

        case NB_NetworkRequestStatus_Success:
        {
            CU_ASSERT(dataSource != NULL);

            dataId = NB_EnhancedContentIdentifierCreate(NB_ECDT_RealisticSign, "USA-SAR",
                                                        "US_CA_59022_SIGN_L1_P", NULL);
            CU_ASSERT_PTR_NOT_NULL(dataId);

            cities = CSL_VectorAlloc(sizeof(NB_EnhancedContentCityData));

            CU_ASSERT(NB_EnhancedContentDataSourceIsValid(dataSource));

            nsl_memset(url, 0, sizeof(url));
            result = NB_EnhancedContentDataSourceGetHostname(dataSource, TEST_MAX_URL_LENGTH, url, &port);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(nsl_strcmp(url, "http://content.qa1.navbuilder.nimlbs.net"), 0);

            nsl_memset(url, 0, sizeof(url));
            result = NB_EnhancedContentDataSourceBuildItemUrl(dataSource, dataId, TEST_MAX_URL_LENGTH, url);
            // Ignore manifest version for testing. It changes from server.
            // "USA/sign/110303/780x585/US_CA_59022_SIGN_L1_P.PNG"
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(nsl_strcmp(url + nsl_strlen(url) - nsl_strlen(TEST_PART_OF_SAR_URL), TEST_PART_OF_SAR_URL), 0);

            result = NB_EnhancedContentDataSourceBuildItemFilePath(dataSource, dataId, FALSE, TEST_MAX_PATH_LENGTH, path);
            CU_ASSERT_EQUAL(result, NE_OK);

            datasetId = NB_EnhancedContentDataSourceGetCommonTexturesDatasetId(dataSource);
            CU_ASSERT_EQUAL(nsl_strcmp(datasetId, "USA-COMMON"), 0);

            datasetId = NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(dataSource);
            CU_ASSERT_EQUAL(nsl_strcmp(datasetId, "USA-MJO"), 0);

            datasetId = NB_EnhancedContentDataSourceGetRealisticSignsDatasetId(dataSource);
            CU_ASSERT_EQUAL(nsl_strcmp(datasetId, "USA-SAR"), 0);

            result = NB_EnhancedContentDataSourceGetCitySummaryDisplay(dataSource, &citySummaryDisplay);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Test available cities.
            CSL_VectorRemoveAll(cities);
            result = NB_EnhancedContentDataSourceGetAvailableCities(dataSource, cities);
            length = CSL_VectorGetLength(cities);
            CU_ASSERT(length > 0);
            city = (NB_EnhancedContentCityData*)CSL_VectorGetPointer(cities, 0);
            CU_ASSERT_EQUAL(city->selected, FALSE);
            CU_ASSERT_EQUAL(city->deleted, FALSE);
            CU_ASSERT_EQUAL(city->updateStatus, NB_ECUS_UpdateStatusNew);
            CU_ASSERT(city->cityModelDataSize > 0);
            CU_ASSERT(city->downloadedCityModelDataSize == 0);
            CU_ASSERT(city->failedCityModelDataSize == 0);
            CU_ASSERT(city->textureDataSize > 0);
            CU_ASSERT(city->downloadedTextureDataSize == 0);
            CU_ASSERT(city->failedTextureDataSize == 0);
            CU_ASSERT(city->box.point1.latitude != INVALID_LATITUDE);
            CU_ASSERT(city->box.point1.longitude != INVALID_LONGITUDE);
            CU_ASSERT(city->box.point2.latitude != INVALID_LATITUDE);
            CU_ASSERT(city->box.point2.longitude != INVALID_LONGITUDE);
            CU_ASSERT(city->center.latitude != INVALID_LATITUDE);
            CU_ASSERT(city->center.longitude != INVALID_LONGITUDE);

            // Test select cities.
            CSL_VectorRemoveAll(cities);
            result = NB_EnhancedContentDataSourceGetSelectedCities(dataSource, cities);
            length = CSL_VectorGetLength(cities);
            CU_ASSERT_EQUAL(length, 0);

            result = NB_EnhancedContentDataSourceSelectCity(g_instances.dataSource, "USA-LA", TRUE);
            CU_ASSERT_EQUAL(result, NE_OK);
            result = NB_EnhancedContentDataSourceSelectCity(g_instances.dataSource, "invalid-dataset", TRUE);
            CU_ASSERT_EQUAL(result, NE_NOENT);

            CSL_VectorRemoveAll(cities);
            result = NB_EnhancedContentDataSourceGetSelectedCities(dataSource, cities);
            length = CSL_VectorGetLength(cities);
            CU_ASSERT_EQUAL(length, 1);

            CU_ASSERT(!NB_EnhancedContentDataSourceIsDatasetSelected(g_instances.dataSource, NB_ECDT_MotorwayJunctionObject, "USA-MJO"));
            CU_ASSERT(!NB_EnhancedContentDataSourceIsDatasetSelected(g_instances.dataSource, NB_ECDT_SpecificTexture, "USA-MJO"));
            CU_ASSERT(!NB_EnhancedContentDataSourceIsDatasetSelected(g_instances.dataSource, NB_ECDT_RealisticSign, "USA-SAR"));
            CU_ASSERT(NB_EnhancedContentDataSourceIsDatasetSelected(g_instances.dataSource, NB_ECDT_CityModel, "USA-LA"));
            CU_ASSERT(NB_EnhancedContentDataSourceIsDatasetSelected(g_instances.dataSource, NB_ECDT_SpecificTexture, "USA-LA"));

            result = NB_EnhancedContentDataSourceSelectCity(g_instances.dataSource, "USA-LA", FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            CSL_VectorRemoveAll(cities);
            result = NB_EnhancedContentDataSourceGetSelectedCities(dataSource, cities);
            length = CSL_VectorGetLength(cities);
            CU_ASSERT_EQUAL(length, 0);

            // Test getting city information.
            nsl_memset(&cityData, 0, sizeof(cityData));
            result = NB_EnhancedContentDataSourceGetCityInformation(dataSource, "invalid-dataset", &cityData);
            CU_ASSERT_EQUAL(result, NE_NOENT);
            result = NB_EnhancedContentDataSourceGetCityInformation(dataSource, "USA-LA", &cityData);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Test getting dataset size.
            result = NB_EnhancedContentDataSourceGetDatasetSize(dataSource, NB_ECDT_CityModel, "USA-MJO", &datasetSize);
            CU_ASSERT(result != NE_OK);

            result = NB_EnhancedContentDataSourceGetDatasetSize(dataSource, NB_ECDT_MotorwayJunctionObject, "USA-MJO", &datasetSize);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT(datasetSize > 0);

            // Test getting city bounding box.
            nsl_memset(&box, 0, sizeof(box));
            result = NB_EnhancedContentDataSourceGetCityBoundingBox(dataSource, NULL, &box);
            CU_ASSERT(result != NE_OK);
            result = NB_EnhancedContentDataSourceGetCityBoundingBox(dataSource, "USA-LA", &box);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Test getting city projection parameters.
            nsl_memset(&location, 0, sizeof(location));
            result = NB_EnhancedContentDataSourceGetCityProjectionParameters(dataSource, "USA-DC", &projectionType, &datum, &location, &scaleFactor, &falseNorthing, &falseEasting, &zOffset);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Test getting storage statistics.
            result = NB_EnhancedContentDataSourceGetStorageStatistics(dataSource, &totalSize, &downloadedSize, &totalCommonTexturesSize, &downloadedCommonTexturesSize, &downloadedPercentage);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT(downloadedSize == 0 && downloadedPercentage == 0);
            CU_ASSERT(totalSize >= totalCommonTexturesSize);

            // Test set location.
            location.latitude = 40.40;
            location.longitude = -80.0;
            result = NB_EnhancedContentDataSourceSetLocation(dataSource, &location);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Test getting next dataset to update.
            result = NB_EnhancedContentDataSourceGetNextDatasetToUpdate(dataSource, FALSE, dataId);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT(dataId->type == NB_ECDT_CommonTexture && nsl_strcmp(dataId->dataset, "USA-COMMON") == 0);

            NB_EnhancedContentIdentifierDestroy(dataId);

            CSL_VectorDealloc(cities);

            break;
        }
        case NB_NetworkRequestStatus_TimedOut:
            CU_FAIL("NB_NetworkRequestStatus_TimedOut");
            break;
        case NB_NetworkRequestStatus_Canceled:
            CU_FAIL("NB_NetworkRequestStatus_Canceled");
            break;
        case NB_NetworkRequestStatus_Failed:
            CU_FAIL("NB_NetworkRequestStatus_Failed");
            break;
    }

    // Trigger main thread
    SetCallbackCompletedEvent(instances->event);
}

static void
SynchronizeManifestCallback(
                            NB_EnhancedContentDataSource* dataSource,       /*!< Enhanced content data source instance */
                            NB_Error result,                                /*!< Result of manifest synchronize */
                            NB_RequestStatus status,                        /*!< Status of manifest synchronize */
                            NB_EnhancedContentDataType contentType,         /*!< Content type of removed manifest replies */
                            const char* datasetId,                          /*!< IDataset identifier of removed manifest replies */
                            CSL_Vector* removedManifestReplies,             /*!< Vector of NB_MetadataManifestReply. Should delete contained persistent files. */
                            void* userData                                  /*!< User data of NB_EnhancedContentDataSourceManifestSynchronizeCallback */
                            )
{
    Instances* instances = (Instances*) userData;
    int n = 0;
    int length = 0;
    uint32 dataSize = 0;
    uint32 cityBoundingBoxesCount = 0;
    NB_CityBoundingBox* cityBoundingBoxes = NULL;
    NB_EnhancedContentIdentifier* dataId = NULL;
    CSL_Vector* dataIds = NULL;
    char manifestVersion[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];

    if (result != NE_OK)
    {
        // Abort and trigger event
        CU_FAIL("Callback returned error");
        SetCallbackCompletedEvent(instances->event);
        return;
    }

    switch (status)
    {
        case NB_NetworkRequestStatus_Progress:
            // not complete yet... return
            return;

        case NB_NetworkRequestStatus_Success:
        {
            CU_ASSERT(dataSource != NULL);

            dataId = NB_EnhancedContentIdentifierCreate(NB_ECDT_CityModel, "USA-LA", NULL, NULL);
            CU_ASSERT_PTR_NOT_NULL(dataId);

            // Test getting manifest version.
            nsl_memset(manifestVersion, 0, sizeof(manifestVersion));
            result = NB_EnhancedContentDataSourceGetManifestVersion(dataSource, dataId, MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1, manifestVersion);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Test getting data item size.
            result = NB_EnhancedContentDataSourceGetDataItemSize(dataSource, dataId, &dataSize);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT(dataSize > 0);

            NB_EnhancedContentIdentifierSet(dataId, NB_ECDT_CityModel, "USA-LA", "001_001", NULL);
            result = NB_EnhancedContentDataSourceGetDataItemSize(dataSource, dataId, &dataSize);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT(dataSize > 0);

            // Test if manifest update is needed.
            CU_ASSERT(!NB_EnhancedContentDataSourceIsManifestUpdateNeeded(dataSource, dataId));

            // Test if data item is consistent.
            CU_ASSERT(NB_EnhancedContentDataSourceIsDataItemConsistent(dataSource, dataId));

            // Test if data item is available for use.
            CU_ASSERT(!NB_EnhancedContentDataSourceIsDataItemAvailableForUse(dataSource, dataId));

            // Test if data item is enabled for download.
            CU_ASSERT(NB_EnhancedContentDataSourceIsDataItemEnabledForDownload(dataSource, dataId));

            NB_EnhancedContentIdentifierDestroy(dataId);

            dataIds = CSL_VectorAlloc(sizeof(NB_EnhancedContentIdentifier));
            CU_ASSERT_PTR_NOT_NULL(dataIds);

            // Test getting data item ids.
            result = NB_EnhancedContentDataSourceGetDataItemIDs(dataSource, TRUE, NB_ECDT_SpecificTexture, "USA-MJO", dataIds);
            CU_ASSERT_EQUAL(result, NE_OK);
            length = CSL_VectorGetLength(dataIds);
            CU_ASSERT_EQUAL(length, 1);
            for (n = 0; n < length; ++n)
            {
                dataId = (NB_EnhancedContentIdentifier*)CSL_VectorGetPointer(dataIds, n);
                NB_EnhancedContentIdentifierSet(dataId, NB_ECDT_None, NULL, NULL, NULL);
            }
            CSL_VectorRemoveAll(dataIds);

            result = NB_EnhancedContentDataSourceGetDataItemIDs(dataSource, TRUE, NB_ECDT_CityModel, "USA-LA", dataIds);
            CU_ASSERT_EQUAL(result, NE_OK);
            length = CSL_VectorGetLength(dataIds);
            CU_ASSERT(length > 0);
            length = CSL_VectorGetLength(dataIds);
            for (n = 0; n < length; ++n)
            {
                dataId = (NB_EnhancedContentIdentifier*)CSL_VectorGetPointer(dataIds, n);
                NB_EnhancedContentIdentifierSet(dataId, NB_ECDT_None, NULL, NULL, NULL);
            }
            CSL_VectorRemoveAll(dataIds);

            CSL_VectorDealloc(dataIds);


            // Test getting city tileBoundingBoxes.
            result = NB_EnhancedContentDataSourceGetCityTileBoundingBoxes(dataSource, "USA-LA", &cityBoundingBoxesCount, &cityBoundingBoxes);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT(cityBoundingBoxesCount > 0);
            nsl_free(cityBoundingBoxes);

            break;
        }
        case NB_NetworkRequestStatus_TimedOut:
            CU_FAIL("NB_NetworkRequestStatus_TimedOut");
            break;
        case NB_NetworkRequestStatus_Canceled:
            CU_FAIL("NB_NetworkRequestStatus_Canceled");
            break;
        case NB_NetworkRequestStatus_Failed:
            CU_FAIL("NB_NetworkRequestStatus_Failed");
            break;
    }

    // Trigger main thread
    SetCallbackCompletedEvent(instances->event);
}

/*! Create PAL, context, data source and callback event.

Shared by all unit tests.

@return TRUE on success, FALSE otherwise.
*/
static nb_boolean
CreateInstances(Instances* instances)
{
    NB_Error result = NE_OK;
    uint8 createResult = 0;
    const char* basePath = NULL;
    char fullPath[TEST_MAX_PATH_LENGTH];
    NB_EnhancedContentDataSourceMetadataSynchronizeCallback metadataSynchronizeCallbackData = {0};
    NB_EnhancedContentDataSourceManifestSynchronizeCallback manifestSynchronizeCallbackData = {0};

    nsl_memset(instances, 0, sizeof(*instances));

    metadataSynchronizeCallbackData.callback = SynchronizeMetadataCallback;
    metadataSynchronizeCallbackData.callbackData = instances;
    manifestSynchronizeCallbackData.callback = SynchronizeManifestCallback;
    manifestSynchronizeCallbackData.callbackData = instances;

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

    instances->event = CreateCallbackCompletedEvent();
    CU_ASSERT_PTR_NOT_NULL(instances->event);

    return TRUE;
}

/*! Destroy PAL, context and download manager.

Shared by all unit tests.

@return None
*/
static void
DestroyInstances(Instances* instances)
{
    NB_Error result = NE_OK;

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

/*! @} */

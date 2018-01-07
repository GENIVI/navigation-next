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

    @file     TestPublicTransit.h
    @defgroup TestPublicTransit_h System tests for Public Transit functions
*/
/*
    See description in header file.

    (C) Copyright 2011 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "main.h"
#include "testpublictransit.h"
#include "testnetwork.h"
#include "nbsearchhandler.h"
#include "nbpublictransitmanager.h"
#include "nbplace.h"
#include "paltypes.h"

//const NB_LatitudeLongitude TestingDestination = {37.857506999999998, -122.209396};
//const NB_LatitudeLongitude PlanTripDestinationStart = {37.857506999999998, -122.209396};
//const NB_LatitudeLongitude PlanTripDestinationEnd = {37.853061, -122.269946};

//New York City
const NB_LatitudeLongitude TestingDestination = {40.643419000000002, -73.789739999999995};
const NB_LatitudeLongitude PlanTripDestinationStart = {40.643419000000002, -73.789739999999995};
const NB_LatitudeLongitude PlanTripDestinationEnd = {40.650604999999999, -73.798800499999999};


// Local Constants ...........................................................
typedef struct
{
    NB_SearchHandler*       pSearchHandler;
    NB_SearchInformation*   pSearchInformation;
} SearchTestState;

static PAL_Instance* g_pPal = NULL;
static NB_Context* g_pContext = NULL;
static SearchTestState g_SearchState = {0};

// Local Functions ...........................................................
static void RequestHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
static void RequestStopsCallback(NB_RequestStatus status, NB_Error err, uint32* places, uint32 countOfPlaces, void* userData);
static void RequestStopsWithoutPTMCallback(NB_RequestStatus status, NB_Error err, uint32* places, uint32 countOfPlaces, void* userData);
static void RequestStopsCancelRequestsCallback(NB_RequestStatus status, NB_Error err, uint32* places, uint32 countOfPlaces, void* userData);
static void RequestStopsWithIncorrectPlaceIDCallback(NB_RequestStatus status, NB_Error err, uint32* places, uint32 countOfPlaces, void* userData);

static void RequestStopInfoCallback(void* userData, NB_PublicTransitPlace* place);
static void RequestStopInfoWithIncorrectPlaceIDCallback(void* userData, NB_PublicTransitPlace* place);

static void RequestTransitOverlayStopsCallback(NB_RequestStatus status, NB_Error err, uint32* places, uint32 countOfPlaces, void* userData);
static void PublicTransitRequestTransitOverlayStops(const char* transitTypes[], int transitTypeCount);

static void PlanTripCallBack(NB_Error err, uint32* startTransitPlaceList, int startListCount, uint32* endTransitPlaceList, int endListCount, void* cbData);

static void RequestPlanStopsPlanTripCallBack(NB_Error err, uint32* startTransitPlaceList, int startListCount, uint32* endTransitPlaceList, int endListCount, void* cbData);
static void RequestPlanStopsCallBack(NB_RequestStatus status, NB_Error err, uint32* places, uint32 countOfPlaces, void* cbData);

static NB_SearchInformation* DoRequest(NB_SearchParameters* parameters, int* slice, nb_boolean* hasMoreResults, NB_SearchResultType* type);

static void TestPublicTransitCreateWithoutContext(void);
static void TestPublicTransitCreateWithContext(void);
static void TestPublicTransitDestroyWithoutPTM(void);
static void TestPublicTransitDestroyWithPTM(void);
static void TestPublicTransitRequestStopsWithoutPTM(void);
static void TestPublicTransitRequestStopsWithIncorrectPlaceID(void);
static void TestPublicTransitRequestStops(void);
static void TestPublicTransitCancelRequestsWithoutPTM(void);
static void TestPublicTransitCancelRequestsWithoutRequest(void);
static void TestPublicTransitCancelRequests(void);
static void TestPublicTransitGetCurrentStopInfo(void);
static void TestPublicTransitGetCurrentStopInfoWithoutPTM(void);
static void TestPublicTransitGetCurrentStopInfoWithIncorrectPlaceID(void);
static void TestPublicTransitGetHoursInformationWithoutPTM(void);
static void TestPublicTransitGetHoursInformationWithIncorrectStopID(void);
static void TestPublicTransitGetHoursInformation(void);
static void TestPublicTransitAddTransitStop(void);
static void TestPublicTransitAddTransitStopWithoutPTM(void);
static void TestPublicTransitAddTransitStopWithoutPlace(void);
static void TestPublicTransitAddTransitStopWithoutTransitStop(void);
static void TestPublicTransitRequestTransitOverlayStopsWithoutTransitType(void);
static void TestPublicTransitRequestTransitOverlayStopsWithAllTransitType(void);
static void TestPublicTransitRequestTransitOverlayStopsWithTransitTypes(void);
static void TestPublicTransitCreatePlanTrip(void);
static void TestPublicTransitCreatePlanTripWithoutPTM(void);
static void TestPublicTransitCreatePlanTripWithoutStartPlace(void);
static void TestPublicTransitCreatePlanTripWithoutEndPlace(void);
static void TestPublicTransitRequestPlanStops(void);
static void TestSearchInformationGetPublicTransitPlaceWithoutInformation(void);
static void TestSearchInformationGetPublicTransitPlaceWithIncorrectIndex(void);
static void TestSearchInformationReleasePublicTransitPlaceWithoutPlace(void);
static void TestSearchParametersCreatePublicTransitWithoutContext(void);
static void TestSearchParametersCreatePublicTransitWithoutRegion(void);
static void TestSearchParametersCreatePublicTransitWithoutName(void);
static void TestSearchParametersCreatePublicTransitWithoutCategoryCodes(void);
static void TestSearchParametersCreatePublicTransitWithIncorrectCodeCount(void);
static void TestSearchParametersCreatePublicTransitWithIncorrectSliceSize(void);
static void TestSearchParametersCreatePublicTransitWithoutLanguage(void);
static void TestSearchParametersCreatePublicTransitWithoutParameters(void);
static void TestSearchParametersCreatePublicTransitOverlayWithoutContext(void);
static void TestSearchParametersCreatePublicTransitOverlayWithoutRegion(void);
static void TestSearchParametersCreatePublicTransitOverlayWithoutName(void);
static void TestSearchParametersCreatePublicTransitOverlayWithoutTransitType(void);
static void TestSearchParametersCreatePublicTransitOverlayWithoutLanguage(void);
static void TestSearchParametersCreatePublicTransitOverlayWithoutParameters(void);

static nb_boolean InitializeAll();
static void CleanupAll();
static void FreeParameters(NB_SearchParameters** parameters);
static void FreeInformation(NB_SearchInformation** information);

// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;

/*! Add all your test functions here

@return None
*/
void
TestPublicTransit_AddTests( CU_pSuite pTestSuite, int level )
{
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitCreateWithoutContext", TestPublicTransitCreateWithoutContext);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitCreateWithContext", TestPublicTransitCreateWithContext);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitDestroyWithoutPTM", TestPublicTransitDestroyWithoutPTM);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitDestroyWithPTM", TestPublicTransitDestroyWithPTM);

    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitRequestStopsWithoutPTM", TestPublicTransitRequestStopsWithoutPTM);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitRequestStopsWithIncorrectPlaceID", TestPublicTransitRequestStopsWithIncorrectPlaceID);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitRequestStops", TestPublicTransitRequestStops);

    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitCancelRequestsWithoutPTM", TestPublicTransitCancelRequestsWithoutPTM);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitCancelRequestsWithoutRequest", TestPublicTransitCancelRequestsWithoutRequest);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitCancelRequests", TestPublicTransitCancelRequests);

    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitGetCurrentStopInfoWithoutPTM", TestPublicTransitGetCurrentStopInfoWithoutPTM);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitGetCurrentStopInfoWithIncorrectPlaceID", TestPublicTransitGetCurrentStopInfoWithIncorrectPlaceID);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitGetCurrentStopInfo", TestPublicTransitGetCurrentStopInfo);

    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitGetHoursInformationWithoutPTM", TestPublicTransitGetHoursInformationWithoutPTM);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitGetHoursInformationWithIncorrectStopID", TestPublicTransitGetHoursInformationWithIncorrectStopID);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitGetHoursInformation", TestPublicTransitGetHoursInformation);

    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitAddTransitStop", TestPublicTransitAddTransitStop);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitAddTransitStopWithoutPTM", TestPublicTransitAddTransitStopWithoutPTM);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitAddTransitStopWithoutPlace", TestPublicTransitAddTransitStopWithoutPlace);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitAddTransitStopWithoutTransitStop", TestPublicTransitAddTransitStopWithoutTransitStop);

    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitRequestTransitOverlayStopsWithoutTransitType", TestPublicTransitRequestTransitOverlayStopsWithoutTransitType);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitRequestTransitOverlayStopsWithAllTransitType", TestPublicTransitRequestTransitOverlayStopsWithAllTransitType);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitRequestTransitOverlayStopsWithTransitTypes", TestPublicTransitRequestTransitOverlayStopsWithTransitTypes);

	//servers don't support public-transit queries
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitCreatePlanTrip", TestPublicTransitCreatePlanTrip);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitCreatePlanTripWithoutPTM", TestPublicTransitCreatePlanTripWithoutPTM);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitCreatePlanTripWithoutStartPlace", TestPublicTransitCreatePlanTripWithoutStartPlace);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitCreatePlanTripWithoutEndPlace", TestPublicTransitCreatePlanTripWithoutEndPlace);
	//servers don't support public-transit queries
	//ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPublicTransitRequestPlanStops", TestPublicTransitRequestPlanStops);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchInformationGetPublicTransitPlaceWithoutInformation", TestSearchInformationGetPublicTransitPlaceWithoutInformation);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchInformationGetPublicTransitPlaceWithIncorrectIndex", TestSearchInformationGetPublicTransitPlaceWithIncorrectIndex);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchInformationReleasePublicTransitPlaceWithoutPlace", TestSearchInformationReleasePublicTransitPlaceWithoutPlace);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitWithoutContext", TestSearchParametersCreatePublicTransitWithoutContext);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitWithoutRegion", TestSearchParametersCreatePublicTransitWithoutRegion);
    // name key/value pair is not required in proxpoi request
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitWithoutName", TestSearchParametersCreatePublicTransitWithoutName);
    // category codes are not required for every PT proxpoi request
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitWithoutCategoryCodes", TestSearchParametersCreatePublicTransitWithoutCategoryCodes);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitWithIncorrectCodeCount", TestSearchParametersCreatePublicTransitWithIncorrectCodeCount);
    // specifying 0 slice size is not an error
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitWithIncorrectSliceSize", TestSearchParametersCreatePublicTransitWithIncorrectSliceSize);
    // language is optional in proxpoi query
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitWithoutLanguage", TestSearchParametersCreatePublicTransitWithoutLanguage);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitWithoutParameters", TestSearchParametersCreatePublicTransitWithoutParameters);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitOverlayWithoutContext", TestSearchParametersCreatePublicTransitOverlayWithoutContext);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitOverlayWithoutRegion", TestSearchParametersCreatePublicTransitOverlayWithoutRegion);
    // name is not required for overlay request
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitOverlayWithoutName", TestSearchParametersCreatePublicTransitOverlayWithoutName);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitOverlayWithoutTransitType", TestSearchParametersCreatePublicTransitOverlayWithoutTransitType);
    // in general language is not required for proxpoi request
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitOverlayWithoutLanguage", TestSearchParametersCreatePublicTransitOverlayWithoutLanguage);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSearchParametersCreatePublicTransitOverlayWithoutParameters", TestSearchParametersCreatePublicTransitOverlayWithoutParameters);
}

/*! Add common initialization code here.

@return 0

@see TestPublicTransit_SuiteCleanup
*/
int
TestPublicTransit_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestPublicTransit_SuiteSetup
*/
int
TestPublicTransit_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
    return 0;
}

/*! Initialize Pal, context.

    Called before every test. We can't call this in the suite initialize since we can't call any asserts in those functions

    @return TRUE for success

    @see CleanupAll
*/
nb_boolean
InitializeAll()
{
    NB_Error nbResult = NE_OK;
    NB_RequestHandlerCallback callbackData = { &RequestHandlerCallback, NULL };

    nb_boolean result = CreatePalAndContext(&g_pPal, &g_pContext);
    CU_ASSERT(result);
    if (!result)
    {
        return result;
    }

    // Create search handler
    nbResult = NB_SearchHandlerCreate(g_pContext, &callbackData, &g_SearchState.pSearchHandler);
    CU_ASSERT_EQUAL(nbResult, NE_OK);

    if(nbResult != NE_OK)
    {
        return FALSE;
    }

    return TRUE;
}

/*! Cleanup PAL, context.

    @return None

    @see InitializeAll
*/
void
CleanupAll()
{
    if (g_SearchState.pSearchHandler)
    {
        // Free search handler
        NB_Error result = NB_SearchHandlerDestroy(g_SearchState.pSearchHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        g_SearchState.pSearchHandler = NULL;
    }

    if (g_SearchState.pSearchInformation)
    {
        // Free search information
        NB_Error result = NB_SearchInformationDestroy(g_SearchState.pSearchInformation);
        CU_ASSERT_EQUAL(result, NE_OK);
        g_SearchState.pSearchInformation = NULL;
    }

    if (g_pContext)
    {
        DestroyContext(g_pContext);
        g_pContext = NULL;
    }

    if (g_pPal)
    {
        PAL_DestroyInstance(g_pPal);
        g_pPal = NULL;
    }
}

/*! Cleanup the search parameters.

    Checks the result and resets the information

    @return None. Assert on failure
*/
void
FreeParameters(NB_SearchParameters** parameters)
{
    if (*parameters)
    {
        // Free search parameters
        NB_Error result = NB_SearchParametersDestroy(*parameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        *parameters = NULL;
    }
}

/*! Cleanup the search information.

    Checks the result and resets the information

    @return None. Assert on failure
*/
void
FreeInformation(NB_SearchInformation** information)
{
    if (*information)
    {
        // Free search information
        NB_Error result = NB_SearchInformationDestroy(*information);
        CU_ASSERT_EQUAL(result, NE_OK);
        *information = NULL;
    }
}

void
TestPublicTransitCreateWithoutContext(void)
{
    NB_PublicTransitManager* ptm = NULL;
    NB_Error result = NE_OK;

    result = NB_PublicTransitManagerCreate(NULL, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(ptm);

    if(result == NE_OK && ptm != NULL)
    {
        result = NB_PublicTransitManagerDestroy(ptm);
        CU_ASSERT_EQUAL(result, NE_OK);
    }
}

void
TestPublicTransitCreateWithContext(void)
{
    NB_PublicTransitManager* ptm = NULL;
    NB_Error result = NE_OK;

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result == NE_OK && ptm != NULL)
    {
        result = NB_PublicTransitManagerDestroy(ptm);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    CleanupAll();
}

void
TestPublicTransitDestroyWithoutPTM(void)
{
    NB_PublicTransitManager* ptm = NULL;
    NB_Error result = NE_OK;

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result == NE_OK && ptm != NULL)
    {
        result = NB_PublicTransitManagerDestroy(NULL);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_PublicTransitManagerDestroy(ptm);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    CleanupAll();
}

void
TestPublicTransitDestroyWithPTM(void)
{
    NB_PublicTransitManager* ptm = NULL;
    NB_Error result = NE_OK;

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result == NE_OK && ptm != NULL)
    {
        result = NB_PublicTransitManagerDestroy(ptm);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    CleanupAll();
}

void
TestPublicTransitRequestStopsWithoutPTM(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_PublicTransitRequestCallback callbackDataPT = {0};

        callbackDataPT.callback = &RequestStopsWithoutPTMCallback;
        callbackDataPT.callbackData = NULL;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerRequestStops(NULL, id, &callbackDataPT);
            CU_ASSERT_EQUAL(result, NE_INVAL);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitRequestStopsWithIncorrectPlaceID(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_PublicTransitRequestCallback callbackDataPT = {0};

        callbackDataPT.callback = &RequestStopsWithIncorrectPlaceIDCallback;
        callbackDataPT.callbackData = ptm;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerRequestStops(ptm, id + slice, &callbackDataPT);
            CU_ASSERT_EQUAL(result, NE_INVAL);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitRequestStops(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_PublicTransitRequestCallback callbackDataPT = {0};

        callbackDataPT.callback = &RequestStopsCallback;
        callbackDataPT.callbackData = ptm;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerRequestStops(ptm, id, &callbackDataPT);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (! WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
            {
                CU_FAIL(L"Download timed out!");
                break;
            }
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitCancelRequestsWithoutPTM(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_PublicTransitRequestCallback callbackDataPT = {0};

        callbackDataPT.callback = &RequestStopsCancelRequestsCallback;
        callbackDataPT.callbackData = ptm;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerRequestStops(ptm, id, &callbackDataPT);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerCancelRequests(NULL);
            CU_ASSERT_EQUAL(result, NE_INVAL);

            result = NB_PublicTransitManagerCancelRequests(ptm);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitCancelRequestsWithoutRequest(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_PublicTransitRequestCallback callbackDataPT = {0};

        callbackDataPT.callback = &RequestStopsCancelRequestsCallback;
        callbackDataPT.callbackData = ptm;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerCancelRequests(NULL);
            CU_ASSERT_EQUAL(result, NE_INVAL);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitCancelRequests(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_PublicTransitRequestCallback callbackDataPT = {0};

        callbackDataPT.callback = &RequestStopsCancelRequestsCallback;
        callbackDataPT.callbackData = ptm;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerRequestStops(ptm, id, &callbackDataPT);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerCancelRequests(ptm);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitGetCurrentStopInfoWithoutPTM(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};
            NB_PublicTransitPlace* transitPlace = NULL;

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerGetCurrentStopInfo(NULL, id, &transitPlace);
            CU_ASSERT_EQUAL(result, NE_INVAL);
            CU_ASSERT_PTR_NULL(transitPlace);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitGetCurrentStopInfoWithIncorrectPlaceID(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};
            NB_PublicTransitPlace* transitPlace = NULL;

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerGetCurrentStopInfo(ptm, id+slice, &transitPlace);
            CU_ASSERT_EQUAL(result, NE_NOENT);
            CU_ASSERT_PTR_NULL(transitPlace);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitGetCurrentStopInfo(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};
            NB_PublicTransitPlace* transitPlace = NULL;

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerGetCurrentStopInfo(ptm, id, &transitPlace);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(transitPlace);
            NB_PublicTransitManagerReleaseTransitPlace(transitPlace);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitGetHoursInformationWithoutPTM(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlace* transitPlace = NULL;
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerGetCurrentStopInfo(ptm, id, &transitPlace);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(transitPlace);

/*
TODO: NB_PublicTransitManagerGetHoursInformation() have been removed. See CL#381119 for details
Need to redevelop this test using new function specification.
            if (transitPlace != NULL)
            {
                for (i = 0; i < transitPlace->countOfStops; i++)
                {
                    NB_PublicTransitHours* pHours = NULL;
                    int countOfHoursRecords = 0;
                    NB_PublicTransitStop* transitStop = transitPlace->stops + i;
                    uint32 stopID = transitStop->stopId;
                    result = NB_PublicTransitManagerGetHoursInformation(NULL, stopID, &pHours, &countOfHoursRecords);
                    CU_ASSERT_EQUAL(result, NE_INVAL);
                    if(pHours)
                    {
                        nsl_free(pHours);
                    }
                }
            }
*/

            NB_PublicTransitManagerReleaseTransitPlace(transitPlace);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitGetHoursInformationWithIncorrectStopID(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlace* transitPlace = NULL;
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerGetCurrentStopInfo(ptm, id, &transitPlace);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(transitPlace);

/*
TODO: NB_PublicTransitManagerGetHoursInformation() have been removed. See CL#381119 for details
Need to redevelop this test using new function specification.
            if (transitPlace != NULL)
            {
                for (i = 0; i < transitPlace->countOfStops; i++)
                {
                    NB_PublicTransitHours* pHours = NULL;
                    int countOfHoursRecords = 0;
                    NB_PublicTransitStop* transitStop = transitPlace->stops + i;
                    uint32 stopID = transitStop->stopId;
                    result = NB_PublicTransitManagerGetHoursInformation(ptm, 0xFFFFFFFF-stopID, &pHours, &countOfHoursRecords);
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                    if(pHours)
                    {
                        nsl_free(pHours);
                    }
                }
            }
*/

            NB_PublicTransitManagerReleaseTransitPlace(transitPlace);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitGetHoursInformation(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlace* transitPlace = NULL;
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerGetCurrentStopInfo(ptm, id, &transitPlace);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(transitPlace);

/*
TODO: NB_PublicTransitManagerGetHoursInformation() have been removed. See CL#381119 for details
Need to redevelop this test using new function specification.
            if (transitPlace != NULL)
            {
                for (i = 0; i < transitPlace->countOfStops; i++)
                {
                    NB_PublicTransitHours* pHours = NULL;
                    int countOfHoursRecords = 0;
                    NB_PublicTransitStop* transitStop = transitPlace->stops + i;
                    uint32 stopID = transitStop->stopId;
                    result = NB_PublicTransitManagerGetHoursInformation(ptm, stopID, &pHours, &countOfHoursRecords);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if(pHours)
                    {
                        nsl_free(pHours);
                    }
                }
            }
*/

            NB_PublicTransitManagerReleaseTransitPlace(transitPlace);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitAddTransitStopWithoutPTM(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(NULL, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_INVAL);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitAddTransitStopWithoutPlace(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, NULL, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_INVAL);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitAddTransitStopWithoutTransitStop(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, NULL, &id);
            CU_ASSERT_EQUAL(result, NE_INVAL);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitAddTransitStop(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    uint32 id = 0;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        int resultIndex = 0;
        int count = 0;

        NB_SearchInformationGetResultCount(information, &count);

        // For all results in this slice. Could be less then requested number
        for (resultIndex = 0; resultIndex < slice; resultIndex++)
        {
            // Structures for results of request
            NB_Place place = {0};
            NB_PublicTransitPlaceInfo placeInfo = {0};

            result = NB_SearchInformationGetPublicTransitPlace(information, resultIndex, &placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationGetPlace(information, resultIndex, &place, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_PublicTransitManagerAddTransitStop(ptm, &place, &placeInfo, &id);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestPublicTransitRequestTransitOverlayStopsWithoutTransitType(void)
{
    PublicTransitRequestTransitOverlayStops(NULL, 0);
}

void
TestPublicTransitRequestTransitOverlayStopsWithAllTransitType(void)
{
    const char* transitTypes[1] = {"All"};
    PublicTransitRequestTransitOverlayStops(transitTypes, 1);
}

void
TestPublicTransitRequestTransitOverlayStopsWithTransitTypes(void)
{
    const char* transitTypes[] = {"Tram", "Metro", "Rail", "Bus", "Ferry", "Cable car"};
    PublicTransitRequestTransitOverlayStops(transitTypes, 6);
}

void
TestPublicTransitCreatePlanTrip(void)
{
    NB_PublicTransitManager* ptm = NULL;
    NB_Place start = {0};
    NB_Place end = {0};
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }
    if (NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm) || !ptm)
    {
        return;
    }

    start.location.latitude = PlanTripDestinationStart.latitude;
    start.location.longitude = PlanTripDestinationStart.longitude;
    end.location.latitude = PlanTripDestinationEnd.latitude;
    end.location.longitude = PlanTripDestinationEnd.longitude;

    result = NB_PublicTransitManagerCreatePlanTrip(ptm, &start, &end, 30, PlanTripCallBack, ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (! WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
    {
        CU_FAIL(L"Download timed out!");
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CleanupAll();
}

void
TestPublicTransitCreatePlanTripWithoutPTM(void)
{
    NB_PublicTransitManager* ptm = NULL;
    NB_Place start = {0};
    NB_Place end = {0};
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }
    if (NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm) || !ptm)
    {
        return;
    }

    start.location.latitude = PlanTripDestinationStart.latitude;
    start.location.longitude = PlanTripDestinationStart.longitude;
    end.location.latitude = PlanTripDestinationEnd.latitude;
    end.location.longitude = PlanTripDestinationEnd.longitude;

    result = NB_PublicTransitManagerCreatePlanTrip(NULL, &start, &end, 30, PlanTripCallBack, NULL);
    CU_ASSERT_EQUAL(result, NE_INVAL);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CleanupAll();
}

void
TestPublicTransitCreatePlanTripWithoutStartPlace(void)
{
    NB_PublicTransitManager* ptm = NULL;
    NB_Place end = {0};
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }
    if (NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm) || !ptm)
    {
        return;
    }

    end.location.latitude = PlanTripDestinationEnd.latitude;
    end.location.longitude = PlanTripDestinationEnd.longitude;

    result = NB_PublicTransitManagerCreatePlanTrip(ptm, NULL, &end, 30, PlanTripCallBack, ptm);
    CU_ASSERT_EQUAL(result, NE_INVAL);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CleanupAll();
}

void
TestPublicTransitCreatePlanTripWithoutEndPlace(void)
{
    NB_PublicTransitManager* ptm = NULL;
    NB_Place start = {0};
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }
    if (NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm) || !ptm)
    {
        return;
    }

    start.location.latitude = PlanTripDestinationStart.latitude;
    start.location.longitude = PlanTripDestinationStart.longitude;

    result = NB_PublicTransitManagerCreatePlanTrip(ptm, &start, NULL, 30, PlanTripCallBack, ptm);
    CU_ASSERT_EQUAL(result, NE_INVAL);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CleanupAll();
}

void
TestSearchInformationGetPublicTransitPlaceWithoutInformation(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};
    NB_PublicTransitPlaceInfo placeInfo = {0};


    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        result = NB_SearchInformationGetPublicTransitPlace(NULL, 0, &placeInfo);
        CU_ASSERT_EQUAL(result, NE_INVAL);

        result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestSearchInformationGetPublicTransitPlaceWithIncorrectIndex(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};
    NB_PublicTransitPlaceInfo placeInfo = {0};


    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        result = NB_SearchInformationGetPublicTransitPlace(information, -1, &placeInfo);
        CU_ASSERT_EQUAL(result, NE_NOENT);

        result = NB_SearchInformationGetPublicTransitPlace(information, 1000, &placeInfo);
        CU_ASSERT_EQUAL(result, NE_NOENT);

        result = NB_SearchInformationReleasePublicTransitPlace(&placeInfo);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestSearchInformationReleasePublicTransitPlaceWithoutPlace(void)
{
    const int NUMBER_PER_SLICE = 5;
    int slice = 0;
    nb_boolean hasMoreResults = FALSE;
    NB_SearchResultType type = NB_SRT_None;
    const char* categoryCodes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
    CU_ASSERT_PTR_NOT_NULL(information);

    if (information)
    {
        result = NB_SearchInformationReleasePublicTransitPlace(NULL);
        CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitWithoutContext(void)
{
    const int NUMBER_PER_SLICE = 5;
    const char* categoryCodes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(NULL, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitWithoutRegion(void)
{
    const int NUMBER_PER_SLICE = 5;
    const char* categoryCodes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, NULL, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitWithoutName(void)
{
    const int NUMBER_PER_SLICE = 5;
    const char* categoryCodes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, NULL, categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitWithoutCategoryCodes(void)
{
    const int NUMBER_PER_SLICE = 5;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", NULL, 0, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitWithIncorrectCodeCount(void)
{
    const int NUMBER_PER_SLICE = 5;
    const char* categoryCodes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, -1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitWithIncorrectSliceSize(void)
{
    const char* categoryCodes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, 0, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitWithoutLanguage(void)
{
    const int NUMBER_PER_SLICE = 5;
    const char* categoryCodes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, NULL, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitWithoutParameters(void)
{
    const int NUMBER_PER_SLICE = 5;
    const char* categoryCodes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    categoryCodes[0] = "PT";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransit(g_pContext, &region, "", categoryCodes, 1, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, NULL);
    CU_ASSERT_EQUAL(result, NE_INVAL);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitOverlayWithoutContext(void)
{
    const int NUMBER_PER_SLICE = 5;
    const char* transitTypes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    transitTypes[0] = "All";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransitOverlay(NULL, &region, "", transitTypes, 1, 0, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitOverlayWithoutRegion(void)
{
    const int NUMBER_PER_SLICE = 5;
    const char* transitTypes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    transitTypes[0] = "All";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransitOverlay(g_pContext, NULL, "", transitTypes, 1, 0, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitOverlayWithoutName(void)
{
    const int NUMBER_PER_SLICE = 5;
    const char* transitTypes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    transitTypes[0] = "All";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransitOverlay(g_pContext, &region, NULL, transitTypes, 1, 0, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitOverlayWithoutTransitType(void)
{
    const int NUMBER_PER_SLICE = 5;

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransitOverlay(g_pContext, &region, "", NULL, 0, 0, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitOverlayWithoutLanguage(void)
{
    const int NUMBER_PER_SLICE = 5;
    const char* transitTypes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    transitTypes[0] = "All";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransitOverlay(g_pContext, &region, "", transitTypes, 1, 0, NUMBER_PER_SLICE, NULL, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_PTR_NULL(searchParameters);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    FreeParameters(&searchParameters);
    CleanupAll();
}

void
TestSearchParametersCreatePublicTransitOverlayWithoutParameters(void)
{
    const int NUMBER_PER_SLICE = 5;
    const char* transitTypes[1] = {0};

    NB_PublicTransitManager* ptm = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude  = TestingDestination.latitude;
    region.center.longitude = TestingDestination.longitude;

    transitTypes[0] = "All";

    result = InitializeAll();
    CU_ASSERT_EQUAL(result, TRUE);

    if(result == FALSE)
    {
        return;
    }

    result = NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(ptm);

    if(result || !ptm)
    {
        return;
    }

    result = NB_SearchParametersCreatePublicTransitOverlay(g_pContext, &region, "", transitTypes, 1, 0, NUMBER_PER_SLICE, Test_OptionsGet()->currentLanguage, NULL);
    CU_ASSERT_EQUAL(result, NE_INVAL);

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);

    CleanupAll();
}
void
TestPublicTransitRequestPlanStops(void)
{
    NB_PublicTransitManager* ptm = NULL;
    NB_Place start = {0};
    NB_Place end = {0};
    NB_Error err = NE_OK;

    if (!InitializeAll())
    {
        return;
    }
    if (NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm) || !ptm)
    {
        return;
    }

    start.location.latitude = PlanTripDestinationStart.latitude;
    start.location.longitude = PlanTripDestinationStart.longitude;
    end.location.latitude = PlanTripDestinationEnd.latitude;
    end.location.longitude = PlanTripDestinationEnd.longitude;

    err = NB_PublicTransitManagerCreatePlanTrip(ptm, &start, &end, 30, RequestPlanStopsPlanTripCallBack, ptm);
    CU_ASSERT_EQUAL(err, NE_OK);
    if (! WaitForCallbackCompletedEvent(g_CallbackEvent, 60000)) 
    {
        CU_FAIL(L"Download timed out!");
    }

    err = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(err, NE_OK);
    CleanupAll();
}
/*! Callback for all downloads.

    @return None
*/
void
RequestHandlerCallback(void* handler,
                       NB_RequestStatus status,
                       NB_Error err,
                       uint8 up,
                       int percent,
                       void* userData)
{
    if (err != NE_OK)
    {
        // Abort and trigger event
        CU_FAIL("Callback returned error");
        LOGOUTPUT(LogLevelLow, ("Callback error = %d, status = %d\n", err, status));
        SetCallbackCompletedEvent(g_CallbackEvent);
        return;
    }

    LOGOUTPUT(LogLevelHigh, ("POI Download progress: %d\n", percent));

    if (percent != 100)
    {
        // The status should be set to progress
        CU_ASSERT_EQUAL(status, NB_NetworkRequestStatus_Progress);
    }
    else
    {
        NB_Error local_error = NB_SearchHandlerGetSearchInformation(handler, &g_SearchState.pSearchInformation);
        if (local_error != NE_OK || g_SearchState.pSearchInformation == NULL)
        {
            // Abort and trigger event
            CU_FAIL("Failed to retrieve search information.");
            SetCallbackCompletedEvent(g_CallbackEvent);
            return;
        }

        CU_ASSERT_EQUAL(status, NB_NetworkRequestStatus_Success);

        // Trigger main thread
        SetCallbackCompletedEvent(g_CallbackEvent);
        return;
    }
}

void
RequestStopsCallback(NB_RequestStatus status,
                     NB_Error err,
                     uint32* places,
                     uint32 countOfPlaces,
                     void* userData)
{
    int i = 0;
    NB_PublicTransitManager *ptManager = (NB_PublicTransitManager*)userData;

    CU_ASSERT_EQUAL(err, NE_OK);
    if (status == NB_NetworkRequestStatus_Progress)
    {
        return;
    }

    CU_ASSERT_EQUAL(status, NB_NetworkRequestStatus_Success);

    if (err == NE_OK && status == NB_NetworkRequestStatus_Success)
    {
        CU_ASSERT_PTR_NOT_NULL(ptManager);
        if(ptManager == NULL)
        {
            SetCallbackCompletedEvent(g_CallbackEvent);
            return;
        }

        for(i = 0; i < (int)countOfPlaces; i++)
        {
            NB_PublicTransitPlace* transitPlace = NULL;
            uint32 placeID = *(places + i);
            err = NB_PublicTransitManagerGetCurrentStopInfo(ptManager, placeID, &transitPlace);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(transitPlace);
            NB_PublicTransitManagerReleaseTransitPlace(transitPlace);
        }
    }
    SetCallbackCompletedEvent(g_CallbackEvent);
}

void RequestStopsCancelRequestsCallback(NB_RequestStatus status,
                                          NB_Error err,
                                          uint32* places,
                                          uint32 countOfPlaces,
                                          void* userData)
{

}

void
RequestStopsWithoutPTMCallback(NB_RequestStatus status,
                    NB_Error err,
                    uint32* places,
                    uint32 countOfPlaces,
                    void* userData)
{
    SetCallbackCompletedEvent(g_CallbackEvent);
}

void
RequestStopsWithIncorrectPlaceIDCallback(NB_RequestStatus status,
                    NB_Error err,
                    uint32* places,
                    uint32 countOfPlaces,
                    void* userData)
{
    SetCallbackCompletedEvent(g_CallbackEvent);
}

void
RequestStopInfoWithIncorrectPlaceIDCallback(void* userData, NB_PublicTransitPlace* place)
{
    NB_PublicTransitManager *ptManager = NULL;

    CU_ASSERT_PTR_NOT_NULL(place);

    ptManager = (NB_PublicTransitManager*)userData;
    CU_ASSERT_PTR_NOT_NULL(ptManager);

    if(place && ptManager)
    {
        //ToDo
        //CU_ASSERT(place->isTransitPlaceComplete);
        //NB_PublicTransitManagerReleaseTransitPlace(place);
    }
    NB_PublicTransitManagerReleaseTransitPlace(place);
    SetCallbackCompletedEvent(g_CallbackEvent);
}

void
RequestStopInfoCallback(void* userData, NB_PublicTransitPlace* place)
{
    NB_PublicTransitManager *ptManager = NULL;

    CU_ASSERT_PTR_NOT_NULL(place);

    ptManager = (NB_PublicTransitManager*)userData;
    CU_ASSERT_PTR_NOT_NULL(ptManager);
    if(ptManager == NULL)
    {
        SetCallbackCompletedEvent(g_CallbackEvent);
        return;
    }

    if(place && ptManager)
    {
        CU_ASSERT(place->isTransitPlaceComplete);
    }

    NB_PublicTransitManagerReleaseTransitPlace(place);
    SetCallbackCompletedEvent(g_CallbackEvent);
}

void
PublicTransitRequestTransitOverlayStops(const char* transitTypes[], int transitTypeCount)
{
    NB_PublicTransitManager* ptm = NULL;
    NB_Error result = NE_OK;
    NB_BoundingBox boundingBox = {0};
    NB_PublicTransitRequestCallback callBack = {0};

    if (!InitializeAll())
    {
        return;
    }
    if (NB_PublicTransitManagerCreate(g_pContext, Test_OptionsGet()->currentLanguage, &ptm) || !ptm)
    {
        return;
    }

    boundingBox.topLeft.latitude = PlanTripDestinationStart.latitude;
    boundingBox.topLeft.longitude = PlanTripDestinationStart.longitude;
    boundingBox.bottomRight.latitude = PlanTripDestinationEnd.latitude;
    boundingBox.bottomRight.longitude = PlanTripDestinationEnd.longitude;

    callBack.callback = RequestTransitOverlayStopsCallback;
    callBack.callbackData = ptm;

    result = NB_PublicTransitManagerRequestTransitOverlayStops(ptm, &boundingBox, transitTypes, transitTypeCount, &callBack);
    if (transitTypes == NULL)
    {
        CU_ASSERT_EQUAL(result, NE_INVAL);
    }
    else
    {
        CU_ASSERT_EQUAL(result, NE_OK);
        if (! WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
        {
            CU_FAIL(L"Download timed out!");
        }
    }

    result = NB_PublicTransitManagerDestroy(ptm);
    CU_ASSERT_EQUAL(result, NE_OK);
    CleanupAll();
}

void
RequestTransitOverlayStopsCallback(NB_RequestStatus status, NB_Error err, uint32* places, uint32 countOfPlaces, void* userData)
{
    int i = 0;
    NB_PublicTransitManager *ptManager = (NB_PublicTransitManager*)userData;

    CU_ASSERT_EQUAL(err, NE_OK);
    if (status == NB_NetworkRequestStatus_Progress)
    {
        return;
    }

    CU_ASSERT_EQUAL(status, NB_NetworkRequestStatus_Success);

    if (err == NE_OK && status == NB_NetworkRequestStatus_Success)
    {
        CU_ASSERT_PTR_NOT_NULL(ptManager);
        if(ptManager == NULL)
        {
            SetCallbackCompletedEvent(g_CallbackEvent);
            return;
        }

        for(i = 0; i < (int)countOfPlaces; i++)
        {
            NB_PublicTransitPlace* transitPlace = NULL;
            uint32 placeID = *(places + i);
            err = NB_PublicTransitManagerGetCurrentStopInfo(ptManager, placeID, &transitPlace);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(transitPlace);
            NB_PublicTransitManagerReleaseTransitPlace(transitPlace);
        }
    }
    SetCallbackCompletedEvent(g_CallbackEvent);
}

void
PlanTripCallBack(NB_Error err,
                 uint32* startTransitPlaceList,
                 int startListCount,
                 uint32* endTransitPlaceList,
                 int endListCount,
                 void* cbData)
{
    int i = 0;
    NB_PublicTransitManager *ptManager = (NB_PublicTransitManager*)cbData;
    CU_ASSERT_EQUAL(err, NE_OK);
    if (err == NE_OK)
    {
        for (i = 0; i < startListCount; i++)
        {
            NB_PublicTransitPlace* transitPlace = NULL;
            uint32 placeID = *(startTransitPlaceList + i);
            err = NB_PublicTransitManagerGetCurrentStopInfo(ptManager, placeID, &transitPlace);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(transitPlace);
            NB_PublicTransitManagerReleaseTransitPlace(transitPlace);
        }

        for (i = 0; i < endListCount; i++)
        {
            NB_PublicTransitPlace* transitPlace = NULL;
            uint32 placeID = *(endTransitPlaceList + i);
            err = NB_PublicTransitManagerGetCurrentStopInfo(ptManager, placeID, &transitPlace);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(transitPlace);
            NB_PublicTransitManagerReleaseTransitPlace(transitPlace);
        }
    }

    nsl_free(startTransitPlaceList);
    nsl_free(endTransitPlaceList);

    SetCallbackCompletedEvent(g_CallbackEvent);
}

void RequestPlanStopsPlanTripCallBack(NB_Error err, uint32* startTransitPlaceList, int startListCount, uint32* endTransitPlaceList, int endListCount, void* cbData)
{
    NB_PublicTransitManager *ptManager = (NB_PublicTransitManager*)cbData;
    CU_ASSERT_EQUAL(err, NE_OK);

    if (err == NE_OK)
    {
        if (startListCount || endListCount)
        {
            NB_PublicTransitRequestCallback callBack = {0};
            int transitCount = startListCount + endListCount;
            uint32 * placeIdList = nsl_malloc(transitCount * sizeof(uint32));
            CU_ASSERT_PTR_NOT_NULL(placeIdList);
            nsl_memcpy(placeIdList, startTransitPlaceList, sizeof(uint32) * startListCount);
            nsl_memcpy(placeIdList + startListCount, endTransitPlaceList, sizeof(uint32) * endListCount);

            callBack.callbackData = ptManager;
            callBack.callback = RequestPlanStopsCallBack;
            err = NB_PublicTransitManagerRequestPlanStops(ptManager, placeIdList, transitCount, &callBack);
            CU_ASSERT_EQUAL(err, NE_OK);
            nsl_free(placeIdList);
            nsl_free(startTransitPlaceList);
            nsl_free(endTransitPlaceList);
        }
        else
        {
            CU_ASSERT_PTR_NULL(startTransitPlaceList);
            CU_ASSERT_PTR_NULL(endTransitPlaceList);
            SetCallbackCompletedEvent(g_CallbackEvent);
        }
    }
    else
    {
        nsl_free(startTransitPlaceList);
        nsl_free(endTransitPlaceList);
        SetCallbackCompletedEvent(g_CallbackEvent);
    }
}

void RequestPlanStopsCallBack(NB_RequestStatus status, NB_Error err, uint32* places, uint32 countOfPlaces, void* cbData)
{
    NB_PublicTransitManager *ptManager = (NB_PublicTransitManager*)cbData;
    CU_ASSERT_EQUAL(err, NE_OK);
    if (status == NB_NetworkRequestStatus_Progress)
    {
        return;
    }
    CU_ASSERT_EQUAL(status, NB_NetworkRequestStatus_Success);
    if (err == NE_OK && status == NB_NetworkRequestStatus_Success)
    {
        if (places && countOfPlaces)
        {
            int i = 0;
            for (i = 0; i < (int)countOfPlaces; i++)
            {
                uint32 placeID = *(places + i);
                NB_PublicTransitPlace* transitPlace = NULL;
                NB_Error error = NB_PublicTransitManagerGetCurrentStopInfo(ptManager, placeID, &transitPlace);
                CU_ASSERT_EQUAL(error, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(transitPlace);
                NB_PublicTransitManagerReleaseTransitPlace(transitPlace);
            }
        }
    }
    SetCallbackCompletedEvent(g_CallbackEvent);
}

/*! Do a search request.

    Gets called from all tests. Returns the search information once the download has finished.

    @return Search information on success. NULL on failure.

    @see RequestHandlerCallback
*/
NB_SearchInformation*
DoRequest(NB_SearchParameters* parameters,      /*!< The initialized parameters to use for the request */
          int* slice,                           /*!< See parameter description of NB_SearchInformationGetResultInformation() */
          nb_boolean* hasMoreResults,
          NB_SearchResultType* type
          )
{
    NB_SearchInformation* information = NULL;

    // Make the server request
    NB_Error result = NB_SearchHandlerStartRequest(g_SearchState.pSearchHandler, parameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result != NE_OK)
    {
        return NULL;
    }

    // Wait for event here from callback. Timeout 60 seconds
    if (! WaitForCallbackCompletedEvent(g_CallbackEvent, 60000)) 
    {
        CU_FAIL(L"Download timed out!");
        return NULL;
    }

    // The search information gets set in RequestHandlerCallback()
    CU_ASSERT_PTR_NOT_NULL(g_SearchState.pSearchInformation);
    if (g_SearchState.pSearchInformation == NULL)
    {
        return NULL;
    }

    // Get result information
    result = NB_SearchInformationGetResultType(g_SearchState.pSearchInformation, type);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result != NE_OK)
    {
        // Destroy the search information object and return NULL
        NB_SearchInformationDestroy(g_SearchState.pSearchInformation);
        g_SearchState.pSearchInformation = NULL;
        return NULL;
    }

    result = NB_SearchInformationGetResultCount(g_SearchState.pSearchInformation, slice);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_SearchInformationHasMoreResults(g_SearchState.pSearchInformation, hasMoreResults);
    CU_ASSERT_EQUAL(result, NE_OK);

    LOGOUTPUT(LogLevelMedium, ("Result Slice Count: %d, more: %s\n", *slice, *hasMoreResults ? "true" : "false"));

    information = g_SearchState.pSearchInformation;
    g_SearchState.pSearchInformation = NULL;

    // Return the search information object
    return information;
}

/*! @} */

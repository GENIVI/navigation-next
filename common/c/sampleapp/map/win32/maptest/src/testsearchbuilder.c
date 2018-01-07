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

    @file     TestSearchBuilder.c
    @date     04/30/2009
    @defgroup TestSearchBuilder_h
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


#include "testsearchbuilder.h"
#include "main.h"
#include "testnetwork.h"
#include "statictpslib.h"
#include "platformutil.h"
#include "nbanalytics.h"
#include "nbsearchhandler.h"
#include "nbsearchparameters.h"
#include "nbsearchinformation.h"
#include "nbpointsofinterestinformation.h"
#include "nbroutehandler.h"
#include "nbspatial.h"
#include "palclock.h"
#include "palmath.h"
#include "palfile.h"
#include "testprofile.h"
#include "nbpublictransitmanager.h"
#include "nbcontextaccess.h"
#include "testdirection.h"

// Local functions ...............................................................................

// All tests. Add all tests to the TestSearchBuilder_AddTests function
static void TestSearchBuilderSimplePoi(void);
static void TestSearchBuilderTransitStop(void);
static void TestSearchBuilderFuel(void);
static void TestSearchBuilderFuelAllTypes(void);
static void TestSearchBuilderWeather(void);
static void TestSearchBuilderTrafficIncident(void);
static void TestSearchBuilderEvents(void);
static void TestSearchBuilderVenues(void);
static void TestSearchBuilderMovies(void);
static void TestSearchBuilderTheaters(void);
static void TestSearchBuilderEventSubSearchForDifferentRegion(void);
static void TestSearchBuilderMovieSubSearchForDifferentRegion(void);

static void TestSearchBuilderCarouselFuel(void);
static void TestSearchBuilderCarouselWeather(void);
static void TestSearchBuilderCarouselEvents(void);
static void TestSearchBuilderCarouselMovies(void);

static void TestSearchBuilderAlongRouteGas(void);
static void TestSearchBuilderAlongRouteLodging(void);
static void TestSearchBuilderAlongRouteBanks(void);
static void TestSearchBuilderAlongRouteDining(void);
static void TestSearchBuilderAlongRouteTourist(void);
static void TestSearchBuilderAlongRouteEmergency(void);
static void TestSearchBuilderAlongRoutePublicTransit(void);
static void TestSearchBuilderAlongRouteGasDining(void);
static void TestSearchBuilderAlongRouteDiningGas(void);

static void TestSearchBuilderSearchAlongRouteInSegments(void);

static void TestSearchBuilderServerConsistency(void);
static void TestSearchBuilderSearchSpeedCameras(void);

// Premium placement tests
static void TestSearchBuilderPremiumSimplePoi(void);
static void TestSearchBuilderPremiumFuel(void);
static void TestSearchBuilderPremiumWeather(void);
static void TestSearchBuilderPremiumTrafficIncident(void);
static void TestSearchBuilderPremiumServerConsistency(void);

static void TestSearchBuilderIlap(void);

static void TestSearchBuilderMicrosoftSpellingSuggestions(void);
static void TestSearchBuilderMicrosoftSearch(void);

static void TestPremiumSimplePoi(char* poiName, NB_POIExtendedConfiguration premiumFlags, nb_boolean useIlap);
static void TestPremiumFuel(NB_POIExtendedConfiguration premiumFlags);
static void TestPremiumWeather(NB_POIExtendedConfiguration premiumFlags);
static void TestPremiumTrafficIncident(NB_POIExtendedConfiguration premiumFlags);
static void TestPremiumServerConsistency(char* searchForName, char* categoryCodes[], int categoryCodesCount, NB_POIExtendedConfiguration premiumFlags);

static void SavePlaceNoPremiumSearch(const NB_Place* pNewPlace);
static void SavePlacePremiumSearch(const NB_Place* pNewPlace, const NB_ExtendedPlace* pExtendedPlace);


// Initialize/Cleanup functions
static nb_boolean InitializeAll();
static void CleanupAll();
static void FreeInformation(NB_SearchInformation** information);
static void FreeParameters(NB_SearchParameters** parameters);

static NB_SearchInformation* DoRequest(NB_SearchParameters* parameters, int* slice, nb_boolean* hasMoreResults, NB_SearchResultType* type);
static NB_Error FormattedTextCallback(NB_Font font, nb_color color, const char* text, uint8 newline, void* userData);

static void EventSubsearch(NB_SearchParameters* parameters);

static NB_RouteInformation* CreateRouteToSearchAlong(NB_LatitudeLongitude* origin, NB_LatitudeLongitude* destination);
static void RouteHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);

static void SearchAlongEntireRoute(char* iconName, char* categoryCodes[], int categoryCount);

// Subfunctions of consistency test
static int ConsistencyTest(int numberPerSlice);
static int ConsistencyTestPremium(int numberPerSlice, char* searchForName, char* categoryCodes[], int categoryCodesCount, NB_POIExtendedConfiguration premiumFlags);
static void CheckPlaceConsistency(const NB_Place* pNewPlace);
static void CheckPlaceConsistencyPremium(void);
static void OutputConsistencyResults();

// Output functions
static void OutputPoiResults(int index, double distance, const NB_Place* pPlace, const NB_ExtendedPlace* pExtendedPlace);
static void OutputFormattedString(const char* pName, const char* pValue);
static void OutputFuelSummary(const NB_FuelSummary* summary);
static void OutputFuelDetails(const NB_FuelDetails* details);
static void OutputWeatherResults(const NB_WeatherConditions* conditions, const NB_WeatherForecasts* forecasts);
static void OutputEventSummary(int resultIndex, const NB_SummaryResult* summary);
static void OutputEvents(int resultIndex, int eventCount, NB_Event* eventArray);
static void OutputTrafficIncident(int resultIndex, const NB_TrafficIncident* incident);

static PAL_File* Test_StartKML(PAL_Instance* pal, const char* displayName, const char* kmlFileName);
static PAL_Error Test_WriteRoutePolylineToKML(PAL_File* kmlFile, NB_RouteInformation* route);
static PAL_Error Test_WritePOIsToKML(PAL_File* kmlFile, int sliceOffset, const char* image, NB_SearchInformation* searchInformation);
static void Test_FinishKML(PAL_File* kmlFile);
// Local variables ...............................................................................

// Get initialized in TestSearchBuilder_SuiteSetup()

typedef struct  
{
    NB_SearchHandler*       pSearchHandler;
    NB_SearchInformation*   pSearchInformation;
} SearchTestState;

static PAL_Instance* g_pPal = NULL;
static NB_Context* g_pContext = NULL;
static SearchTestState g_SearchState = {0};

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;

// In order to check for consistency with the returned results we make an array with all the places
// and then compare new entries to all existing results. Not very efficient but this is a system test.
// We store the most important place information as one single string so that we can easily compare it.
#define MAX_PLACES 500
#define MAX_PLACE_STRING 500
static char g_allPlaces[MAX_PLACES][MAX_PLACE_STRING];
static int  g_placeCount = 0;

static char g_allPlacesPremium[MAX_PLACES][MAX_PLACE_STRING];
static int  g_placeCountPremium = 0;



// Public Functions ..............................................................................

/*! Add all your test functions here.

    @return none
*/
void
TestSearchBuilder_AddTests( CU_pSuite pTestSuite, int level )
{
    // Normal Search Test without requesting premium placements

    //Comments out test cases for public transit feature which are not available on QA server now. Tested them on guru private server.
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderSimplePoi",                   TestSearchBuilderSimplePoi);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderFuel",                        TestSearchBuilderFuel);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderWeather",                     TestSearchBuilderWeather);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderTrafficIncident",             TestSearchBuilderTrafficIncident);

    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderFuelTypeAll",                 TestSearchBuilderFuelAllTypes);

    // Carousel-lite search queries
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderCarouselFuel",                TestSearchBuilderCarouselFuel);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderCarouselWeather",             TestSearchBuilderCarouselWeather);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderCarouselEvents",              TestSearchBuilderCarouselEvents);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderCarouselMovies",              TestSearchBuilderCarouselMovies);

    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderAlongRouteGas",               TestSearchBuilderAlongRouteGas);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderAlongRouteLodging",           TestSearchBuilderAlongRouteLodging);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderAlongRouteBanks",             TestSearchBuilderAlongRouteBanks);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderAlongRouteDining",            TestSearchBuilderAlongRouteDining);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderAlongRouteTourist",           TestSearchBuilderAlongRouteTourist);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderAlongRouteEmergency",         TestSearchBuilderAlongRouteEmergency);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderAlongRoutePublicTransit",     TestSearchBuilderAlongRoutePublicTransit);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderAlongRouteGasDining",         TestSearchBuilderAlongRouteGasDining);
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderAlongRouteDiningGas",         TestSearchBuilderAlongRouteDiningGas);

    ADD_TEST(level, TestLevelIntermediate,  pTestSuite, "TestSearchBuilderSearchAlongRouteInSegments",  TestSearchBuilderSearchAlongRouteInSegments);
    ADD_TEST(level, TestLevelIntermediate,  pTestSuite, "TestSearchBuilderSearchSpeedCameras",          TestSearchBuilderSearchSpeedCameras);

    ADD_TEST(level, TestLevelIntermediate,  pTestSuite, "TestSearchBuilderEvents",                      TestSearchBuilderEvents);
    ADD_TEST(level, TestLevelIntermediate,  pTestSuite, "TestSearchBuilderVenues",                      TestSearchBuilderVenues);
    ADD_TEST(level, TestLevelIntermediate,  pTestSuite, "TestSearchBuilderMovies",                      TestSearchBuilderMovies);
    ADD_TEST(level, TestLevelIntermediate,  pTestSuite, "TestSearchBuilderTheaters",                    TestSearchBuilderTheaters);
    ADD_TEST(level, TestLevelIntermediate,  pTestSuite, "TestSearchBuilderEventSubSearchForDifferentRegion",    TestSearchBuilderEventSubSearchForDifferentRegion);
    ADD_TEST(level, TestLevelIntermediate,  pTestSuite, "TestSearchBuilderMovieSubSearchForDifferentRegion",    TestSearchBuilderMovieSubSearchForDifferentRegion);

    /*
        @todo:
        The current server design makes the consistency test fail. The server team has currently no plan on fixing this. See
        related bug numbers in the description of TestSearchBuilderServerConsistency(). For now we will disable this test
        to avoid getting flooded with errors (the test reports hundreds of errors). One the server fixes this then we should
        revisit this test.
    */
    //ADD_TEST(level, TestLevelFull,          pTestSuite, "TestSearchBuilderServerConsistency",   TestSearchBuilderServerConsistency);

    // Search with premium placements
    ADD_TEST(level, TestLevelSmoke,         pTestSuite, "TestSearchBuilderPremiumSimplePoi",         TestSearchBuilderPremiumSimplePoi);    

    /*
        This test is totally overkill (IMO). It takes 4 times as long as TestSearchBuilderServerConsistency() and (I assume)
        will throw 4 times as many errors (if it ever finishes). Comment out for now. See comments above.
    */
    //ADD_TEST(level, TestLevelFull,          pTestSuite, "TestSearchBuilderPremiumServerConsistency", TestSearchBuilderPremiumServerConsistency);

    if (Test_OptionsGet()->carrier & TestCarrierVerizon)
    {
        ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestSearchBuilderIlap", TestSearchBuilderIlap);
        ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestSearchBuilderMicrosoftSearch", TestSearchBuilderMicrosoftSearch);
        ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestSearchBuilderMicrosoftSpellingSuggestions", TestSearchBuilderMicrosoftSpellingSuggestions);
    }
}

/*! Add common initialization code here.

    @return 0

    @see TestSearchBuilder_SuiteCleanup
*/
int 
TestSearchBuilder_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
	return 0;
}

/*! Add common cleanup code here.

    @return 0

    @see TestSearchBuilder_SuiteSetup
*/
int 
TestSearchBuilder_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
	return 0;
}


// Local functions ...............................................................................

/*! Test a simple point of interest search.

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderSimplePoi(void)
{
    const int NUMBER_PER_SLICE = 5;
    const int SLICES_TO_DOWNLOAD = 5;
    int sliceIndex = 0;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            // Set center to New York
            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 40.47;
            region.center.longitude = -73.58;

            // Create search parameters 
            result = NB_SearchParametersCreatePOI(g_pContext, 
                                                  &region, 
                                                  "Thai Restaurant", 
                                                  NULL,
                                                  NULL, 
                                                  0, 
                                                  NUMBER_PER_SLICE, 
                                                  NB_EXT_None,
                                                  NULL, 
                                                  &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);

            result = NB_SearchParametersSetSource(searchParameters, NB_SS_User);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information)
            {
                int resultIndex = 0;

                // The place type has to be set
                CU_ASSERT(type & NB_SRT_Place);

                // For all results in this slice. Could be less then requested number
                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    // Structures for results of request
                    NB_Place place = {{0}};
                    double distance = 0.0;

                    // Get place information
                    result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, NULL);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        // Output place information
                        OutputPoiResults(resultIndex + sliceIndex * NUMBER_PER_SLICE, distance, &place, NULL );
                    }
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! Test a fuel search.

    @return None. CUnit Asserts get called on failures.
*/
void
DoFuelSearch(NB_FuelSearchType fuelType)
{
    const int NUMBER_PER_SLICE = 5;
    const int SLICES_TO_DOWNLOAD = 5;
    int sliceIndex = 0;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            // Set center of search to Dallas
            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 32.80;
            region.center.longitude = -96.80;

            // Create search parameters for gas stations in Dallas.
            result = NB_SearchParametersCreateFuel(g_pContext, 
                &region, 
                "",
                NULL,
                NULL,
                0,
                fuelType,
                NB_FRT_All,
                NUMBER_PER_SLICE,
                NB_EXT_None,
                NULL,
                &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information)
            {
                int resultIndex = 0;
                NB_FuelSummary summary = {{{0}}};

                // The place, fuel and fuel summary type has to be set
                CU_ASSERT(type & NB_SRT_Place);
                CU_ASSERT(type & NB_SRT_FuelSummary);

                // Get fuel summary information for this slice
                result = NB_SearchInformationGetFuelSummary(information, &summary);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result == NE_OK)
                {
                    OutputFuelSummary(&summary);
                }

                // For all results in this slice. Could be less then requested number
                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    // Structures for results of request
                    NB_Place place = {{0}};
                    NB_FuelDetails details = {{{{0}}}};
                    double distance = 0.0;

                    // Get place information
                    result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, NULL);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        // Output place information
                        OutputPoiResults(resultIndex + sliceIndex * NUMBER_PER_SLICE, distance, &place, NULL);
                    }

                    // Not all of the results have fuel details.
                    if (type & NB_SRT_FuelDetails)
                    {
                        // Get fuel information for this location
                        result = NB_SearchInformationGetFuelDetails(information, resultIndex, &details);
                        CU_ASSERT_EQUAL(result, NE_OK);
                        if (result == NE_OK)
                        {
                            OutputFuelDetails(&details);
                        }
                    }
                    else
                    {
                        LOGOUTPUT(LogLevelHigh, ("   No fuel details.\n"));
                    }
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! Test a fuel search.

    @return None. CUnit Asserts get called on failures.
*/
void
TestSearchBuilderFuel(void)
{
    DoFuelSearch(NB_FST_Regular);
}

/*! Test a fuel search for all fuel types.

    @return None. CUnit Asserts get called on failures.
*/
void
TestSearchBuilderFuelAllTypes(void)
{
    DoFuelSearch(NB_FST_All);
}

/*! Test a weather search.

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderWeather(void)
{
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    // Set center of search to Dallas
    NB_LatitudeLongitude center = { 32.80, -96.80 };

    if (!InitializeAll())
    {
        return;
    }

    // Create search parameters for weather search 
    result = NB_SearchParametersCreateWeather(g_pContext, &center, 5, NB_WRT_All, NB_EXT_None, "", &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    if (result == NE_OK)
    {
        int slice = 0;
        nb_boolean hasMoreResults = FALSE;
        NB_SearchResultType type = NB_SRT_None;

        // Do the server request and download the results
        information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
        CU_ASSERT_PTR_NOT_NULL(information);
        if (information)
        {
            int resultIndex = 0;
            NB_Place place = {{0}};
            NB_WeatherConditions weatherConditions = {0};
            NB_WeatherForecasts weatherForecasts = {{{0}}}; 
            double distance = 0.0;

            // The place and weather type has to be set
            CU_ASSERT(type & NB_SRT_Place);
            CU_ASSERT(type & NB_SRT_Weather);

            // Get place information
            result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                // Output place information
                OutputPoiResults(0, distance, &place, NULL);
            }

            // Get weather information
            result = NB_SearchInformationGetWeather(information, &weatherConditions, &weatherForecasts);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                // Output place information
                OutputWeatherResults(&weatherConditions, &weatherForecasts);
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! Test a traffic incident search.

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderTrafficIncident(void)
{
    const int NUMBER_PER_SLICE = 5;
    const int SLICES_TO_DOWNLOAD = 5;
    int sliceIndex = 0;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            // Set bounding box for New York
            NB_SearchRegion region = {0};
            region.type = NB_ST_BoundingBox;
            region.boundingBox.topLeft.latitude         = 41.0;
            region.boundingBox.topLeft.longitude        = -74.5;
            region.boundingBox.bottomRight.latitude     = 40.0;
            region.boundingBox.bottomRight.longitude    = -73.5;

            // Create search parameters
            result = NB_SearchParametersCreateTrafficIncident(g_pContext, 
                                                              &region, 
                                                              5, 
                                                              NB_TIRT_All, 
                                                              NUMBER_PER_SLICE, 
                                                              NB_EXT_None,
                                                              NULL, 
                                                              &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information && slice > 0)
            {
                int resultIndex = 0;

                // The place and traffic incident type has to be set
                CU_ASSERT(type & NB_SRT_Place);
                CU_ASSERT(type & NB_SRT_TrafficIncident);

                // For all results in this slice. Could be less then requested number
                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    // Structures for results of request
                    NB_Place place = {{0}};
                    NB_TrafficIncident incident = {0};
                    double distance = 0.0;

                    // Get place information
                    result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, NULL);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        // Output place information
                        OutputPoiResults(resultIndex + sliceIndex * NUMBER_PER_SLICE, distance, &place, NULL);
                    }

                    // Get the traffic incident
                    result = NB_SearchInformationGetTrafficIncident(information, resultIndex, &incident, &distance);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        OutputTrafficIncident(resultIndex + sliceIndex * NUMBER_PER_SLICE, &incident);
                    }
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! Test an event search.

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderEvents(void)
{
    const int NUMBER_PER_SLICE = 10;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    // Categories to search for
    #define LOCAL_CATEGORY_COUNT 1
    const char* categories[LOCAL_CATEGORY_COUNT] = { "All" };     

    // Center of search
    NB_SearchRegion region = {0};
    region.type = NB_ST_Center;
    region.center.latitude = 40.47;
    region.center.longitude = -73.58;

    if (!InitializeAll())
    {
        return;
    }

    // Create search parameters for events in New York

    /*
        @bug QA8 is 7 hours ahead of Aliso Viejo local time. And since we can't get events from
        yesterday we have to add 8 hours so that the test works between 5pm and midnight local time.

        The following bug has been filed for this: http://nimble.nimone.com/show_bug.cgi?id=36563
    */

    result = NB_SearchParametersCreateEvent(g_pContext, 
                                            &region,
                                            "",
                                            categories,
                                            LOCAL_CATEGORY_COUNT,
                                            PAL_ClockGetGPSTime() + 8 * 60 * 60,    
                                            PAL_ClockGetGPSTime() + 8 * 60 * 60,
                                            NB_ER_All,
                                            NB_ERT_All,
                                            NUMBER_PER_SLICE,
                                            NB_EXT_None,
                                            "",
                                            &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);
    if (result == NE_OK)
    {
        int slice = 0;
        nb_boolean hasMoreResults = FALSE;
        NB_SearchResultType type = NB_SRT_None;

        // Do the server request and download the results
        information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
        CU_ASSERT_PTR_NOT_NULL(information);
        if (information)
        {
            int resultIndex = 0;
            NB_SearchParameters** parametersArray = NULL;
            int parametersArraySize = 0;

            // The event summary and subsearch type has to be set
            CU_ASSERT(type & NB_SRT_EventSummary);
            CU_ASSERT(type & NB_SRT_SubSearch);

            // Create parameter array for subsearches
            parametersArraySize = sizeof(NB_SearchParameters*) * slice;
            parametersArray = nsl_malloc(parametersArraySize);
            if (parametersArray)
            {
                nsl_memset(parametersArray, 0, parametersArraySize);

                // For all results in this slice. Could be less then requested number
                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    NB_SummaryResult summaryResult = {{{0}}};

                    // The initial search returns event summary information
                    result = NB_SearchInformationGetEventSummary(information, resultIndex, &summaryResult);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        OutputEventSummary(resultIndex, &summaryResult);

                        // Create parameters for subsearch. Save it in our temporary array.
                        result = NB_SearchParametersCreateSubSearch(g_pContext, information, resultIndex, &(parametersArray[resultIndex]));
                        CU_ASSERT_EQUAL(result, NE_OK);
                    }
                }

                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    if (parametersArray[resultIndex])
                    {
                        // Do a subsearch and then free the parameter
                        EventSubsearch(parametersArray[resultIndex]);
                        FreeParameters(&(parametersArray[resultIndex]));
                    }
                }

                nsl_free(parametersArray);
            }
        }
    }

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

/*! Test an event venue search.

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderVenues(void)
{
    const int NUMBER_PER_SLICE = 5;
    const int SLICES_TO_DOWNLOAD = 5;
    int sliceIndex = 0;
    nb_boolean moreSlices = TRUE;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    // Categories to search for
    #define LOCAL_CATEGORY_COUNT 1
    const char* categories[LOCAL_CATEGORY_COUNT] = { "All" };     

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; (sliceIndex < SLICES_TO_DOWNLOAD) && moreSlices; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 47.37;
            region.center.longitude = -122.20;

            // Create search parameters 
            result = NB_SearchParametersCreateEventVenue(g_pContext,
                                                         &region,
                                                         "Casino",
                                                         categories,
                                                         LOCAL_CATEGORY_COUNT,
                                                         PAL_ClockGetGPSTime() + 8 * 60 * 60,    
                                                         PAL_ClockGetGPSTime() + (8 * 60 * 60) + (7 * 24 * 60 * 60),
                                                         NB_ER_All,
                                                         NB_EVRT_All,
                                                         NUMBER_PER_SLICE,
                                                         NB_EXT_None,
                                                         "",
                                                         &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &moreSlices, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information)
            {
                int resultIndex = 0;
                NB_SearchParameters** parametersArray = NULL;
                int parametersArraySize = 0;

                // The place and subsearch type has to be set
                CU_ASSERT(type & NB_SRT_Place);
                CU_ASSERT(type & NB_SRT_SubSearch);

                // Create parameter array for subsearches
                parametersArraySize = sizeof(NB_SearchParameters*) * slice;
                parametersArray = nsl_malloc(parametersArraySize);
                if (parametersArray)
                {
                    nsl_memset(parametersArray, 0, parametersArraySize);

                    // For all results in this slice. Could be less then requested number
                    for (resultIndex = 0; resultIndex < slice; resultIndex++)
                    {
                        NB_Place place = {{0}};
                        double distance = 0.0;

                        // Get the venue
                        result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, NULL);
                        CU_ASSERT_EQUAL(result, NE_OK);
                        if (result == NE_OK)
                        {
                            // Output venue details
                            OutputPoiResults(resultIndex + sliceIndex * NUMBER_PER_SLICE, distance, &place, NULL);

                            // Create parameters for subsearch. Save it in our temporary array.
                            result = NB_SearchParametersCreateSubSearch(g_pContext, information, resultIndex, &(parametersArray[resultIndex]));
                            CU_ASSERT_EQUAL(result, NE_OK);
                        }
                    }

                    for (resultIndex = 0; resultIndex < slice; resultIndex++)
                    {
                        if (parametersArray[resultIndex])
                        {
                            // Do a subsearch and then free the parameter
                            EventSubsearch(parametersArray[resultIndex]);
                            FreeParameters(&(parametersArray[resultIndex]));
                        }
                    }

                    nsl_free(parametersArray);
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! Test a movie search.

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderMovies(void)
{
    const int NUMBER_PER_SLICE = 2;
    const int SLICES_TO_DOWNLOAD = 2;
    int sliceIndex = 0;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            #define MOVE_GENRE_COUNT 6
            const char* genres[MOVE_GENRE_COUNT] = 
            {
                "GAA",    // Action/Adventure
                "GCO",    // Comedy
                "GDR",    // Drama/Romance
                "GFA",    // Family
                "GHS",    // Horror/Science Fiction
                "GFD"     // Foreign Documentary
            };

            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 47.37;
            region.center.longitude = -122.20;

            // Create search parameters 
            result = NB_SearchParametersCreateMovie(g_pContext,
                                                    &region,
                                                    "",
                                                    0,                      // Only use if search is for 'NB_MS_SpecificTime'
                                                    0,                      // Only use if search is for 'NB_MS_SpecificTime'
                                                    NB_MS_NowInTheaters,
                                                    genres,
                                                    MOVE_GENRE_COUNT,
                                                    NB_MSB_MostPopular,
                                                    NB_MRT_All,
                                                    NUMBER_PER_SLICE,
                                                    NB_EXT_None,
                                                    NULL,
                                                    &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information)
            {
                int resultIndex = 0;
                NB_SearchParameters** parametersArray = NULL;
                int parametersArraySize = 0;

                // The event (movie) and subsearch type has to be set
                CU_ASSERT(type & NB_SRT_Events);
                CU_ASSERT(type & NB_SRT_SubSearch);

                // Create parameter array for subsearches
                parametersArraySize = sizeof(NB_SearchParameters*) * slice;
                parametersArray = nsl_malloc(parametersArraySize);
                if (parametersArray)
                {
                    nsl_memset(parametersArray, 0, parametersArraySize);

                    // For all results in this slice. Could be less then requested number
                    for (resultIndex = 0; resultIndex < slice; resultIndex++)
                    {
                        // Structures for results of request
                        NB_Event* eventArray = NULL;
                        int eventCount = 0;

                        // get movies
                        result = NB_SearchInformationGetEvents(information, resultIndex, &eventCount, &eventArray);
                        CU_ASSERT_EQUAL(result, NE_OK);
                        if (result == NE_OK)
                        {
                            // Output movies
                            OutputEvents(resultIndex + sliceIndex * NUMBER_PER_SLICE, eventCount, eventArray);

                            // @todo: get movie content

                            // Create parameters for subsearch. Save it in our temporary array.
                            result = NB_SearchParametersCreateSubSearch(g_pContext, information, resultIndex, &(parametersArray[resultIndex]));
                            CU_ASSERT_EQUAL(result, NE_OK);
                        }
                    }

                    for (resultIndex = 0; resultIndex < slice; resultIndex++)
                    {
                        if (parametersArray[resultIndex])
                        {
                            // Do a subsearch and then free the parameter
                            EventSubsearch(parametersArray[resultIndex]);
                            FreeParameters(&(parametersArray[resultIndex]));
                        }
                    }

                    nsl_free(parametersArray);
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! Test a movie theater search.

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderTheaters(void)
{
    const int NUMBER_PER_SLICE = 2;
    const int SLICES_TO_DOWNLOAD = 2;
    int sliceIndex = 0;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            // Center of search
            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 47.37;
            region.center.longitude = -122.20;

            // Create search parameters 
            result = NB_SearchParametersCreateMovieTheater(g_pContext,
                                                           &region,
                                                           "AMC",
                                                           PAL_ClockGetGPSTime() + 8 * 60 * 60,    
                                                           PAL_ClockGetGPSTime() + 8 * 60 * 60,
                                                           NB_MTRT_All,
                                                           NUMBER_PER_SLICE,
                                                           NB_EXT_None,
                                                           NULL,
                                                           &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information && slice > 0)
            {
                int resultIndex = 0;
                NB_SearchParameters** parametersArray = NULL;
                int parametersArraySize = 0;

                // The place (theater) and subsearch type has to be set
                CU_ASSERT(type & NB_SRT_Place);
                CU_ASSERT(type & NB_SRT_SubSearch);

                // Create parameter array for subsearches
                parametersArraySize = sizeof(NB_SearchParameters*) * slice;
                parametersArray = nsl_malloc(parametersArraySize);
                if (parametersArray)
                {
                    nsl_memset(parametersArray, 0, parametersArraySize);

                    // For all results in this slice. Could be less then requested number
                    for (resultIndex = 0; resultIndex < slice; resultIndex++)
                    {
                        NB_Place place = {{0}};
                        double distance = 0.0;

                        // Get the theater
                        result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, NULL);
                        CU_ASSERT_EQUAL(result, NE_OK);
                        if (result == NE_OK)
                        {
                            // Output theater details
                            OutputPoiResults(resultIndex + sliceIndex * NUMBER_PER_SLICE, distance, &place, NULL);

                            // Create parameters for subsearch. Save it in our temporary array.
                            result = NB_SearchParametersCreateSubSearch(g_pContext, information, resultIndex, &(parametersArray[resultIndex]));
                            CU_ASSERT_EQUAL(result, NE_OK);
                        }
                    }

                    for (resultIndex = 0; resultIndex < slice; resultIndex++)
                    {
                        if (parametersArray[resultIndex])
                        {
                            // Do a subsearch and then free the parameter
                            EventSubsearch(parametersArray[resultIndex]);
                            FreeParameters(&(parametersArray[resultIndex]));
                        }
                    }

                    nsl_free(parametersArray);
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! Test an event sub search for different regions

    @return None. CUnit Asserts get called on failures.
*/
void
TestSearchBuilderEventSubSearchForDifferentRegion(void)
{
    const int NUMBER_PER_SLICE = 10;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    // Categories to search for
#define LOCAL_CATEGORY_COUNT 1
    const char* categories[LOCAL_CATEGORY_COUNT] = { "All" };

    // Center of search
    NB_SearchRegion region = {0};
    region.type = NB_ST_Center;
    region.center.latitude = 90.0;
    region.center.longitude = 90.0;

    if (!InitializeAll())
    {
        return;
    }

    // Create search parameters for events in New York

    /*
     @bug QA8 is 7 hours ahead of Aliso Viejo local time. And since we can't get events from
     yesterday we have to add 8 hours so that the test works between 5pm and midnight local time.

     The following bug has been filed for this: http://nimble.nimone.com/show_bug.cgi?id=36563
     */
    result = result ? result : NB_SearchParametersCreateEvent(g_pContext,
                                                              &region,
                                                              "",
                                                              categories,
                                                              LOCAL_CATEGORY_COUNT,
                                                              PAL_ClockGetGPSTime() + 8 * 60 * 60,
                                                              PAL_ClockGetGPSTime() + 8 * 60 * 60,
                                                              NB_ER_All,
                                                              NB_ERT_All,
                                                              NUMBER_PER_SLICE,
                                                              NB_EXT_None,
                                                              "",
                                                              &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    // Set the region to the parameters
    region.type = NB_ST_Center;
    region.center.latitude = 40.47;
    region.center.longitude = -73.58;
    result = result ? result : NB_SearchParametersSetSearchRegion(searchParameters, &region);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (result == NE_OK)
    {
        int slice = 0;
        nb_boolean hasMoreResults = FALSE;
        NB_SearchResultType type = NB_SRT_None;

        // Do the server request and download the results
        information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
        CU_ASSERT_PTR_NOT_NULL(information);
        if (information)
        {
            // The event summary and subsearch type has to be set
            CU_ASSERT(type & NB_SRT_EventSummary);
            CU_ASSERT(type & NB_SRT_SubSearch);

            if (slice > 0)
            {
                NB_SummaryResult summaryResult = {{{0}}};

                // The initial search returns event summary information
                result = NB_SearchInformationGetEventSummary(information, 0, &summaryResult);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result == NE_OK)
                {
                    NB_SearchParameters* subSearchParameters = NULL;

                    OutputEventSummary(0, &summaryResult);

                    // Create parameters for subsearch. Save it in our temporary array.
                    result = result ? result : NB_SearchParametersCreateSubSearch(g_pContext, information, 0, &subSearchParameters);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(subSearchParameters);

                    // Reset the search region to Dallas
                    region.type = NB_ST_Center;
                    region.center.latitude = 32.80;
                    region.center.longitude = -96.80;
                    result = result ? result : NB_SearchParametersSetSearchRegion(subSearchParameters, &region);
                    CU_ASSERT_EQUAL(result, NE_OK);

                    if (subSearchParameters)
                    {
                        // Do a subsearch and then free the parameter
                        EventSubsearch(subSearchParameters);
                        FreeParameters(&subSearchParameters);
                    }
                }
            }
        }
    }

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

/*! Test a movie sub search for different regions

    @return None. CUnit Asserts get called on failures.
*/
void
TestSearchBuilderMovieSubSearchForDifferentRegion(void)
{
    const int NUMBER_PER_SLICE = 2;
    const int SLICES_TO_DOWNLOAD = 2;
    int sliceIndex = 0;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
#define MOVE_GENRE_COUNT 6
            const char* genres[MOVE_GENRE_COUNT] =
            {
                "GAA",    // Action/Adventure
                "GCO",    // Comedy
                "GDR",    // Drama/Romance
                "GFA",    // Family
                "GHS",    // Horror/Science Fiction
                "GFD"     // Foreign Documentary
            };

            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 90.0;
            region.center.longitude = 90.0;

            // Create search parameters
            result = result ? result : NB_SearchParametersCreateMovie(g_pContext,
                                                                      &region,
                                                                      "",
                                                                      0,                      // Only use if search is for 'NB_MS_SpecificTime'
                                                                      0,                      // Only use if search is for 'NB_MS_SpecificTime'
                                                                      NB_MS_NowInTheaters,
                                                                      genres,
                                                                      MOVE_GENRE_COUNT,
                                                                      NB_MSB_MostPopular,
                                                                      NB_MRT_All,
                                                                      NUMBER_PER_SLICE,
                                                                      NB_EXT_None,
                                                                      NULL,
                                                                      &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);

            // Set the region to the parameters
            region.type = NB_ST_Center;
            region.center.latitude = 47.37;
            region.center.longitude = -122.20;
            result = result ? result : NB_SearchParametersSetSearchRegion(searchParameters, &region);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information)
            {
                int eventCount = 0;
                NB_Event* eventArray = NULL;
                NB_SearchParameters* subSearchParameters = NULL;
                NB_SearchRegion subSearchRegion = {0};

                // The event (movie) and subsearch type has to be set
                CU_ASSERT(type & NB_SRT_Events);
                CU_ASSERT(type & NB_SRT_SubSearch);

                // get movies
                result = NB_SearchInformationGetEvents(information, 0, &eventCount, &eventArray);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result == NE_OK)
                {
                    // Output movies
                    OutputEvents(sliceIndex * NUMBER_PER_SLICE, eventCount, eventArray);

                    // Create parameters for subsearch. Save it in our temporary array.
                    result = result ? result : NB_SearchParametersCreateSubSearch(g_pContext, information, 0, &subSearchParameters);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(subSearchParameters);

                    // Reset the search region to Dallas
                    subSearchRegion.type = NB_ST_Center;
                    subSearchRegion.center.latitude = 32.80;
                    subSearchRegion.center.longitude = -96.80;
                    result = result ? result : NB_SearchParametersSetSearchRegion(subSearchParameters, &subSearchRegion);
                    CU_ASSERT_EQUAL(result, NE_OK);
                }

                if (subSearchParameters)
                {
                    // Do a subsearch and then free the parameter
                    EventSubsearch(subSearchParameters);
                    FreeParameters(&subSearchParameters);
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! Test a carousel-lite fuel search.

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderCarouselFuel(void)
{
    const int NUMBER_PER_SLICE = 5;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_SearchRegion region = {0};

    if (!InitializeAll())
    {
        return;
    }

    // Set center of search to New York
    region.type = NB_ST_Center;
    region.center.latitude = 40.47;
    region.center.longitude = -73.58;

    // Create search parameters for gas stations in New York
    result = NB_SearchParametersCreateFuel(g_pContext, 
                                           &region, 
                                           0,
                                           NULL,
                                           NULL,
                                           0,
                                           NB_FST_Regular,
                                           NB_FRT_CarouselLite,
                                           NUMBER_PER_SLICE,
                                           NB_EXT_None,
                                           NULL,
                                           &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    result = NB_SearchParametersSetSource(searchParameters, NB_SS_Carousel);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (result == NE_OK)
    {
        int slice = 0;
        nb_boolean hasMoreResults = FALSE;
        NB_SearchResultType type = NB_SRT_None;

        // Do the server request and download the results
        information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
        CU_ASSERT_PTR_NOT_NULL(information);
        if (information)
        {
            int resultIndex = 0;
            NB_FuelSummary summary = {{{0}}};

            // The place, fuel and fuel summary type has to be set
            CU_ASSERT(type & NB_SRT_Place);
            CU_ASSERT(type & NB_SRT_FuelSummary);

            // Get fuel summary information for this slice
            result = NB_SearchInformationGetFuelSummary(information, &summary);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                OutputFuelSummary(&summary);
            }

            // For all results in this slice. Could be less then requested number
            for (resultIndex = 0; resultIndex < slice; resultIndex++)
            {
                // Structures for results of request
                NB_Place place = {{0}};
                NB_FuelDetails details = {{{{0}}}};
                double distance = 0.0;

                // Get place information
                result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, NULL);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result == NE_OK)
                {
                    // Output place information
                    OutputPoiResults(resultIndex, distance, &place, NULL);
                }

                // Not all of the results have fuel details.
                if (type & NB_SRT_FuelDetails)
                {
                    // Get fuel information for this location
                    result = NB_SearchInformationGetFuelDetails(information, resultIndex, &details);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        OutputFuelDetails(&details);
                    }
                }
                else
                {
                    LOGOUTPUT(LogLevelHigh, ("   No fuel details.\n"));
                }
            }
        }
    }

    FreeParameters(&searchParameters);

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! Test a carousel-lite weather search.

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderCarouselWeather(void)
{
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    // Set center of search to New York
    NB_LatitudeLongitude center = { 40.47, -73.58 };

    if (!InitializeAll())
    {
        return;
    }

    // Create search parameters for weather search 
    result = NB_SearchParametersCreateWeather(g_pContext, &center, 5, NB_WRT_CarouselLite, NB_EXT_None, Test_OptionsGet()->currentLanguage, &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    if (result == NE_OK)
    {
        int slice = 0;
        nb_boolean hasMoreResults = FALSE;
        NB_SearchResultType type = NB_SRT_None;

        // Do the server request and download the results
        information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
        CU_ASSERT_PTR_NOT_NULL(information);
        if (information)
        {
            int resultIndex = 0;
            NB_Place place = {{0}};
            NB_WeatherConditions weatherConditions = {0};
            NB_WeatherForecasts weatherForecasts = {{{0}}}; 
            double distance = 0.0;

            // The place and weather type has to be set
            CU_ASSERT(type & NB_SRT_Place);
            CU_ASSERT(type & NB_SRT_Weather);

            // Get place information
            result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                // Output place information
                OutputPoiResults(0, distance, &place, NULL);
            }

            // Get weather information
            result = NB_SearchInformationGetWeather(information, &weatherConditions, &weatherForecasts);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                // Output place information
                OutputWeatherResults(&weatherConditions, &weatherForecasts);
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! Test a carousel-lite event search.

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderCarouselEvents(void)
{
    const int NUMBER_PER_SLICE = 10;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    // Categories to search for
    #define LOCAL_CATEGORY_COUNT 1
    const char* categories[LOCAL_CATEGORY_COUNT] = { "ETH" };

    // Center of search - New York
    NB_SearchRegion region = {0};
    region.type = NB_ST_Center;
    region.center.latitude = 40.47;
    region.center.longitude = -73.58;

    if (!InitializeAll())
    {
        return;
    }

    // Create search parameters for events in New York
    result = NB_SearchParametersCreateEvent(g_pContext, 
                                            &region,
                                            0,
                                            categories,
                                            LOCAL_CATEGORY_COUNT,
                                            PAL_ClockGetGPSTime() + 8 * 60 * 60,    
                                            0,
                                            NB_ER_All,
                                            NB_ERT_CarouselLite,
                                            NUMBER_PER_SLICE,
                                            NB_EXT_None,
                                            Test_OptionsGet()->currentLanguage,
                                            &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);
    
    result = result ? result : NB_SearchParametersSetSource(searchParameters, NB_SS_Carousel);
    CU_ASSERT_EQUAL(result, NE_OK);
    
    if (result == NE_OK)
    {
        int slice = 0;
        nb_boolean hasMoreResults = FALSE;
        NB_SearchResultType type = NB_SRT_None;

        // Do the server request and download the results
        information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
        CU_ASSERT_PTR_NOT_NULL(information);
        if (information)
        {
            int resultIndex = 0;
            
            // For all results in this slice. Could be less then requested number
            for (resultIndex = 0; resultIndex < slice; resultIndex++)
            {
                // Structures for results of request
                NB_Event* eventArray = NULL;
                int eventCount = 0;

                result = NB_SearchInformationGetEvents(information, resultIndex, &eventCount, &eventArray);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result == NE_OK)
                {
                    OutputEvents(resultIndex, eventCount, eventArray);
                }
            }
        }
    }

    FreeParameters(&searchParameters);
    FreeInformation(&information);

    CleanupAll();
}

/*! Test a carousel-lite movie search.

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderCarouselMovies(void)
{
    const int NUMBER_PER_SLICE = 10;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    #define CAROUSEL_MOVE_GENRE_COUNT 6
    const char* genres[CAROUSEL_MOVE_GENRE_COUNT] = 
    {
        "GAA",    // Action/Adventure
        "GCO",    // Comedy
        "GDR",    // Drama/Romance
        "GFA",    // Family
        "GHS",    // Horror/Science Fiction
        "GFD"     // Foreign Documentary
    };

    NB_SearchRegion region = {0};

    if (!InitializeAll())
    {
        return;
    }

    // Center of search - New York
    region.type = NB_ST_Center;
    region.center.latitude = 40.47;
    region.center.longitude = -73.58;

    // Create search parameters 
    result = NB_SearchParametersCreateMovie(g_pContext,
                                            &region,
                                            0,
                                            0,                      // Only use if search is for 'NB_MS_SpecificTime'
                                            0,                      // Only use if search is for 'NB_MS_SpecificTime'
                                            NB_MS_NowInTheaters,
                                            genres,
                                            CAROUSEL_MOVE_GENRE_COUNT,
                                            NB_MSB_MostPopular,
                                            NB_MRT_CarouselLite,
                                            NUMBER_PER_SLICE,
                                            NB_EXT_None,
                                            NULL,
                                            &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    result = result ? result : NB_SearchParametersSetSource(searchParameters, NB_SS_Carousel);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (result == NE_OK)
    {
        int slice = 0;
        nb_boolean hasMoreResults = FALSE;
        NB_SearchResultType type = NB_SRT_None;

        // Do the server request and download the results
        information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
        CU_ASSERT_PTR_NOT_NULL(information);
        if (information)
        {
            int resultIndex = 0;
            NB_SearchParameters** parametersArray = NULL;
            int parametersArraySize = 0;

            // The event (movie) and subsearch type has to be set
            CU_ASSERT(type & NB_SRT_Events);
            CU_ASSERT(type & NB_SRT_SubSearch);

            nsl_memset(parametersArray, 0, parametersArraySize);

            // For all results in this slice. Could be less then requested number
            for (resultIndex = 0; resultIndex < slice; resultIndex++)
            {
                // Structures for results of request
                NB_Event* eventArray = NULL;
                int eventCount = 0;

                // get movies
                result = NB_SearchInformationGetEvents(information, resultIndex, &eventCount, &eventArray);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result == NE_OK)
                {
                    // Output movies
                    OutputEvents(resultIndex, eventCount, eventArray);
                }
            }
        }
    }

    FreeParameters(&searchParameters);

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}



/*! Test gas POI search along an entire route.

@return None. CUnit Asserts get called on failures.
*/
void TestSearchBuilderAlongRouteGas(void)
{
    char* categoryCodes[] = { "ACC" };
    SearchAlongEntireRoute("gas_stations", categoryCodes, sizeof(categoryCodes) / sizeof(categoryCodes[0]));
}

/*! Test lodging POI search along an entire route.

@return None. CUnit Asserts get called on failures.
*/
void TestSearchBuilderAlongRouteLodging(void)
{
    char* categoryCodes[] = { "AH" };
    SearchAlongEntireRoute("lodging", categoryCodes, sizeof(categoryCodes) / sizeof(categoryCodes[0]));
}

/*! Test bank/atm POI search along an entire route.

@return None. CUnit Asserts get called on failures.
*/
void TestSearchBuilderAlongRouteBanks(void)
{
    char* categoryCodes[] = { "AA" };
    SearchAlongEntireRoute("dollar", categoryCodes, sizeof(categoryCodes) / sizeof(categoryCodes[0]));
}

/*! Test dining POI search along an entire route.

@return None. CUnit Asserts get called on failures.
*/
void TestSearchBuilderAlongRouteDining(void)
{
    char* categoryCodes[] = { "AE" };
    SearchAlongEntireRoute("dining", categoryCodes, sizeof(categoryCodes) / sizeof(categoryCodes[0]));
}

/*! Test tourist POI search along an entire route.

@return None. CUnit Asserts get called on failures.
*/
void TestSearchBuilderAlongRouteTourist(void)
{
    char* categoryCodes[] = { "ALG" };
    SearchAlongEntireRoute("info", categoryCodes, sizeof(categoryCodes) / sizeof(categoryCodes[0]));
}

/*! Test emergency POI search along an entire route.

@return None. CUnit Asserts get called on failures.
*/
void TestSearchBuilderAlongRouteEmergency(void)
{
    char* categoryCodes[] = { "ACA", "AIC", "AID", "AIE", "AKE", "AFC" };
    SearchAlongEntireRoute("hospitals", categoryCodes, sizeof(categoryCodes) / sizeof(categoryCodes[0]));
}

/*! Test public transit POI search along an entire route.

@return None. CUnit Asserts get called on failures.
*/
void TestSearchBuilderAlongRoutePublicTransit(void)
{
    char* categoryCodes[] = { "ALB", "ALH", "ALI", "ALL" };
    SearchAlongEntireRoute("bus", categoryCodes, sizeof(categoryCodes) / sizeof(categoryCodes[0]));
}

/*! Test dining & gas POI search along an entire route.

@return None. CUnit Asserts get called on failures.
*/
void TestSearchBuilderAlongRouteGasDining(void)
{
    char* categoryCodes[] = { "AE", "ACC" };
    SearchAlongEntireRoute("open-diamond", categoryCodes, sizeof(categoryCodes) / sizeof(categoryCodes[0]));
}

/*! Test gas & dining POI search along an entire route.

@return None. CUnit Asserts get called on failures.
*/
void TestSearchBuilderAlongRouteDiningGas(void)
{
    char* categoryCodes[] = { "ACC", "AE" };
    SearchAlongEntireRoute("open-diamond", categoryCodes, sizeof(categoryCodes) / sizeof(categoryCodes[0]));
}

/*! Test making POI searches along a route in segments.

@return None. CUnit Asserts get called on failures.
*/
static void TestSearchBuilderSearchAlongRouteInSegments(void)
{
    const char* outputFileNamePrefix = "POI_Alicia_To_RedHill_Dining-";

    const uint32 searchMaxCount = 500;
    const uint32 searchWidth = 800;
    const uint32 searchDistance = 5000;
    const double queryAtPercent = .95;

    const char* categories[] = { "XE" };
    const int categoryCount = sizeof(categories) / sizeof(categories[0]);

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* searchInformation = NULL;
    NB_Error result = NE_OK;

    NB_RouteInformation* routeInformation = NULL;

    NB_LatitudeLongitude origin = { 33.607386, -117.695228 };       // 5 & Alicia
    NB_LatitudeLongitude destination = { 33.734283, -117.814528 };  // 5 & Red Hill

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (!InitializeAll())
    {
        return;
    }

    // Delete previous test files
    DeleteTemporaryTestFiles(g_pPal, outputFileNamePrefix);

    // Create route to search along
    routeInformation = CreateRouteToSearchAlong(&origin, &destination);

    if (routeInformation)
    {
        PAL_File* kmlFile = 0;
        NB_SearchRegion region = {0};
        NB_MercatorPolyline* polyline = 0;
        NB_LatitudeLongitude searchPoint = origin;
        NB_PointsOfInterestInformation* poiInformation = 0;

        double currentDistance = 0.0;

        int searchSegment = 0;
        int numManeuvers = 0;
        int numRequests = 0;
        int poiCount = 0;
        int index = 0;

        result = NB_PointsOfInterestInformationCreate(g_pContext, &poiInformation);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(poiInformation);

        numManeuvers = NB_RouteInformationGetManeuverCount(routeInformation);
        CU_ASSERT(numManeuvers > 0);

        result = NB_RouteInformationGetRoutePolyline(routeInformation, 0, numManeuvers, &polyline);    
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(polyline);

        while (searchSegment < polyline->count)
        {
            char kmlFileName[80];
            
            region.type = NB_ST_RouteAndCenter;
            region.route = routeInformation;
            region.distance = searchDistance;
            region.width = searchWidth;
            region.center = searchPoint;

            nsl_sprintf(kmlFileName, "%s%03d.kml", outputFileNamePrefix, numRequests);

            kmlFile = Test_StartKML(g_pPal, kmlFileName,  kmlFileName);
            if (!kmlFile)
            {
                CU_FAIL("Test_StartKML() failed");
                break;
            }

            // Create search parameters 
            result = NB_SearchParametersCreatePOI(
                g_pContext, 
                &region, 
                NULL, 
                NULL,
                categories, 
                categoryCount,      
                searchMaxCount, 
                NB_EXT_None, 
                NULL,
                &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);

            if (result == NE_OK)
            {
                int resultCount = 0;
                nb_boolean hasMoreResults = FALSE;
                NB_SearchResultType type = NB_SRT_None;

                searchInformation = DoRequest(searchParameters, &resultCount, &hasMoreResults, &type);
                CU_ASSERT_PTR_NOT_NULL(searchInformation);

                if (!resultCount)
                {
                    CU_FAIL("Zero POIs returned; POIs expected...");
                }
                else if (resultCount == (int)searchMaxCount)
                {
                    CU_FAIL("Maximum count returned; should not be that many along given segment...")            
                }

                if (searchInformation)
                {
                    Test_WritePOIsToKML(kmlFile, resultCount, "http://maps.google.com/mapfiles/kml/shapes/dining.png", searchInformation);
                
                    LOGOUTPUT(LogLevelHigh, (" POI request %d orginating at (%f, %f) distance along route: %f  distance searched: %d\n",
                        numRequests, region.center.latitude, region.center.longitude, currentDistance, region.distance));
                    
                    result = NB_PointsOfInterestInformationUpdateWithSearch(poiInformation, NULL, searchInformation);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    result = NB_PointsOfInterestInformationGetPointOfInterestCount(poiInformation, &poiCount);
                    CU_ASSERT_EQUAL(result, NE_OK);

                    for (index = 0; index < poiCount; index++)
                    {
                        NB_PointOfInterest poi = {{{ 0 }}};
                        result = NB_PointsOfInterestInformationGetPointOfInterest(poiInformation, index, &poi);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        LOGOUTPUT(LogLevelHigh, ("  %s (%f, %f) %f",
                            poi.place.name, poi.place.location.latitude, poi.place.location.longitude, poi.routeDistanceRemaining));

                        if (poi.extendedPlace && poi.extendedPlace->poiContentCount)
                        {
                            int n = 0;
                            for (n = 0; n < poi.extendedPlace->poiContentCount; n++)
                            {
                                LOGOUTPUT(LogLevelHigh, (" [%s: %s]",
                                    poi.extendedPlace->poiContent[n].key,
                                    poi.extendedPlace->poiContent[n].value));
                            }
                        }
                        LOGOUTPUT(LogLevelHigh, ("\n"));
                    }
                    LOGOUTPUT(LogLevelHigh, ("\n"));
                }
            }

            Test_FinishKML(kmlFile);

            FreeParameters(&searchParameters);
            FreeInformation(&searchInformation);

            numRequests++;
            
            for (; searchSegment < polyline->count; searchSegment++)
            {
                double latitude = 0.0;
                double longitude = 0.0;
                double segmentDistance = 0.0;

                NB_SpatialConvertMercatorToLatLong(polyline->points[searchSegment].mx, polyline->points[searchSegment].my, &latitude, &longitude);
                segmentDistance = NB_SpatialGetLineOfSightDistance(searchPoint.latitude, searchPoint.longitude, latitude, longitude, 0);
                if (segmentDistance > (searchDistance * queryAtPercent)) 
                {
                    searchPoint.latitude = latitude;
                    searchPoint.longitude = longitude;
                    currentDistance += segmentDistance;
                    break;
                }
            }
        }
        NB_MercatorPolylineDestroy(polyline);
        NB_PointsOfInterestInformationDestroy(poiInformation);
    }

    NB_RouteInformationDestroy(routeInformation);

    CleanupAll();

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

static void TestSearchBuilderSearchSpeedCameras(void)
{
    const char* outputFileName = "SpeedCameras.kml";
    
    const uint32 searchSliceCount = 100;
    const uint32 searchWidth = 0;
    const uint32 searchDistance = 99999999;
    
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_RouteInformation* routeInformation = NULL;
    NB_LatitudeLongitude origin = { 51.5073, -0.5012 };      
    NB_LatitudeLongitude destination = { 51.6571, -0.49309 };

    if (!InitializeAll())
    {
        return;
    }

    // Delete previous test files
    DeleteTemporaryTestFiles(g_pPal, outputFileName);

    // Create route to search along
    routeInformation = CreateRouteToSearchAlong(&origin, &destination);

    if (routeInformation)
    {
        PAL_File* kmlFile = 0;
        NB_Place place = {{0}};
        NB_SearchRegion region = {0};

        result = NB_RouteInformationGetOrigin(routeInformation, &place);
        CU_ASSERT_EQUAL(result, NE_OK);

        region.type = NB_ST_RouteAndCenter;
        region.route = routeInformation;
        region.center.latitude = place.location.latitude;
        region.center.longitude = place.location.longitude;
        region.distance = searchDistance;
        region.width = searchWidth;

        kmlFile = Test_StartKML(g_pPal, "Speed Cameras On Route",  outputFileName);
        if (kmlFile)
        {
            Test_WriteRoutePolylineToKML(kmlFile, routeInformation);
        }

        // Create search parameters 
        result = NB_SearchParametersCreatePOI(g_pContext, &region, NULL, "safety-cameras", NULL, 0, searchSliceCount, NB_EXT_None, NULL, &searchParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(searchParameters);

        if (result == NE_OK)
        {
            int resultCount = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            information = DoRequest(searchParameters, &resultCount, &hasMoreResults, &type);
            CU_ASSERT_EQUAL(type & NB_SRT_SpeedCamera, NB_SRT_SpeedCamera);
            CU_ASSERT_TRUE(resultCount > 3);  // Should be at least three cameras on the route
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information && kmlFile)
            {
                Test_WritePOIsToKML(kmlFile, resultCount, "http://maps.google.com/mapfiles/kml/shapes/camera.png", information);
            }

            FreeParameters(&searchParameters);
        }

        NB_RouteInformationDestroy(routeInformation);
        Test_FinishKML(kmlFile);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! This tests the consistency of the returned search results.

    This bug is used to verify the following server bugs.

    http://nimble.nimone.com/show_bug.cgi?id=30684
    http://nimble.nimone.com/show_bug.cgi?id=30694

    @return None. Asserts are called on failure.
*/
void 
TestSearchBuilderServerConsistency(void)
{
    int totalCount1 = 0;
    int totalCount2 = 0;
    int totalCount3 = 0;
    int totalCount4 = 0;
    int totalCount5 = 0;
    int totalCount6 = 0;
    int totalCount7 = 0;
    int totalCount8 = 0;
    int totalCount9 = 0;
    int totalCount10 = 0;
    int totalCount15 = 0;
    int totalCount20 = 0;
    int totalCount50 = 0;

    if (!InitializeAll())
    {
        return;
    }


    // Make same request multiple times with different slice/number per slice combinations.
    // They should always return the same results.

    // Coffee shops in Aliso Viejo

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  1 ---\n\n"));
    totalCount1 = ConsistencyTest(1);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  2 ---\n\n"));
    totalCount2 = ConsistencyTest(2);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  3 ---\n\n"));
    totalCount3 = ConsistencyTest(3);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  4 ---\n\n"));
    totalCount4 = ConsistencyTest(4);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  5 ---\n\n"));
    totalCount5 = ConsistencyTest(5);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  6 ---\n\n"));
    totalCount6 = ConsistencyTest(6);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  7 ---\n\n"));
    totalCount7 = ConsistencyTest(7);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  8 ---\n\n"));
    totalCount8 = ConsistencyTest(8);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  9 ---\n\n"));
    totalCount9 = ConsistencyTest(9);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size: 10 ---\n\n"));
    totalCount10 = ConsistencyTest(10);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size: 15 ---\n\n"));
    totalCount15 = ConsistencyTest(15);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size: 20 ---\n\n"));
    totalCount20 = ConsistencyTest(20);
    OutputConsistencyResults();

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size: 50 ---\n\n"));
    totalCount50 = ConsistencyTest(50);
    OutputConsistencyResults();

    // Output the count per slice
    LOGOUTPUT(LogLevelMedium, ("Slice size:  1, Total returned results: %d\n", totalCount1));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  2, Total returned results: %d\n", totalCount2));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  3, Total returned results: %d\n", totalCount3));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  4, Total returned results: %d\n", totalCount4));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  5, Total returned results: %d\n", totalCount5));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  6, Total returned results: %d\n", totalCount6));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  7, Total returned results: %d\n", totalCount7));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  8, Total returned results: %d\n", totalCount8));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  9, Total returned results: %d\n", totalCount9));
    LOGOUTPUT(LogLevelMedium, ("Slice size: 10, Total returned results: %d\n", totalCount10));
    LOGOUTPUT(LogLevelMedium, ("Slice size: 15, Total returned results: %d\n", totalCount15));
    LOGOUTPUT(LogLevelMedium, ("Slice size: 20, Total returned results: %d\n", totalCount20));
    LOGOUTPUT(LogLevelMedium, ("Slice size: 50, Total returned results: %d\n", totalCount50));

    // Check if all the counts match. Compare to the 10 per slice test
    CU_ASSERT_EQUAL(totalCount10,totalCount1);
    CU_ASSERT_EQUAL(totalCount10,totalCount2);
    CU_ASSERT_EQUAL(totalCount10,totalCount3);
    CU_ASSERT_EQUAL(totalCount10,totalCount4);
    CU_ASSERT_EQUAL(totalCount10,totalCount5);
    CU_ASSERT_EQUAL(totalCount10,totalCount6);
    CU_ASSERT_EQUAL(totalCount10,totalCount7);
    CU_ASSERT_EQUAL(totalCount10,totalCount8);
    CU_ASSERT_EQUAL(totalCount10,totalCount9);
    CU_ASSERT_EQUAL(totalCount10,totalCount15);
    CU_ASSERT_EQUAL(totalCount10,totalCount20);
    CU_ASSERT_EQUAL(totalCount10,totalCount50);

    CleanupAll();
}

//------------------------------------------------------------------------------
// Start - Premium Placement Tests
//------------------------------------------------------------------------------

/*! Test a simple point of interest search with premium placement ads

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderPremiumSimplePoi(void)
{
    // Premium placement ads comes from external feeds so it's not guaranteed that
    // we will get any result sets.
    // Enhanced POIs comes from our database. Result sets will depend on what
    // your searching for.
    // The test preceded with "nimtest" should always return a premium placement.
    TestPremiumSimplePoi("nimtestfood", NB_EXT_WantPremiumPlacement, FALSE);
    TestPremiumSimplePoi("nimtestfood", NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI, FALSE);
    TestPremiumSimplePoi("nimtestfood", NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantFormattedText, FALSE);

    TestPremiumSimplePoi("hotels", NB_EXT_WantEnhancedPOI, FALSE);
}

void 
TestSearchBuilderIlap(void)
{
    TestPremiumSimplePoi("hotels", NB_EXT_WantPremiumPlacement, TRUE);
}

void
TestPremiumSimplePoi(char* poiName, NB_POIExtendedConfiguration premiumFlags, nb_boolean useIlap)
{
    const int NUMBER_PER_SLICE = 5;
    const int SLICES_TO_DOWNLOAD = 5;
    int sliceIndex = 0;
    nb_boolean gotPremiumPlacement = FALSE;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            // Set center to Aliso Viejo
            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 33.58;
            region.center.longitude = -117.73;

            gotPremiumPlacement = FALSE;

            // Create search parameters 
            result = NB_SearchParametersCreatePOI(g_pContext, 
                                                  &region, 
                                                  poiName, 
                                                  NULL,
                                                  NULL, 
                                                  0, 
                                                  NUMBER_PER_SLICE, 
                                                  premiumFlags,
                                                  NULL, 
                                                  &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);

            if (useIlap)
            {
                char* ilapUserId = 0;
                NB_ImageSize imageSize = { 16, 16 };

                result = TestProfile_GetIlapUserId(&ilapUserId);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (ilapUserId)
                {
                    CU_ASSERT_EQUAL(NB_SearchParametersAddExtendedKeyValue(searchParameters, premiumFlags, "ilap-userid", ilapUserId), NE_OK);
                    nsl_free(ilapUserId);
                }

                CU_ASSERT_EQUAL(NB_SearchParametersAddExtendedImage(searchParameters, premiumFlags, NB_IF_PNG, &imageSize, 0), NE_OK);
            }
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information)
            {
                int resultIndex = 0;

                // The place type has to be set
                CU_ASSERT(type & NB_SRT_Place);

                // For all results in this slice. Could be less then requested number
                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    // Structures for results of request
                    NB_Place place = {{0}};
                    NB_ExtendedPlace* extendedPlace = NULL;
                    double distance = 0.0;

                    // Get place information
                    result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, &extendedPlace);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        // if we requested premium placements then we should have one "gokivo" entry
                        // in the first result set
                        if ((premiumFlags & NB_EXT_WantPremiumPlacement) && !gotPremiumPlacement)
                        {
                            gotPremiumPlacement = extendedPlace->extendedInformation & NB_EPI_PremiumPlacement ? TRUE : FALSE;
                            CU_ASSERT(gotPremiumPlacement);

                            if (useIlap)
                            {
                                CU_ASSERT_NOT_EQUAL(extendedPlace->imageDataSize, 0);
                                CU_ASSERT_PTR_NOT_NULL(extendedPlace->imageData);
                            }
                        }

                        // Output place information
                        OutputPoiResults(resultIndex + sliceIndex * NUMBER_PER_SLICE, distance, &place, extendedPlace);
                    }
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}


/*! Test a fuel search with premium placement ad

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderPremiumFuel(void)
{
    // TODO: Server may or may not return any premium ads
    //TestPremiumFuel(NB_EXT_WantPremiumPlacement);
    //TestPremiumFuel(NB_EXT_WantEnhancedPOI);
    //TestPremiumFuel(NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI);
    TestPremiumFuel(NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantFormattedText);
}

void 
TestPremiumFuel(NB_POIExtendedConfiguration premiumFlags)
{
    const int NUMBER_PER_SLICE = 5;
    const int SLICES_TO_DOWNLOAD = 5;
    int sliceIndex = 0;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            // Set center of search to Dallas
            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 32.80;
            region.center.longitude = -96.80;

            // Create search parameters for gas stations in Dallas.
            result = NB_SearchParametersCreateFuel(g_pContext, 
                                                   &region, 
                                                   "",
                                                   NULL,
                                                   NULL,
                                                   0,
                                                   NB_FST_Regular,
                                                   NB_FRT_All,
                                                   NUMBER_PER_SLICE,
                                                   premiumFlags,
                                                   NULL,
                                                   &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information)
            {
                int resultIndex = 0;
                NB_FuelSummary summary = {{{0}}};

                // The place, fuel and fuel summary type has to be set
                CU_ASSERT(type & NB_SRT_Place);
                CU_ASSERT(type & NB_SRT_FuelSummary);

                // Get fuel summary information for this slice
                result = NB_SearchInformationGetFuelSummary(information, &summary);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result == NE_OK)
                {
                    OutputFuelSummary(&summary);
                }

                // For all results in this slice. Could be less then requested number
                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    // Structures for results of request
                    NB_Place place = {{0}};
                    NB_ExtendedPlace* extendedPlace = NULL;
                    NB_FuelDetails details = {{{{0}}}};
                    double distance = 0.0;

                    // Get place information
                    result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, &extendedPlace);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        // Output place information
                        OutputPoiResults(resultIndex + sliceIndex * NUMBER_PER_SLICE, distance, &place, extendedPlace);
                    }

                    // Not all of the results have fuel details.
                    if (type & NB_SRT_FuelDetails)
                    {
                        // Get fuel information for this location
                        result = NB_SearchInformationGetFuelDetails(information, resultIndex, &details);
                        CU_ASSERT_EQUAL(result, NE_OK);
                        if (result == NE_OK)
                        {
                            OutputFuelDetails(&details);
                        }
                    }
                    else
                    {
                        LOGOUTPUT(LogLevelHigh, ("   No fuel details.\n"));
                    }
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}


/*! Test a weather search with premium ads

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderPremiumWeather(void)
{
    // TODO: Server may or may not return any premium ads
    //TestPremiumWeather(NB_EXT_WantPremiumPlacement);
    //TestPremiumWeather(NB_EXT_WantEnhancedPOI);
    //TestPremiumWeather(NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI);
    TestPremiumWeather(NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantFormattedText);
}

void
TestPremiumWeather(NB_POIExtendedConfiguration premiumFlags)
{
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    // Set center of search to Dallas
    NB_LatitudeLongitude center = { 32.80, -96.80 };

    if (!InitializeAll())
    {
        return;
    }

    // Create search parameters for weather search 
    result = NB_SearchParametersCreateWeather(g_pContext, &center, 5, NB_WRT_All, premiumFlags, "", &searchParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    if (result == NE_OK)
    {
        int slice = 0;
        nb_boolean hasMoreResults = FALSE;
        NB_SearchResultType type = NB_SRT_None;

        // Do the server request and download the results
        information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
        CU_ASSERT_PTR_NOT_NULL(information);
        if (information)
        {
            int resultIndex = 0;
            NB_Place place = {{0}};
            NB_ExtendedPlace* extendedPlace = NULL;
            NB_WeatherConditions weatherConditions = {0};
            NB_WeatherForecasts weatherForecasts = {{{0}}}; 
            double distance = 0.0;

            // The place and weather type has to be set
            CU_ASSERT(type & NB_SRT_Place);
            CU_ASSERT(type & NB_SRT_Weather);

            // Get place information
            result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, &extendedPlace);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                // Output place information
                OutputPoiResults(0, distance, &place, extendedPlace);
            }

            // Get weather information
            result = NB_SearchInformationGetWeather(information, &weatherConditions, &weatherForecasts);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                // Output place information
                OutputWeatherResults(&weatherConditions, &weatherForecasts);
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}



/*! Test a traffic incident search with premium ads

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderPremiumTrafficIncident(void)
{
    // TODO: Server may or may not return any premium ads
    //TestPremiumTrafficIncident(NB_EXT_WantPremiumPlacement);
    //TestPremiumTrafficIncident(NB_EXT_WantEnhancedPOI);
    //TestPremiumTrafficIncident(NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI);
    TestPremiumTrafficIncident(NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantFormattedText);
}

void
TestPremiumTrafficIncident(NB_POIExtendedConfiguration premiumFlags)
{
    const int NUMBER_PER_SLICE = 5;
    const int SLICES_TO_DOWNLOAD = 5;
    int sliceIndex = 0;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            // Set center of search to New York
            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 40.755856;
            region.center.longitude = -73.985045;

            // Create search parameters
            result = NB_SearchParametersCreateTrafficIncident(g_pContext, 
                                                              &region, 
                                                              5, 
                                                              NB_TIRT_All, 
                                                              NUMBER_PER_SLICE, 
                                                              premiumFlags,
                                                              NULL, 
                                                              &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information)
            {
                int resultIndex = 0;

                // The place and traffic incident type has to be set
                CU_ASSERT(type & NB_SRT_Place);
                CU_ASSERT(type & NB_SRT_TrafficIncident);

                // For all results in this slice. Could be less then requested number
                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    // Structures for results of request
                    NB_Place place = {{0}};
                    NB_ExtendedPlace* extendedPlace = NULL;
                    NB_TrafficIncident incident = {0};
                    double distance = 0.0;

                    // Get place information
                    result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, &extendedPlace);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        // Output place information
                        OutputPoiResults(resultIndex + sliceIndex * NUMBER_PER_SLICE, distance, &place, extendedPlace);
                    }

                    // Get the traffic incident
                    result = NB_SearchInformationGetTrafficIncident(information, resultIndex, &incident, &distance);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        OutputTrafficIncident(resultIndex + sliceIndex * NUMBER_PER_SLICE, &incident);
                    }
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}



/*! This tests the consistency of the returned search results for premium results

    @return None. Asserts are called on failure.
*/
void 
TestSearchBuilderPremiumServerConsistency(void)
{
    char* categoryCodes[] = { "XEAAU" };
    TestPremiumServerConsistency("Italian Restaurants", categoryCodes, 1, NB_EXT_WantPremiumPlacement);
    TestPremiumServerConsistency("Italian Restaurants", categoryCodes, 1, NB_EXT_WantEnhancedPOI);
    TestPremiumServerConsistency("Italian Restaurants", categoryCodes, 1, NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI);
    TestPremiumServerConsistency("Italian Restaurants", categoryCodes, 1, NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantFormattedText);
}

void
TestPremiumServerConsistency(char* searchForName, char* categoryCodes[], int categoryCodesCount, NB_POIExtendedConfiguration premiumFlags)
{
    int totalCount1 = 0;
    int totalCount2 = 0;
    int totalCount3 = 0;
    int totalCount4 = 0;
    int totalCount5 = 0;
    int totalCount6 = 0;
    int totalCount7 = 0;
    int totalCount8 = 0;
    int totalCount9 = 0;
    int totalCount10 = 0;
    int totalCount15 = 0;
    int totalCount20 = 0;
    int totalCount50 = 0;

    if (!InitializeAll())
    {
        return;
    }


    // Make same request multiple times with different slice/number per slice combinations.

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  1 ---\n\n"));
    totalCount1 = ConsistencyTestPremium(1, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  2 ---\n\n"));
    totalCount2 = ConsistencyTestPremium(2, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  3 ---\n\n"));
    totalCount3 = ConsistencyTestPremium(3, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  4 ---\n\n"));
    totalCount4 = ConsistencyTestPremium(4, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  5 ---\n\n"));
    totalCount5 = ConsistencyTestPremium(5, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  6 ---\n\n"));
    totalCount6 = ConsistencyTestPremium(6, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  7 ---\n\n"));
    totalCount7 = ConsistencyTestPremium(7, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  8 ---\n\n"));
    totalCount8 = ConsistencyTestPremium(8, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size:  9 ---\n\n"));
    totalCount9 = ConsistencyTestPremium(9, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size: 10 ---\n\n"));
    totalCount10 = ConsistencyTestPremium(10, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size: 15 ---\n\n"));
    totalCount15 = ConsistencyTestPremium(11, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size: 20 ---\n\n"));
    totalCount20 = ConsistencyTestPremium(12, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    LOGOUTPUT(LogLevelMedium, ("\n--- Slice size: 50 ---\n\n"));
    totalCount50 = ConsistencyTestPremium(13, searchForName, categoryCodes, categoryCodesCount, premiumFlags);

    // Output the count per slice
    LOGOUTPUT(LogLevelMedium, ("Slice size:  1, Total returned results: %d\n", totalCount1));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  2, Total returned results: %d\n", totalCount2));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  3, Total returned results: %d\n", totalCount3));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  4, Total returned results: %d\n", totalCount4));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  5, Total returned results: %d\n", totalCount5));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  6, Total returned results: %d\n", totalCount6));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  7, Total returned results: %d\n", totalCount7));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  8, Total returned results: %d\n", totalCount8));
    LOGOUTPUT(LogLevelMedium, ("Slice size:  9, Total returned results: %d\n", totalCount9));
    LOGOUTPUT(LogLevelMedium, ("Slice size: 10, Total returned results: %d\n", totalCount10));
    LOGOUTPUT(LogLevelMedium, ("Slice size: 15, Total returned results: %d\n", totalCount15));
    LOGOUTPUT(LogLevelMedium, ("Slice size: 20, Total returned results: %d\n", totalCount20));
    LOGOUTPUT(LogLevelMedium, ("Slice size: 50, Total returned results: %d\n", totalCount50));

    // Check if all the counts match. Compare to the 10 per slice test
    CU_ASSERT_EQUAL(totalCount10,totalCount1);
    CU_ASSERT_EQUAL(totalCount10,totalCount2);
    CU_ASSERT_EQUAL(totalCount10,totalCount3);
    CU_ASSERT_EQUAL(totalCount10,totalCount4);
    CU_ASSERT_EQUAL(totalCount10,totalCount5);
    CU_ASSERT_EQUAL(totalCount10,totalCount6);
    CU_ASSERT_EQUAL(totalCount10,totalCount7);
    CU_ASSERT_EQUAL(totalCount10,totalCount8);
    CU_ASSERT_EQUAL(totalCount10,totalCount9);
    CU_ASSERT_EQUAL(totalCount10,totalCount15);
    CU_ASSERT_EQUAL(totalCount10,totalCount20);
    CU_ASSERT_EQUAL(totalCount10,totalCount50);

    CleanupAll();
}

//------------------------------------------------------------------------------
// End - Premium Placement Tests
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Begin - Microsoft Search Tests
//------------------------------------------------------------------------------

/*! Test MSFT-supporting search additions

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderMicrosoftSearch(void)
{
    const int NUMBER_PER_SLICE = 5;
    const int SLICES_TO_DOWNLOAD = 5;
    int sliceIndex = 0;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            // Set center to Aliso Viejo
            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 33.58;
            region.center.longitude = -117.73;

            // Create search parameters 
            result = NB_SearchParametersCreatePOI(g_pContext, 
                          &region, 
                          "Pizza", 
                          NULL,
                          NULL, 
                          0, 
                          NUMBER_PER_SLICE, 
                          NB_EXT_WantPremiumPlacement | NB_EXT_WantAccuracy,
                          NULL, 
                          &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);

            result = NB_SearchParametersSetSource(searchParameters, NB_SS_User);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchParametersSetSearchSortBy(searchParameters, NB_SSB_Relevance);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchParametersSetSearchInputMethod(searchParameters, NB_SIM_Screen);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information)
            {
                int resultIndex = 0;

                // The place type has to be set
                CU_ASSERT(type & NB_SRT_Place);

                // For all results in this slice. Could be less then requested number
                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    // Structures for results of request
                    NB_Place place = {{0}};
                    NB_ExtendedPlace* extendedPlace = 0;
                    double distance = 0.0;
                    nb_boolean isMappable = FALSE;

                    // Get place information
                    result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, &extendedPlace);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(extendedPlace);

                    result = NB_SearchInformationIsMappable(information, resultIndex, &isMappable);

                    if (result == NE_OK)
                    {
                        // Output place information
                        OutputPoiResults(resultIndex + sliceIndex * NUMBER_PER_SLICE, distance, &place, NULL );
                    }
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

/*! Test MSFT-supporting search additions

    @return None. CUnit Asserts get called on failures.
*/
void 
TestSearchBuilderMicrosoftSpellingSuggestions(void)
{
    const int NUMBER_PER_SLICE = 5;
    const int SLICES_TO_DOWNLOAD = 5;
    const char* MISPELLED_SEARCH = "Stabucks";
    int sliceIndex = 0;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    if (!InitializeAll())
    {
        return;
    }

    // For all slices
    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            // Set center to Aliso Viejo
            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 33.58;
            region.center.longitude = -117.73;

            // Create search parameters 
            result = NB_SearchParametersCreatePOI(
                        g_pContext, 
                        &region, 
                        MISPELLED_SEARCH,
                        NULL,
                        NULL, 
                        0, 
                        NUMBER_PER_SLICE,
                        NB_EXT_WantPremiumPlacement | NB_EXT_WantAccuracy | NB_EXT_WantSpellingSuggestions,
                        NULL, 
                        &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);

            result = NB_SearchParametersSetSource(searchParameters, NB_SS_User);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_SearchParametersSetSearchSortBy(searchParameters, NB_SSB_Relevance);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information)
            {
                int resultIndex = 0;

                // check if spelling suggestions provided
                uint32 suggestionCount = 0;
                uint32 suggestionIndex = 0;

                result = NB_SearchInformationGetSpellingSuggestionCount(information, &suggestionCount);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT(suggestionCount || slice > 1);

                for (suggestionIndex = 0; suggestionIndex <= suggestionCount; suggestionIndex++)
                {
                    char suggestion[40] = { 0 };

                    result = NB_SearchInformationGetSpellingSuggestion(information, suggestionIndex, sizeof(suggestion), suggestion);
                    if (suggestionIndex < suggestionCount)
                    {
                        LOGOUTPUT(LogLevelMedium, ("Spelling suggestion for '%s' = '%s'\n", MISPELLED_SEARCH, suggestion));
                        CU_ASSERT_EQUAL(result, NE_OK);
                    }
                    else
                    {
                        CU_ASSERT_EQUAL(result, NE_NOENT);
                    }
                }

                // The place type has to be set
                CU_ASSERT(type & NB_SRT_Place);

                // For all results in this slice. Could be less then requested number
                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    // Structures for results of request
                    NB_Place place = {{0}};
                    NB_ExtendedPlace* extendedPlace = 0;
                    double distance = 0.0;
                    nb_boolean isMappable = FALSE;

                    // Get place information
                    result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, &extendedPlace);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(extendedPlace);

                    result = NB_SearchInformationIsMappable(information, resultIndex, &isMappable);
                    
                    if (result == NE_OK)
                    {
                        // Output place information
                        OutputPoiResults(resultIndex + sliceIndex * NUMBER_PER_SLICE, distance, &place, NULL );
                    }
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
}

//------------------------------------------------------------------------------
// End - Microsoft Search Tests
//------------------------------------------------------------------------------


/*! Initialize Pal, context and search handler.

    Called before every test. We can't call this in the suite initialize since we can't call any asserts in those functions

    @return TRUE for success

    @see CleanupAll
*/
nb_boolean 
InitializeAll()
{
    NB_Error nbResult = NE_OK;
    NB_RequestHandlerCallback callbackData = { &RequestHandlerCallback, NULL };
    NB_Analytics* analytics = NULL;

    nb_boolean result = CreatePalAndContext(&g_pPal, &g_pContext);
    CU_ASSERT(result);
    if (!result)
    {
        return result;
    }

    nbResult = NB_AnalyticsCreate(g_pContext, NULL, &analytics);
    if(nbResult != NE_OK)
    {
        return FALSE;
    }
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(g_pContext, analytics);

    // Create search handler
    nbResult = NB_SearchHandlerCreate(g_pContext, &callbackData, &g_SearchState.pSearchHandler);
    CU_ASSERT_EQUAL(nbResult, NE_OK);

    if(nbResult != NE_OK)
    {
        return FALSE;
    }

    return TRUE;
}

/*! Cleanup PAL, context and search handler.

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

/*! Callback for formatted event content text.

    Gets called in response of calling NB_SearchInformationGetFormattedContentText

    @return NE_OK on success

    @see OutputEvents
*/
NB_Error 
FormattedTextCallback(NB_Font font,       /*!< Font used for returned text */ 
                      nb_color color,     /*!< Font color for returned text */ 
                      const char* text,   /*!< Returned text */ 
                      uint8 newline,      /*!< TRUE if a new line should be added. */ 
                      void* userData      /*!< User data passed in NB_SearchInformationGetFormattedContentText() */ 
                      )
{
    // The text field always has to be valid
    CU_ASSERT_PTR_NOT_NULL(text);

    if (newline)
    {
        LOGOUTPUT(LogLevelHigh, ("\n"));
    }
    else
    {
        // Output the event details
        LOGOUTPUT(LogLevelHigh, ("      Content Details (font:%d, color:%d): %s\n", font, color, text));
    }

    return NE_OK;
}

/*! Do a event subsearch.

    Subfunction of event, venue, movie and theater search.

    @return None. Assert on failure

    @see TestSearchBuilderEvents
    @see TestSearchBuilderVenues
    @see TestSearchBuilderMovies
    @see TestSearchBuilderTheaters
    @see TestSearchBuilderEventSubSearchForDifferentRegion
    @see TestSearchBuilderMovieSubSearchForDifferentRegion
*/
void 
EventSubsearch(NB_SearchParameters* parameters          /*!< parameters for the subsearches */
               )
{
    const int NUMBER_PER_SLICE = 5;
    const int SLICES_TO_DOWNLOAD = 2;
    int sliceIndex = 0;
    int totalEventCount = 0;
    nb_boolean moreSlices = TRUE;

    NB_SearchParameters* newParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    // For all slices
    for (sliceIndex = 0; (sliceIndex < SLICES_TO_DOWNLOAD) && moreSlices; sliceIndex++)
    {
        // We have the parameters for the first slice, but we need to create new parameters for
        // all the other slices.
        if (sliceIndex != 0)
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &newParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(newParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results. Either use the passed in parameter or the new
            // paramters for all subsequent searches
            information = DoRequest((sliceIndex == 0) ? parameters : newParameters, &slice, &moreSlices, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information && slice > 0)
            {
                int resultIndex = 0;

                // The place and events type has to be set
                CU_ASSERT(type & NB_SRT_Place);
                CU_ASSERT(type & NB_SRT_Events);

                // For all results in this slice. Could be less then requested number
                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    // Structures for results of request
                    NB_Place place = {{0}};
                    double distance = 0.0;
                    NB_Event* eventArray = NULL;
                    int eventCount = 0;

                    // Get place information
                    result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, NULL);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        // Output place information
                        OutputPoiResults(resultIndex + sliceIndex * NUMBER_PER_SLICE, distance, &place, NULL);
                    }

                    // get events
                    result = NB_SearchInformationGetEvents(information, resultIndex, &eventCount, &eventArray);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        // Output event information
                        OutputEvents(resultIndex + sliceIndex * NUMBER_PER_SLICE, eventCount, eventArray);

                        totalEventCount += eventCount;
                    }
                }
            }
        }

        // Free the new parameters for all subsequent searches. Has no effect if it is not set.
        FreeParameters(&newParameters);
    }

    LOGOUTPUT(LogLevelMedium, ("Total event count for subsearch: %d\n\n", totalEventCount));

    // Free last search information
    FreeInformation(&information);
}

/*! Subfunction of search along route test.

    @return NB_RouteInformation object, caller should destroy
*/
NB_RouteInformation*
CreateRouteToSearchAlong(NB_LatitudeLongitude* routeOrigin, NB_LatitudeLongitude* routeDestination)
{
    NB_Error result = NE_OK;

    NB_RouteParameters* routeParameters = 0;
    NB_RouteInformation* routeInformation = 0;
    NB_RouteHandler* routeHandler = 0;

    NB_Place origin = {{0}};  
    NB_Place dest = {{0}}; 
    NB_RouteOptions options = {0};
    NB_RouteConfiguration config = {0};
    
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) RouteHandlerCallback, 0 };    
    routeHandlerCallback.callbackData = &routeInformation;

    // Set route origin and destination
    {
        origin.location.type = NB_Location_LatLon;
        origin.location.latitude = routeOrigin->latitude;
        origin.location.longitude = routeOrigin->longitude;

        dest.location.type = NB_Location_LatLon;
        dest.location.latitude = routeDestination->latitude;
        dest.location.longitude = routeDestination->longitude;
    }
  
    // Set options and config
    {
        NB_GuidanceInformation* guidanceInfo = 0;
        guidanceInfo = CreateGuidanceInformation(g_pPal, g_pContext);                
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        options.avoid = NB_RouteAvoid_Toll; // NB_RouteAvoid_Highway | NB_RouteAvoid_Toll;
        options.transport = NB_TransportationMode_Car;
        options.type = NB_RouteType_Fastest;
        options.traffic = NB_TrafficType_None;
        options.pronunStyle = Test_OptionsGet()->voiceStyle;

        config.wantDestinationLocation = TRUE;
        config.wantManeuvers = TRUE;
        config.wantOriginLocation = TRUE;
        config.wantPolyline = TRUE;
        config.wantRouteMap = FALSE;
        config.commandSetVersion = NB_GuidanceInformationGetCommandVersion(guidanceInfo);
        
        NB_GuidanceInformationDestroy(guidanceInfo);
    }

    result = NB_RouteParametersCreateFromPlace(g_pContext, &origin, &dest, &options, &config, &routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(routeParameters);

    result = NB_RouteHandlerCreate(g_pContext, &routeHandlerCallback, &routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(routeHandler);

    if (routeParameters && routeHandler)
    {
        result = NB_RouteHandlerStartRequest(routeHandler, routeParameters);            
        CU_ASSERT_EQUAL(result, NE_OK);

        if (NE_OK == result)
        {                
            // Wait for event here from callback. Timeout 1 min.
            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
            {
                CU_ASSERT_NOT_EQUAL(routeInformation, 0);
            }
        }
    }

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    return routeInformation;
}

/*! Subfunction of search along route test.

@return None. CUnit Asserts get called on failures.
*/
void SearchAlongEntireRoute(char* iconName, char* categoryCodes[], int categoryCodesCount)
{
    const char* outputFileNameBase = "POI_Alicia_To_RedHill";

    const uint32 searchMaxCount = 100;
    const uint32 searchWidth = 800;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    NB_RouteInformation* routeInformation = NULL;

    NB_LatitudeLongitude origin = { 33.607386, -117.695228 };       // 5 & Alicia
    NB_LatitudeLongitude destination = { 33.734283, -117.814528 };  // 5 & Red Hill

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (!InitializeAll())
    {
        return;
    }

    // Create route to search along
    routeInformation = CreateRouteToSearchAlong(&origin, &destination);

    if (routeInformation)
    {
        PAL_File* kmlFile = 0;
        NB_Place place = {{0}};
        NB_SearchRegion region = {0};

        uint32 time = 0;
        uint32 delay = 0;
        double distance = 0.0;

        char outputFileName[80] = { 0 };
        int n = 0;
        
        result = NB_RouteInformationGetOrigin(routeInformation, &place);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_RouteInformationGetSummaryInformation(routeInformation, &time, &delay, &distance);
        CU_ASSERT_EQUAL(result, NE_OK);

        region.type = NB_ST_RouteAndCenter;
        region.route = routeInformation;
        region.center.latitude = place.location.latitude;
        region.center.longitude = place.location.longitude;
        region.distance = (uint32)(distance * 1.05);
        region.width = searchWidth;

        nsl_strcpy(outputFileName, outputFileNameBase);
        for (n = 0; n < categoryCodesCount; n++)
        {
            nsl_strcat(outputFileName, "_");
            nsl_strcat(outputFileName, categoryCodes[n]);
        }
        nsl_strcat(outputFileName, ".kml");

        // Delete previous test file
        DeleteTemporaryTestFiles(g_pPal, outputFileName);

        kmlFile = Test_StartKML(g_pPal, outputFileName, outputFileName);
        if (kmlFile)
        {
            Test_WriteRoutePolylineToKML(kmlFile, routeInformation);
        }

        // Create search parameters 
        result = NB_SearchParametersCreatePOI(
            g_pContext, 
            &region, 
            NULL, 
            "atlasbook-standard",
            (const char**)categoryCodes, 
            categoryCodesCount,      
            searchMaxCount, 
            NB_EXT_None, 
            NULL,
            &searchParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(searchParameters);

        if (result == NE_OK)
        {
            int resultCount = 0;
            nb_boolean hasMoreResults = FALSE;
            NB_SearchResultType type = NB_SRT_None;

            information = DoRequest(searchParameters, &resultCount, &hasMoreResults, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            
            if (!resultCount)
            {
                CU_FAIL("Zero POIs returned; POIs expected...");
            }
            
            if (information && kmlFile)
            {
                char iconUrl[80] = { 0 };
                nsl_sprintf(iconUrl, "http://maps.google.com/mapfiles/kml/shapes/%s.png", iconName);
                Test_WritePOIsToKML(kmlFile, resultCount, iconUrl, information);
            }

            FreeParameters(&searchParameters);
        }

        NB_RouteInformationDestroy(routeInformation);
        Test_FinishKML(kmlFile);
    }

    // Free last search information
    FreeInformation(&information);

    CleanupAll();
    
    LOGOUTPUT(LogLevelHigh, ("\n"));
}

void RouteHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    if (!up)
    {        
        NB_RouteInformation** route = (NB_RouteInformation**)userData;
        *route = 0;

        if (err != NE_OK)
        {
            // Abort and trigger event
            CU_FAIL("Callback returned error");
            SetCallbackCompletedEvent(g_CallbackEvent);
            return;
        }

        switch (status)
        {
        case NB_NetworkRequestStatus_Progress:
            // not complete yet... return
            return;

        case NB_NetworkRequestStatus_Success:        
            err = NB_RouteHandlerGetRouteInformation(handler, route);
            CU_ASSERT_EQUAL(err, NE_OK);
            break;
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
        SetCallbackCompletedEvent(g_CallbackEvent);
    }
}


/*! Subfunction of consistency test.

    See TestSearchBuilderServerConsistency() for more details.

    @return Total count of returned results.
    
    @see TestSearchBuilderServerConsistency
*/
int
ConsistencyTest(int numberPerSlice)
{
    const int SLICES_TO_DOWNLOAD = 100;     // The test will abort once the search is exhausted
    int sliceIndex = 0;
    nb_boolean moreSlices = TRUE;
    int totalCount = 0;
    char *coffeShopCategoryCode[1] = { "XEABM" };

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    // Reset static place array
    g_placeCount = 0;
    nsl_memset(g_allPlaces, 0, sizeof(*g_allPlaces));

    // For all slices
    for (sliceIndex = 0; (sliceIndex < SLICES_TO_DOWNLOAD) && moreSlices; sliceIndex++)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            // Set center of search to Aliso Viejo (I think)
            NB_SearchRegion region = {0};
            region.type = NB_ST_Center;
            region.center.latitude = 33.58;
            region.center.longitude = -117.73;

            // Create search parameters 
            result = NB_SearchParametersCreatePOI(g_pContext, 
                                                  &region, 
                                                  "Coffee",
                                                  NULL, 
                                                  (const char**)coffeShopCategoryCode,
                                                  1, 
                                                  numberPerSlice, 
                                                  NB_EXT_None,
                                                  NULL, 
                                                  &searchParameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(searchParameters);
        }
        else
        {
            if (information)
            {
                // Create search parameters for next slice
                result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);

                // Free the old search information
                FreeInformation(&information);
            }
        }

        if (result == NE_OK)
        {
            int slice = 0;
            NB_SearchResultType type = NB_SRT_None;

            // Do the server request and download the results
            information = DoRequest(searchParameters, &slice, &moreSlices, &type);
            CU_ASSERT_PTR_NOT_NULL(information);
            if (information)
            {
                int resultIndex = 0;

                // The place type has to be set
                CU_ASSERT(type & NB_SRT_Place);

                // For all results in this slice. Could be less then requested number
                for (resultIndex = 0; resultIndex < slice; resultIndex++)
                {
                    // Structures for results of request
                    NB_Place place = {{0}};
                    double distance = 0.0;

                    // Get place information
                    result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, NULL);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        totalCount++;

                        // Check if the new entry already exists
                        CheckPlaceConsistency(&place);

                        // Output place information
                        OutputPoiResults(resultIndex + sliceIndex * numberPerSlice, distance, &place, NULL);
                    }
                }
            }
        }

        FreeParameters(&searchParameters);
    }

    // Free last search information
    FreeInformation(&information);

    return totalCount;
}


/*! Subfunction of consistency test for premium search.

    See TestSearchBuilderServerConsistency() for more details.

    @return Total count of returned results.
    
    @see TestSearchBuilderServerConsistency
*/
int
ConsistencyTestPremium(int numberPerSlice, char* searchForName, char* categoryCodes[], int categoryCodesCount, NB_POIExtendedConfiguration premiumFlags)
{
    const int SLICES_TO_DOWNLOAD = 100;     // The test will abort once the search is exhausted
    const int NO_PREMIUM_SEARCH_PASS = 0;
    const int PREMIUM_SEARCH_PASS = 1;
    int pass = 0;
    int sliceIndex = 0;
    nb_boolean moreSlices = TRUE;
    int totalCount = 0;

    NB_SearchParameters* searchParameters = NULL;
    NB_SearchInformation* information = NULL;
    NB_Error result = NE_OK;

    // Reset static place arrays
    // Contains search result without requesting premium placement ads
    g_placeCount = 0;
    nsl_memset(g_allPlaces, 0, sizeof(*g_allPlaces));

    // Contains search result WITH requesting premium placement ads
    g_placeCountPremium = 0;
    nsl_memset(g_allPlacesPremium, 0, sizeof(*g_allPlacesPremium));

    for (pass = NO_PREMIUM_SEARCH_PASS; pass <= PREMIUM_SEARCH_PASS; pass++)
    {
        moreSlices = TRUE;
        totalCount = 0;

        // For all slices
        for (sliceIndex = 0; (sliceIndex < SLICES_TO_DOWNLOAD) && moreSlices; sliceIndex++)
        {
            // If it is the first slice then we need to create our initial parameter object
            if (sliceIndex == 0)
            {
                // Set center of search to Aliso Viejo (I think)
                NB_POIExtendedConfiguration extendedConfig = NB_EXT_None;
                NB_SearchRegion region = {0};
                region.type = NB_ST_Center;
                region.center.latitude = 33.58;
                region.center.longitude = -117.73;

                if (pass == PREMIUM_SEARCH_PASS)
                {
                    extendedConfig = premiumFlags;
                }

                // Create search parameters 
                result = NB_SearchParametersCreatePOI(g_pContext, 
                                                      &region, 
                                                      searchForName,
                                                      NULL, 
                                                      (const char**)categoryCodes,
                                                      categoryCodesCount, 
                                                      numberPerSlice, 
                                                      extendedConfig,
                                                      NULL, 
                                                      &searchParameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(searchParameters);
            }
            else
            {
                if (information)
                {
                    // Create search parameters for next slice
                    result = NB_SearchParametersCreateIteration(g_pContext, information, NB_IterationCommand_Next, &searchParameters);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(searchParameters);

                    // Free the old search information
                    FreeInformation(&information);
                }
            }

            if (result == NE_OK)
            {
                int slice = 0;
                NB_SearchResultType type = NB_SRT_None;

                // Do the server request and download the results
                information = DoRequest(searchParameters, &slice, &moreSlices, &type);
                CU_ASSERT_PTR_NOT_NULL(information);
                if (information)
                {
                    int resultIndex = 0;

                    // The place type has to be set
                    CU_ASSERT(type & NB_SRT_Place);

                    // For all results in this slice. Could be less then requested number
                    for (resultIndex = 0; resultIndex < slice; resultIndex++)
                    {
                        // Structures for results of request
                        NB_Place place = {{0}};
                        NB_ExtendedPlace* extendedPlace = NULL;
                        double distance = 0.0;

                        // Get place information
                        if (pass == NO_PREMIUM_SEARCH_PASS)
                        {
                            result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, NULL);
                        }
                        else
                        {
                            result = NB_SearchInformationGetPlace(information, resultIndex, &place, &distance, &extendedPlace);
                        }

                        CU_ASSERT_EQUAL(result, NE_OK);
                        if (result == NE_OK)
                        {
                            totalCount++;

                            if (pass == NO_PREMIUM_SEARCH_PASS)
                            {
                                SavePlaceNoPremiumSearch(&place);

                            }
                            else
                            {
                                SavePlacePremiumSearch(&place, extendedPlace);
                            }

                            // Output place information
                            OutputPoiResults(resultIndex + sliceIndex * numberPerSlice, distance, &place, extendedPlace);
                        }
                    }
                }
            }

            FreeParameters(&searchParameters);
        }

        // Free last search information
        FreeInformation(&information);
    }

    // The two search should match
    CheckPlaceConsistencyPremium();

    return totalCount;
}




/*! Check the consistency of the returned places.

    This function adds all new places into a static array and then compares any
    new entries to all existing entries. It asserts if an entry is identical to an
    existing entry

    @return None. CUnit asserts get called on failures.

    @see TestSearchBuilderServerConsistency
    @see ConsistencyTest
*/
void 
CheckPlaceConsistency(const NB_Place* pNewPlace)
{
    int placeIndex = 0;

    // Convert the most important information of the place into one concatenated string
    // so that we can easier compare it.
    char newPlaceString[MAX_PLACE_STRING];
    memset(&newPlaceString, 0, sizeof(newPlaceString));

    // Add name of place to the string
    strcat(newPlaceString, pNewPlace->name);
    strcat(newPlaceString, " ");

    // Add all the location information to the string
    strcat(newPlaceString, pNewPlace->location.areaname);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.streetnum);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.street1);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.street2);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.city);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.county);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.state);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.postal);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.country);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.airport);

    // Add it to our static array
    strcpy(g_allPlaces[g_placeCount], newPlaceString);

    // for all the existing places
    for (; placeIndex < g_placeCount; ++placeIndex) 
    {
        // Ensure that the entries are unique. Don't output them here. We'll do this in the 
        // function OutputConsistencyResults()
        CU_ASSERT_STRING_NOT_EQUAL(g_allPlaces[placeIndex], newPlaceString);
    }

    ++g_placeCount;
}


/*! Check the consistency of non-premium vs. premium search result.

    It checks to make sure that the number of non-premium POIs returned
    by the server is the same as when premium placement is requested.

    @return None. CUnit asserts get called on failures.

    @see TestSearchBuilderPremiumServerConsistency
    @see ConsistencyTestPremium
*/
void 
CheckPlaceConsistencyPremium()
{
    int placeIndex = 0;

    CU_ASSERT_EQUAL(g_placeCountPremium, g_placeCount);

    // Compare results between non-premium and premium
    for (; placeIndex < g_placeCount; ++placeIndex) 
    {
        // Ensure that the entries are the same.
        CU_ASSERT_STRING_EQUAL(g_allPlaces[placeIndex], g_allPlacesPremium[placeIndex]);
    }

}


/*! Save Place information into static array for non premium search.

    This function adds all new places into a static array for
    no premium searches.

    @return None. CUnit asserts get called on failures.

    @see TestSearchBuilderPremiumServerConsistency
    @see ConsistencyTestPremium
*/
void
SavePlaceNoPremiumSearch(const NB_Place* pNewPlace)
{
    // Convert the most important information of the place into one concatenated string
    // so that we can easier compare it.
    char newPlaceString[MAX_PLACE_STRING];
    memset(&newPlaceString, 0, sizeof(newPlaceString));

    // Add name of place to the string
    strcat(newPlaceString, pNewPlace->name);
    strcat(newPlaceString, " ");

    // Add all the location information to the string
    strcat(newPlaceString, pNewPlace->location.areaname);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.streetnum);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.street1);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.street2);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.city);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.county);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.state);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.postal);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.country);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.airport);

    // Add it to our static array
    strcpy(g_allPlaces[g_placeCount], newPlaceString);

    ++g_placeCount;
}


/*! Save Place information into static array for non premium search.

    This function adds all new places into a static array for
    no premium searches.

    @return None. CUnit asserts get called on failures.

    @see TestSearchBuilderPremiumServerConsistency
    @see ConsistencyTestPremium
*/
void
SavePlacePremiumSearch(const NB_Place* pNewPlace, const NB_ExtendedPlace* pExtendedPlace)
{
    // Convert the most important information of the place into one concatenated string
    // so that we can easier compare it.
    char newPlaceString[MAX_PLACE_STRING];
    memset(&newPlaceString, 0, sizeof(newPlaceString));

    // Discard any premium places
    if ((pExtendedPlace->extendedInformation & NB_EPI_PremiumPlacement) ||
        (pExtendedPlace->extendedInformation & NB_EPI_EnhancedPOI))
    {
        return;
    }

    // Add name of place to the string
    strcat(newPlaceString, pNewPlace->name);
    strcat(newPlaceString, " ");

    // Add all the location information to the string
    strcat(newPlaceString, pNewPlace->location.areaname);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.streetnum);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.street1);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.street2);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.city);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.county);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.state);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.postal);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.country);
    strcat(newPlaceString, " ");
    strcat(newPlaceString, pNewPlace->location.airport);

    // Add it to our static array
    strcpy(g_allPlacesPremium[g_placeCountPremium], newPlaceString);

    ++g_placeCountPremium;
}



/*! Output all places in the place array and check for consistency

    The place array only contains entries if the consistency test was turned on.
    This function outputs all duplicates.

    @return None

    @see TestSearchBuilderServerConsistency
    @see ConsistencyTest
    @see CheckPlaceConsistency
*/
void 
OutputConsistencyResults()
{
    int i = 0;
    int j = 0;
    nb_boolean duplicatesDetected = FALSE;

    LOGOUTPUT(LogLevelLow, ("\nAll Entries:\n"));

    // Output all the places
    for (i = 0; i < g_placeCount; ++i) 
    {
        LOGOUTPUT(LogLevelLow, ("Entry (%d): %s\n", i + 1, g_allPlaces[i]));
    }

    // Output all the duplicates

    LOGOUTPUT(LogLevelLow, ("\nDuplicates detected:\n"));

    // for all the existing places
    for (i = 0; i < g_placeCount; ++i) 
    {
        // Inner loop. All remaining places
        for (j = i + 1; j < g_placeCount; ++j) 
        {
            // Compare the entries and make sure we don't have duplicates.
            if (strcmp(g_allPlaces[i], g_allPlaces[j]) == 0) 
            {
                // Output the duplicates
                // Don't assert here. We've already done this in CheckPlaceConsistency().
                LOGOUTPUT(LogLevelLow, ("\t[%d - %d]\n", i + 1, j + 1));

                duplicatesDetected = TRUE;
            }
        }
    }

    if (! duplicatesDetected)
    {
        LOGOUTPUT(LogLevelLow, ("None\n"));
    }
}


/*! Output results from a poi request.

    @return None
*/
void 
OutputPoiResults(int index,                 /*!< Index of the place to output. */
                 double distance,           /*!< Distance to current position. */
                 const NB_Place* pPlace,    /*!< Place to output. */
                 const NB_ExtendedPlace* pExtendedPlace)   /*!< More info about the Place. */
{
    NB_Error result = NE_OK;
    int number = 0;
    int category = 0;
    int i = 0;

    LOGOUTPUT(LogLevelHigh, ("\nPlace %d: (Distance: %f)\n", index + 1, distance));

    LOGOUTPUT(LogLevelHigh, ("Name: %s\n", pPlace->name));

    // Output location
    OutputFormattedString("Areaname",     pPlace->location.areaname);
    OutputFormattedString("Streetnum",    pPlace->location.streetnum);
    OutputFormattedString("Street1",      pPlace->location.street1);
    OutputFormattedString("Street2",      pPlace->location.street2);
    OutputFormattedString("City",         pPlace->location.city);
    OutputFormattedString("County",       pPlace->location.county);
    OutputFormattedString("State",        pPlace->location.state);
    OutputFormattedString("Postal",       pPlace->location.postal);
    OutputFormattedString("Country",      pPlace->location.country);
    OutputFormattedString("Airport",      pPlace->location.airport);

    LOGOUTPUT(LogLevelHigh, ("Latitude:\t%f\n",     pPlace->location.latitude));
    LOGOUTPUT(LogLevelHigh, ("Longitude:\t%f\n",    pPlace->location.longitude));

//	pPlace->location.type;

    LOGOUTPUT(LogLevelHigh, ("\nPhone numbers:\n"));

    // Output phone numbers
    for (number = 0; number < pPlace->numphone; ++number) 
    {
//        pPlace->phone->type;
        LOGOUTPUT(LogLevelHigh, ("Country:\t%s\n",  pPlace->phone->country));
        LOGOUTPUT(LogLevelHigh, ("Area:\t%s\n",     pPlace->phone->area));
        LOGOUTPUT(LogLevelHigh, ("Number:\t%s\n",   pPlace->phone->number));
    }

    LOGOUTPUT(LogLevelHigh, ("\nCategories:\n"));

    // Output categories
    for (category = 0; category < pPlace->numcategory; ++category) 
    {
        LOGOUTPUT(LogLevelHigh, ("Code:\t%s\n", pPlace->category->code));
        LOGOUTPUT(LogLevelHigh, ("Name:\t%s\n", pPlace->category->name));
    }

    // Output additional information
    if (pExtendedPlace && 
        ((pExtendedPlace->extendedInformation & NB_EPI_PremiumPlacement) ||
        (pExtendedPlace->extendedInformation & NB_EPI_EnhancedPOI)))
    {
        // Premium placement?
        if (pExtendedPlace->extendedInformation & NB_EPI_PremiumPlacement)
        {
            LOGOUTPUT(LogLevelHigh, ("\nPremium Placement Ad\n"));
        }

        // Enhanced POI?
        if (pExtendedPlace->extendedInformation & NB_EPI_EnhancedPOI)
        {
            LOGOUTPUT(LogLevelHigh, ("Enhanced POI\n"));
        }

        // Ad provider
        LOGOUTPUT(LogLevelHigh, ("Ad Source:\t%s\n", pExtendedPlace->goldenCookie.providerId));

        // POI rating
        LOGOUTPUT(LogLevelHigh, ("Average Rating:\t%f\n", pExtendedPlace->overallRating.averageRating));
        LOGOUTPUT(LogLevelHigh, ("Rating Count:\t%d\n", pExtendedPlace->overallRating.ratingCount));

        // Output POI content
        if (pExtendedPlace->extendedInformation & NB_EPI_POIContent)
        {
            LOGOUTPUT(LogLevelHigh, ("Tagline:\t%s\n", pExtendedPlace->tagline));

            if (pExtendedPlace->poiContentCount != 0)
            {
                for (i = 0; i < pExtendedPlace->poiContentCount; i++)
                {
                    LOGOUTPUT(LogLevelHigh, ("%s:\t%s\n", pExtendedPlace->poiContent[i].key, pExtendedPlace->poiContent[i].value));
                }
            }
        }

        if (pExtendedPlace && pExtendedPlace->formattedContentHandle)
        {
            // Get the formatted text output. See FormattedTextCallback() for the actual data.
            // @todo: Remove cast once the structures are updated.
            result = NB_SearchInformationGetFormattedContentText((NB_SearchFormattedTextContentHandle*)(pExtendedPlace->formattedContentHandle), &FormattedTextCallback, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }
}

/*! Simple helper function for string output.

    Formates the output and only outputs the string if the value is not empty.

    @return None
*/
void 
OutputFormattedString(const char* pName,    /*!< Name of value to output */
                      const char* pValue)   /*!< Value string. Can be NULL or empty */
{
    // Only output if the value is present
    if ((! pValue) || (strlen(pValue) == 0))
    {
        return;
    }

    LOGOUTPUT(LogLevelHigh, ("%s\t:\t%s\n", pName, pValue));
}

/*! Output the fuel summary information.

    @return None.
*/
void 
OutputFuelSummary(const NB_FuelSummary* summary)
{
    // Don't assert if the average and low values are empty. They can be empty if
    // none of the gas stations in the slice have any fuel details.

    // Output average price
    LOGOUTPUT(LogLevelHigh, ("\nAverage price: %f %s, Units: %s. ", 
           summary->average.price.value, 
           summary->average.price.currency,
           summary->average.units));

    // Output average fuel type
    LOGOUTPUT(LogLevelHigh, ("Product: %s, Code: %s, Type: %s",
           summary->average.fuel_type.product_name,
           summary->average.fuel_type.code,
           summary->average.fuel_type.type_name));

    // Output low price
    LOGOUTPUT(LogLevelHigh, ("\nLow price: %f %s, Units: %s. ", 
           summary->low.price.value, 
           summary->low.price.currency,
           summary->low.units));

    // Output low fuel type
    LOGOUTPUT(LogLevelHigh, ("Product: %s, Code: %s, Type: %s\n",
           summary->low.fuel_type.product_name,
           summary->low.fuel_type.code,
           summary->low.fuel_type.type_name));
}

/*! Output fuel details.
    
    @return None
*/
void
OutputFuelDetails(const NB_FuelDetails* details)
{
    int fuelIndex = 0;

    if (details->num_fuelproducts == 0)
    {
        LOGOUTPUT(LogLevelHigh, ("   No fuel details.\n"));
        return;
    }

    // For all fuel products
    for (fuelIndex = 0; fuelIndex < details->num_fuelproducts; ++fuelIndex)
    {
        const NB_FuelProduct* pFuel = &(details->fuel_product[fuelIndex]);

        // Output fuel price
        LOGOUTPUT(LogLevelHigh, ("   Fuel [%d] price: %f %s, Units: %s. ", 
               fuelIndex,
               pFuel->price.value, 
               pFuel->price.currency,
               pFuel->units));

        // Output fuel type
        LOGOUTPUT(LogLevelHigh, ("Product: %s, Code: %s, Type: %s\n",
               pFuel->fuel_type.product_name,
               pFuel->fuel_type.code,
               pFuel->fuel_type.type_name));
    }
}

/*! Output weather conditions and weather forecasts.

    @return None
*/
void 
OutputWeatherResults(const NB_WeatherConditions* conditions, const NB_WeatherForecasts* forecasts)
{
    int forecastIndex = 0;

    // Output the current weather condition
    LOGOUTPUT(LogLevelHigh, ("Current weather:\n"
           "   temp:\t%1.1f\n"
           "   dewpt:\t%1.1f\n"
           "   rel_humidity:\t%d\n"
           "   wind_speed:\t%1.1f\n"
           "   wind_dir:\t%d\n"
           "   wind_gust:\t%1.1f\n"
           "   pressure:\t%1.1f\n"
           "   condition_code:\t%d\n"
           "   condition:\t%s\n"
           "   sky:\t%s\n"
           "   ceil:\t%1.1f\n"
           "   visibility:\t%d\n"
           "   heat_index:\t%1.1f\n"
           "   wind_chill:\t%1.1f\n"
           "   snow_depth:\t%1.1f\n"
           "   max_temp_6hr:\t%1.1f\n"
           "   max_temp_24hr:\t%1.1f\n"
           "   min_temp_6hr:\t%1.1f\n"
           "   min_temp_24hr:\t%1.1f\n"
           "   precipitation_3hr:\t%1.1f\n"
           "   precipitation_6hr:\t%1.1f\n"
           "   precipitation_24hr:\t%1.1f\n"
           "   update_time:\t%d\n"
           "   utc_offset:\t%d\n\n",
           conditions->temp,
           conditions->dewpt,
           conditions->rel_humidity,
           conditions->wind_speed,
           conditions->wind_dir,
           conditions->wind_gust,
           conditions->pressure,
           conditions->condition_code,
           conditions->condition,
           conditions->sky,
           conditions->ceil,
           conditions->visibility,
           conditions->heat_index,
           conditions->wind_chill,
           conditions->snow_depth,
           conditions->max_temp_6hr,
           conditions->max_temp_24hr,
           conditions->min_temp_6hr,
           conditions->min_temp_24hr,
           conditions->precipitation_3hr,
           conditions->precipitation_6hr,
           conditions->precipitation_24hr,
           conditions->update_time,
           conditions->utc_offset));

    // For all returned forcasts
    for (forecastIndex = 0; forecastIndex < forecasts->num_weather_forecasts; ++forecastIndex)
    {
        const NB_WeatherForecast* forecast = &(forecasts->weather_forecasts[forecastIndex]);

        // Output the forcast
        LOGOUTPUT(LogLevelHigh, ("Forcast[%d]:\n"
               "   date:\t%d\n"
               "   high_temp:\t%1.1f\n"
               "   low_temp:\t%1.1f\n"
               "   wind_dir:\t%d\n"
               "   wind_speed:\t%1.1f\n"
               "   precipitation_probability:\t%d\n"
               "   rel_humidity:\t%d\n"
               "   condition_code:\t%d\n"
               "   condition:\t%s\n"
               "   uv_index:\t%d\n"
               "   uv_desc:\t%s\n"
               "   sunrise:\t%s\n"
               "   sunset:\t%s\n"
               "   moonrise:\t%s\n"
               "   moonset:\t%s\n"
               "   moon_phase:\t%d\n"
               "   day_number:\t%d\n"
               "   update_time:\t%d\n"
               "   utc_offset:\t%d\n",
               forecastIndex,
               forecast->date,
               forecast->high_temp,
               forecast->low_temp,
               forecast->wind_dir,
               forecast->wind_speed,
               forecast->precipitation_probability,
               forecast->rel_humidity,
               forecast->condition_code,
               forecast->condition,
               forecast->uv_index,
               forecast->uv_desc,
               forecast->sunrise,
               forecast->sunset,
               forecast->moonrise,
               forecast->moonset,
               forecast->moon_phase,
               forecast->day_number,
               forecast->update_time,
               forecast->utc_offset));
    }
}

/*! Output event summary information.

    @return None
*/
void
OutputEventSummary(int resultIndex, const NB_SummaryResult* summary)
{
    // Print out the summary result
    LOGOUTPUT(LogLevelLow, ("Event Summary [%d], Code: %s, Count: %d, Name: %s, Time: %d\n", 
           resultIndex,
           summary->category.code, 
           summary->count,
           summary->category.name,
           summary->time));
}

/*! Output all events and performances.

    @return None
*/
void 
OutputEvents(int resultIndex, 
             int eventCount,            /*!< Number in event array */
             NB_Event* eventArray       
             )
{
    NB_Error result = NE_OK;

    // For all events for this venue
    int eventIndex = 0;
    for (eventIndex = 0; eventIndex < eventCount; ++eventIndex)
    {
        NB_Event* pEvent = &(eventArray[eventIndex]);

        int performanceIndex = 0;

        // Output the event information
        LOGOUTPUT(LogLevelLow, ("Event[%d] name: %s, MPAA: %s, Stars: %d\n",
        eventIndex,
        pEvent->name,
        pEvent->mpaa,
        pEvent->star));

        // For all performances for this event
        for (performanceIndex = 0; performanceIndex < pEvent->num_performance; ++performanceIndex)
        {
            NB_EventPerformance* pPerformance = &(pEvent->performances[performanceIndex]);

            // Output the performance
            LOGOUTPUT(LogLevelLow, ("   Performance[%d]: Start: %d, End: %d, Bargain: %s\n",
                   performanceIndex,
                   pPerformance->start_time,
                   pPerformance->end_time,
                   pPerformance->bargain_price ? "Yes" : "No"));
        }

        // Get the formatted text output. See FormattedTextCallback() for the actual data.
        // @todo: Remove cast once the structures are updated.
        result = NB_SearchInformationGetFormattedContentText((NB_SearchFormattedTextContentHandle*)(pEvent->contentHandle), &FormattedTextCallback, NULL);
        CU_ASSERT_EQUAL(result, NE_OK);
    }
}

/*! Output traffic incident details.

    @return None
*/
void 
OutputTrafficIncident(int resultIndex, const NB_TrafficIncident* incident)
{
    LOGOUTPUT(LogLevelMedium, 
             ("Traffic Incident[%d] type: %d, severity: %d, entry time: %d, start time: %d, end time: %d, utc offset: %d, id: %d\n",
              resultIndex,
              incident->type,
              incident->severity,
              incident->entry_time,
              incident->start_time,
              incident->end_time,
              incident->utc_offset,
              incident->original_item_id));

    LOGOUTPUT(LogLevelMedium, ("   Road: %s\n", incident->road));
    LOGOUTPUT(LogLevelMedium, ("   Description: %s\n", incident->description));
}


static PAL_Error
Test_WriteToKML(PAL_File* kmlFile, const char* data)
{
    uint32 size = nsl_strlen(data);
    uint32 written = 0;
    return PAL_FileWrite(kmlFile, (const uint8*) data, size, &written);
}

static PAL_File*
Test_StartKML(PAL_Instance* pal, const char* displayName, const char* kmlFileName)
{
    PAL_Error error = PAL_Ok;
    PAL_File* kmlFile = 0;

    error = error ? error : PAL_FileOpen(pal, kmlFileName, PFM_Create, &kmlFile);
    error = error ? error : Test_WriteToKML(kmlFile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
    error = error ? error : Test_WriteToKML(kmlFile, "<Document>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "  <name>");
    error = error ? error : Test_WriteToKML(kmlFile, displayName);
    error = error ? error : Test_WriteToKML(kmlFile, "</name>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "  <open>1</open>\n");

        
    return kmlFile;
}    

static PAL_Error
Test_WriteRoutePolylineToKML(PAL_File* kmlFile, NB_RouteInformation* route)
{
    NB_Error error = NE_OK;
    PAL_Error result = PAL_Ok;
    NB_MercatorPolyline* polyline;
    int index;
    double latitude;
    double longitude;
    char buffer[64];
    uint32 numManeuvers = 0;

    numManeuvers = NB_RouteInformationGetManeuverCount(route);
    CU_ASSERT(numManeuvers > 0);

    result = NB_RouteInformationGetRoutePolyline(route, 0, numManeuvers, &polyline);    
    CU_ASSERT_EQUAL(error, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(polyline);

    if (polyline)
    {
        result = result ? result : Test_WriteToKML(kmlFile, "<Style id=\"routePolylineStyle\">\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   <LineStyle>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <color>ff00ff00</color>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <width>10</width>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   </LineStyle>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "</Style>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "<Placemark>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   <name>Route Polyline</name>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   <styleUrl>#routePolylineStyle</styleUrl>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   <LineString>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <extrude>1</extrude>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <tessellate>1</tessellate>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <altitudeMode>clampToGround</altitudeMode>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <coordinates>\n");

        for (index = 0; index < polyline->count; index++)
        {
            NB_SpatialConvertMercatorToLatLong(polyline->points[index].mx, polyline->points[index].my, &latitude, &longitude);

            nsl_doubletostr(longitude, buffer, sizeof(buffer));
            result = result ? result : Test_WriteToKML(kmlFile, buffer);
            result = result ? result : Test_WriteToKML(kmlFile, ",");
            nsl_doubletostr(latitude, buffer, sizeof(buffer));
            result = result ? result : Test_WriteToKML(kmlFile, buffer);
            result = result ? result : Test_WriteToKML(kmlFile, " ");
        }

        result = result ? result : Test_WriteToKML(kmlFile, "\n       </coordinates>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   </LineString>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "</Placemark>\n");

        error = NB_MercatorPolylineDestroy(polyline);
        CU_ASSERT_EQUAL(error, NE_OK);
    }

    return result;
}

static PAL_Error
Test_WritePOIsToKML(PAL_File* kmlFile, int count, const char* image, NB_SearchInformation* searchInformation)
{
    PAL_Error error = PAL_Ok;
    int index;

    // For all results in this slice. Could be less then requested number
    for (index = 0; index < count; index++)
    {
        // Structures for results of request
        NB_Place place = {{0}};
        double distance = 0.0;

        // Get place information
        error = NB_SearchInformationGetPlace(searchInformation, index, &place, &distance, NULL);
        CU_ASSERT_EQUAL(error, NE_OK);
        if (error == NE_OK)
        {
            char buffer[64];
            uint16 n = 0;

            nsl_sprintf(buffer, "%s\n", place.name);
            for (n = 0; n < nsl_strlen(buffer); n++)
            {
                if (buffer[n] == '&')
                {
                    buffer[n] = '+';
                }
            }
            
            error = error ? error : Test_WriteToKML(kmlFile, "<Placemark>\n");        
            error = error ? error : Test_WriteToKML(kmlFile, "  <name>\n");
            error = error ? error : Test_WriteToKML(kmlFile, buffer);
            error = error ? error : Test_WriteToKML(kmlFile, "  </name>\n");   

            error = error ? error : Test_WriteToKML(kmlFile, "<StyleMap id=\"msn_dining\">");
            error = error ? error : Test_WriteToKML(kmlFile, "  <Pair>");
            error = error ? error : Test_WriteToKML(kmlFile, "      <key>normal</key>");
            error = error ? error : Test_WriteToKML(kmlFile, "      <styleUrl>#sn_dining</styleUrl>");
            error = error ? error : Test_WriteToKML(kmlFile, "  </Pair>");
            error = error ? error : Test_WriteToKML(kmlFile, "  <Pair>");
            error = error ? error : Test_WriteToKML(kmlFile, "      <key>highlight</key>");
            error = error ? error : Test_WriteToKML(kmlFile, "      <styleUrl>#sh_dining</styleUrl>");
            error = error ? error : Test_WriteToKML(kmlFile, "  </Pair>");
            error = error ? error : Test_WriteToKML(kmlFile, "</StyleMap>");
            error = error ? error : Test_WriteToKML(kmlFile, "<Style id=\"sn_dining\">");
            error = error ? error : Test_WriteToKML(kmlFile, "  <IconStyle>");
            error = error ? error : Test_WriteToKML(kmlFile, "      <scale>0.4</scale>");
            error = error ? error : Test_WriteToKML(kmlFile, "      <Icon>");
            error = error ? error : Test_WriteToKML(kmlFile, "          <href>");
            error = error ? error : Test_WriteToKML(kmlFile, image);
            error = error ? error : Test_WriteToKML(kmlFile, "</href>");
            error = error ? error : Test_WriteToKML(kmlFile, "      </Icon>");
            error = error ? error : Test_WriteToKML(kmlFile, "  <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
            error = error ? error : Test_WriteToKML(kmlFile, "  </IconStyle>");        
            error = error ? error : Test_WriteToKML(kmlFile, "  <ListStyle>");
            error = error ? error : Test_WriteToKML(kmlFile, "  </ListStyle>");
            error = error ? error : Test_WriteToKML(kmlFile, "</Style>");
            error = error ? error : Test_WriteToKML(kmlFile, "<Style id=\"sh_dining\">");
            error = error ? error : Test_WriteToKML(kmlFile, "  <IconStyle>");
            error = error ? error : Test_WriteToKML(kmlFile, "  <scale>0.6</scale>");
            error = error ? error : Test_WriteToKML(kmlFile, "  <Icon>");
            error = error ? error : Test_WriteToKML(kmlFile, "          <href>");
            error = error ? error : Test_WriteToKML(kmlFile, image);
            error = error ? error : Test_WriteToKML(kmlFile, "</href>");
            error = error ? error : Test_WriteToKML(kmlFile, "  </Icon>");
            error = error ? error : Test_WriteToKML(kmlFile, "  <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
            error = error ? error : Test_WriteToKML(kmlFile, "  </IconStyle>");        
            error = error ? error : Test_WriteToKML(kmlFile, "  <ListStyle>");
            error = error ? error : Test_WriteToKML(kmlFile, "  </ListStyle>");
            error = error ? error : Test_WriteToKML(kmlFile, "</Style>");
            error = error ? error : Test_WriteToKML(kmlFile, "  <Point>\n");        
            error = error ? error : Test_WriteToKML(kmlFile, "    <coordinates>");
            nsl_doubletostr(place.location.longitude, buffer, sizeof(buffer));
            error = error ? error : Test_WriteToKML(kmlFile, buffer);
            error = error ? error : Test_WriteToKML(kmlFile, ",");
            nsl_doubletostr(place.location.latitude, buffer, sizeof(buffer));
            error = error ? error : Test_WriteToKML(kmlFile, buffer);
            error = error ? error : Test_WriteToKML(kmlFile, ",0");
            error = error ? error : Test_WriteToKML(kmlFile, "</coordinates>\n");
            error = error ? error : Test_WriteToKML(kmlFile, "  </Point>\n");
            error = error ? error : Test_WriteToKML(kmlFile, "</Placemark>\n");
        }
    }

    return error;
}

static void
Test_FinishKML(PAL_File* kmlFile)
{
    PAL_Error error = PAL_Ok;

    if (kmlFile)
    {
        error = error ? error : Test_WriteToKML(kmlFile, "</Document>\n");
        error = error ? error : Test_WriteToKML(kmlFile, "</kml>\n");
        error = error ? error : PAL_FileClose(kmlFile);
    }
}


/*! @} */

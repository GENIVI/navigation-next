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

    @file testsinglesearch.c
    @defgroup testsinglesearch_h system tests for single search functions

    See description in header file.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#include "testsinglesearch.h"
#include "testnetwork.h"
#include "main.h"
#include "nbsinglesearchhandler.h"
#include "nbsinglesearchinformation.h"
#include "nbsinglesearchparameters.h"
#include "nbsearchinformation.h"

// Local Constants ...............................................................................

#define TEST_NUMBER_PER_SLICE       20
#define TEST_SLICES_TO_DOWNLOAD     5
#define TEST_RIGHT_MAX_KEYWORD_SIZE 16      // We just check if the requested keyword is same with 'TEST_SEARCH_NAME'
                                            // for suggestion search. Because single search will use the line 1 of
                                            // suggestion search result as the keyword. This size should be greater
                                            // than the length of 'TEST_SEARCH_NAME'.
#define TEST_WRONG_MAX_KEYWORD_SIZE 3       // Wrong size for testing.

#define TEST_LATITUDE               33.60095
#define TEST_LONGITUDE              -117.69163
#define TEST_SEARCH_SCHEME          "tcs-single-search-2"
#define TEST_SEARCH_NAME            "Coffee Shops"


// Local Types ...................................................................................

/*! Collection of all instances used for the unit tests */
typedef struct
{
    PAL_Instance*               pal;                /*!< Pointer to PAL */
    NB_Context*                 context;            /*!< Pointer to current context */

    NB_SingleSearchHandler*     handler;            /*!< Single search handler instance */
    NB_SingleSearchInformation* information;        /*!< Single search information instance */

    void*                       event;              /*!< Event gets triggered from the download callback */

} Instances;


// Test Functions ................................................................................

static void TestSingleSearchSimpleSearch(void);
static void TestSingleSearchSuggestionSearch(void);
static void TestSingleSearchDataAvailability(void);
static void TestSingleSearchAddressFormatting(void);
static void TestSingleSearchParentCategory(void);
static void TestSingleSearchHoursOfOperation(void);


// Helper Functions ..............................................................................

static nb_boolean CreateInstances(Instances* instances);
static void DestroyInstances(Instances* instances);
static nb_boolean IsInstancesValid(Instances* instances);

static NB_Error DoSuggestionSearch(Instances* instances, NB_SingleSearchInformation** suggestionInformation);
static NB_Error DoSearchBySuggestion(Instances* instances, NB_SingleSearchInformation* suggestionInformation, uint32 suggestionIndex);

NB_Error DoRequest(Instances* instances, NB_SingleSearchParameters* parameters, NB_SingleSearchInformation** information);

static NB_Error CheckSearchResultForResultStyleTypeOfSingleSearchAndGeocode(Instances* instances, NB_SingleSearchInformation* information);
static NB_Error CheckSearchResultForResultStyleTypeOfSuggestion(Instances* instances, NB_SingleSearchInformation* information);
static NB_Error CheckSuggestMatch(Instances* instances, NB_SuggestMatch* suggestMatch);

static void RequestHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);

static void LogOutputFormattedString(const char* name, const char* value);
static void LogOutputPlace(uint32 index, double distance, const NB_Place* place, const NB_ExtendedPlace* extendedPlace);
static void LogOutputSuggestMatchData(NB_SuggestMatchData* suggestMatchData);
static void LogOutputHoursOfOperation(uint32 index, const NB_HoursOfOperation* hoursOfOperation);


// Public Functions ..............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestSingleSearch_AddTests(CU_pSuite pTestSuite, int level)
{
    // Add all your function names here
//    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSingleSearchSimpleSearch", TestSingleSearchSimpleSearch);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSingleSearchSuggestionSearch", TestSingleSearchSuggestionSearch);
//    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSingleSearchDataAvailability", TestSingleSearchDataAvailability);
//    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSingleSearchAddressFormatting", TestSingleSearchAddressFormatting);
//    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSingleSearchParentCategory", TestSingleSearchParentCategory);
//    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSingleSearchHoursOfOperation", TestSingleSearchHoursOfOperation);
}

/*! Add common initialization code here.

    @return 0

    @see TestSingleSearch_SuiteCleanup
*/
int
TestSingleSearch_SuiteSetup()
{
    // Nothing to do

    return 0;
}

/*! Add common cleanup code here.

    @return 0

    @see TestSingleSearch_SuiteSetup
*/
int
TestSingleSearch_SuiteCleanup()
{
    // Nothing to do

    return 0;
}


// Test Functions ................................................................................

/*! Test simple search of result style of single search

    @return None. CUnit Asserts get called on failures.
*/
void
TestSingleSearchSimpleSearch()
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;
        int i = 0;
        NB_SingleSearchParameters* parameters = NULL;
        NB_SingleSearchInformation* information = NULL;

        for (i = 0; i < TEST_SLICES_TO_DOWNLOAD; ++i)
        {
            if (i == 0)
            {
                NB_SearchRegion region = {0};
                NB_ImageSize imageSize = {0};

                // Set center to New York
                region.type = NB_ST_Center;
                region.center.latitude = TEST_LATITUDE;
                region.center.longitude = TEST_LONGITUDE;

                result = NB_SingleSearchParametersCreateByResultStyleType(instances.context,
                                                                          &region,
                                                                          TEST_SEARCH_NAME,
                                                                          TEST_SEARCH_SCHEME,
                                                                          TEST_NUMBER_PER_SLICE,
                                                                          NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantNonProximityPois,
                                                                          NULL,
                                                                          NB_SRST_SingleSearch,
                                                                          &parameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(parameters);
                if (result != NE_OK)
                {
                    DestroyInstances(&instances);
                    return;
                }

                // Add an extended image specification for premium placements
                imageSize.width = 64;
                imageSize.height = 64;
                result = NB_SingleSearchParametersAddExtendedImage(parameters, NB_EXT_WantPremiumPlacement, NB_IF_PNG, &imageSize, 96);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result != NE_OK)
                {
                    NB_SingleSearchParametersDestroy(parameters);
                    DestroyInstances(&instances);
                    return;
                }

                result = NB_SingleSearchParametersSetSource(parameters, NB_SS_Carousel);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result != NE_OK)
                {
                    NB_SingleSearchParametersDestroy(parameters);
                    DestroyInstances(&instances);
                    return;
                }

                result = NB_SingleSearchParametersSetSource(parameters, NB_SS_None);
                CU_ASSERT_EQUAL(result, NE_INVAL);

                // Test to get search source.
                {
                    NB_SearchSource searchSource = NB_SS_None;

                    result = NB_SingleSearchParametersGetSource(parameters, &searchSource);
                    CU_ASSERT((result == NE_OK) && (searchSource == NB_SS_Place));
                    if (result != NE_OK)
                    {
                        NB_SingleSearchParametersDestroy(parameters);
                        DestroyInstances(&instances);
                        return;
                    }
                }
            }
            else
            {
                CU_ASSERT_PTR_NOT_NULL(information);
                if (!information)
                {
                    DestroyInstances(&instances);
                    return;
                }

                // Create search parameters for next slice
                result = NB_SingleSearchParametersCreateByIteration(instances.context,
                                                                    information,
                                                                    NB_IterationCommand_Next,
                                                                    &parameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(parameters);
                if (result != NE_OK)
                {
                    DestroyInstances(&instances);
                    return;
                }

                // Destroy the last search information
                NB_SingleSearchInformationDestroy(information);
                information = NULL;
            }

            result = DoRequest(&instances, parameters, &information);
            NB_SingleSearchParametersDestroy(parameters);
            parameters = NULL;
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result != NE_OK)
            {
                DestroyInstances(&instances);
                return;
            }

            CU_ASSERT_PTR_NOT_NULL(information);
            if (!information)
            {
                DestroyInstances(&instances);
                return;
            }

            result = CheckSearchResultForResultStyleTypeOfSingleSearchAndGeocode(&instances, information);
            CU_ASSERT_EQUAL(result, NE_OK);

            {
                nb_boolean hasMoreResults = FALSE;

                result = NB_SingleSearchInformationHasMoreResults(information, &hasMoreResults);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result != NE_OK || !hasMoreResults)
                {
                    NB_SingleSearchInformationDestroy(information);
                    information = NULL;
                    break;
                }
            }
        }

        if (parameters)
        {
            NB_SingleSearchParametersDestroy(parameters);
            parameters = NULL;
        }

        if (information)
        {
            NB_SingleSearchInformationDestroy(information);
            information = NULL;
        }

        DestroyInstances(&instances);
    }
}

/*! Test simple search of result style of suggestion search and search by a suggestion

    @return None. CUnit Asserts get called on failures.
*/
void
TestSingleSearchSuggestionSearch()
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;
        uint32 resultCount = 0;
        NB_SingleSearchInformation* information = NULL;

        result = DoSuggestionSearch(&instances, &information);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(information);
        if (result != NE_OK || !information)
        {
            DestroyInstances(&instances);
            return;
        }

        result = NB_SingleSearchInformationGetResultCount(information, &resultCount);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT(resultCount > 0);
        if (result != NE_OK || resultCount == 0)
        {
            DestroyInstances(&instances);
            return;
        }

        result = DoSearchBySuggestion(&instances, information, 0);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (information)
        {
            NB_SingleSearchInformationDestroy(information);
            information = NULL;
        }

        DestroyInstances(&instances);
    }
}

/*! Test getting data availability information

    @return None. CUnit Asserts get called on failures.
*/
void
TestSingleSearchDataAvailability()
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;
        NB_SingleSearchParameters* parameters = NULL;
        NB_SingleSearchInformation* information = NULL;

        NB_SearchRegion region = {0};

        // Set center to New York
        region.type = NB_ST_Center;
        region.center.latitude = TEST_LATITUDE;
        region.center.longitude = TEST_LONGITUDE;

        result = NB_SingleSearchParametersCreateByResultStyleType(instances.context,
                                                                  &region,
                                                                  NULL,
                                                                  TEST_SEARCH_SCHEME,
                                                                  TEST_NUMBER_PER_SLICE,
                                                                  NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantNonProximityPois,
                                                                  NULL,
                                                                  NB_SRST_DefaultLocation,
                                                                  &parameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(parameters);
        if (result != NE_OK)
        {
            DestroyInstances(&instances);
            return;
        }

        result = NB_SingleSearchParametersSetWantDataAvailability(parameters, TRUE);
        CU_ASSERT_EQUAL(result, NE_OK);
        if (result != NE_OK)
        {
            NB_SingleSearchParametersDestroy(parameters);
            DestroyInstances(&instances);
            return;
        }

        result = NB_SingleSearchParametersAddSearchFilterKeyValue(parameters, "countrycode", "USA");
        result = result ? result : NB_SingleSearchParametersSetSource(parameters, NB_SS_DefaultLocation);
        CU_ASSERT_EQUAL(result, NE_OK);
        if (result != NE_OK)
        {
            NB_SingleSearchParametersDestroy(parameters);
            DestroyInstances(&instances);
            return;
        }

        result = DoRequest(&instances, parameters, &information);
        NB_SingleSearchParametersDestroy(parameters);
        parameters = NULL;
        CU_ASSERT_EQUAL(result, NE_OK);
        if (result != NE_OK)
        {
            DestroyInstances(&instances);
            return;
        }

        CU_ASSERT_PTR_NOT_NULL(information);
        if (!information)
        {
            DestroyInstances(&instances);
            return;
        }

        result = CheckSearchResultForResultStyleTypeOfSingleSearchAndGeocode(&instances, information);
        CU_ASSERT_EQUAL(result, NE_OK);

        {
            uint32 count = 0;
            NB_SingleSearchInformationGetResultCount(information, &count);
            if (count > 0)
            {
                NB_DataAvailability dataAvail = {0};
                result = NB_SingleSearchInformationGetDataAvailability(information, 0, &dataAvail);
                CU_ASSERT_EQUAL(result, NE_OK);
                if( result == NE_OK )
                {
                    LOGOUTPUT(LogLevelHigh, ("noMapData:             %d\n", dataAvail.noMapData));
                    LOGOUTPUT(LogLevelHigh, ("partialVisualMapData:  %d\n", dataAvail.partialVisualMapData));
                    LOGOUTPUT(LogLevelHigh, ("fullVisualMapData:     %d\n", dataAvail.fullVisualMapData));
                    LOGOUTPUT(LogLevelHigh, ("partialRoutingData:    %d\n", dataAvail.partialRoutingData));
                    LOGOUTPUT(LogLevelHigh, ("fullRoutingData:       %d\n", dataAvail.fullRoutingData));
                    LOGOUTPUT(LogLevelHigh, ("partialGeocodingData:  %d\n", dataAvail.partialGeocodingData));
                    LOGOUTPUT(LogLevelHigh, ("fullGeocodingData:     %d\n", dataAvail.fullGeocodingData));
                    LOGOUTPUT(LogLevelHigh, ("partialPoiData:        %d\n", dataAvail.partialPoiData));
                    LOGOUTPUT(LogLevelHigh, ("fullPoiData:           %d\n", dataAvail.fullPoiData));
                }
            }
        }

        if (parameters)
        {
            NB_SingleSearchParametersDestroy(parameters);
            parameters = NULL;
        }

        if (information)
        {
            NB_SingleSearchInformationDestroy(information);
            information = NULL;
        }

        DestroyInstances(&instances);
    }
}

/*! Test getting address formatting information

    @return None. CUnit Asserts get called on failures.
*/
void
TestSingleSearchAddressFormatting()
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;
        int i = 0;
        NB_SingleSearchParameters* parameters = NULL;
        NB_SingleSearchInformation* information = NULL;

        for (i = 0; i < TEST_SLICES_TO_DOWNLOAD; ++i)
        {
            if (i == 0)
            {
                NB_SearchRegion region = {0};
                NB_ImageSize imageSize = {0};

                // Set center to New York
                region.type = NB_ST_Center;
                region.center.latitude = TEST_LATITUDE;
                region.center.longitude = TEST_LONGITUDE;

                result = NB_SingleSearchParametersCreateByResultStyleType(instances.context,
                                                                          &region,
                                                                          TEST_SEARCH_NAME,
                                                                          TEST_SEARCH_SCHEME,
                                                                          TEST_NUMBER_PER_SLICE,
                                                                          NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantNonProximityPois | NB_EXT_WantExtendedAddress | NB_EXT_WantCompactAddress | NB_EXT_WantFormattedPhone,
                                                                          NULL,
                                                                          NB_SRST_SingleSearch,
                                                                          &parameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(parameters);
                if (result != NE_OK)
                {
                    DestroyInstances(&instances);
                    return;
                }

                // Add an extended image specification for premium placements
                imageSize.width = 64;
                imageSize.height = 64;
                result = NB_SingleSearchParametersAddExtendedImage(parameters, NB_EXT_WantPremiumPlacement, NB_IF_PNG, &imageSize, 96);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result != NE_OK)
                {
                    NB_SingleSearchParametersDestroy(parameters);
                    DestroyInstances(&instances);
                    return;
                }

                result = NB_SingleSearchParametersSetSource(parameters, NB_SS_Place);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result != NE_OK)
                {
                    NB_SingleSearchParametersDestroy(parameters);
                    DestroyInstances(&instances);
                    return;
                }

                result = NB_SingleSearchParametersSetSource(parameters, NB_SS_None);
                CU_ASSERT_EQUAL(result, NE_INVAL);

                // Test to get search source.
                {
                    NB_SearchSource searchSource = NB_SS_None;

                    result = NB_SingleSearchParametersGetSource(parameters, &searchSource);
                    CU_ASSERT((result == NE_OK) && (searchSource == NB_SS_Place));
                    if (result != NE_OK)
                    {
                        NB_SingleSearchParametersDestroy(parameters);
                        DestroyInstances(&instances);
                        return;
                    }
                }
            }
            else
            {
                CU_ASSERT_PTR_NOT_NULL(information);
                if (!information)
                {
                    DestroyInstances(&instances);
                    return;
                }

                // Create search parameters for next slice
                result = NB_SingleSearchParametersCreateByIteration(instances.context,
                                                                    information,
                                                                    NB_IterationCommand_Next,
                                                                    &parameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(parameters);
                if (result != NE_OK)
                {
                    DestroyInstances(&instances);
                    return;
                }

                // Destroy the last search information
                NB_SingleSearchInformationDestroy(information);
                information = NULL;
            }

            result = DoRequest(&instances, parameters, &information);
            NB_SingleSearchParametersDestroy(parameters);
            parameters = NULL;
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result != NE_OK)
            {
                DestroyInstances(&instances);
                return;
            }

            CU_ASSERT_PTR_NOT_NULL(information);
            if (!information)
            {
                DestroyInstances(&instances);
                return;
            }

            result = CheckSearchResultForResultStyleTypeOfSingleSearchAndGeocode(&instances, information);
            CU_ASSERT_EQUAL(result, NE_OK);

            {
                nb_boolean hasMoreResults = FALSE;

                result = NB_SingleSearchInformationHasMoreResults(information, &hasMoreResults);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result != NE_OK || !hasMoreResults)
                {
                    NB_SingleSearchInformationDestroy(information);
                    information = NULL;
                    break;
                }
            }
        }

        if (parameters)
        {
            NB_SingleSearchParametersDestroy(parameters);
            parameters = NULL;
        }

        if (information)
        {
            NB_SingleSearchInformationDestroy(information);
            information = NULL;
        }

        DestroyInstances(&instances);
    }
}

/*! Test getting parent category

    @return None. CUnit Asserts get called on failures.
*/
void
TestSingleSearchParentCategory()
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;
        int i = 0;
        NB_SingleSearchParameters* parameters = NULL;
        NB_SingleSearchInformation* information = NULL;

        for (i = 0; i < TEST_SLICES_TO_DOWNLOAD; ++i)
        {
            if (i == 0)
            {
                NB_SearchRegion region = {0};

                // Set center to New York
                region.type = NB_ST_Center;
                region.center.latitude = TEST_LATITUDE;
                region.center.longitude = TEST_LONGITUDE;

                result = NB_SingleSearchParametersCreateByResultStyleType(instances.context,
                                                                          &region,
                                                                          TEST_SEARCH_NAME,
                                                                          TEST_SEARCH_SCHEME,
                                                                          TEST_NUMBER_PER_SLICE,
                                                                          NB_EXT_WantParentCategory,
                                                                          NULL,
                                                                          NB_SRST_SingleSearch,
                                                                          &parameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(parameters);
                if (result != NE_OK)
                {
                    DestroyInstances(&instances);
                    return;
                }

                result = NB_SingleSearchParametersSetSource(parameters, NB_SS_Place);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result != NE_OK)
                {
                    NB_SingleSearchParametersDestroy(parameters);
                    DestroyInstances(&instances);
                    return;
                }

                // Test to get search source.
                {
                    NB_SearchSource searchSource = NB_SS_None;

                    result = NB_SingleSearchParametersGetSource(parameters, &searchSource);
                    CU_ASSERT((result == NE_OK) && (searchSource == NB_SS_Place));
                    if (result != NE_OK)
                    {
                        NB_SingleSearchParametersDestroy(parameters);
                        DestroyInstances(&instances);
                        return;
                    }
                }
            }
            else
            {
                CU_ASSERT_PTR_NOT_NULL(information);
                if (!information)
                {
                    DestroyInstances(&instances);
                    return;
                }

                // Create search parameters for next slice
                result = NB_SingleSearchParametersCreateByIteration(instances.context,
                                                                    information,
                                                                    NB_IterationCommand_Next,
                                                                    &parameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(parameters);
                if (result != NE_OK)
                {
                    DestroyInstances(&instances);
                    return;
                }

                // Destroy the last search information
                NB_SingleSearchInformationDestroy(information);
                information = NULL;
            }

            result = DoRequest(&instances, parameters, &information);
            NB_SingleSearchParametersDestroy(parameters);
            parameters = NULL;
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result != NE_OK)
            {
                DestroyInstances(&instances);
                return;
            }

            CU_ASSERT_PTR_NOT_NULL(information);
            if (!information)
            {
                DestroyInstances(&instances);
                return;
            }

            result = CheckSearchResultForResultStyleTypeOfSingleSearchAndGeocode(&instances, information);
            CU_ASSERT_EQUAL(result, NE_OK);

            {
                nb_boolean hasMoreResults = FALSE;

                result = NB_SingleSearchInformationHasMoreResults(information, &hasMoreResults);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result != NE_OK || !hasMoreResults)
                {
                    NB_SingleSearchInformationDestroy(information);
                    information = NULL;
                    break;
                }
            }
        }

        if (parameters)
        {
            NB_SingleSearchParametersDestroy(parameters);
            parameters = NULL;
        }

        if (information)
        {
            NB_SingleSearchInformationDestroy(information);
            information = NULL;
        }

        DestroyInstances(&instances);
    }
}

/*! Test getting hours of operation

    @return None. CUnit Asserts get called on failures.
*/
void
TestSingleSearchHoursOfOperation()
{
    Instances instances = {0};

    if (CreateInstances(&instances))
    {
        NB_Error result = NE_OK;
        int i = 0;
        NB_SingleSearchParameters* parameters = NULL;
        NB_SingleSearchInformation* information = NULL;

        for (i = 0; i < TEST_SLICES_TO_DOWNLOAD; ++i)
        {
            if (i == 0)
            {
                NB_SearchRegion region = {0};

                // Set center to New York
                region.type = NB_ST_Center;
                region.center.latitude = TEST_LATITUDE;
                region.center.longitude = TEST_LONGITUDE;

                result = NB_SingleSearchParametersCreateByResultStyleType(instances.context,
                                                                          &region,
                                                                          TEST_SEARCH_NAME,
                                                                          TEST_SEARCH_SCHEME,
                                                                          TEST_NUMBER_PER_SLICE,
                                                                          NB_EXT_WantParentCategory |
                                                                          NB_EXT_WantStructuredHoursOfOperation,
                                                                          NULL,
                                                                          NB_SRST_SingleSearch,
                                                                          &parameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(parameters);
                if (result != NE_OK)
                {
                    DestroyInstances(&instances);
                    return;
                }

                result = NB_SingleSearchParametersSetSource(parameters, NB_SS_Place);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result != NE_OK)
                {
                    NB_SingleSearchParametersDestroy(parameters);
                    DestroyInstances(&instances);
                    return;
                }

                // Test to get search source.
                {
                    NB_SearchSource searchSource = NB_SS_None;

                    result = NB_SingleSearchParametersGetSource(parameters, &searchSource);
                    CU_ASSERT((result == NE_OK) && (searchSource == NB_SS_Place));
                    if (result != NE_OK)
                    {
                        NB_SingleSearchParametersDestroy(parameters);
                        DestroyInstances(&instances);
                        return;
                    }
                }
            }
            else
            {
                CU_ASSERT_PTR_NOT_NULL(information);
                if (!information)
                {
                    DestroyInstances(&instances);
                    return;
                }

                // Create search parameters for next slice
                result = NB_SingleSearchParametersCreateByIteration(instances.context,
                                                                    information,
                                                                    NB_IterationCommand_Next,
                                                                    &parameters);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(parameters);
                if (result != NE_OK)
                {
                    DestroyInstances(&instances);
                    return;
                }

                // Destroy the last search information
                NB_SingleSearchInformationDestroy(information);
                information = NULL;
            }

            result = DoRequest(&instances, parameters, &information);
            NB_SingleSearchParametersDestroy(parameters);
            parameters = NULL;
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result != NE_OK)
            {
                DestroyInstances(&instances);
                return;
            }

            CU_ASSERT_PTR_NOT_NULL(information);
            if (!information)
            {
                DestroyInstances(&instances);
                return;
            }

            result = CheckSearchResultForResultStyleTypeOfSingleSearchAndGeocode(&instances, information);
            CU_ASSERT_EQUAL(result, NE_OK);

            {
                nb_boolean hasMoreResults = FALSE;

                result = NB_SingleSearchInformationHasMoreResults(information, &hasMoreResults);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result != NE_OK || !hasMoreResults)
                {
                    NB_SingleSearchInformationDestroy(information);
                    information = NULL;
                    break;
                }
            }
        }

        if (parameters)
        {
            NB_SingleSearchParametersDestroy(parameters);
            parameters = NULL;
        }

        if (information)
        {
            NB_SingleSearchInformationDestroy(information);
            information = NULL;
        }

        DestroyInstances(&instances);
    }
}


// Helper Functions ..............................................................................

/*! Create PAL, context, single search handler and event for callback

    Shared by all unit tests.

    @return TRUE on success, FALSE otherwise
*/
nb_boolean
CreateInstances(Instances* instances                /*!< The Instances object to create */
                )
{
    NB_Error result = NE_OK;
    NB_RequestHandlerCallback callback = {RequestHandlerCallback, NULL};

    if (!instances)
    {
        return FALSE;
    }

    callback.callbackData = instances;

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

    result = NB_SingleSearchHandlerCreate(instances->context, &callback, &(instances->handler));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(instances->handler);

    if (result != NE_OK)
    {
        DestroyInstances(instances);
        return FALSE;
    }

    instances->information = NULL;

    return TRUE;
}

/*! Destroy PAL, context, single search handler and event for callback

    Shared by all unit tests.

    @return None
*/
void
DestroyInstances(Instances* instances               /*!< The Instances object to destroy */
                 )
{
    if (instances->information)
    {
        NB_SingleSearchInformationDestroy(instances->information);
    }

    if (instances->handler)
    {
        NB_SingleSearchHandlerDestroy(instances->handler);
    }

    if (instances->context)
    {
        DestroyContext(instances->context);
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

/*! Check if the Instances object is valid

    @return TRUE if the Instances object is valid, FALSE otherwise
*/
nb_boolean
IsInstancesValid(Instances* instances               /*!< The Instances object to check */
                 )
{
    if (!instances)
    {
        return FALSE;
    }

    return (nb_boolean) (instances->pal && instances->context && instances->handler && instances->event);
}

/*! Do a suggestion search

    @return NE_OK if success
*/
NB_Error
DoSuggestionSearch(Instances* instances,                                /*!< The Instances object */
                   NB_SingleSearchInformation** suggestionInformation   /*!< On return the result of suggestion search */
                   )
{
    NB_Error result = NE_OK;
    NB_SingleSearchParameters* parameters = NULL;
    NB_SingleSearchInformation* information = NULL;
    NB_SearchRegion region = {0};

    if (!instances || !suggestionInformation || !IsInstancesValid(instances))
    {
        return NE_INVAL;
    }

    // Set center to Dallas
    region.type = NB_ST_Center;
    region.center.latitude = 32.80;
    region.center.longitude = -96.80;

    result = NB_SingleSearchParametersCreateByResultStyleType(instances->context,
                                                              &region,
                                                              TEST_SEARCH_NAME,
                                                              TEST_SEARCH_SCHEME,
                                                              TEST_NUMBER_PER_SLICE,
                                                              NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantNonProximityPois,
                                                              NULL,
                                                              NB_SRST_Suggestion,
                                                              &parameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(parameters);
    if (result != NE_OK)
    {
        return result;
    }

    result = NB_SingleSearchParametersSetSource(parameters, NB_SS_Carousel);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result != NE_OK)
    {
        NB_SingleSearchParametersDestroy(parameters);
        return result;
    }

    result = DoRequest(instances, parameters, &information);
    NB_SingleSearchParametersDestroy(parameters);
    parameters = NULL;
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result != NE_OK)
    {
        return result;
    }

    CU_ASSERT_PTR_NOT_NULL(information);
    if (!information)
    {
        return NE_UNEXPECTED;
    }

    result = CheckSearchResultForResultStyleTypeOfSuggestion(instances, information);
    CU_ASSERT_EQUAL(result, NE_OK);

    *suggestionInformation = information;

    return NE_OK;
}

/*! Do a search by a result of suggestion search

    @return NE_OK if success
*/
NB_Error
DoSearchBySuggestion(Instances* instances,                              /*!< The Instances object */
                     NB_SingleSearchInformation* suggestionInformation, /*!< The result of suggestion search */
                     uint32 suggestionIndex                             /*!< The index of the suggest match */
                     )
{
    NB_Error result = NE_OK;
    int i = 0;
    NB_SingleSearchParameters* parameters = NULL;
    NB_SingleSearchInformation* information = NULL;

    if (!instances || !suggestionInformation || !IsInstancesValid(instances))
    {
        return NE_INVAL;
    }

    for (i = 0; i < TEST_SLICES_TO_DOWNLOAD; ++i)
    {
        if (i == 0)
        {
            NB_SuggestMatch* suggestMatch = NULL;
            NB_SearchRegion region = {0};

            result = NB_SingleSearchInformationGetSuggestMatch(suggestionInformation,
                                                               suggestionIndex,
                                                               &suggestMatch);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result != NE_OK)
            {
                return result;
            }

            // Set center to Dallas
            region.type = NB_ST_Center;
            region.center.latitude = 32.80;
            region.center.longitude = -96.80;

            result = NB_SingleSearchParametersCreateBySuggestion(instances->context,
                                                                 &region,
                                                                 "",
                                                                 TEST_SEARCH_SCHEME,
                                                                 TEST_NUMBER_PER_SLICE,
                                                                 NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantNonProximityPois,
                                                                 "",
                                                                 suggestMatch,
                                                                 &parameters);
            NB_SuggestMatchDestroy(suggestMatch);
            suggestMatch = NULL;
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(parameters);
            if (result != NE_OK)
            {
                return result;
            }

            result = NB_SingleSearchParametersSetSource(parameters, NB_SS_Carousel);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result != NE_OK)
            {
                NB_SingleSearchParametersDestroy(parameters);
                return result;
            }
        }
        else
        {
            CU_ASSERT_PTR_NOT_NULL(information);
            if (!information)
            {
                return NE_UNEXPECTED;
            }

            // Create search parameters for next slice
            result = NB_SingleSearchParametersCreateByIteration(instances->context,
                                                                information,
                                                                NB_IterationCommand_Next,
                                                                &parameters);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(parameters);
            if (result != NE_OK)
            {
                return result;
            }

            // Destroy the last search information
            NB_SingleSearchInformationDestroy(information);
            information = NULL;
        }

        result = DoRequest(instances, parameters, &information);
        NB_SingleSearchParametersDestroy(parameters);
        parameters = NULL;
        CU_ASSERT_EQUAL(result, NE_OK);
        if (result != NE_OK)
        {
            return result;
        }

        CU_ASSERT_PTR_NOT_NULL(information);
        if (!information)
        {
            return result;
        }

        result = CheckSearchResultForResultStyleTypeOfSingleSearchAndGeocode(instances, information);
        CU_ASSERT_EQUAL(result, NE_OK);

        {
            nb_boolean hasMoreResults = FALSE;

            result = NB_SingleSearchInformationHasMoreResults(information, &hasMoreResults);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result != NE_OK || !hasMoreResults)
            {
                NB_SingleSearchInformationDestroy(information);
                information = NULL;
                break;
            }
        }
    }

    if (parameters)
    {
        NB_SingleSearchParametersDestroy(parameters);
        parameters = NULL;
    }

    if (information)
    {
        NB_SingleSearchInformationDestroy(information);
        information = NULL;
    }

    return result;
}

/*! Do a search request

    Gets called from all tests. Returns the search information once the download has finished.

    @return NE_OK if success
    @see RequestHandlerCallback
*/
NB_Error
DoRequest(Instances* instances,                     /*!< The Instances object */
          NB_SingleSearchParameters* parameters,    /*!< The parameters used for the search request */
          NB_SingleSearchInformation** information  /*!< On return the search result */
          )
{
    NB_Error result = NE_OK;

    if (!instances || !parameters || !information || !IsInstancesValid(instances))
    {
        return NE_INVAL;
    }

    if (instances->information)
    {
        NB_SingleSearchInformationDestroy(instances->information);
        instances->information = NULL;
    }

    result = NB_SingleSearchHandlerStartRequest(instances->handler, parameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result != NE_OK)
    {
        return result;
    }

    if (!WaitForCallbackCompletedEvent(instances->event, 60000))
    {
        CU_FAIL("Download timeout.");
        return NE_UNEXPECTED;
    }

    CU_ASSERT_PTR_NOT_NULL(instances->information);
    if (!(instances->information))
    {
        return NE_UNEXPECTED;
    }

    *information = instances->information;
    instances->information = NULL;

    return result;
}

/*! Check if the search result for result style type of single search and geocode is correct

    @return NE_OK if success
*/
NB_Error
CheckSearchResultForResultStyleTypeOfSingleSearchAndGeocode(Instances* instances,
                                                            NB_SingleSearchInformation* information)
{
    NB_Error result = NE_OK;
    uint32 i = 0;
    uint32 resultCount = 0;
    uint32 analyticsEventId = 0;
    NB_SearchResultStyleType searchResultStyleType = NB_SRST_None;
    NB_FuelSummary fuelSummary = {0};

    result = NB_SingleSearchInformationGetResultStyleType(information, &searchResultStyleType);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT(searchResultStyleType == NB_SRST_SingleSearch || searchResultStyleType == NB_SRST_Geocode || searchResultStyleType == NB_SRST_DefaultLocation);

    result = NB_SingleSearchInformationGetFuelSummary(information, &fuelSummary);
    CU_ASSERT(result == NE_OK || result == NE_NOENT);

    result = NB_SingleSearchInformationGetAnalyticsEventId(information, &analyticsEventId);
    CU_ASSERT(result == NE_OK || result == NE_NOENT);

    result = NB_SingleSearchInformationGetResultCount(information, &resultCount);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT(resultCount > 0);

    for (i = 0; i < resultCount; ++i)
    {
        double distance = 0;
        nb_boolean isMappable = FALSE;
        NB_SearchResultType resultType = NB_SRT_None;
        NB_ExtendedPlace* extendedPlace = NULL;
        NB_Place place = {0};
        NB_FuelDetails fuelDetails = {0};
        NB_HoursOfOperation hoursOfOperation = {0};

        result = NB_SingleSearchInformationGetResultType(information, i, &resultType);
        CU_ASSERT_EQUAL(result, NE_OK);

        CU_ASSERT(resultType & NB_SRT_Place);

        result = NB_SingleSearchInformationGetHoursOfOperation(information, i, &hoursOfOperation);
        if (result == NE_OK)
        {
            LogOutputHoursOfOperation(i, &hoursOfOperation);
            NB_HoursOfOperationDestroy(&hoursOfOperation);
            nsl_memset(&hoursOfOperation, 0, sizeof(hoursOfOperation));
        }
        else if (result != NE_NOENT)
        {
            CU_FAIL("Failed to retrieve hours of operation");
            return result;
        }

        result = NB_SingleSearchInformationGetPlace(information, i, &place, &distance, &extendedPlace);
        CU_ASSERT_EQUAL(result, NE_OK);
        if (result == NE_OK)
        {
            LogOutputPlace(i, distance, &place, extendedPlace);

            // Log output the parent categories
            {
                int i = 0;
                int count = place.numcategory;

                LOGOUTPUT(LogLevelHigh, ("\nParent categories:\n"));

                for (i = 0; i < count; ++i)
                {
                    NB_Category parentCategory = {0};

                    LogOutputFormattedString("Code", place.category[i].code);
                    LogOutputFormattedString("Name", place.category[i].name);

                    result = NB_SingleSearchInformationGetParentCategory(information,
                                                                         place.category[i].code,
                                                                         &parentCategory);
                    if (result == NE_OK)
                    {
                        LogOutputFormattedString("Parent Code", parentCategory.code);
                        LogOutputFormattedString("Parent Name", parentCategory.name);
                    }
                    else if (result == NE_NOENT)
                    {
                        LOGOUTPUT(LogLevelHigh, ("This category does not have a parent.\n"));
                    }
                    else
                    {
                        CU_FAIL("Failed to retrieve parent category");
                    }
                }
            }
        }

        if (resultType & NB_SRT_FuelDetails)
        {
            result = NB_SingleSearchInformationGetFuelDetails(information, i, &fuelDetails);
            CU_ASSERT_EQUAL(result, NE_OK);

            // @todo: Log output fuel details
        }

        result = NB_SingleSearchInformationIsMappable(information, i, &isMappable);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    result = NB_SingleSearchInformationFreeExtendedPlaces(information);
    CU_ASSERT_EQUAL(result, NE_OK);

    return result;
}

/*! Check if the search result for result style type of suggestion is correct

    @return NE_OK if success
*/
NB_Error
CheckSearchResultForResultStyleTypeOfSuggestion(Instances* instances,
                                                NB_SingleSearchInformation* information)
{
    NB_Error result = NE_OK;
    uint32 i = 0;
    uint32 resultCount = 0;
    uint32 analyticsEventId = 0;
    char requestedKeyword[TEST_RIGHT_MAX_KEYWORD_SIZE];
    NB_SearchResultStyleType searchResultStyleType = NB_SRST_None;
    NB_FuelSummary fuelSummary = {0};

    nsl_memset(requestedKeyword, 0, TEST_RIGHT_MAX_KEYWORD_SIZE);

    result = NB_SingleSearchInformationGetRequestedKeyword(information, 0, requestedKeyword);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_SingleSearchInformationGetRequestedKeyword(information, TEST_WRONG_MAX_KEYWORD_SIZE, requestedKeyword);
    CU_ASSERT_EQUAL(result, NE_RANGE);
    result = NB_SingleSearchInformationGetRequestedKeyword(information, TEST_RIGHT_MAX_KEYWORD_SIZE, requestedKeyword);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT(nsl_strcmp(requestedKeyword, TEST_SEARCH_NAME) == 0);

    result = NB_SingleSearchInformationGetResultStyleType(information, &searchResultStyleType);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT(searchResultStyleType == NB_SRST_Suggestion);

    // Suggestion search result should not contain fuel summary
    result = NB_SingleSearchInformationGetFuelSummary(information, &fuelSummary);
    CU_ASSERT(result == NE_NOENT);

    result = NB_SingleSearchInformationGetAnalyticsEventId(information, &analyticsEventId);
    CU_ASSERT(result == NE_OK || result == NE_NOENT);

    result = NB_SingleSearchInformationGetResultCount(information, &resultCount);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT(resultCount > 0);

    for (i = 0; i < resultCount; ++i)
    {
        nb_boolean isMappable = FALSE;
        NB_SearchResultType resultType = NB_SRT_None;
        NB_SuggestMatch* suggestMatch = NULL;

        // Returns NE_NOENT if calling the function 'NB_SingleSearchInformationGetResultType' for suggestion search
        result = NB_SingleSearchInformationGetResultType(information, i, &resultType);
        CU_ASSERT_EQUAL(result, NE_NOENT);

        result = NB_SingleSearchInformationGetSuggestMatch(information, i, &suggestMatch);
        CU_ASSERT((result == NE_OK) && (suggestMatch != NULL));
        if (result == NE_OK)
        {
            result = CheckSuggestMatch(instances, suggestMatch);
            NB_SuggestMatchDestroy(suggestMatch);
            suggestMatch = NULL;
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        // Returns NE_NOENT if calling the function 'NB_SingleSearchInformationIsMappable' for suggestion search
        result = NB_SingleSearchInformationIsMappable(information, i, &isMappable);
        CU_ASSERT_EQUAL(result, NE_NOENT);
    }

    return NE_OK;
}

/*! Check if the 'NB_SuggestMatch' object is correct

    @return NE_OK if success
*/
NB_Error
CheckSuggestMatch(Instances* instances, NB_SuggestMatch* suggestMatch)
{
    NB_Error result = NE_OK;

    CU_ASSERT((instances != NULL) && (suggestMatch != NULL));
    if (!instances || !suggestMatch)
    {
        return NE_INVAL;
    }

    // Test to get data to display.
    {
        NB_SuggestMatchData suggestMatchData = {0};

        result = NB_SuggestMatchGetDataToDisplay(suggestMatch, &suggestMatchData);
        CU_ASSERT_EQUAL(result, NE_OK);
        if (result != NE_OK)
        {
            return result;
        }

        CU_ASSERT_NOT_EQUAL(suggestMatchData.matchType, NB_MT_None);
        LogOutputSuggestMatchData(&suggestMatchData);
    }

    // Test to clone a 'NB_SuggestMatch' object.
    {
        NB_SuggestMatch* newSuggestMatch = NULL;

        result = NB_SuggestMatchCreateByCopy(suggestMatch, &newSuggestMatch);
        CU_ASSERT((result == NE_OK) && (newSuggestMatch != NULL));
        if (result != NE_OK)
        {
            return result;
        }

        CU_ASSERT(NB_SuggestMatchIsEqual(suggestMatch, newSuggestMatch));
        NB_SuggestMatchDestroy(newSuggestMatch);
        newSuggestMatch = NULL;
    }

    // Test to serialize a 'NB_SuggestMatch' object.
    {
        nb_size dataSize = 0;
        uint8* data = NULL;
        NB_SuggestMatch* newSuggestMatch = NULL;

        result = NB_SuggestMatchSerialize(suggestMatch, &data, &dataSize);
        CU_ASSERT((result == NE_OK) && (data != NULL) && (dataSize > 0));
        if (result != NE_OK)
        {
            return result;
        }

        result = NB_SuggestMatchCreateBySerializedData(instances->context, data, dataSize, &newSuggestMatch);
        nsl_free(data);
        data = NULL;
        dataSize = 0;
        CU_ASSERT((result == NE_OK) && (newSuggestMatch != NULL));
        if (result != NE_OK)
        {
            return result;
        }

        CU_ASSERT(NB_SuggestMatchIsEqual(suggestMatch, newSuggestMatch));
        NB_SuggestMatchDestroy(newSuggestMatch);
        newSuggestMatch = NULL;
    }

    return result;
}

/*! Callback for all search requests

    @return None
*/
void
RequestHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    Instances* instances = (Instances*)userData;

    CU_ASSERT_PTR_NOT_NULL(instances);

    if (err != NE_OK)
    {
        // Abort and trigger event
        CU_FAIL("Callback returned error.");
        LOGOUTPUT(LogLevelLow, ("Callback error = %d, status = %d\n", err, status));
        SetCallbackCompletedEvent(instances->event);
        return;
    }

    LOGOUTPUT(LogLevelHigh, ("Download progress: %d\n", percent));

    if (percent != 100)
    {
        // The status should be set to progress
        CU_ASSERT_EQUAL(status, NB_NetworkRequestStatus_Progress);
    }
    else
    {
        NB_Error localError = NE_OK;

        if (instances->information)
        {
            NB_SingleSearchInformationDestroy(instances->information);
            instances->information = NULL;
        }

        localError = NB_SingleSearchHandlerGetInformation(handler, &(instances->information));

        CU_ASSERT_EQUAL(localError, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(instances->information);

        if (localError != NE_OK || !(instances->information))
        {
            // Abort and trigger event
            CU_FAIL("Failed to retrieve search information");
            SetCallbackCompletedEvent(instances->event);
            return;
        }

        CU_ASSERT_EQUAL(status, NB_NetworkRequestStatus_Success);

        // Trigger main thread
        SetCallbackCompletedEvent(instances->event);
    }
}

/*! Callback for formatted content text

    @return NE_OK if success
*/
NB_Error
FormattedTextCallback(NB_Font font, nb_color color, const char* text, nb_boolean newline, void* userData)
{
    CU_ASSERT_PTR_NOT_NULL(text);

    if (newline)
    {
        LOGOUTPUT(LogLevelHigh, ("\n"));
    }
    else
    {
        LOGOUTPUT(LogLevelHigh, ("\tContent details(font:%d, color:%d):\t%s\n", font, color, text));
    }

    return NE_OK;
}

/*! Log output the formatted string

    @return None
*/
void
LogOutputFormattedString(const char* name, const char* value)
{
    if (!name || !value || (nsl_strlen(name) == 0) || (nsl_strlen(value) == 0))
    {
        return;
    }

    LOGOUTPUT(LogLevelHigh, ("%s\t:\t%s\n", name, value));
}

/*! Log output the result of place

    @return None
*/
void
LogOutputPlace(uint32 index, double distance, const NB_Place* place, const NB_ExtendedPlace* extendedPlace)
{
    NB_Error result = NE_OK;

    if (!place || !extendedPlace)
    {
        return;
    }

    LOGOUTPUT(LogLevelHigh, ("\nPlace %d: (Distance: %f)\n", index + 1, distance));
    LogOutputFormattedString("Name", place->name);

    // Log output the location
    LogOutputFormattedString("Areaname", place->location.areaname);
    LogOutputFormattedString("Streetnum", place->location.streetnum);
    LogOutputFormattedString("Street1", place->location.street1);
    LogOutputFormattedString("Street2", place->location.street2);
    LogOutputFormattedString("City", place->location.city);
    LogOutputFormattedString("County", place->location.county);
    LogOutputFormattedString("State", place->location.state);
    LogOutputFormattedString("Postal", place->location.postal);
    LogOutputFormattedString("Country", place->location.country);
    LogOutputFormattedString("Airport", place->location.airport);
    LogOutputFormattedString("Freeform", place->location.freeform);
    LogOutputFormattedString("Country Name", place->location.country_name);

    LOGOUTPUT(LogLevelHigh, ("Latitude:\t%f\n", place->location.latitude));
    LOGOUTPUT(LogLevelHigh, ("Longitude:\t%f\n", place->location.longitude));
    LOGOUTPUT(LogLevelHigh, ("Location type:\t%d\n", place->location.type));

    // Log output the extended address
    {
        int lineCount = place->location.extendedAddress.lineCount;

        if ((--lineCount) >= 0)
        {
            LOGOUTPUT(LogLevelHigh, ("\nExtended address:\n"));
            LogOutputFormattedString("Line 1", place->location.extendedAddress.addressLine1);
        }

        if ((--lineCount) >= 0)
        {
            LogOutputFormattedString("Line 2", place->location.extendedAddress.addressLine2);
        }

        if ((--lineCount) >= 0)
        {
            LogOutputFormattedString("Line 3", place->location.extendedAddress.addressLine3);
        }

        if ((--lineCount) >= 0)
        {
            LogOutputFormattedString("Line 4", place->location.extendedAddress.addressLine4);
        }

        if ((--lineCount) >= 0)
        {
            LogOutputFormattedString("Line 5", place->location.extendedAddress.addressLine5);
        }
    }

    // Log output the compact address
    {
        int lineCount = place->location.compactAddress.lineCount;

        if ((--lineCount) >= 0)
        {
            LOGOUTPUT(LogLevelHigh, ("\nCompact address:\n"));
            LogOutputFormattedString("Line 1", place->location.compactAddress.addressLine1);
        }

        if ((--lineCount) >= 0)
        {
            LogOutputFormattedString("Line 2", place->location.compactAddress.addressLine2);
        }

        if (place->location.compactAddress.hasLineSeparator)
        {
            LogOutputFormattedString("Compact address line separator", place->location.compactAddress.lineSeparator);
        }
    }

    // Log output the phone numbers
    {
        int i = 0;
        int count = place->numphone;

        LOGOUTPUT(LogLevelHigh, ("\nPhone numbers:\n"));
        for (i = 0; i < count; ++i)
        {
            LOGOUTPUT(LogLevelHigh, ("Phone type:\t%d\n", place->phone[i].type));
            LogOutputFormattedString("Country", place->phone[i].country);
            LogOutputFormattedString("Area", place->phone[i].area);
            LogOutputFormattedString("Number", place->phone[i].number);
            LogOutputFormattedString("Formatted text", place->phone[i].formattedText);
        }
    }

    // Log output the categories
    {
        int i = 0;
        int count = place->numcategory;

        LOGOUTPUT(LogLevelHigh, ("\nCategories:\n"));
        for (i = 0; i < count; ++i)
        {
            LogOutputFormattedString("Code", place->category[i].code);
            LogOutputFormattedString("Name", place->category[i].name);
        }
    }

    // Log output extended place
    {
        int i = 0;

        LOGOUTPUT(LogLevelHigh, ("Extended information:\t%d\n", extendedPlace->extendedInformation));
        LogOutputFormattedString("POI content id", extendedPlace->poiContentId);
        LogOutputFormattedString("Tagline", extendedPlace->tagline);

        if (extendedPlace->formattedContentHandle)
        {
            result = NB_SearchInformationGetFormattedContentText(extendedPlace->formattedContentHandle, FormattedTextCallback, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        if (extendedPlace->extendedInformation & NB_EPI_POIContent)
        {
            int i = 0;

            for (i = 0; i < extendedPlace->poiContentCount; ++i)
            {
                if (extendedPlace->poiContent[i].key && extendedPlace->poiContent[i].value)
                {
                    LOGOUTPUT(LogLevelHigh, ("POI content key:\t%s\tvalue:\t%s\n", extendedPlace->poiContent[i].key, extendedPlace->poiContent[i].value));
                }
            }
        }

        LOGOUTPUT(LogLevelHigh, ("Average rating:\t%f\n", extendedPlace->overallRating.averageRating));
        LOGOUTPUT(LogLevelHigh, ("Rating count:\t%d\n", extendedPlace->overallRating.ratingCount));
        LogOutputFormattedString("Provider id of golden cookie", extendedPlace->goldenCookie.providerId);
        LOGOUTPUT(LogLevelHigh, ("Accuracy:\t%d\n", extendedPlace->accuracy));
        LogOutputFormattedString("Provider id of place event cookie", extendedPlace->placeEventCookie.providerId);

        /* Log of Vendor content */
        for (i = 0; i < extendedPlace->vendorContentCount; ++i)
        {
            int ii = 0;
            NB_VendorContent* vendorContent = &extendedPlace->vendorContent[i];
            LOGOUTPUT(LogLevelNone, ("\nName of Vendor[%d] content\t:\t%s\n", i + 1, vendorContent->name));

            for (ii = 0; ii < vendorContent->contentCount; ++ii)
            {
                LOGOUTPUT(LogLevelNone, ("Key\t:\t%s\n", vendorContent->content[ii].key));
                LOGOUTPUT(LogLevelNone, ("Value\t:\t%s\n", vendorContent->content[ii].value));
            }

            LOGOUTPUT(LogLevelNone, ("Vendor Content - Average rating:\t%f\n", vendorContent->overallRating.averageRating));
            LOGOUTPUT(LogLevelNone, ("Vendor Content - Rating count:\t%d\n", vendorContent->overallRating.ratingCount));
        }
    }
}

/*! Log output the result of 'NB_SuggestMatchData' object

    @return None
*/
void
LogOutputSuggestMatchData(NB_SuggestMatchData* suggestMatchData)
{
    int i = 0;
    if (!suggestMatchData)
    {
        return;
    }

    LogOutputFormattedString("Line1", suggestMatchData->line1);
    LogOutputFormattedString("Line2", suggestMatchData->line2);
    LOGOUTPUT(LogLevelHigh, ("Match type:\t%d\n", suggestMatchData->matchType));

    for (i = 0; i < suggestMatchData->countIconID; ++i)
    {
        LOGOUTPUT(LogLevelNone, ("Icon ID: %s\n", suggestMatchData->iconIDs[i]));
    }
}

/*! Log output the hours of operation

    @return None
*/
void
LogOutputHoursOfOperation(uint32 index, const NB_HoursOfOperation* hoursOfOperation)
{
    if (!hoursOfOperation)
    {
        return;
    }

    LOGOUTPUT(LogLevelHigh,
              ("\nSearch result %d: time range count = %d, ",
               index + 1,
               hoursOfOperation->timeRangeCount));

    if (hoursOfOperation->hasUtcOffset)
    {
        LOGOUTPUT(LogLevelHigh, ("UTC offset = %d.\n", hoursOfOperation->utcOffset));
    }
    else
    {
        LOGOUTPUT(LogLevelHigh, ("no UTC offset.\n"));
    }

    // Log output the time range array
    {
        int i = 0;
        int count = (int) (hoursOfOperation->timeRangeCount);

        LOGOUTPUT(LogLevelHigh, ("\nTime range %d:\n", i));

        for (i = 0; i < count; ++i)
        {
            const char* day = NULL;
            NB_TimeRange* timeRange = hoursOfOperation->timeRangeArray + i;

            if (!timeRange)
            {
                CU_FAIL("Failed to retrieve time range");
                return;
            }

            switch (timeRange->dayOfWeek)
            {
            case NB_DOW_Sunday:
            {
                day = "Sunday";
                break;
            }
            case NB_DOW_Monday:
            {
                day = "Monday";
                break;
            }
            case NB_DOW_Tuesday:
            {
                day = "Tuesday";
                break;
            }
            case NB_DOW_Wednesday:
            {
                day = "Wednesday";
                break;
            }
            case NB_DOW_Thursday:
            {
                day = "Thursday";
                break;
            }
            case NB_DOW_Friday:
            {
                day = "Friday";
                break;
            }
            case NB_DOW_Saturday:
            {
                day = "Saturday";
                break;
            }
            default:
            {
                break;
            }
            }

            if (day)
            {
                LogOutputFormattedString("Day", day);
            }
            else
            {
                CU_FAIL("Failed to retrieve day of week");
            }

            LOGOUTPUT(LogLevelHigh, ("Start time:\t%u\n", timeRange->startTime));
            LOGOUTPUT(LogLevelHigh, ("End time:\t%u\n", timeRange->endTime));
        }
    }
}

/*! @} */

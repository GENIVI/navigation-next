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

    @file     testbatchtoolprocessor.cpp

    This file contains implementation for TestBatchToolProcessor class.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunications Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunications Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "math.h"

extern "C"
{
    #include "palclock.h"
    #include "testnetwork.h"
    #include "main.h"
    #include "platformutil.h"
    #include "palmath.h"
    #include "paltaskqueue.h"
    #include "palfile.h"
	#include "nbcontextaccess.h"
    #include "nbcontextprotected.h"
    #include "testdirection.h"
}

#include "testbatchtoolprocessor.h"

// How much should we download
const int NUMBER_PER_SLICE = 8;
const int FIX_DELAY_MILLISECONDS = 100;
const char* DOCUMENTS_DIRECTORY_PATH = "";

TestBatchToolProcessor::TestBatchToolProcessor(void* completeEvent, BatchFileString* batchParams)
  :  m_pal(NULL),
     m_context(NULL),
     m_ECManager(NULL),
     m_GeocodeHandler(NULL),
     m_ReverceGeocodeHandler(NULL),
     m_searchHandler(NULL),
     m_testFunction(NULL),
     m_completeEvent(completeEvent),
     m_initialized(false),
     m_batchParams(batchParams),
     m_getCoords(false),
     m_routeHandler(NULL),
     m_guidanceInfo(NULL),
     m_navigationSession(false),
     m_gpsSource(NULL),
     m_gpsFix(NULL),
     m_navigation(NULL),
     m_statusCounter(0),
     m_fpOutFile(NULL)
{
    // Incorrect coords
    m_point.latitude = -2000;
    m_point.longitude = -2000;

    m_initialized = (CreatePalAndContext(&m_pal, &m_context) != 0);
    CU_ASSERT(m_initialized);
}

TestBatchToolProcessor::~TestBatchToolProcessor(void)
{
    PAL_Destroy(m_pal);
}

bool TestBatchToolProcessor::IsInitialized() {
    return m_initialized;
}

PAL_Instance* TestBatchToolProcessor::GetPal() {
    return m_pal;
}

void TestBatchToolProcessor::Destroy(void)
{
    if(m_initialized)
    {
		NB_Error result = NE_OK;

		if (m_GeocodeHandler)
		{
			result = NB_GeocodeHandlerDestroy(m_GeocodeHandler);
			m_GeocodeHandler = NULL;
			CU_ASSERT_EQUAL(result, NE_OK);
		}
		if (m_ReverceGeocodeHandler)
		{
			result = NB_ReverseGeocodeHandlerDestroy(m_ReverceGeocodeHandler);
			m_ReverceGeocodeHandler = NULL;
			CU_ASSERT_EQUAL(result, NE_OK);
		}
		if (m_searchHandler)
		{
			result = NB_SearchHandlerDestroy(m_searchHandler);
			m_searchHandler = NULL;
			CU_ASSERT_EQUAL(result, NE_OK);
		}

		if (m_guidanceInfo != NULL)
		{
			result = NB_GuidanceInformationDestroy(m_guidanceInfo);
			CU_ASSERT_EQUAL(result, NE_OK);
			m_guidanceInfo = NULL;
		}

		if (m_routeHandler)
		{
			result = NB_RouteHandlerDestroy(m_routeHandler);
			CU_ASSERT_EQUAL(result, NE_OK);
			m_routeHandler = NULL;
		}

        if (m_ECManager)
        {
            result = NB_EnhancedContentManagerDestroy(m_ECManager);
			CU_ASSERT_EQUAL(result, NE_OK);
			m_ECManager = NULL;
        }

		DestroyContext(m_context);
		m_context = NULL;

		m_testFunction = NULL;

		SetCallbackCompletedEvent(m_completeEvent);
		m_completeEvent = NULL;

    }
}

void TestBatchToolProcessor::CallTestFunction(void)
{
    if (!m_initialized)
    {
        CU_FAIL("Test is not initialized.");
        SetCallbackCompletedEvent(m_completeEvent);
        return;
    }

    (this->*m_testFunction)();
}

void TestBatchToolProcessor::EventTaskCallback(PAL_Instance* pal, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    if (me)
    {
        (me->*me->m_testFunction)();
    }
}

// ==================================================
// Public test functions
// ==================================================
void TestBatchToolProcessor::TestBatchToolGeocode(void)
{
    m_testFunction = &TestBatchToolProcessor::BatchToolGeocode;
    CallTestFunction();
}

void TestBatchToolProcessor::TestBatchToolReverseGeocode(void)
{
    m_testFunction = &TestBatchToolProcessor::BatchToolReverseGeocode;
    CallTestFunction();
}

void TestBatchToolProcessor::TestBatchToolPOI(void)
{
    m_testFunction = &TestBatchToolProcessor::BatchToolPOI;
    CallTestFunction();
}

void TestBatchToolProcessor::TestBatchToolRoute(void)
{
    m_testFunction = &TestBatchToolProcessor::BatchToolRoute;
    CallTestFunction();
}

void TestBatchToolProcessor::TestBatchToolNavigationSession(void)
{
    m_navigationSession = true;
    m_testFunction = &TestBatchToolProcessor::BatchToolNavigationSession;
    CallTestFunction();
}

// ==================================================
// Internal test functions
// ==================================================

/// TODO: This works for offboard only
/// TODO: For onboard using new geocode API. There is dummy callback.
void TestBatchToolProcessor::BatchToolGeocode(void)
{
    NB_Error err = NE_OK;
    NB_GeocodeParameters* parameters = NULL;
    NB_Address address = {0};

    // Copy address from params
    nsl_strcpy(address.number,  &(m_batchParams->buf[m_batchParams->params[4]]));
    nsl_strcpy(address.street,  &(m_batchParams->buf[m_batchParams->params[5]]));
    nsl_strcpy(address.city,    &(m_batchParams->buf[m_batchParams->params[7]]));
    nsl_strcpy(address.county,  &(m_batchParams->buf[m_batchParams->params[8]]));
    nsl_strcpy(address.state,   &(m_batchParams->buf[m_batchParams->params[9]]));
    nsl_strcpy(address.postal,  &(m_batchParams->buf[m_batchParams->params[10]]));
    nsl_strcpy(address.country, &(m_batchParams->buf[m_batchParams->params[11]]));

    // FreeForm
    if (nsl_strlen(&(m_batchParams->buf[m_batchParams->params[13]])) ||
        nsl_strlen(&(m_batchParams->buf[m_batchParams->params[14]])))
    {
        char buf[BATCH_STRING_MAX_LEN]={0};
        nsl_strcpy(buf,&(m_batchParams->buf[m_batchParams->params[13]]));
        nsl_strcat(buf," ");
        nsl_strcat(buf,&(m_batchParams->buf[m_batchParams->params[14]]));

        PerformanceMeasurement_StartTimer("test");

        err = NB_GeocodeParametersCreateFreeForm(m_context,
                  buf,
                  &(m_batchParams->buf[m_batchParams->params[11]]),
                  NUMBER_PER_SLICE,
                  &parameters);
        PmPrintInputParametersGeocode(&(m_batchParams->buf[m_batchParams->params[0]]));
    }

    // Intersection
    else if (nsl_strlen(&(m_batchParams->buf[m_batchParams->params[6]]))) {
        PerformanceMeasurement_StartTimer("test");

        err = NB_GeocodeParametersCreateIntersection(m_context,
                  &address,
                  &(m_batchParams->buf[m_batchParams->params[6]]),
                  NUMBER_PER_SLICE,
                  &parameters);
        PmPrintInputParametersGeocode(&(m_batchParams->buf[m_batchParams->params[0]]));
    }

    // Airport
    else if (nsl_strlen(&(m_batchParams->buf[m_batchParams->params[12]]))) {
        PerformanceMeasurement_StartTimer("test");
        err = NB_GeocodeParametersCreateAirport(m_context,
                  &(m_batchParams->buf[m_batchParams->params[12]]),
                  NUMBER_PER_SLICE,
                  &parameters);
        PmPrintInputParametersGeocode(&(m_batchParams->buf[m_batchParams->params[0]]));
    }
    // Address
    else {
        PerformanceMeasurement_StartTimer("test");
        err = NB_GeocodeParametersCreateAddress(m_context, &address, NUMBER_PER_SLICE, &parameters);
        PmPrintInputParametersGeocode(&(m_batchParams->buf[m_batchParams->params[0]]));
    }
    CU_ASSERT_EQUAL(err, NE_OK);

    // Coords should be specified only in geocode tests with geographic posi
    //if (!(m_point.latitude > -2000)&&!(m_point.longitude > -2000))
    //{
    //    // Get lat lon from params
    //    latLong.latitude  = nsl_atof(&(m_batchParams->buf[m_batchParams->params[1]]));
    //    latLong.longitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[2]]));

    //    if ((latLong.latitude != 0)&&(latLong.longitude != 0)) {
    //        err = NB_GeocodeParametersSetGeographicPosition(parameters, &latLong, 50, PAL_ClockGetGPSTime());
    //        CU_ASSERT_EQUAL(err, NE_OK);
    //    }
    //}

    if (!err)
    {
        DoGeocodeRequestWithParams(parameters);
    }

    if (parameters)
    {
        err = NB_GeocodeParametersDestroy(parameters);
        parameters = NULL;
        CU_ASSERT_EQUAL(err, NE_OK);
    }
}

bool TestBatchToolProcessor::DoGeocodeRequestWithParams(NB_GeocodeParameters* params)
{
    bool res = false;
    NB_Error err = NE_OK;
    NB_RequestHandlerCallback callback = {0};

    if (params == NULL)
    {
        return res;
    }

    callback.callback = &Static_GeocodeCallback;
    callback.callbackData = this;

    if (!m_GeocodeHandler)
    {
        err = NB_GeocodeHandlerCreate(m_context, &callback, &m_GeocodeHandler);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    if ((err == NE_OK) && m_GeocodeHandler)
    {
        err = NB_GeocodeHandlerStartRequest(m_GeocodeHandler, params);
        CU_ASSERT_EQUAL(err, NE_OK);
        res = (err == NE_OK);
    }
    return res;
}

void TestBatchToolProcessor::BatchToolReverseGeocode(void)
{
    NB_Error err = NE_OK;
    NB_LatitudeLongitude latLong = {0};
    NB_RequestHandlerCallback callback = {0};
    NB_ReverseGeocodeParameters* parameters=NULL;

    callback.callback = &Static_ReverseGeocodeCallback;
    callback.callbackData = this;

    PRINTF("\nIN: %s, %s, %s\n",&(m_batchParams->buf[m_batchParams->params[0]])
                           ,&(m_batchParams->buf[m_batchParams->params[1]])
                           ,&(m_batchParams->buf[m_batchParams->params[2]]));

    // Get lat lon from params
    latLong.latitude  = nsl_atof(&(m_batchParams->buf[m_batchParams->params[1]]));
    latLong.longitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[2]]));

    PerformanceMeasurement_StartTimer("test");

    err = NB_ReverseGeocodeParametersCreate(m_context, &latLong, TRUE, &parameters);
    CU_ASSERT_EQUAL(err, NE_OK);

    if (!err)
    {
        if (!m_ReverceGeocodeHandler)
        {
            err = NB_ReverseGeocodeHandlerCreate(m_context, &callback, &m_ReverceGeocodeHandler);
            CU_ASSERT_EQUAL(err, NE_OK);
        }

        if (!err && m_ReverceGeocodeHandler)
        {
            err = NB_ReverseGeocodeHandlerStartRequest(m_ReverceGeocodeHandler, parameters);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
    }

    if (parameters)
    {
        err = NB_ReverseGeocodeParametersDestroy(parameters);
        parameters = NULL;
        CU_ASSERT_EQUAL(err, NE_OK);
    }
}

void TestBatchToolProcessor::BatchToolPOI(void)
{
    NB_Error result = NE_OK;
    NB_SearchParameters* searchParameters = NULL;
    NB_RequestHandlerCallback callbackData = { &Static_POISearchCallback, this };
    NB_SearchRegion region = {NB_ST_None, 0};
    char* name = NULL;
    char* searchScheme = NULL;
    char* language = NULL;
    char* categories = NULL;
    char** categoryCodes = NULL;
    int categoryCodesCount = 0;
    int resultsCount = 0;
    int extendedConfiguration = 0;

    if (m_batchParams == NULL || !m_batchParams->buf[m_batchParams->params[0]])
    {
        CU_FAIL("Can't get input data");
        ScheduleFinishQuery();
        return;
    }

    PRINTF("\n%s:\n",&(m_batchParams->buf[m_batchParams->params[0]]) );
    PerformanceMeasurement_StartTimer("test");

    //Create search handler
    if (!m_searchHandler)
    {
        result = NB_SearchHandlerCreate(m_context, &callbackData, &m_searchHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(m_searchHandler);

        if (result != NE_OK || m_searchHandler == NULL)
        {
            CU_FAIL("Can't create search handler");
            m_searchHandler = NULL;
            ScheduleFinishQuery();
            return;
        }
    }

    region.type = (NB_SearchRegionType)nsl_atoi(&(m_batchParams->buf[m_batchParams->params[1]]));
    if (region.type == NB_ST_Center || region.type == NB_ST_DirectionAndCenter)
    {
        region.center.latitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[2]]));
        region.center.longitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[3]]));
    }
    if (region.type == NB_ST_DirectionAndCenter)
    {
        region.direction.heading = nsl_atof(&(m_batchParams->buf[m_batchParams->params[4]]));
        region.direction.speed = nsl_atof(&(m_batchParams->buf[m_batchParams->params[5]]));
    }
    if (region.type == NB_ST_BoundingBox)
    {
        region.boundingBox.topLeft.latitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[6]]));
        region.boundingBox.topLeft.longitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[7]]));
        region.boundingBox.bottomRight.latitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[8]]));
        region.boundingBox.bottomRight.longitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[9]]));
    }

    name = &(m_batchParams->buf[m_batchParams->params[10]]);
    searchScheme = &(m_batchParams->buf[m_batchParams->params[11]]);

    if (nsl_strlen(name) == 0)
    {
        name = NULL;
    }
    if (nsl_strlen(searchScheme) == 0)
    {
        searchScheme = NULL;
    }

    categories = &(m_batchParams->buf[m_batchParams->params[12]]);

    if (categories)
    {
        int index = 0;
        char* cat = NULL;
        char* categoriesTmp = categories;

        do
        {
            cat = nsl_strchr(categoriesTmp, '+');

            if (cat)
            {
                categoryCodesCount++;
                categoriesTmp = ++cat;
            }
            else
            {
                if (index == 0)
                {
                    categoryCodesCount++;
                }

                break;
            }

            index++;
        }
        while(1);

        categoryCodes = (char**)nsl_malloc(categoryCodesCount * sizeof(char*));

        for (index = 0; index < categoryCodesCount; index++)
        {
            cat = nsl_strchr(categories, '+');

            categoryCodes[index] = (char*)nsl_malloc(20 * sizeof(char));

            if (!cat)
            {
                strcpy(categoryCodes[index], categories);
            }
            else
            {
                strncpy(categoryCodes[index], categories, (cat - categories) / sizeof(char));

                categoryCodes[index][(cat - categories)/sizeof(char)] = '\0';
                categories = ++cat;
            }
        }
    }

    if (categoryCodesCount == 0 && categoryCodes)
    {
        nsl_free(categoryCodes);
        categoryCodes = NULL;
        ScheduleFinishQuery();
        return;
    }

    resultsCount = nsl_atoi(&(m_batchParams->buf[m_batchParams->params[14]]));
    extendedConfiguration = nsl_atoi(&(m_batchParams->buf[m_batchParams->params[15]]));
    language = &(m_batchParams->buf[m_batchParams->params[16]]);

    if (nsl_strlen(language) == 0)
    {
        language = NULL;
    }
    PmPrintInputParametersSearchBuilder(name, &region.center, 50, NULL);

    // Create search parameters
    result = NB_SearchParametersCreatePOI(m_context,
                                          &region,
                                          name,
                                          searchScheme,
                                          (const char**)categoryCodes,
                                          categoryCodesCount,
                                          resultsCount,
                                          (NB_POIExtendedConfiguration)extendedConfiguration,
                                          language,
                                          &searchParameters);

    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchParameters);

    if (categoryCodes)
    {
        for (int i = 0; i < categoryCodesCount; i++)
        {
            nsl_free(categoryCodes[i]);
            categoryCodes[i] = NULL;
        }

        nsl_free(categoryCodes);
        categoryCodes = NULL;
    }

    if (searchParameters)
    {
        result = NB_SearchParametersSetSource(searchParameters, NB_SS_User);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    if (result == NE_OK && m_searchHandler && searchParameters)
    {
        // Make the server request
        result = NB_SearchHandlerStartRequest(m_searchHandler, searchParameters);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_SearchParametersDestroy(searchParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        searchParameters = NULL;
    }
    else
    {
        ScheduleFinishQuery();
    }
}


void TestBatchToolProcessor::BatchToolRoute(void)
{
    NB_Place                  origin = {{0}};
    NB_Place                  destination = {{0}};
    NB_RouteOptions           options = {NB_RouteType_Fastest, NB_RouteAvoid_None, NB_TransportationMode_Car, NB_TrafficType_None, NULL};
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { 0 };
    NB_Error                  result = NE_OK;
    uint32                    command = 0;
    NB_RouteParameters*       routeParameters = NULL;
    char                      fullFileName[BATCH_STRING_MAX_LEN] = {0};
    PAL_Error                 fileErr = PAL_Ok;
    bool                      isFirstQuery = false;
    char                      buf[BATCH_STRING_MAX_LEN] = {0};
    uint32                    bytesWritten = 0;

    if (m_batchParams == NULL || !m_batchParams->buf[m_batchParams->params[0]])
    {
        CU_FAIL("Can't get input data");
        ScheduleFinishQuery();
        return;
    }

    PRINTF("\n%s:\n",&(m_batchParams->buf[m_batchParams->params[0]]) );

    nsl_strlcat(fullFileName, GetDocumentsDirectoryPath(), sizeof(m_batchParams->outputFileName));
    nsl_strlcat(fullFileName, m_batchParams->outputFileName, sizeof(m_batchParams->outputFileName));
    fileErr = PAL_FileExists(m_pal, fullFileName);

    if ( fileErr == PAL_ErrFileNotExist )
    {
        isFirstQuery = true;
    }

    fileErr = PAL_FileOpen(m_pal, fullFileName, PFM_Append, &m_fpOutFile);

    if (fileErr == PAL_Ok)
    {
        if (isFirstQuery)
        {
            // Output Caption - first line
            nsl_strcpy(buf,"#CaseId,Count-Maneuvers,Index-Maneuver,ImageCode,Turn information,Point,Origin,Destinition,TransportMode\n");
            PAL_FileWrite(m_fpOutFile, (uint8*)buf, nsl_strlen(buf), &bytesWritten);
            nsl_memset(buf,0,sizeof(buf));
        }
    }
    else
    {
        PRINTF("\nERROR: Output file wasn't created\n");
        return;
    }

    routeHandlerCallback.callback = &Static_DownloadRouteCallback;
    routeHandlerCallback.callbackData = this;

    PerformanceMeasurement_StartTimer("test");
    PmPrintInputParametersRoute(&(m_batchParams->buf[m_batchParams->params[0]]));

    UpdateGuidanceInformation();
    command = NB_GuidanceInformationGetCommandVersion(m_guidanceInfo);
    // If this asserts, the command instructions sets (imageStyle, pronunStyle and stepsStyle) cannot be found
    // See CreateGuidanceInformation for the three directories that are expected to contain appropriate config.tps files

    if(m_guidanceInfo == NULL && command == 0)
    {
        PrintRouteToFile(NULL, "m_guidanceInfo == NULL && command == 0");
    }
    else
    {

        ParseCsvForRouteOptions(&options, &config);

        config.commandSetVersion = command;

        origin.location.type = NB_Location_Address;
        origin.location.latitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[Route_Orig_Lat]]));
        origin.location.longitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[Route_Orig_Lon]]));

        destination.location.type = NB_Location_Address;
        destination.location.latitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[Route_Dest_Lat]]));
        destination.location.longitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[Route_Dest_Lon]]));

        result = NB_RouteParametersCreateFromPlace(m_context, &origin, &destination, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(m_context, &routeHandlerCallback, &m_routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(m_routeHandler);

        if (result != NE_OK || m_routeHandler == NULL)
        {
            PrintRouteToFile(NULL, "Route wasn't created");
            CU_FAIL("Can't create route handler");
            return;
        }

        result = NB_RouteHandlerStartRequest(m_routeHandler, routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);

        NB_RouteParametersDestroy(routeParameters);
        routeParameters = NULL;

        if( NE_OK != result )
        {
           PrintRouteToFile(NULL, "Can't start request for route");
           CU_FAIL("Can't start request for route");
        }
    }

}

void TestBatchToolProcessor::BatchToolNavigationSession(void)
{
    NB_Place                                 destination = {{0}};
    NB_RouteOptions                          options = {NB_RouteType_Fastest, NB_RouteAvoid_None, NB_TransportationMode_Car, NB_TrafficType_None, NULL};
    NB_RouteConfiguration                    routeConfig = {0};
    NB_NavigationCallbacks                   callbacks = {0};
    NB_Error                                 result = NE_OK;
    uint32                                   command = 0;
    NB_RouteParameters*                      routeParameters = NULL;
    NB_NavigationConfiguration               configuration = { 0 };
    NB_VectorMapPrefetchConfiguration        vectorPrefetchConfig = {0};
    NB_NavigateStatus                        status = NB_NS_Invalid;
    NB_GpsHistory*                           history = NULL;
    const char*                              gpsFileName =  NULL;
    char                                     fullFileName[BATCH_STRING_MAX_LEN] = {0};
    PAL_Error                                fileErr = PAL_Ok;
    bool                                     isFirstQuery = false;
    char                                     buf[BATCH_STRING_MAX_LEN] = {0};
    uint32                                   bytesWritten = 0;
    NB_PointsOfInterestPrefetchConfiguration poiConfig = {{0}};
    NB_EnhancedContentMapConfiguration       enhancedConfig = {{0}};
    NB_Analytics* analytics = NULL;

    if (m_batchParams == NULL || !m_batchParams->buf[m_batchParams->params[0]])
    {
        CU_FAIL("Can't get input data");
        ScheduleFinishQuery();
        return;
    }

    PRINTF("\n%s:\n",&(m_batchParams->buf[m_batchParams->params[0]]) );

    nsl_strlcat(fullFileName, GetDocumentsDirectoryPath(), sizeof(m_batchParams->outputFileName));
    nsl_strlcat(fullFileName, m_batchParams->outputFileName, sizeof(m_batchParams->outputFileName));
    fileErr = PAL_FileExists(m_pal, fullFileName);

    if ( fileErr == PAL_ErrFileNotExist )
    {
        isFirstQuery = true;
    }

    fileErr = PAL_FileOpen(m_pal, fullFileName, PFM_Append, &m_fpOutFile);

    if (fileErr == PAL_Ok)
    {
        if (isFirstQuery)
        {
            // Output Caption - first line
            nsl_strcpy(buf,"#CaseId,Callback Name,Information from callback\n");
            PAL_FileWrite(m_fpOutFile, (uint8*)buf, nsl_strlen(buf), &bytesWritten);
            nsl_memset(buf,0,sizeof(buf));
        }
    }
    else
    {
        PRINTF("\nERROR: Output file wasn't created\n");
        return;
    }

    do
    {
        UpdateGuidanceInformation();
        command = NB_GuidanceInformationGetCommandVersion(m_guidanceInfo);

        if(m_guidanceInfo == NULL && command == 0)
        {
            PrintNavigationToFile(PRINT_TYPE_ERROR, "BatchToolNavigationSession", "m_guidanceInfo == NULL && command == 0");
            break;
        }

        ParseCsvForRouteOptions(&options, &routeConfig);

        routeConfig.commandSetVersion = command;

        SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
        SetupNavigationConfiguration(&configuration);

        //cameraSearchEnabled
        char* cameraSearchEnabled = &(m_batchParams->buf[m_batchParams->params[Camera]]);

        if (0 != nsl_strcmp("", cameraSearchEnabled))
        {
            configuration.cameraSearchEnabled = TRUE;
        }

        destination.location.type = NB_Location_LatLon;
        destination.location.latitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[Route_Dest_Lat]]));
        destination.location.longitude = nsl_atof(&(m_batchParams->buf[m_batchParams->params[Route_Dest_Lon]]));

        gpsFileName = &(m_batchParams->buf[m_batchParams->params[GPS_File]]);

        result = NB_AnalyticsCreate(m_context, NULL, &analytics);
        CU_ASSERT_EQUAL(result, NE_OK);
        (void)NB_ContextSetAnalyticsNoOwnershipTransfer(m_context, analytics);

        CreateEnhancedContentManager(&routeConfig);

        history = NB_ContextGetGpsHistory(m_context);
        CU_ASSERT_PTR_NOT_NULL(history);

        if( NULL == history )
        {
            PrintNavigationToFile(PRINT_TYPE_ERROR, "BatchToolNavigationSession", "NULL == history");
            break;
        }

        m_gpsSource = TestGPSFileLoad(gpsFileName);
        CU_ASSERT_PTR_NOT_NULL(m_gpsSource);

        if( NULL == m_gpsSource )
        {
            PrintNavigationToFile(PRINT_TYPE_ERROR, "BatchToolNavigationSession", "NULL == m_gpsSource");
            break;
        }

        if(((m_gpsFix = TestGPSFileGetNextFix(FIX_DELAY_MILLISECONDS)) != NULL))
        {
            if (m_gpsFix->status == NE_OK)
            {
                result = NB_GpsHistoryAdd(history, m_gpsFix);
                CU_ASSERT_EQUAL(result, NE_OK);
            }
        }

        callbacks.statusChangeCallback = &Static_NavigationSessionStatusChange;
        callbacks.guidanceMessageCallback = &Static_NavigationSessionGuidance;
        callbacks.routeCompleteDownloadCallback = &Static_NavigationCompleteRouteDownload;
        callbacks.routeDownloadCallback = &Static_NavigationRouteDownload;
        callbacks.pointsOfInterestNotificationCallback = &Static_NavigationPointsOfInterestNotifcationCallback;
        callbacks.trafficNotificationCallback = &Static_TrafficNotifcationCallback;
        callbacks.cameraNotificationCallback = &Static_CameraNotifcationCallback;
        callbacks.maneuverPositionChangeCallback = &Static_ManeuverPositionChangeCallback;
        callbacks.enhancedContentNotificationCallback = &Static_EnhancedContentStateNotificationCallback;
        callbacks.userData = this;

        PerformanceMeasurement_StartTimer("test");
        PmPrintInputParametersNavigationSession(&(m_batchParams->buf[m_batchParams->params[0]]), m_gpsSource->curFix);

        result = NB_NavigationCreate(m_context, &configuration, &callbacks, &vectorPrefetchConfig, &m_navigation);
        CU_ASSERT_PTR_NOT_NULL(m_navigation);
        CU_ASSERT_EQUAL(result, NE_OK);

        if( NULL == m_navigation && NE_OK != result )
        {
            PrintNavigationToFile(PRINT_TYPE_ERROR, "BatchToolNavigationSession", "NULL == m_navigation && NE_OK != result");
            break;
        }

        callbacks.userData = this;
        NB_NavigationSetCallbacks(m_navigation, &callbacks);

        //enhanced content
        if (routeConfig.wantCityModels || routeConfig.wantJunctionModels || routeConfig.wantRealisticSigns)
        {
            SetupEnhancedContentMapConfiguration(&enhancedConfig);

            result = NB_NavigationSetEnhancedContentMapConfiguration(m_navigation, &enhancedConfig);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_NavigationSetMapOrientation(m_navigation, NB_MO_Portrait);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        //traffic
        char* traffic = &(m_batchParams->buf[m_batchParams->params[Traffic]]);

        if (0 != nsl_strcmp("", traffic))
        {
            result = NB_NavigationSetTrafficPollingInterval(m_navigation, 0);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        result = NB_RouteParametersCreateFromGpsHistory(m_context, history, &destination,
                                                        &options, &routeConfig, &routeParameters);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);

        if( NULL == routeParameters && NE_OK != result )
        {
            PrintNavigationToFile(PRINT_TYPE_ERROR, "BatchToolNavigationSession", "NULL == routeParameters && NE_OK != result");
            break;
        }

        result = NB_RouteParametersSetLanguage(routeParameters, Test_OptionsGet()->currentLanguage);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationSetGuidanceInformation(m_navigation, m_guidanceInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        //poi
        char* poi = &(m_batchParams->buf[m_batchParams->params[POI]]);

        if (0 != nsl_strcmp("", poi))
        {
            nsl_strcpy(poiConfig.searchScheme,"atlasbook-bing");
            // set up POI prefetch configuration - Matches the Vehicle Settings from ABV5
            poiConfig.categoryCount = 9;
            nsl_strcpy(poiConfig.categories[0], "AE");   // Restaurant
            nsl_strcpy(poiConfig.categories[1], "ACC");  // Gas Stations
            nsl_strcpy(poiConfig.categories[2], "AA");   // Banks & ATMs
            nsl_strcpy(poiConfig.categories[3], "ACA");   // Lodging
            nsl_strcpy(poiConfig.categories[4], "AIC");   // Dentists
            nsl_strcpy(poiConfig.categories[5], "AID");   // Emergency Medical Service
            nsl_strcpy(poiConfig.categories[6], "AIE");   // Hospitals
            nsl_strcpy(poiConfig.categories[7], "AKE");   // Pharmacies
            nsl_strcpy(poiConfig.categories[8], "AFC");   // Police

            poiConfig.densityFactor = 5;
            poiConfig.searchWidthMeters = 400;

            result = NB_NavigationSetPointsOfInterestConfiguration(m_navigation, &poiConfig);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        result = NB_NavigationGetStatus(m_navigation, &status);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(status, NB_NS_Created);

        result = NB_NavigationStartWithRouteParameters(m_navigation, routeParameters, FALSE);
        CU_ASSERT_EQUAL(result, NE_OK);

        if( NE_OK != result )
        {
            PrintNavigationToFile(PRINT_TYPE_ERROR, "BatchToolNavigationSession", "NE_OK != result");
            break;
        }
    }while(0);

    NB_RouteParametersDestroy(routeParameters);
    routeParameters = NULL;

}

// ==================================================
// Callback functions
// ==================================================
void TestBatchToolProcessor::Static_ReverseGeocodeCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->ReverseGeocodeCallback(static_cast<NB_ReverseGeocodeHandler*>(handler), status, err, up, percent);
}

void TestBatchToolProcessor::ReverseGeocodeCallback(NB_ReverseGeocodeHandler* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent)
{
    LOGOUTPUT(LogLevelHigh, ("Reverse Geocode %s progress: %d\n", up ? "upload" : "download", percent));

    NB_Place place={0};
    NB_ReverseGeocodeInformation* information=NULL;

    if (!up)
    {
        if (err)
        {
            CU_FAIL("Callback returned error");
            ScheduleFinishQuery();
            return;
        }
        else if (percent == 100 && status != NB_NetworkRequestStatus_Canceled)
        {
            NB_Error err = NB_ReverseGeocodeHandlerGetReverseGeocodeInformation(m_ReverceGeocodeHandler, &information);
            CU_ASSERT_EQUAL(err, NE_OK);

            if (!err)
            {
                err = NB_ReverseGeocodeInformationGetLocation(information, &(place.location));
                CU_ASSERT_EQUAL(err, NE_OK);

                if (err == NE_OK)
                {
                    PmPrintResultsReverseGeocode(information);
                }
                (void)NB_ReverseGeocodeInformationDestroy(information);
                information = NULL;

                PrintPlaceToFile(&place, 0, m_batchParams->outputFileName);
            }

            ScheduleFinishQuery();
        }
    }
}

void TestBatchToolProcessor::Static_GeocodeCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->GeocodeCallback(static_cast<NB_GeocodeHandler*>(handler), status, err, up, percent);
}

void TestBatchToolProcessor::GeocodeCallback(NB_GeocodeHandler* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent)
{
    LOGOUTPUT(LogLevelHigh, ("Geocode %s progress: %d\n", up ? "upload" : "download", percent));

    NB_Place place={0};
    NB_GeocodeInformation* information = NULL;

    if (!up)
    {
        if (err)
        {
            PmPrintResultsGeocode(0, err);
            CU_FAIL("Callback returned error");
            ScheduleFinishQuery();
            return;
        }
        else if (percent == 100 && status != NB_NetworkRequestStatus_Canceled)
        {
            NB_Error err = NB_GeocodeHandlerGetGeocodeInformation(m_GeocodeHandler, &information);
            CU_ASSERT_EQUAL(err, NE_OK);

            if (m_getCoords)
            {
                NB_Location location = {0};
                err = NB_GeocodeInformationGetLocation(information, 0, &location);
                CU_ASSERT_EQUAL(err, NE_OK);

                m_point.latitude = location.latitude;
                m_point.longitude = location.longitude;

                (void)NB_GeocodeInformationDestroy(information);
                information = NULL;

                BatchToolGeocode();
                return;
            }
            int32  total=0;
            if (!err)
            {

                uint32 start=0;
                uint32 end=0;

                err = NB_GeocodeInformationGetSliceInformation(information, &total, &start, &end);
                CU_ASSERT_EQUAL(err, NE_OK);

                for (uint32 i = 0; !err && i < (end - start); i++)
                {
                    uint32 accuracy = 0;

                    err = NB_GeocodeInformationGetLocationWithAccuracy(information, i, &(place.location), &accuracy);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    if (!err)
                    {
                        PrintPlaceToFile(&place, 0, m_batchParams->outputFileName);
                    }
                }

                (void)NB_GeocodeInformationDestroy(information);
                information = NULL;
            }
            PmPrintResultsGeocode(total, err);
            ScheduleFinishQuery();
        }
    }
}

void TestBatchToolProcessor::Static_POISearchCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->POISearchCallback(status, err, up, percent);
}

void TestBatchToolProcessor::POISearchCallback(NB_RequestStatus status, NB_Error err, uint8 up, int percent)
{
    if (up)
    {
        return;
    }

    if (err != NE_OK)
    {
        PRINTF("ERROR: %d\n",err);
        ScheduleFinishQuery();
        return;
    }

    if (percent == 100)
    {
        NB_SearchInformation* searchInformation = NULL;
        NB_Error result = NB_SearchHandlerGetSearchInformation(m_searchHandler, &searchInformation);
        int resultCount = 0;
        int resultIndex = 0;

        if (result != NE_OK || searchInformation == NULL)
        {
            ScheduleFinishQuery();
            return;
        }

        result = NB_SearchInformationGetResultCount(searchInformation, &resultCount);
        CU_ASSERT_EQUAL(result, NE_OK);
        PmPrintResultsSearchBuilder(resultCount, result);

        if (result != NE_OK)
        {
            ScheduleFinishQuery();
            return;
        }

        for (resultIndex = 0; resultIndex < resultCount; resultIndex++)
        {
            NB_Place place = {{0}};
            double distance = 0.0;

            // Get place information
            result = NB_SearchInformationGetPlace(searchInformation, resultIndex, &place, &distance, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                // Output place information
                PrintPlaceToFile(&place, distance, m_batchParams->outputFileName);
            }
        }

        if (searchInformation)
        {
            NB_SearchInformationDestroy(searchInformation);
            searchInformation = NULL;
        }

        ScheduleFinishQuery();
    }
}

void TestBatchToolProcessor::Static_DownloadRouteCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->DownloadRouteCallback(handler, status, err, percent);
}

void TestBatchToolProcessor::DownloadRouteCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, int percent)
{
    NB_Error result = NE_OK;

    if (err != NE_OK)
    {
        char buffer[1024] = {0};

        PRINTF("ERROR: %d\n",err);

        nsl_sprintf(buffer, "Call back return error: %d",
                    err
                    );
        PrintRouteToFile(NULL, buffer);
        return;
    }

    if (percent == 100)
    {
        NB_RouteInformation*        route;

        result = NB_RouteHandlerGetRouteInformation(m_routeHandler, &route);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(route);

        PmPrintResultsRoute("Test passed");

        if (route)
        {
            PrintRouteToFile(route);
            result = NB_RouteInformationDestroy(route);
            CU_ASSERT_EQUAL(result, NE_OK);
            route = NULL;
        }

        ScheduleFinishQuery();
    }

    return;
}

void TestBatchToolProcessor::Static_NavigationSessionStatusChange(NB_Navigation* navigation, NB_NavigateStatus status, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->m_testFunction = &TestBatchToolProcessor::NavigationSessionStatusChange;
    (me->*me->m_testFunction)();
}

void TestBatchToolProcessor::NavigationSessionStatusChange()
{
    NB_Error                    result = NE_OK;
    NB_RouteInformation*        route = NULL;
    NB_TrafficInformation*      trafficInfo = NULL;
    NB_NavigateStatus           status = NB_NS_Invalid;
    NB_GuidanceMessage*         message = NULL;
    char                        buf[BATCH_STRING_MAX_LEN] = {0};

    uint32 endManeuver = 0;
    double meter = 0;
    uint32 incidents = 0;
    uint32 time = 0;
    uint32 delay = 0;

    m_statusCounter++;

    result = NB_NavigationGetStatus(m_navigation, &status);
    CU_ASSERT_EQUAL(result, NE_OK);

    char* strStatus = GetStatusString(status);

    PrintNavigationToFile(PRINT_TYPE_DEFAULT, "NavigationSessionStatusChange", strStatus);

    do
    {
        if (m_statusCounter == 1)
        {
            CU_ASSERT_EQUAL(status, NB_NS_InitialRoute);
            if(status != NB_NS_InitialRoute)
            {
                PrintNavigationToFile(PRINT_TYPE_ERROR, "NavigationSessionStatusChange", "status != NB_NS_InitialRoute");
            }
            break;
        }
        else if( m_statusCounter == 2 )
        {
            if( status == NB_NS_Invalid || status == NB_NS_Error )
            {
                PrintNavigationToFile(PRINT_TYPE_ERROR, "NavigationSessionStatusChange", "status == NB_NS_Invalid || status == NB_NS_Error");
                break;
            }

            result = NB_NavigationGetInformation(m_navigation, &route, &trafficInfo, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(route);

            if( NE_OK != result || NULL == route )
            {
                PrintNavigationToFile(PRINT_TYPE_ERROR, "NavigationSessionStatusChange", "NE_OK != result || NULL == route");
                break;
            }

            PrintNavigationToFile(PRINT_TYPE_ROUTE, "NavigationSessionStatusChange", NULL);

            result = NB_TrafficInformationGetSummary(trafficInfo, route, 0, 0, endManeuver, &meter, &incidents, &time, &delay);

            nsl_sprintf(buf
                       ,"time = %u delay = %u meter = %f incidents = %d\n"
                       ,time
                       ,delay
                       ,meter
                       ,incidents
                       );

            PrintNavigationToFile(PRINT_TYPE_DEFAULT, "NavigationSessionStatusChange", buf);

            m_gpsFix = TestGPSFileGetNextFix(FIX_DELAY_MILLISECONDS);

            if ((m_gpsFix == NULL) )
            {
                ScheduleFinishQuery();
                break;
            }

            PerformanceMeasurement_StartTimer("test");
            PmPrintInputParametersNavigationSession(&(m_batchParams->buf[m_batchParams->params[0]]), m_gpsSource->curFix);

            result = NB_NavigationUpdateGPSLocation(m_navigation, m_gpsFix);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
        else
        {
            if ( status == NB_NS_UpdatingRoute)
            {
                result = NB_NavigationGetInformation(m_navigation, &route, &trafficInfo, NULL, NULL);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(route);

                if( NE_OK != result || NULL == route )
                {
                    PrintNavigationToFile(PRINT_TYPE_ERROR, "NavigationSessionStatusChange", "NE_OK != result || NULL == route");
                    break;
                }

                PrintNavigationToFile(PRINT_TYPE_ROUTE, "NavigationSessionStatusChange", NULL);
            }

            if( status == NB_NS_Navigating)
            {
                result = NB_NavigationGetManualGuidanceMessage(m_navigation, NB_NAT_Street, NB_NAU_Miles, FALSE, &message);

                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(message);

                if (result != NE_OK && NULL == message)
                {
                    PrintNavigationToFile(PRINT_TYPE_ERROR, "NavigationSessionStatusChange", "result != NE_OK && NULL == message");
                    break;
                }
            }

                m_gpsFix = TestGPSFileGetNextFix(FIX_DELAY_MILLISECONDS);

                if ((m_gpsFix == NULL) )
                {
                    ScheduleFinishQuery();
                    break;
                }

                PerformanceMeasurement_StartTimer("test");
                PmPrintInputParametersNavigationSession(&(m_batchParams->buf[m_batchParams->params[0]]), m_gpsSource->curFix);

                result = NB_NavigationUpdateGPSLocation(m_navigation, m_gpsFix);
                CU_ASSERT_EQUAL(result, NE_OK);
        }

    }while(0);
}

void TestBatchToolProcessor::Static_NavigationSessionGuidance(NB_Navigation* navigation, NB_GuidanceMessage* message, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->NavigationSessionGuidance(message);
}
void TestBatchToolProcessor::NavigationSessionGuidance(NB_GuidanceMessage* message)
{
    int                 num = 0;
    const char*         code = NULL;
    char                bufCode[BATCH_STRING_MAX_LEN] = {0};

    CU_ASSERT_PTR_NOT_NULL(m_navigation);
    CU_ASSERT_PTR_NOT_NULL(message);

    PrintNavigationToFile(PRINT_TYPE_DEFAULT, "NavigationSessionGuidance", "Success");

    if( NULL == m_navigation && NULL == message )
    {
        PrintNavigationToFile(PRINT_TYPE_ERROR, "NavigationSessionGuidance", "NULL == m_navigation && NULL == message");
        return;
    }

    int count = NB_GuidanceMessageGetCodeCount(message);

    for (num = 0; num < count; num++)
    {
        code = NB_GuidanceMessageGetCode(message, num);
        CU_ASSERT_PTR_NOT_NULL(code);

        if (code)
        {
            CU_ASSERT(nsl_strlen(code) > 0);
            nsl_strcat(bufCode, code);
            nsl_strcat(bufCode, " ");

        }
    }

    PrintNavigationToFile(PRINT_TYPE_DEFAULT, "NavigationSessionGuidance", bufCode); 

    NB_GuidanceMessagePlayed(message);
    NB_GuidanceMessageDestroy(message);
}

void TestBatchToolProcessor::Static_NavigationCompleteRouteDownload(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->NavigationCompleteRouteDownload(handler, status, err, up, percent);
}
void TestBatchToolProcessor::NavigationCompleteRouteDownload(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent)
{
    if (status == NB_NetworkRequestStatus_Success)
    {
        PrintNavigationToFile(PRINT_TYPE_DEFAULT, "NavigationCompleteRouteDownload", "Success");
    }
    else
    {
        // Fail on any other status except progress
        if( status != NB_NetworkRequestStatus_Progress )
        {
           PrintNavigationToFile(PRINT_TYPE_ERROR, "NavigationCompleteRouteDownload", "status != NB_NetworkRequestStatus_Progress");
        }
    }

}

void TestBatchToolProcessor::Static_NavigationRouteDownload(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->NavigationRouteDownload(handler, status, err, up, percent);
}
void TestBatchToolProcessor::NavigationRouteDownload(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent)
{

    if (status == NB_NetworkRequestStatus_Success)
    {
        PrintNavigationToFile(PRINT_TYPE_DEFAULT, "NavigationRouteDownload", "Success");
    }
    else
    {
        // Fail on any other status except progress
        if( status != NB_NetworkRequestStatus_Progress )
        {
           PrintNavigationToFile(PRINT_TYPE_ERROR, "NavigationRouteDownload", "status != NB_NetworkRequestStatus_Progress");
        }
    }
}

void TestBatchToolProcessor::Static_NavigationPointsOfInterestNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->NavigationPointsOfInterestNotifcationCallback(handler, status, err, up, percent);
}
void TestBatchToolProcessor::NavigationPointsOfInterestNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent)
{
    NB_PointsOfInterestInformation* poiInfo = NULL;
    NB_Error result = NE_OK;

    if (status == NB_NetworkRequestStatus_Success)
    {
        PrintNavigationToFile(PRINT_TYPE_DEFAULT, "NavigationPointsOfInterestNotifcationCallback", "Success");

        result = NB_NavigationGetInformation(m_navigation, NULL, NULL, NULL, &poiInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        if(!result && poiInfo )
        {
            int poiIndex = 0;
            int count = 0;

            result = NB_PointsOfInterestInformationGetPointOfInterestCount(poiInfo, &count);
            CU_ASSERT_EQUAL(result, NE_OK);

            for (poiIndex = 0; poiIndex < count; poiIndex++)
            {
                char buf[BATCH_STRING_MAX_LEN] = {0};
                NB_PointOfInterest poi = {{{0}}};
                result = NB_PointsOfInterestInformationGetPointOfInterest(poiInfo, poiIndex, &poi);
                CU_ASSERT_EQUAL(result, NE_OK);

                nsl_sprintf(buf
                            ,"\"%d\",\"%d\",\"%f\",\"%f\",\"%s\""
                            ,count
                            ,poiIndex
                            ,poi.place.location.latitude
                            ,poi.place.location.longitude
                            ,poi.place.name
                            );
                nsl_strcat(buf, "\n");

                PrintNavigationToFile(PRINT_TYPE_DEFAULT, "NavigationPointsOfInterestNotifcationCallback", buf);
            }
        }
    }
    else
    {
        // Fail on any other status except progress
        if( status != NB_NetworkRequestStatus_Progress )
        {
            PrintNavigationToFile(PRINT_TYPE_ERROR, "NavigationPointsOfInterestNotifcationCallback", "status != NB_NetworkRequestStatus_Progress");
        }
    }
}

void TestBatchToolProcessor::Static_TrafficNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->TrafficNotifcationCallback(handler, status, err, up, percent);
}
void TestBatchToolProcessor::TrafficNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent)
{
    if (status == NB_NetworkRequestStatus_Success)
    {
        NB_TrafficInformation* trafficInfo = NULL;
        NB_RouteInformation*  route = NULL;
        uint32 endManeuver = 0;
        double meter = 0.0;
        uint32 incidents = 0;
        uint32 time = 0;
        uint32 delay = 0;
        uint32 maneuverIndex = 0;
        uint32 incidentIndex = 0;
        NB_Place incidentPlace = { { 0 } };
        NB_TrafficIncident incidentInfo = { 0 };
        double distance = 0.0;
        int incidentsCount = 0;
        NB_Error result = NE_OK;

        PrintNavigationToFile(PRINT_TYPE_DEFAULT, "TrafficNotifcationCallback", "Success");

        result = NB_NavigationGetInformation(m_navigation, &route, &trafficInfo, NULL, NULL);
        CU_ASSERT_EQUAL(result, NE_OK);

        incidentsCount = NB_TrafficInformationGetIncidentCount(trafficInfo);

        endManeuver = NB_RouteInformationGetManeuverCount(route);

        for (maneuverIndex = 0; maneuverIndex < endManeuver; maneuverIndex++)
        {
            result = NB_TrafficInformationGetSummary(trafficInfo, route, maneuverIndex, 0, maneuverIndex, &meter, &incidents, &time, &delay);
            LOGOUTPUT(LogLevelMedium, ("    maneuver: %d, meter: %f, incidents: %d, time: %d, delay: %d\n", maneuverIndex, meter, incidents, time, delay));
            CU_ASSERT_EQUAL(result, NE_OK);

            for (incidentIndex = 0; incidentIndex < incidents; incidentIndex++)
            {
                char buf[BATCH_STRING_MAX_LEN] = {0};

                NB_TrafficInformationGetRouteManeuverIncident(trafficInfo, route, maneuverIndex, incidentIndex, &incidentPlace, &incidentInfo, &distance);
                CU_ASSERT_EQUAL(result, NE_OK);
                LOGOUTPUT(LogLevelMedium, ("      incident: %d - %s\n", incidentIndex, incidentInfo.description));

                nsl_sprintf(buf
                            ,"\"%d\",\"%f\",\"%d\",\"%d\",\"%d\",\"%d\",\"%s\""
                            ,maneuverIndex
                            ,meter
                            ,incidents
                            ,time
                            ,delay
                            ,incidentIndex
                            ,incidentInfo.description
                            );
                nsl_strcat(buf, "\n");

                PrintNavigationToFile(PRINT_TYPE_DEFAULT, "TrafficNotifcationCallback", buf);
            }
        }

        incidents = NB_TrafficInformationGetIncidentCount(trafficInfo);

        for (incidentIndex = 0; incidentIndex < incidents; incidentIndex++)
        {
            char buf[BATCH_STRING_MAX_LEN] = {0};

            NB_TrafficInformationGetIncident(trafficInfo, incidentIndex, &incidentPlace, &incidentInfo, &distance);
            CU_ASSERT_EQUAL(result, NE_OK);

            nsl_sprintf(buf
                        ,"\"%d\",\"%d\",\"%d\",\"%s\",\"%f\",\"%f\",\"%d\",\"%d\",\"%d\",\"%d\",\"%s\""
                        ,incidentIndex
                        ,incidentInfo.type
                        ,incidentInfo.severity
                            ,incidentInfo.description
                        ,incidentPlace.location.latitude
                        ,incidentPlace.location.longitude
                        ,incidentInfo.entry_time
                        ,incidentInfo.start_time
                        ,incidentInfo.end_time
                        ,incidentInfo.utc_offset
                        ,incidentInfo.road
                        );

            nsl_strcat(buf, "\n");

            PrintNavigationToFile(PRINT_TYPE_DEFAULT, "TrafficNotifcationCallback", buf);
        }
    }
    else
    {
         // Fail on any other status except progress
        if( status != NB_NetworkRequestStatus_Progress )
        {
            PrintNavigationToFile(0, "TrafficNotifcationCallback", "Error: status != NB_NetworkRequestStatus_Progress");
        }
    }

}

void TestBatchToolProcessor::Static_CameraNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->CameraNotifcationCallback(handler, status, err, up, percent);
}

void TestBatchToolProcessor::CameraNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent)
{
    if (status == NB_NetworkRequestStatus_Success)
    {
        NB_CameraInformation* cameraInfo = NULL;
        NB_RouteInformation* route = NULL;
        uint32 endManeuver = 0;
        uint32 maneuverIndex = 0;
        NB_Error result = NE_OK;
        uint32 cameraCount = 0;
        uint32 i = 0;
        uint32 maneuverCameraCount = 0;

        PrintNavigationToFile(PRINT_TYPE_DEFAULT, "CameraNotifcationCallback", "Success");

        result = NB_NavigationGetInformation(m_navigation, &route, NULL, &cameraInfo, NULL);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL_FATAL(cameraInfo);

        if (NE_OK == result && NULL != cameraInfo)
        {
            result = NB_CameraInformationGetCameraCount(cameraInfo, &cameraCount);

            for(i = 0; i < cameraCount; i++)
            {
                char buf[BATCH_STRING_MAX_LEN] = {0};
                NB_Camera camera = {0};

                result = NB_CameraInformationGetCamera(cameraInfo, i, &camera);
                CU_ASSERT_EQUAL(result, NE_OK);

                nsl_sprintf(buf
                            ,"\"%d\",\"%f\",\"%f\",\"%f\",\"%d\",\"%d\",\"%s\""
                            ,i 
                            ,camera.distance
                            ,camera.speedLimit
                            ,camera.heading
                            ,camera.isBidirectional
                            ,camera.status
                            ,camera.description
                            );
                nsl_strcat(buf, "\n");

                PrintNavigationToFile(PRINT_TYPE_DEFAULT, "CameraNotifcationCallback", buf);
            }

            endManeuver = NB_RouteInformationGetManeuverCount(route);

            for (maneuverIndex = 0; maneuverIndex < endManeuver; maneuverIndex++)
            {
                result = NB_CameraInformationGetManeuverCameraCount(cameraInfo, maneuverIndex, &maneuverCameraCount);
                CU_ASSERT_EQUAL(result, NE_OK);

                for (i = 0; i < maneuverCameraCount; i++)
                {
                    char buf[BATCH_STRING_MAX_LEN] = {0};
                    NB_Camera camera = {0};
                    result = NB_CameraInformationGetManeuverCamera(cameraInfo, maneuverIndex, i, &camera);
                    CU_ASSERT_EQUAL(result, NE_OK);

                    nsl_sprintf(buf
                                ,"\"%d\",\"%d\",\"%d\",\"%f\",\"%f\",\"%f\",\"%s\""
                                ,maneuverIndex
                                ,maneuverCameraCount
                                ,i
                                ,camera.routeDistance
                                ,camera.place.location.latitude
                                ,camera.place.location.longitude
                                ,camera.description
                                );
                nsl_strcat(buf, "\n");

                PrintNavigationToFile(PRINT_TYPE_DEFAULT, "CameraNotifcationCallback", buf);
                }
            }
        }
        else
        {
            PrintNavigationToFile(PRINT_TYPE_ERROR, "CameraNotifcationCallback", "NE_OK == result && NULL != cameraInfo");
        }
    }
    else
    {
         // Fail on any other status except progress
        if( status != NB_NetworkRequestStatus_Progress )
        {
            PrintNavigationToFile(PRINT_TYPE_ERROR, "CameraNotifcationCallback", "status != NB_NetworkRequestStatus_Progress");
        }
    }
}

void TestBatchToolProcessor::Static_ManeuverPositionChangeCallback(NB_Navigation* navigation, uint32 maneuver, NB_NavigateManeuverPos maneuvPosition, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->ManeuverPositionChangeCallback(maneuver, maneuvPosition);
}
void TestBatchToolProcessor::ManeuverPositionChangeCallback(uint32 maneuver, NB_NavigateManeuverPos maneuvPosition)
{
    PrintNavigationToFile(PRINT_TYPE_DEFAULT, "ManeuverPositionChangeCallback", GetManeuverPostionString(maneuvPosition));
}

void TestBatchToolProcessor::Static_EnhancedContentStateNotificationCallback(NB_Navigation* navigation, NB_EnhancedContentState* state, NB_EnhancedContentStateData available, void* userData)
{
    TestBatchToolProcessor* me = static_cast<TestBatchToolProcessor*>(userData);
    me->EnhancedContentStateNotificationCallback(state, available);
}
void TestBatchToolProcessor::EnhancedContentStateNotificationCallback(NB_EnhancedContentState* state, NB_EnhancedContentStateData available)
{
    if(available == NB_ECSD_None)
    {
        PrintNavigationToFile(PRINT_TYPE_DEFAULT, "EnhancedContentStateNotificationCallback", "No enhanced content available");
    }
    else
    {
        NB_Error result = NE_OK;

        PrintNavigationToFile(PRINT_TYPE_DEFAULT, "EnhancedContentStateNotificationCallback", "Success");

        // Check if map frame data available
        if ((available & NB_ECSD_JunctionMapFrame) || (available & NB_ECSD_CityModelMapFrame))
        {
            uint32 tileCount = 0;
            const char** tileIds = NULL;
            uint32 splineCount = 0;
            const NB_MapFrameSpline* splines = NULL;
            NB_MapFrameProjectionParameters projectionParameters = {NB_ECPT_Invalid, NB_ECPD_Invalid, 0};
            uint32 index = 0;

            result = NB_EnhancedContentStateGetMapFrameTileIds(state, &tileCount, &tileIds);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_NOT_EQUAL(tileCount, 0);

            result = NB_EnhancedContentStateGetMapFrameSplines(state, &splineCount, &splines);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_NOT_EQUAL(splineCount, 0);

            for (index = 0; index < tileCount; index++)
            {
                char buf[BATCH_STRING_MAX_LEN] = {0};
                nsl_sprintf(buf
                            ,"\"%s\",\"%d\",\"%s\""
                            ,(available & NB_ECSD_JunctionMapFrame) ? "MJO" : "ECM"
                            ,tileCount
                            ,tileIds[index]
                            );
                nsl_strcat(buf, "\n");

                PrintNavigationToFile(PRINT_TYPE_DEFAULT, "EnhancedContentStateNotificationCallback", buf);
            }

            for (index = 0; index < splineCount; index++)
            {
                char buf[BATCH_STRING_MAX_LEN] = {0};

                nsl_sprintf(buf
                            ,"\"%s\",\"%d\",\"%s\""
                            ,(available & NB_ECSD_JunctionMapFrame) ? "MJO" : "ECM"
                            ,splineCount
                            ,splines[index].pathId
                            );
                nsl_strcat(buf, "\n");

                PrintNavigationToFile(PRINT_TYPE_DEFAULT, "EnhancedContentStateNotificationCallback", buf);
            }

            result = NB_EnhancedContentStateGetMapFrameProjectionParameters(state, &projectionParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            char buf[BATCH_STRING_MAX_LEN] = {0};

            nsl_sprintf(buf
                        ,"\"%d\",\"%d\",\"%f\",\"%f\",\"%f\",\"%f\",\"%f\",\"%f\""
                        ,projectionParameters.projection
                        ,projectionParameters.datum
                        ,projectionParameters.originLatitude
                        ,projectionParameters.originLongitude
                        ,projectionParameters.scaleFactor
                        ,projectionParameters.falseEasting
                        ,projectionParameters.falseNorthing
                        ,projectionParameters.zOffset
                        );
            nsl_strcat(buf, "\n");

            PrintNavigationToFile(PRINT_TYPE_DEFAULT, "EnhancedContentStateNotificationCallback", buf);
        }

        // Check if realistic sign available
        if (available & NB_ECSD_RealisticSign)
        {
            char buf[BATCH_STRING_MAX_LEN] = {0};
            NB_RealisticSign sign = { 0 };

            result = NB_EnhancedContentStateGetRealisticSign(state, &sign);
            CU_ASSERT_EQUAL(result, NE_OK);

            nsl_sprintf(buf
                        ,"\"%s\""
                        ,sign.signId
                        );
            nsl_strcat(buf, "\n");

            PrintNavigationToFile(PRINT_TYPE_DEFAULT, "EnhancedContentStateNotificationCallback", buf);

            result = NB_EnhancedContentStateFreeRealisticSign(state, &sign);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }

}

// ==================================================
// Utility functions
// ==================================================

void TestBatchToolProcessor::PrintPlaceToFile(NB_Place* place, double distance, char* fileName)
{
    char   buf[BATCH_STRING_MAX_LEN] = {0};
    char   addressBuf[BATCH_STRING_MAX_LEN] = {0};
    char   fullFileName[BATCH_STRING_MAX_LEN] = {0};
    int    index = 0;
    uint32 bytesWritten = 0;

    PAL_File* fp = NULL;
    PAL_Error fileErr = PAL_Ok;
    bool isFirstQuery = false;

    nsl_strlcat(fullFileName, GetDocumentsDirectoryPath(), sizeof(fullFileName));
    nsl_strlcat(fullFileName, fileName, sizeof(fullFileName));
    fileErr=PAL_FileExists(m_pal, fullFileName);
    if (fileErr==PAL_ErrFileNotExist) isFirstQuery = true;

    fileErr=PAL_FileOpen(m_pal, fullFileName, PFM_Append, &fp);

    if (fileErr == PAL_Ok)
    {
        if (isFirstQuery)
        {
            // Output Caption - first line
            nsl_strcpy(buf,"#CaseId,Lat/Lon,Name,Address,AreaName,Airport,Distance,Phones,Categories\n");
            PAL_FileWrite(fp, (uint8*)buf, nsl_strlen(buf), &bytesWritten);
            nsl_memset(buf,0,sizeof(buf));
        }

        // Output Address
        if (nsl_strlen(place->location.streetnum))
        {
            nsl_strcat(addressBuf, place->location.streetnum);
            nsl_strcat(addressBuf," ");
        }

        if (nsl_strlen(place->location.street1))
        {
            nsl_strcat(addressBuf, place->location.street1);
            nsl_strcat(addressBuf," ");
        }

        if (nsl_strlen(place->location.street2))
        {
            nsl_strcat(addressBuf,"& ");
            nsl_strcat(addressBuf, place->location.street1);
            nsl_strcat(addressBuf," ");
        }

        if (nsl_strlen(place->location.city))
        {
            nsl_strcat(addressBuf, place->location.city);
            nsl_strcat(addressBuf," ");
        }

        if (nsl_strlen(place->location.state))
        {
            nsl_strcat(addressBuf, place->location.state);
            nsl_strcat(addressBuf," ");
        }

        if (nsl_strlen(place->location.county))
        {
            nsl_strcat(addressBuf, place->location.county);
            nsl_strcat(addressBuf," ");
        }

        if (nsl_strlen(place->location.postal))
        {
            nsl_strcat(addressBuf, place->location.postal);
            nsl_strcat(addressBuf," ");
        }

        if (nsl_strlen(place->location.country))
        {
            nsl_strcat(addressBuf, place->location.country);
        }

        nsl_sprintf(buf
            ,"\"%s\",\"%f,%f\",\"%s\",\"%s\",\"%s\",\"%s\",\"%f\""
                    ,m_batchParams->buf
                    ,place->location.latitude
                    ,place->location.longitude
                    ,place->name
                    ,addressBuf
                    ,place->location.areaname
                    ,place->location.airport
                    ,distance
                    );

        nsl_strcat(buf, ",\"");


        // Output phones
        for (index = 0; index < place->numphone; ++index)
        {
            if (index) nsl_strcat(buf, ";");
            nsl_strcat(buf, place->phone[index].country);
            nsl_strcat(buf, "(");
            nsl_strcat(buf, place->phone[index].area);
            nsl_strcat(buf, ")");
            nsl_strcat(buf, place->phone[index].number);
        }

        nsl_strcat(buf, "\",\"");

        // Output categories
        for (index = 0; index < place->numcategory; ++index)
        {
            if (index) nsl_strcat(buf, "+");
            nsl_strcat(buf, place->category[index].code);
        }

        nsl_strcat(buf, "\"\n");

        PAL_FileWrite(fp, (uint8*)buf, nsl_strlen(buf), &bytesWritten);
        PAL_FileClose(fp);
    }

    fp=NULL;

    PRINTF("%s", buf);
}

void TestBatchToolProcessor::PrintNavigationToFile(int type, char* callbackName, char* buffer)
{
     char                      buf[BATCH_STRING_MAX_LEN] = {0};
     uint32                    bytesWritten = 0;
     NB_Error                  result = NE_OK;

    switch(type)
    {

        case(PRINT_TYPE_DEFAULT): // callback name+str
        {
            if( 0 == nsl_strcmp("Success", buffer))
            {
                PmPrintResultsNavigationSession(callbackName, m_gpsSource->curFix);
            }

            if( 0 == nsl_strcmp("NavigationSessionStatusChange", callbackName))
            {
                char buf[BATCH_STRING_MAX_LEN] = {0};

                nsl_strcat(buf, callbackName);
                nsl_strcat(buf, " with status:");
                nsl_strcat(buf, buffer);
                PmPrintResultsNavigationSession(buf, m_gpsSource->curFix);
            }

            nsl_sprintf(buf
                        ,"\"%s\",\"%s\",\"%s\""
                        ,m_batchParams->buf
                        ,callbackName
                        ,buffer
                       );
            nsl_strcat(buf, "\n");

            PAL_FileWrite(m_fpOutFile, (uint8*)buf, nsl_strlen(buf), &bytesWritten);
            break;
        }

        case (PRINT_TYPE_ROUTE): //route information
        {
            NB_RouteInformation* route = NULL;

            nsl_sprintf(buf
                       ,"\"%s\",\"%s\""
                       ,m_batchParams->buf
                       ,"Route Information"
                       );
            nsl_strcat(buf, "\n");

            PAL_FileWrite(m_fpOutFile, (uint8*)buf, nsl_strlen(buf), &bytesWritten);

            result = NB_NavigationGetInformation(m_navigation, &route, NULL, NULL, NULL);

            if(NE_OK == result || NULL != route)
            {
                PrintRouteToFile(route);
            }
            break;
        }

        case(PRINT_TYPE_ERROR): // error
        {
            PmPrintResultsNavigationSession(callbackName, m_gpsSource->curFix, buffer);

            nsl_sprintf(buf
                        ,"\"%s\",\"%s\",\"Error: %s\""
                        ,m_batchParams->buf
                        ,callbackName
                        ,buffer
                        );
            nsl_strcat(buf, "\n");
            PAL_FileWrite(m_fpOutFile, (uint8*)buf, nsl_strlen(buf), &bytesWritten);

            ScheduleFinishQuery();
            break;
        }
    }

}

void TestBatchToolProcessor::PrintRouteToFile(NB_RouteInformation* route, char* err )
{
    char                      buf[BATCH_STRING_MAX_LEN] = {0};
    uint32                    bytesWritten = 0;
    NB_Error                  result = NE_OK;
    uint32                    numManeuvers = 0;
    uint32                    maneuverIndex = 0;
    double                    distance = 0.0;
    char                      imageCode[40] = {0};
    NB_Place                  origin = {{0}};
    NB_Place                  destination = {{0}};
    NB_NavigateTransportMode  mode =  NB_NTM_Invalid;
    char                      bufferOrigin[1024] = {0};
    char                      bufferDest[1024] = {0};

    // Output
    if( NULL != err )
    {
        PmPrintResultsRoute(err);
        nsl_sprintf(buf
                    ,"\"%s\",\"Error\",\"%s\""
                    ,m_batchParams->buf
                    ,err
                    );
        nsl_strcat(buf, "\n");
        PAL_FileWrite(m_fpOutFile, (uint8*)buf, nsl_strlen(buf), &bytesWritten);


        ScheduleFinishQuery();
        return;
    }
    else
    {
        //Get count of maneuvers
        numManeuvers = NB_RouteInformationGetManeuverCount(route);

        result = NB_RouteInformationGetOrigin(route, &origin);
        result = NB_RouteInformationGetDestination(route, &destination);
        result = NB_RouteInformationGetTransportMode(route, &mode);

        nsl_sprintf(bufferOrigin, "Name: %s Phone: %s Areaname: %s Streetnum: %s Street1: %s Street2: %s City: %s Country: %s State: %s Postal: %s Country: %s Airoport: %s Freeform: %s Lat: %f Lon: %f Type: %d",
                    origin.name,
                    origin.phone,
                    origin.location.areaname,
                    origin.location.streetnum,
                    origin.location.street1,
                    origin.location.street2,
                    origin.location.city,
                    origin.location.country,
                    origin.location.state,
                    origin.location.postal,
                    origin.location.country,
                    origin.location.airport,
                    origin.location.freeform,
                    origin.location.latitude,
                    origin.location.longitude,
                    origin.location.type
                    );

        nsl_sprintf(bufferDest, "Name: %s Phone: %s Areaname: %s Streetnum: %s Street1: %s Street2: %s City: %s Country: %s State: %s Postal: %s Country: %s Airoport: %s Freeform: %s Lat: %f Lon: %f Type: %d",
                    destination.name,
                    destination.phone,
                    destination.location.areaname,
                    destination.location.streetnum,
                    destination.location.street1,
                    destination.location.street2,
                    destination.location.city,
                    destination.location.country,
                    destination.location.state,
                    destination.location.postal,
                    destination.location.country,
                    destination.location.airport,
                    destination.location.freeform,
                    destination.location.latitude,
                    destination.location.longitude,
                    destination.location.type
                    );

        for (maneuverIndex = 0; maneuverIndex < numManeuvers; maneuverIndex++)
        {
            #define _SIZE 1024
            char primaryManeuver[_SIZE] = {0};
            char secondaryManeuver[_SIZE] = {0};
            char currentRoad[_SIZE] = {0};
            char buffer[_SIZE] = {0};
            NB_MercatorPolyline* polyline = NULL;
            double x = -999.0, y = -999.0;

            result = NB_RouteInformationGetTurnInformation(route,
                                                           maneuverIndex,
                                                           NULL,
                                                           primaryManeuver,
                                                           _SIZE,
                                                           secondaryManeuver,
                                                           _SIZE,
                                                           currentRoad,
                                                           _SIZE,
                                                           &distance,
                                                           FALSE);

            nsl_sprintf(buffer, "Primary: %s Secondary: %s Current Road: %s Distance: %f",
                        primaryManeuver,
                        secondaryManeuver,
                        currentRoad,
                        distance);

            // Get turn image code
            result = NB_RouteInformationGetTurnImage(route, m_guidanceInfo, maneuverIndex, NB_NMP_Current, NB_NAU_Miles, imageCode, sizeof(imageCode));

            result = NB_RouteInformationGetRoutePolyline(route, maneuverIndex, maneuverIndex + 1, &polyline);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(polyline)

            if( polyline != NULL )
            {
                CU_ASSERT_PTR_NOT_NULL(polyline->points)
                NB_SpatialConvertMercatorToLatLong(polyline->points[0].mx, polyline->points[0].my,&x, &y);
            }

            nsl_sprintf(buf
                        ,"\"%s\",\"%d\",\"%d\",\"%s\",\"%s\",\"%f,%f\",\"%s\",\"%s\",\"%d\""
                        ,m_batchParams->buf
                        ,numManeuvers
                        ,maneuverIndex
                        ,imageCode
                        ,buffer
                        ,x
                        ,y
                        ,bufferOrigin
                        ,bufferDest
                        ,mode
                        );

            bufferOrigin[0] = '\0';
            bufferDest[0] = '\0';

            nsl_strcat(buf, "\n");
            PAL_FileWrite(m_fpOutFile, (uint8*)buf, nsl_strlen(buf), &bytesWritten);

            if (polyline)
            {
                result = NB_MercatorPolylineDestroy(polyline);
                CU_ASSERT_EQUAL(result, NE_OK);
                polyline = NULL;
            }
        }
    }

    PRINTF("end");
}

void TestBatchToolProcessor::FinishQuery(void)
{
    NB_Error result = NE_OK;

    m_getCoords = false;
    m_statusCounter = 0;

    if(m_fpOutFile != NULL)
    {
        PAL_FileClose(m_fpOutFile);
        m_fpOutFile = NULL;
    }

    if (m_gpsSource != NULL)
    {
        TestGPSFileDestroy(m_gpsSource);
        m_gpsSource = NULL;
    }

    if (m_navigation != NULL)
    {
        result = NB_NavigationDestroy(m_navigation);
        CU_ASSERT_EQUAL(result, NE_OK);
        m_navigation = NULL;
    }

    if (m_initialized)
    {
        SetCallbackCompletedEvent(m_completeEvent);
    }
}

void TestBatchToolProcessor::ScheduleFinishQuery(void)
{
    m_testFunction = &TestBatchToolProcessor::FinishQuery;
    (this->*m_testFunction)();
}

void TestBatchToolProcessor::ParseCsvForRouteOptions(NB_RouteOptions* options, NB_RouteConfiguration* config)
{
    char* strMode = NULL;
    char* strType = NULL;
    char* strAvoid = NULL;
    char* strManeuvers = NULL;
    NB_TransportationMode transport = NB_TransportationMode_Car;
    NB_RouteType          type = NB_RouteType_Fastest;
    NB_RouteAvoid         avoid = NB_RouteAvoid_None;
    NB_TrafficType        traffic = NB_TrafficType_None;


    strMode = &(m_batchParams->buf[m_batchParams->params[Route_Mode]]);

    if ( 0 == nsl_strcmp("bicycle", strMode) )
    {
        transport = NB_TransportationMode_Bicycle;
    }
    if ( 0 == nsl_strcmp("truck", strMode))
    {
        transport = NB_TransportationMode_Truck;
    }
    if ( 0 == nsl_strcmp("pedestrian", strMode))
    {
        transport = NB_TransportationMode_Pedestrian;
    }

    strType = &(m_batchParams->buf[m_batchParams->params[Route_Type]]);

    if ( 0 == nsl_strcmp("shortest", strType) )
    {
        type = NB_RouteType_Shortest;
    }
    if ( 0 == nsl_strcmp("easiest", strType))
    {
        type = NB_RouteType_Easiest;
    }

    strAvoid = &(m_batchParams->buf[m_batchParams->params[Route_Avoide]]);

    if ( 0 == nsl_strcmp("hov", strAvoid) )
    {
        avoid = NB_RouteAvoid_HOV;
    }
    if ( 0 == nsl_strcmp("toll", strAvoid) )
    {
        avoid = NB_RouteAvoid_Toll;
    }
    if ( 0 == nsl_strcmp("highway", strAvoid) )
    {
        avoid = NB_RouteAvoid_Highway;
    }
    if ( 0 == nsl_strcmp("uturn", strAvoid) )
    {
        avoid = NB_RouteAvoid_UTurn;
    }
    if ( 0 == nsl_strcmp("unpaved", strAvoid) )
    {
        avoid = NB_RouteAvoid_Unpaved;
    }
    if ( 0 == nsl_strcmp("ferry", strAvoid) )
    {
        avoid = NB_RouteAvoid_Ferry;
    }

    char* strTraffic = &(m_batchParams->buf[m_batchParams->params[Traffic]]);

    if (0 != nsl_strcmp("", strTraffic))
    {
        traffic = NB_TrafficType_Default;
    }

    options->transport = transport;
    options->type = type;
    options->traffic = traffic;
    options->avoid = avoid;
    options->pronunStyle = Test_OptionsGet()->voiceStyle;

    strManeuvers = &(m_batchParams->buf[m_batchParams->params[Route_Maneuvers]]);

    if ( 0 != nsl_strcmp("", strManeuvers))
    {
        char* man = NULL;
        char* manTmp = NULL;
        bool fExit = false;

        do
        {
            man = nsl_strchr(strManeuvers, '+');

            if (man)
            {
                manTmp = (char*)nsl_malloc((nsl_strlen(strManeuvers) + 1) * sizeof(char));
                strncpy(manTmp, strManeuvers, (man - strManeuvers) / sizeof(char));
                manTmp[(man - strManeuvers)/sizeof(char)] = '\0';
                strManeuvers = ++man;
            }
            else
            {
                manTmp = (char*)nsl_malloc((nsl_strlen(strManeuvers) + 1) * sizeof(char));
                strcpy(manTmp, strManeuvers);
                fExit = true;
            }

            if ( 0 == nsl_strcmp("ferry", manTmp) )
            {
                config->wantFerryManeuvers = TRUE;
            }
            if ( 0 == nsl_strcmp("bridge", manTmp) )
            {
                config->wantBridgeManeuvers = TRUE;
            }
            if ( 0 == nsl_strcmp("tunnel", manTmp) )
            {
                config->wantTunnelManeuvers = TRUE;
            }
            if ( 0 == nsl_strcmp("roundabout", manTmp) )
            {
                config->wantEnterRoundaboutManeuvers = TRUE;
            }

            nsl_free(manTmp);
            manTmp = NULL;

            if(true == fExit) break;

        }
        while(1);
    }

    config->wantDestinationLocation = TRUE;
    config->wantManeuvers = TRUE;
    config->wantOriginLocation = TRUE;
    config->wantPolyline = FALSE;
    config->wantRouteMap = FALSE;
    config->wantCrossStreets = TRUE;
    config->maxPronunFiles = 64; /* 64 for speedup GPS file playback, 3 for normal GPS */

    if(m_navigationSession)
    {
        config->wantOriginLocation = TRUE;
        config->wantDestinationLocation = TRUE;
        config->wantRouteMap = FALSE;
        config->wantCrossStreets = TRUE;
        config->wantPolyline = FALSE;
        config->wantManeuvers = TRUE;
        config->wantFerryManeuvers = TRUE;
        config->wantTowardsRoadInformation = FALSE;
        config->wantIntersectionRoadInformation = FALSE;
        config->wantCountryInformation = TRUE;
        config->wantEnterRoundaboutManeuvers = TRUE;
        config->wantEnterCountryManeuvers = TRUE;
        config->wantBridgeManeuvers = TRUE;
        config->wantTunnelManeuvers = TRUE;
        config->maxPronunFiles = 64; // 64 for speedup GPS file playback, 3 for normal GPS
        config->wantRouteExtents = TRUE;

        char* enhanced = &(m_batchParams->buf[m_batchParams->params[Enhanced]]);

        if ( 0 != nsl_strcmp("", enhanced))
        {
            char* man = NULL;
            char* manTmp = NULL;
            bool fExit = false;

            do
            {
                man = nsl_strchr(enhanced, '+');

                if (man)
                {
                    manTmp = (char*)nsl_malloc((nsl_strlen(enhanced) + 1) * sizeof(char));
                    strncpy(manTmp, enhanced, (man - enhanced) / sizeof(char));
                    manTmp[(man - enhanced)/sizeof(char)] = '\0';
                    enhanced = ++man;
                }
                else
                {
                    manTmp = (char*)nsl_malloc((nsl_strlen(enhanced) + 1) * sizeof(char));
                    strcpy(manTmp, enhanced);
                    fExit = true;
                }

                if ( 0 == nsl_strcmp("realisticSigns", manTmp) )
                {
                    config->wantCityModels = TRUE;
                }
                if ( 0 == nsl_strcmp("junctionModels", manTmp) )
                {
                    config->wantJunctionModels = TRUE;
                }
                if ( 0 == nsl_strcmp("cityModels", manTmp) )
                {
                    config->wantRealisticSigns = TRUE;
                }

                nsl_free(manTmp);
                manTmp = NULL;

                if(true == fExit) break;

            }while(1);
        }
    // want speed camera request disabled by default
        config->wantSpeedCameras = FALSE;
    }

}

void TestBatchToolProcessor::UpdateGuidanceInformation()
{
    if (m_guidanceInfo != NULL)
    {
        NB_Error result = NB_GuidanceInformationDestroy(m_guidanceInfo);
        CU_ASSERT_EQUAL(result, NE_OK);
        m_guidanceInfo = NULL;
    }
    m_guidanceInfo = CreateGuidanceInformation(m_pal, m_context);
}

void TestBatchToolProcessor::SetupVectorMapPrefetchConfiguration(NB_VectorMapPrefetchConfiguration* config)
{
    config->segmentExtensionLengthMeters = 1000.0;
    config->prefetchWindowWidthMeters = 750.0;
    config->minPrefetchDistance = 1000.0;

    config->numberOfPrefetchTiles = 75;

    config->tz = 16;
    config->secondsBeforeFirstPrefetch = 5;
    config->secondsBetweenPrefetch = 15;
}

void TestBatchToolProcessor::SetupNavigationConfiguration(NB_NavigationConfiguration* configuration)
{
    configuration->gpsFixMinimumSpeed = 1.0;
    configuration->startupRegionSizeFactor = 1.5;
    configuration->filteredGpsMinimumSpeed = 5.5;
    configuration->filteredGpsConstant = 0.97;
    configuration->trafficNotifyMaximumRetries = 3;
    configuration->trafficNotifyDelay = 15 * 1000;
    configuration->completeRouteMaximumAttempts = 2;
    configuration->completeRouteRequestDelay = 5 * 1000;
    configuration->pronunRequestDelay = 15 * 1000;
    configuration->cameraSearchDelay = 15 * 1000;
    configuration->cameraSearchDistance = 99999999;
    configuration->startupRegionMinimumSize = 150;
    configuration->excessiveRecalcMaximumCount = 3;
    configuration->excessiveRecalcTimePeriod = 120;
    configuration->filteredGpsEnabled = FALSE;
    configuration->cameraSearchEnabled = FALSE;
}

TestGPSFile* TestBatchToolProcessor::TestGPSFileLoad(const char* gpsFileName)
{
    PAL_Error       palError = PAL_Ok;
    TestGPSFile*    pThis = (TestGPSFile*)nsl_malloc(sizeof(TestGPSFile));

    CU_ASSERT_PTR_NOT_NULL(pThis);

    if (pThis == NULL)
    {
        return pThis;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    palError = PAL_FileLoadFile(m_pal, gpsFileName, &pThis->fileBuffer, &pThis->fileSize);

    CU_ASSERT_EQUAL(palError, PAL_Ok);
    CU_ASSERT_PTR_NOT_NULL(pThis->fileBuffer);
    CU_ASSERT(pThis->fileSize > 0);

    pThis->nextFix = (GPSFileRecord*) pThis->fileBuffer;
    pThis->numFixes = pThis->fileSize / sizeof(GPSFileRecord);
    pThis->curFix = 0;

    // Make sure the file makes sense for the record size
    CU_ASSERT_EQUAL(pThis->fileSize % sizeof(GPSFileRecord), 0);

    return pThis;
}

NB_GpsLocation* TestBatchToolProcessor::TestGPSFileGetNextFix(int delayMilliseconds)
{
    if (m_gpsSource->curFix >= m_gpsSource->numFixes)
    {
        /* We are out of fixes */
        return NULL;
    }

    GPSInfo2GPSFix(m_gpsSource->nextFix, &m_gpsSource->curGPSLocation);

    if (delayMilliseconds)
    {
        // Wait for an event that will never be signaled.  This is an easy way to sleep (and pump messages).
        WaitForCallbackCompletedEvent(m_completeEvent, delayMilliseconds);
    }

    m_gpsSource->nextFix++;
    m_gpsSource->curFix++;

    return &m_gpsSource->curGPSLocation;
}

void TestBatchToolProcessor::GPSInfo2GPSFix(GPSFileRecord* pos, NB_GpsLocation* cur)
{
    cur->valid = pos->fValid;
    cur->gpsTime = pos->dwTimeStamp;
    cur->status = (NB_Error)pos->status;

    /* Convert the values stored in a standard NIM GPS File (BREW AEEGPSInfo Structures
       into standard units.  See the BREW SDK documentation and the Qualcomm GPSOne
       Developer Guide for more information */

    cur->status = (NB_Error)pos->status;
    cur->gpsTime = pos->dwTimeStamp;
    cur->valid = pos->fValid;
    cur->latitude = pos->dwLat * 0.00000536441803;
    cur->longitude = pos->dwLon * 0.00000536441803;
    cur->heading = pos->wHeading * 0.3515625;
    cur->horizontalVelocity = pos->wVelocityHor * 0.25;
    cur->altitude = pos->wAltitude - 500.0;
    cur->verticalVelocity = pos->bVelocityVer * 0.5;
    cur->horizontalUncertaintyAngleOfAxis = pos->bHorUncAngle * 5.625;
    cur->horizontalUncertaintyAlongAxis = UncDecode(pos->bHorUnc);
    cur->horizontalUncertaintyAlongPerpendicular = UncDecode(pos->bHorUncPerp);
    cur->verticalUncertainty = pos->wVerUnc;
    cur->utcOffset = 0;
    cur->numberOfSatellites = 0;

}

double TestBatchToolProcessor::UncDecode(uint8 unc)
{
    if (unc < sizeof(unctbl) / sizeof(*unctbl))
        return (unctbl[unc]);
    else
        return (-2.0);
}

char* TestBatchToolProcessor::GetStatusString(NB_NavigateStatus status)
{
    switch(status)
    {
    case NB_NS_Invalid:
        return "NB_NS_Invalid";
    case NB_NS_Created:
        return "NB_NS_Created";
    case NB_NS_InitialRoute:
        return "NB_NS_InitialRoute";
    case NB_NS_NavigatingStartup:
        return "NB_NS_NavigatingStartup";
    case NB_NS_Navigating:
        return "NB_NS_Navigating";
    case NB_NS_NavigatingArriving:
        return "NB_NS_NavigatingArriving";
    case NB_NS_Arrived:
        return "NB_NS_Arrived";
    case NB_NS_UpdatingRoute:
        return "NB_NS_UpdatingRoute";
    case NB_NS_NavigatingConfirmRecalc:
        return "NB_NS_NavigatingConfirmRecalc";
    case NB_NS_NavigatingFerry:
       return "NB_NS_NavigatingFerry";
    case NB_NS_Error:
        return "NB_NS_Error";
    default:
        return "Unknown NB_NavigateStatus";
    }
}

void TestBatchToolProcessor::TestGPSFileDestroy(TestGPSFile* gps)
{
    CU_ASSERT_PTR_NOT_NULL(gps);

    if (gps == NULL)
    {
        return;
    }

    CU_ASSERT_PTR_NOT_NULL(gps->fileBuffer);

    if (gps->fileBuffer != NULL)
    {
        nsl_free(gps->fileBuffer);
    }

    nsl_free(gps);
}

char* TestBatchToolProcessor::GetManeuverPostionString(NB_NavigateManeuverPos position)
{
    switch (position)
    {
    case NB_NMP_Invalid:
        return "NB_NMP_Invalid";
    case NB_NMP_Current:
        return "NB_NMP_Current";
    case NB_NMP_Continue:
        return "NB_NMP_Continue";
    case NB_NMP_ShowTurn:
        return "NB_NMP_ShowTurn";
    case NB_NMP_Prepare:
        return "NB_NMP_Prepare";
    case NB_NMP_Turn:
        return "NB_NMP_Turn";
    case NB_NMP_Past:
        return "NB_NMP_Past";
    case NB_NMP_Recalculate:
        return "NB_NMP_Recalculate";
    case NB_NMP_SoftRecalculate:
        return "NB_NMP_SoftRecalculate";
    case NB_NMP_RecalculateConfirm:
        return "NB_NMP_RecalculateConfirm";
    case NB_NMP_RecalculateTraffic:
        return "NB_NMP_RecalculateTraffic";
    case NB_NMP_TrafficCongestion:
        return "NB_NMP_TrafficCongestion";
    case NB_NMP_TrafficIncident:
        return "NB_NMP_TrafficIncident";
    case NB_NMP_TrafficAlert:
        return "NB_NMP_TrafficAlert";
    case NB_NMP_TrafficDelay:
        return "NB_NMP_TrafficDelay";
    case NB_NMP_Calc:
        return "NB_NMP_Calc";
    case NB_NMP_ConfirmDetour:
        return "NB_NMP_ConfirmDetour";
    default:
        return "Unknown NB_NavigateManeuverPos";
    }
}

void TestBatchToolProcessor::SetupEnhancedContentMapConfiguration(NB_EnhancedContentMapConfiguration* enhancedConfiguration)
{

    // Initialize configuration parameters to default values
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].cameraHeightMeters                = 150;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].cameraToAvatarHorizontalMeters    = 12;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].avatarToHorizonHorizontalMeters   = 750;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].avatarToMapBottomPixel            = 70;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].horizontalFieldOfViewDegrees      = 55;

    enhancedConfiguration->cameraProjection[NB_MO_Landscape].cameraHeightMeters               = 150;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].cameraToAvatarHorizontalMeters   = 12;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].avatarToHorizonHorizontalMeters  = 380;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].avatarToMapBottomPixel           = 70;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].horizontalFieldOfViewDegrees     = 90;

    enhancedConfiguration->waitBetweenCityModelDisplaySeconds = 15;
    enhancedConfiguration->maximumJunctionViewDistanceMeters  = 200;

    enhancedConfiguration->mapFieldWidthPixel     = 480;
    enhancedConfiguration->mapFieldHeightPixel    = 758;
}

void TestBatchToolProcessor::CreateEnhancedContentManager(NB_RouteConfiguration* routeConfiguration)
{
    if (routeConfiguration->wantCityModels || routeConfiguration->wantJunctionModels || routeConfiguration->wantRealisticSigns)
    {
        NB_Error result = NE_OK;

        NB_EnhancedContentManagerConfiguration config = { 0 };

        NB_EnhancedContentSynchronizationCallback synchronizationCallback = {0};
        NB_EnhancedContentDownloadCallback downloadCallback = {0};

        // TODO: enhanced content not working with default nav,global servlet
        NB_TargetMapping targets[] = {{ "nav", "nav,usa" }};
        result = NB_ContextSetTargetMappings(m_context, targets, sizeof(targets) / sizeof(targets[0]));

        // @todo: Support enhanced content suite
        SetupEnhancedContentManagerConfiguration(&config);
        config.enhancedCityModelEnabled = routeConfiguration->wantCityModels;
        config.motorwayJunctionObjectsEnabled = routeConfiguration->wantJunctionModels;
        config.realisticSignsEnabled = routeConfiguration->wantRealisticSigns;

        result = NB_EnhancedContentManagerCreate(m_context, &config, &synchronizationCallback, &downloadCallback, &m_ECManager);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_ContextSetEnhancedContentManagerNoOwnershipTransfer(m_context, m_ECManager);
        CU_ASSERT_EQUAL(result, NE_OK);
    }
}

void TestBatchToolProcessor::SetupEnhancedContentManagerConfiguration(NB_EnhancedContentManagerConfiguration* config)
{
    const char* basePath = NULL;
    char fullPath[TEST_MAX_PATH_LENGTH] = "";

    config->screenWidth = TEST_SCREEN_WIDTH;
    config->screenHeight = TEST_SCREEN_HEIGHT;
    config->screenResolution = TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT;
    config->maxRealisticSignsCacheSize     = 100;
    config->maxMotorwayJunctionsCacheSize  = 100;

    config->enhancedCityModelEnabled = TRUE;
    config->motorwayJunctionObjectsEnabled = TRUE;
    config->realisticSignsEnabled = TRUE;

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

    nsl_strcpy(config->enhancedCityModelFileFormat, TEST_ECM_FILE_FORMAT);
    nsl_strcpy(config->motorwayJunctionObjectFileFormat, TEST_MJO_FILE_FORMAT);
    nsl_strcpy(config->realisticSignsFileFormat, TEST_SAR_FILE_FORMAT);
    nsl_strcpy(config->textureFileFormat, TEST_TEXTURE_FILE_FORMAT);
    nsl_strcpy(config->compressedTextureFileFormat, TEST_COMPRESSED_TEXTURE_FILE_FORMAT);
}


// ==================================================
// Performance utility functions
// ==================================================

void TestBatchToolProcessor::PmPrintResultsReverseGeocode(NB_ReverseGeocodeInformation* information)
{
    NB_Location       location = {0};
    CU_pTest          test  = {0};
    PAL_ClockDateTime dateTime = {0};

    test = CU_get_current_test();
    PAL_ClockGetDateTime(&dateTime);

    NB_ReverseGeocodeInformationGetLocation(information, &location);

    PerformanceMeasurement_Print("\"%d.%d.%d %d:%d:%d\";\"%s\";\"%s\";\"%f\";\"%f\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%f\"\r\n",
                          dateTime.day, dateTime.month, dateTime.year,
                          dateTime.hour, dateTime.minute, dateTime.second,
                          test->pName,
                          location.latitude,
                          location.longitude,
                          location.streetnum,
                          location.street1,
                          location.city,
                          location.state,
                          location.postal,
                          PerformanceMeasurement_GetElapsedTime("test"));
}

void TestBatchToolProcessor::PmPrintResultsGeocode(uint32 total, NB_Error error)
{
    PerformanceMeasurement_Print("\"%d\";\"%d\";\"%f\"\r\n",
                                 total,
                                 error,
                                 PerformanceMeasurement_GetElapsedTime("test"));
}

void TestBatchToolProcessor::PmPrintInputParametersGeocode(const char* str)
{
    CU_pTest          test  = {0};
    PAL_ClockDateTime dateTime = {0};

    test = CU_get_current_test();
    PAL_ClockGetDateTime(&dateTime);

    PerformanceMeasurement_Print("\"%d.%d.%d %d:%d:%d\";\"%s\"%s\";\"%s\";\"%s\";",
                                 dateTime.day, dateTime.month, dateTime.year,
                                 dateTime.hour, dateTime.minute, dateTime.second,
                                 test->pName,
                                 str);
}

void TestBatchToolProcessor::PmPrintInputParametersSearchBuilder(const char* poiname, NB_LatitudeLongitude * latlon, uint32 radius, const char* categories)
{
    CU_pTest          test  = {0};
    PAL_ClockDateTime dateTime = {0};

    test = CU_get_current_test();
    PAL_ClockGetDateTime(&dateTime);

    PerformanceMeasurement_Print("\"%d.%d.%d %d:%d:%d\";\"%s\";\"%s\";\"%f\";\"%f\";\"%u\";\"%s\";\"%s\";",
                                 dateTime.day, dateTime.month, dateTime.year,
                                 dateTime.hour, dateTime.minute, dateTime.second,
                                 test->pName,
                                 latlon->latitude,
                                 latlon->longitude,
                                 radius,
                                 poiname,
                                 categories);
}

void TestBatchToolProcessor::PmPrintResultsSearchBuilder(uint32 total, NB_Error error)
{
    PerformanceMeasurement_Print("\"%d\";\"%d\";\"%f\"\r\n",
                                 total,
                                 error,
                                 PerformanceMeasurement_GetElapsedTime("test"));
}

void TestBatchToolProcessor::PmPrintInputParametersRoute(const char* str)
{
    CU_pTest          test  = {0};
    PAL_ClockDateTime dateTime = {0};

    test = CU_get_current_test();
    PAL_ClockGetDateTime(&dateTime);

    PerformanceMeasurement_Print("\"%d.%d.%d %d:%d:%d\";\"%s\"%s\";\"%s\";\"%s\";",
                                 dateTime.day, dateTime.month, dateTime.year,
                                 dateTime.hour, dateTime.minute, dateTime.second,
                                 test->pName,
                                 str);
}

void TestBatchToolProcessor::PmPrintResultsRoute( const char* err)
{
    PerformanceMeasurement_Print("\"%s\";\"%f\"\r\n",
                                 err,
                                 PerformanceMeasurement_GetElapsedTime("test"));
}

void TestBatchToolProcessor::PmPrintInputParametersNavigationSession(const char* str, uint32 curFix)
{
    CU_pTest          test  = {0};
    PAL_ClockDateTime dateTime = {0};

    test = CU_get_current_test();
    PAL_ClockGetDateTime(&dateTime);

    PerformanceMeasurement_Print("\"%d.%d.%d %d:%d:%d\";\"%s\"%s\";\"%s\";\"%s\"currentGpsFix:%d;",
                                 dateTime.day, dateTime.month, dateTime.year,
                                 dateTime.hour, dateTime.minute, dateTime.second,
                                 test->pName,
                                 str,
                                 curFix);
}

void TestBatchToolProcessor::PmPrintResultsNavigationSession( const char* callbackName, uint32 curFix, const char* err)
{
    if( NULL == err )
    {
        PerformanceMeasurement_Print("\"%s was called for %d current fix\";\"%f\"\r\n",
                                     callbackName,
                                     curFix,
                                     PerformanceMeasurement_GetElapsedTime("test"));
    }
    else
    {
        PerformanceMeasurement_Print("\"%s was called for %d current fix with Error:\";\"%f\"\r\n",
                                     callbackName,
                                     curFix,
                                     err,
                                     PerformanceMeasurement_GetElapsedTime("test"));
    }
}

const char* GetDocumentsDirectoryPath()
{
	return DOCUMENTS_DIRECTORY_PATH;
}

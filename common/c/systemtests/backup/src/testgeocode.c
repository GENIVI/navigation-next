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

    @file     TestGeocode.c
    @date     01/01/2009
    @defgroup TestGeoCode_h System tests for Geocode functions
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

#include "palclock.h"
#include "testgeocode.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"
#include "palmath.h"
#include "nbcontext.h"
#include "nbgeocodehandler.h"

// Local Functions ...........................................................

// All tests. Add all tests to the TestGeocode_AddTests function
static void TestGeocodeAddress(void);
static void TestGeocodeIntersection(void);
static void TestGeocodeAirport(void);
static void TestGeocodeFreeform(void);
static void TestGeocodeAmbiguous(void);
static void TestGeocodeWithGeographicPosition(void);

static void TestGeocodeStartTwice(void);
static void TestGeocodeCancelRequest(void);

static void RequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);

static NB_Error VerifyIterationResults(NB_Context* context, NB_GeocodeHandler* handler, NB_GeocodeInformation* information, NB_IterationCommand iteration, uint32 expectedStart, uint32 expectedEnd, int32 expectedTotal);
static void VerifyGeocodeResults(NB_Context* context, NB_GeocodeParameters* parameters);
static NB_Error GetResults(NB_GeocodeHandler* handler, NB_GeocodeParameters* parameters, NB_GeocodeInformation** information);


// Constants .................................................................

// How much should we download
const int NUMBER_PER_SLICE = 10;


// Variables .................................................................

// @todo Replace global variables with platform-specific implementation
// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;
static NB_GeocodeInformation* g_Information = 0;

void TestGeocode_AddTests(CU_pSuite pTestSuite, int level)
{
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestGeocodeAddress", TestGeocodeAddress);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestGeocodeIntersection", TestGeocodeIntersection);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestGeocodeAirport", TestGeocodeAirport);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestGeocodeFreeform", TestGeocodeFreeform);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestGeocodeAmbiguous", TestGeocodeAmbiguous);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestGeocodeWithGeographicPosition", TestGeocodeWithGeographicPosition);

    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestGeocodeStartTwice", TestGeocodeStartTwice);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestGeocodeCancelRequest", TestGeocodeCancelRequest);
};


/*! Add common initialization code here.

@return 0

@see TestGeocode_SuiteCleanup
*/
int TestGeocode_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
	return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestGeocode_SuiteSetup
*/
int TestGeocode_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
	return 0;
}

/*! Test address geocode requests
*/
void TestGeocodeAddress(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_GeocodeParameters* parameters = 0;
        NB_Address address = { { 0 } };

        // Multiple results
        nsl_strcpy(address.street, "jeffrey rd");
        nsl_strcpy(address.city, "irvine");
        nsl_strcpy(address.state, "ca");
        nsl_strcpy(address.country, "USA");

        err = NB_GeocodeParametersCreateAddress(context, &address, NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (!err)
        {
            VerifyGeocodeResults(context, parameters);
        }

        // Single result
        nsl_strcpy(address.street, "144 state st");
        nsl_strcpy(address.city, "montpelier");
        nsl_strcpy(address.postal, "05602");

        err = NB_GeocodeParametersCreateAddress(context, &address, NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyGeocodeResults(context, parameters);
        }

        // Single result
        nsl_strcpy(address.number, "10");
        nsl_strcpy(address.street, "main st");
        nsl_strcpy(address.city, "montpelier");
        nsl_strcpy(address.postal, "05602");

        err = NB_GeocodeParametersCreateAddress(context, &address, NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyGeocodeResults(context, parameters);
        }

        /// @todo Add tests using country (US and others)

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


/*! Test intersection geocode request
*/
void TestGeocodeIntersection(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_GeocodeParameters* parameters = 0;
        NB_Address address = { { 0 } };

        // Single result
        nsl_strcpy(address.street, "main st");
        nsl_strcpy(address.city, "santa ana");
        nsl_strcpy(address.county, "orange");
        nsl_strcpy(address.state, "ca");
        nsl_strcpy(address.country, "USA");

        err = NB_GeocodeParametersCreateIntersection(context, &address, "macarthur blvd", NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyGeocodeResults(context, parameters);
        }

        // Single result
        nsl_memset(&address, 0, sizeof(address));
        nsl_strcpy(address.street, "state st");
        nsl_strcpy(address.city, "montpelier");
        nsl_strcpy(address.postal, "05602");

        err = NB_GeocodeParametersCreateIntersection(context, &address, "taylor", NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyGeocodeResults(context, parameters);
        }

        /// @todo Add tests using country (US and others)

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


/*! Test airport geocode request
*/
void TestGeocodeAirport(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_GeocodeParameters* parameters = 0;

        err = NB_GeocodeParametersCreateAirport(context, "sna", NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyGeocodeResults(context, parameters);
        }

        err = NB_GeocodeParametersCreateAirport(context, "btv", NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyGeocodeResults(context, parameters);
        }

        err = NB_GeocodeParametersCreateAirport(context, "YVR", NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = err ? err : NB_GeocodeParametersSetCountry(parameters, "CAN");
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyGeocodeResults(context, parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


/*! Test freeform request.

@return None
*/
void TestGeocodeFreeform(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_GeocodeParameters* parameters = 0;

        // Multiple results
        err = NB_GeocodeParametersCreateFreeForm(context, "culver dr irvine ca", "USA", NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyGeocodeResults(context, parameters);
        }

        // Single result
        err = NB_GeocodeParametersCreateFreeForm(context, "32 terrace st 05602", "USA", NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyGeocodeResults(context, parameters);
        }

        /// @todo Add tests using country (US and others)

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


/*! Test ambiguous results
*/
void TestGeocodeAmbiguous(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_GeocodeParameters* parameters = 0;
        NB_Address address = { { 0 } };

        // Multiple results
        nsl_strcpy(address.city, "san");
        nsl_strcpy(address.state, "ca");
        nsl_strcpy(address.country, "USA");

        err = NB_GeocodeParametersCreateAddress(context, &address, NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyGeocodeResults(context, parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

/*! Test requests with geographic position
*/
void TestGeocodeWithGeographicPosition(void)
{
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    uint8 result = CreatePalAndContext(&pal, &context);
    if (result)
    {
        NB_Error error = NE_OK;
        NB_GeocodeParameters* parameters = NULL;
        NB_Address address = { 0 };
        NB_LatitudeLongitude point = { 34.052179, -118.24333 }; // Los Angeles

        // Test address geocode request with geographic position
        nsl_strcpy(address.street, "jeffrey rd");
        nsl_strcpy(address.city, "irvine");
        nsl_strcpy(address.state, "ca");
        nsl_strcpy(address.country, "USA");

        error = NB_GeocodeParametersCreateAddress(context, &address, NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(error, NE_OK);

        error = NB_GeocodeParametersSetGeographicPosition(parameters, &point, 50, PAL_ClockGetGPSTime());
        CU_ASSERT_EQUAL(error, NE_OK);

        if (!error)
        {
            VerifyGeocodeResults(context, parameters);
        }

        // Test intersection geocode request with geographic position
        nsl_strcpy(address.street, "main st");
        nsl_strcpy(address.county, "orange");
        nsl_strcpy(address.state, "ca");
        nsl_strcpy(address.country, "USA");

        error = NB_GeocodeParametersCreateIntersection(context, &address, "macarthur blvd", NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(error, NE_OK);

        error = NB_GeocodeParametersSetGeographicPosition(parameters, &point, 100, PAL_ClockGetGPSTime());
        CU_ASSERT_EQUAL(error, NE_OK);

        if (!error)
        {
            VerifyGeocodeResults(context, parameters);
        }

        // Test airport geocode request with geographic position
        error = NB_GeocodeParametersCreateAirport(context, "sna", NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(error, NE_OK);

        error = NB_GeocodeParametersSetGeographicPosition(parameters, &point, 100, PAL_ClockGetGPSTime());
        CU_ASSERT_EQUAL(error, NE_OK);

        if (!error)
        {
            VerifyGeocodeResults(context, parameters);
        }

        // Test free form geocode request with geographic position
        error = NB_GeocodeParametersCreateFreeForm(context, "culver dr irvine ca", "USA", NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(error, NE_OK);

        error = NB_GeocodeParametersSetGeographicPosition(parameters, &point, 100, PAL_ClockGetGPSTime());
        CU_ASSERT_EQUAL(error, NE_OK);

        if (!error)
        {
            VerifyGeocodeResults(context, parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

/*! Test starting a request multiple times
*/
void TestGeocodeStartTwice(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_GeocodeParameters* parameters = 0;

        err = NB_GeocodeParametersCreateFreeForm(context, "6a liberty 92656", 0, NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            NB_GeocodeHandler* handler = 0;

            err = NB_GeocodeHandlerCreate(context, 0, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (!err && handler)
            {
                err = NB_GeocodeHandlerStartRequest(handler, parameters);
                CU_ASSERT_EQUAL(err, NE_OK);
                if (!err)
                {
                    err = NB_GeocodeHandlerStartRequest(handler, parameters);
                    CU_ASSERT_EQUAL(err, NE_BUSY);
                }

                err = NB_GeocodeHandlerDestroy(handler);
                CU_ASSERT_EQUAL(err, NE_OK);
            }

            NB_GeocodeParametersDestroy(parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


/*! Test cancel request
*/
void TestGeocodeCancelRequest(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_GeocodeParameters* parameters = 0;

        err = NB_GeocodeParametersCreateFreeForm(context, "6a liberty 92656", 0, NUMBER_PER_SLICE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            NB_GeocodeHandler* handler = 0;

            err = NB_GeocodeHandlerCreate(context, 0, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (!err && handler)
            {
                // Should get an error if cancel before starting request
                err = NB_GeocodeHandlerCancelRequest(handler);
                CU_ASSERT_EQUAL(err, NE_UNEXPECTED);

                err = NB_GeocodeHandlerStartRequest(handler, parameters);
                CU_ASSERT_EQUAL(err, NE_OK);
                if (!err)
                {
                    err = NB_GeocodeHandlerCancelRequest(handler);
                    CU_ASSERT_EQUAL(err, NE_OK);

                    if (!err)
                    {
                        // Should get an error if cancel again
                        err = NB_GeocodeHandlerCancelRequest(handler);
                        CU_ASSERT_EQUAL(err, NE_UNEXPECTED);
                    }
                }

                err = NB_GeocodeHandlerDestroy(handler);
                CU_ASSERT_EQUAL(err, NE_OK);
            }

            NB_GeocodeParametersDestroy(parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


/*! Geocode request handler callback

Request handler callback.  Reports progress, gets information on success and
signals that the request has completed

@param handler Handler invoking the callback
@param status Request status
@param err Error state
@param up Non-zero if progress report is for server query; zero for server reply
@param percent Percent of data transferred up or down
@param userData User data provided to request handler
*/
void RequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    LOGOUTPUT(LogLevelHigh, ("Geocode %s progress: %d\n", up ? "upload" : "download", percent));

    if (!up)
    {
        if (err)
        {
            CU_FAIL("Callback returned error");
            g_Information = 0;
            SetCallbackCompletedEvent(g_CallbackEvent);
        }
        else if (percent == 100)
        {
            NB_Error err = NB_GeocodeHandlerGetGeocodeInformation(handler, &g_Information);
            CU_ASSERT_EQUAL(err, NE_OK);

            SetCallbackCompletedEvent(g_CallbackEvent);
        }
    }
}


/*! Start a geocode request, wait for it to complete and return the information object
*/
NB_Error GetResults(NB_GeocodeHandler* handler, NB_GeocodeParameters* parameters, NB_GeocodeInformation** information)
{
    NB_Error err = NB_GeocodeHandlerStartRequest(handler, parameters);
    if (!err)
    {
        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 45000))
        {
            *information = g_Information;
            g_Information = 0;
        }
        else
        {
            err = NE_UNEXPECTED;
        }
    }

    return err;
}


/*! Create an iteration parameters and check to see that the start, end and total values are those expected
*/
NB_Error VerifyIterationResults(
    NB_Context* context,
    NB_GeocodeHandler* handler,
    NB_GeocodeInformation* information,
    NB_IterationCommand iteration,
    uint32 expectedStart,
    uint32 expectedEnd,
    int32 expectedTotal)
{
    NB_Error err = NE_OK;
    NB_GeocodeParameters* iterationParameters = 0;

    err = NB_GeocodeParametersCreateIteration(context, information, iteration, &iterationParameters);
    CU_ASSERT_EQUAL(err, NE_OK);
    if (!err)
    {
        NB_GeocodeInformation* iterationInformation = 0;

        err = GetResults(handler, iterationParameters, &iterationInformation);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            uint32 iterationStart = 0;
            uint32 iterationEnd = 0;
            int32 iterationTotal = 0;

            err = NB_GeocodeInformationGetSliceInformation(iterationInformation, &iterationTotal, &iterationStart, &iterationEnd);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT(iterationTotal >= 1);
            CU_ASSERT_EQUAL(iterationStart, expectedStart);
            CU_ASSERT_EQUAL(iterationEnd, expectedEnd);
            CU_ASSERT_EQUAL(iterationTotal, expectedTotal);

            NB_GeocodeInformationDestroy(iterationInformation);
        }

        NB_GeocodeParametersDestroy(iterationParameters);
    }

    return err;
}


/*! Check consistency of Geocode results
*/
void VerifyGeocodeResults(NB_Context* context, NB_GeocodeParameters* parameters)
{
    NB_Error err = NE_OK;
    NB_RequestHandlerCallback callback = { RequestHandlerCallback, 0 };
    NB_GeocodeHandler* handler = 0;

    err = NB_GeocodeHandlerCreate(context, &callback, &handler);
    CU_ASSERT_EQUAL(err, NE_OK);
    if (!err && handler)
    {
        NB_GeocodeInformation* information = 0;
        int32 getInfoSuccessCount = 0;
        int32 resultTotal = 0;
        uint8 needResultTotal = TRUE;
        uint8 done = FALSE;

        // Loop through the geocode results until there is an error or
        // the result set has been exhausted
        while (!err && !done)
        {
            err = GetResults(handler, parameters, &information);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (!err)
            {
                uint32 start = 0;
                uint32 end = 0;
                int32 total = 0;

                err = NB_GeocodeInformationGetSliceInformation(information, &total, &start, &end);
                CU_ASSERT_EQUAL(err, NE_OK);
                CU_ASSERT(total >= 1);
                if (!err)
                {
                    uint32 i = 0;

                    // If this is the first time through, record the total for comparison with
                    // other slices.  If not, compare the first total value with the current total
                    if (needResultTotal)
                    {
                        resultTotal = total;
                        needResultTotal = FALSE;
                    }
                    else
                    {
                        CU_ASSERT_EQUAL(total, resultTotal);
                    }

                    // If this is not the first slice, check to see that a Previous Iteration
                    // returns a slice with the expected values
                    if (start != 0)
                    {
                        err = VerifyIterationResults(context, handler, information, NB_IterationCommand_Previous, start - NUMBER_PER_SLICE, start, total);
                    }

                    // Get each location in the slice and increment a counter when a get is successful
                    // At the end, compare the number counted with the total number reported
                    for (i = 0; !err && i < (end - start); i++)
                    {
                        NB_Location location;
                        uint32 accuracy = 0;

                        err = NB_GeocodeInformationGetLocationWithAccuracy(information, i, &location, &accuracy);
                        CU_ASSERT_EQUAL(err, NE_OK);
                        if (!err)
                        {
                            LOGOUTPUT(LogLevelHigh, ("\nArea: %s", location.areaname));
                            LOGOUTPUT(LogLevelHigh, ("\nAirport: %s", location.airport));
                            LOGOUTPUT(LogLevelHigh, ("\nNumber: %s", location.streetnum));
                            LOGOUTPUT(LogLevelHigh, ("\nStreet1: %s", location.street1));
                            LOGOUTPUT(LogLevelHigh, ("\nStreet2: %s", location.street2));
                            LOGOUTPUT(LogLevelHigh, ("\nCity: %s", location.city));
                            LOGOUTPUT(LogLevelHigh, ("\nPostal: %s", location.postal));
                            LOGOUTPUT(LogLevelHigh, ("\nCounty: %s", location.county));
                            LOGOUTPUT(LogLevelHigh, ("\nState: %s", location.state));
                            LOGOUTPUT(LogLevelHigh, ("\nCountry: %s", location.country));
                            LOGOUTPUT(LogLevelHigh, ("\nFreeform: %s", location.freeform));
                            LOGOUTPUT(LogLevelHigh, ("\nAccuracy: %u", accuracy));

                            getInfoSuccessCount++;
                        }
                    }

                    // We are done if the slice end is equal to the total
                    done = (uint8)(total == (int32)end);
                    if (!done)
                    {
                        NB_GeocodeParameters* next = 0;
                        err = NB_GeocodeParametersCreateIteration(context, information, NB_IterationCommand_Next, &next);
                        CU_ASSERT_EQUAL(err, NE_OK);
                        if (!err)
                        {
                            (void)NB_GeocodeParametersDestroy(parameters);
                            parameters = next;
                        }
                    }

                    // If this is not the first slice, check to see if the Start Iteration returns the first slice
                    if (done && total > NUMBER_PER_SLICE)
                    {
                        err = VerifyIterationResults(context, handler, information, NB_IterationCommand_Start, 0, NUMBER_PER_SLICE, total);
                    }
                }

                (void)NB_GeocodeInformationDestroy(information);
            }
        }

        (void)NB_GeocodeHandlerDestroy(handler);

        CU_ASSERT_EQUAL(resultTotal, getInfoSuccessCount);
    }

    (void)NB_GeocodeParametersDestroy(parameters);
}


/*! @} */

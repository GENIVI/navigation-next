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

    @file     testreversegeocode.c
    @defgroup testreversegeocode System tests for Geocode functions
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

#include "testreversegeocode.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"

#include "nbcontext.h"
#include "nbreversegeocodehandler.h"
#include "nbreversegeocodeinformation.h"
#include "nbreversegeocodeparameters.h"

static void TestReverseGeocodeRequest1(void);
static void TestReverseGeocodeRequest2(void);
static void TestReverseGeocodeRequest3(void);

static void TestReverseGeocodeStartTwice(void);
static void TestReverseGeocodeCancelRequest(void);

static NB_Error GetReverseGeocodeResults(NB_ReverseGeocodeHandler* handler, NB_ReverseGeocodeParameters* parameters, NB_Location* location);
static void RequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData);

static void VerifyReverseGeocodeResults(NB_Context* context, NB_ReverseGeocodeParameters* parameters);
static NB_Error GetReverseGeocodeResults(NB_ReverseGeocodeHandler* handler, NB_ReverseGeocodeParameters* parameters, NB_Location* location);


// @todo Replace global variables with platform-specific implementation
static void* g_CallbackEvent = NULL;
static NB_Location g_Location = { { 0 } };

void TestReverseGeocode_AddTests(CU_pSuite pTestSuite, int level)
{
	ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestReverseGeocodeRequest1", TestReverseGeocodeRequest1);
	ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestReverseGeocodeRequest2", TestReverseGeocodeRequest2);
	ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestReverseGeocodeRequest3", TestReverseGeocodeRequest3);

    /// @todo Add test to compare routable = TRUE and routable = FALSE
    /// @todo Add test to use different scale values

	ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestReverseGeocodeStartTwice", TestReverseGeocodeStartTwice);
	ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestReverseGeocodeCancelRequest", TestReverseGeocodeCancelRequest);
};


int TestReverseGeocode_SuiteSetup()
{
    g_CallbackEvent = CreateCallbackCompletedEvent();
	return 0;
}


int TestReverseGeocode_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
	return 0;
}


void TestReverseGeocodeRequest1(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_ReverseGeocodeParameters* parameters = 0;
        NB_LatitudeLongitude latLong = { 0 };

        // Las Vegas
        latLong.latitude = 36.10;
        latLong.longitude = -115.12;

        err = NB_ReverseGeocodeParametersCreate(context, &latLong, TRUE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyReverseGeocodeResults(context, parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

void TestReverseGeocodeRequest2(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_ReverseGeocodeParameters* parameters = 0;
        NB_LatitudeLongitude latLong = { 0 };

        // San Francisco
        latLong.latitude = 37.775239;
        latLong.longitude = -122.419170;

        err = NB_ReverseGeocodeParametersCreate(context, &latLong, TRUE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyReverseGeocodeResults(context, parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


void TestReverseGeocodeRequest3(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_ReverseGeocodeParameters* parameters = 0;
        NB_LatitudeLongitude latLong = { 0 };

        // Dallas
        latLong.latitude = 32.786810;
        latLong.longitude = -96.801120;

        err = NB_ReverseGeocodeParametersCreate(context, &latLong, TRUE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            VerifyReverseGeocodeResults(context, parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


/*! Test starting a request multiple times
*/
void TestReverseGeocodeStartTwice(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_ReverseGeocodeParameters* parameters = 0;
        NB_LatitudeLongitude latLong = { 0 };

        // Dallas
        latLong.latitude = 32.786810;
        latLong.longitude = -96.801120;

        err = NB_ReverseGeocodeParametersCreate(context, &latLong, FALSE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            NB_ReverseGeocodeHandler* handler = 0;

            err = NB_ReverseGeocodeHandlerCreate(context, 0, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (!err && handler)
            {
                err = NB_ReverseGeocodeHandlerStartRequest(handler, parameters);
                CU_ASSERT_EQUAL(err, NE_OK);
                if (!err)
                {
                    err = NB_ReverseGeocodeHandlerStartRequest(handler, parameters);
                    CU_ASSERT_EQUAL(err, NE_BUSY);
                }

                err = NB_ReverseGeocodeHandlerDestroy(handler);
                CU_ASSERT_EQUAL(err, NE_OK);
            }

            NB_ReverseGeocodeParametersDestroy(parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


/*! Test cancel request
*/
void TestReverseGeocodeCancelRequest(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_ReverseGeocodeParameters* parameters = 0;
        NB_LatitudeLongitude latLong = { 0 };

        // Dallas
        latLong.latitude = 32.786810;
        latLong.longitude = -96.801120;

        err = NB_ReverseGeocodeParametersCreate(context, &latLong, FALSE, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            NB_ReverseGeocodeHandler* handler = 0;

            err = NB_ReverseGeocodeHandlerCreate(context, 0, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (!err && handler)
            {
                // Should get an error if cancel before starting request
                err = NB_ReverseGeocodeHandlerCancelRequest(handler);
                CU_ASSERT_EQUAL(err, NE_UNEXPECTED);

                err = NB_ReverseGeocodeHandlerStartRequest(handler, parameters);
                CU_ASSERT_EQUAL(err, NE_OK);
                if (!err)
                {
                    err = NB_ReverseGeocodeHandlerCancelRequest(handler);
                    CU_ASSERT_EQUAL(err, NE_OK);

                    if (!err)
                    {
                        // Should get an error if cancel again
                        err = NB_ReverseGeocodeHandlerCancelRequest(handler);
                        CU_ASSERT_EQUAL(err, NE_UNEXPECTED);
                    }
                }

                err = NB_ReverseGeocodeHandlerDestroy(handler);
                CU_ASSERT_EQUAL(err, NE_OK);
            }

            NB_ReverseGeocodeParametersDestroy(parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


void RequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    LOGOUTPUT(LogLevelHigh, ("Reverse Geocode %s progress: %d\n", up ? "upload" : "download", percent));

    if (!up)
    {
        if (err) 
        {
            CU_FAIL("Callback returned error");
            SetCallbackCompletedEvent(g_CallbackEvent);
        }
        else if (percent == 100) 
        {
            NB_ReverseGeocodeInformation* information = 0;
            NB_Error err = NE_OK;
            
            err = NB_ReverseGeocodeHandlerGetReverseGeocodeInformation(handler, &information);
            CU_ASSERT_EQUAL(err, NE_OK);

            if (!err)
            {
                err = NB_ReverseGeocodeInformationGetLocation(information, &g_Location);
                CU_ASSERT_EQUAL(err, NE_OK);

                (void)NB_ReverseGeocodeInformationDestroy(information);
            }

            SetCallbackCompletedEvent(g_CallbackEvent);
        }
    }
}


/*! Start reverse geocode request, wait for response and return location
*/
NB_Error GetReverseGeocodeResults(NB_ReverseGeocodeHandler* handler, NB_ReverseGeocodeParameters* parameters, NB_Location* location)
{
    NB_Error err = NB_ReverseGeocodeHandlerStartRequest(handler, parameters);
    if (!err)
    {
        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 45000))
        {
            nsl_memcpy(location, &g_Location, sizeof(*location));
        }
        else
        {
            err = NE_UNEXPECTED;
        }
    }

    return err;
}


/*! Check consistency of Reverse Geocode results
*/
void VerifyReverseGeocodeResults(NB_Context* context, NB_ReverseGeocodeParameters* parameters)
{
    NB_Error err = NE_OK;
    NB_RequestHandlerCallback callback = { RequestHandlerCallback, 0 };
    NB_ReverseGeocodeHandler* handler = 0;

    err = NB_ReverseGeocodeHandlerCreate(context, &callback, &handler);
    CU_ASSERT_EQUAL(err, NE_OK);
    if (!err && handler)
    {
        NB_Location location = { { 0 } };

        err = GetReverseGeocodeResults(handler, parameters, &location);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            char lat[20] = { 0 };
            char lon[20] = { 0 };

            nsl_doubletostr(location.latitude, lat, sizeof(lat));
            nsl_doubletostr(location.longitude, lon, sizeof(lon));

            /// @todo Verify that result is what was expected
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
            LOGOUTPUT(LogLevelHigh, ("\nLat, Long: (%s, %s)\n", lat, lon));
        }

        (void)NB_ReverseGeocodeHandlerDestroy(handler);
    }

    (void)NB_ReverseGeocodeParametersDestroy(parameters);
}


/*! @} */



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

    @file testsinglesearchservices.cpp
    @defgroup testsinglesearchservices_h system tests for single search services functions

    See description in header file.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */


#include "CUnit.h"

extern "C"
{
    #include "testnetwork.h"
    #include "main.h"
	#include "testsinglesearchservices.h"
    #include "nbrouteparameters.h"
    #include "nbroutehandler.h"
    #include "testdirection.h"
}
#include "CommonTypes.h"
#include "SingleSearch.h"

// Local Constants ...............................................................................

const int TEST_NUMBER_PER_SLICE = 5;

const char* TEST_SEARCH_SCHEME = "tcs-single-search";
const char* TEST_SEARCH_NAME = "kfc";

static void* g_CallbackEvent = NULL;

// Test Functions ................................................................................

static void TestSingleSearchSuggestionServices(void);
static void TestSingleSearchPlaceWithCategoriesServices(void);
static void TestSingleSearchFuelServices(void);
static void TestSingleSearchPlaceForNavigationServices(void);

// Helper Functions ..............................................................................

static void RouteHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
static NB_RouteInformation* CreateRouteToSearchAlong(PAL_Instance* pal, NB_Context* context, NB_LatitudeLongitude* routeOrigin, NB_LatitudeLongitude* routeDestination);

// Public Functions ..............................................................................

void
TestSingleSearchServices_AddTests(CU_pSuite pTestSuite, int level)
{
    // Add all your function names here
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSingleSearchSuggestionSearchServices", TestSingleSearchSuggestionServices);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSingleSearchPlaceWithCategoriesServices", TestSingleSearchPlaceWithCategoriesServices);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSingleSearchFuelServices", TestSingleSearchFuelServices);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSingleSearchPlaceForNavigationServices", TestSingleSearchPlaceForNavigationServices);

}


int
TestSingleSearchServices_SuiteSetup()
{
    return 0;
}

int
TestSingleSearchServices_SuiteCleanup()
{
    return 0;
}

using namespace nbcommon;
using namespace nbmap;

/*! Collection of all instances used for the unit tests */
typedef struct
{
    PAL_Instance*               pal;                /*!< Pointer to PAL */
    NB_Context*                 context;            /*!< Pointer to current context */

    NB_SingleSearchHandler*     handler;            /*!< Single search handler instance */
    NB_SingleSearchInformation* information;        /*!< Single search information instance */

    void*                       event;              /*!< Event gets triggered from the download callback */

} Instances;


class SingleSearchSuggestionCallback: public AsyncCallback<SingleSearchResultList<SuggestionMatch>* >
{
    public:
        SingleSearchSuggestionCallback(void* callbackEvent):event(callbackEvent){}
        ~SingleSearchSuggestionCallback(){}

        void Success(SingleSearchResultList<SuggestionMatch>* res)
        {
            SuggestionMatch* suggestion = NULL;
            int count = 0;

            while((suggestion = res->getNext()) != NULL)
            {
                const char* firstLine = suggestion->GetFirstLine();
                const char* secondLine = suggestion->GetSecondLine();

                CU_ASSERT_PTR_NOT_NULL(firstLine);
                CU_ASSERT_PTR_NOT_NULL(secondLine);
                count++;
            }
            if (count == 0)
            {
                CU_FAIL("No results");
            }

            SetCallbackCompletedEvent(event);
        }


        void Error(NB_Error err)
        {
            CU_FAIL("Callback recieved error");
            SetCallbackCompletedEvent(event);
        }
        void* GetEvent() { return event; }
    private:
        void* event;
};
class SingleSearchPlaceCallback: public AsyncCallback<SingleSearchResultList<Place>* >
{
    public:
        SingleSearchPlaceCallback(void* callbackEvent):event(callbackEvent){}
        ~SingleSearchPlaceCallback(){}

        void Success(SingleSearchResultList<Place>* res)
        {
            Place* place = NULL;
            int count = 0;

            while((place = res->getNext()) != NULL)
            {
                CU_ASSERT_PTR_NOT_NULL(place->GetCategory());
                CU_ASSERT_PTR_NOT_NULL(place->GetName());
                CU_ASSERT_PTR_NOT_NULL(place->GetLocation());
                count++;
            }
            if (count == 0)
            {
                CU_FAIL("No results");
            }
            SetCallbackCompletedEvent(event);
        }

        void Error(NB_Error err)
        {
            CU_FAIL("Callback recieved error");
            SetCallbackCompletedEvent(event);
        }
        void* GetEvent() { return event; }
    private:
        void* event;
};
class SingleSearchFuelCallback: public AsyncCallback<SingleSearchResultList<FuelCommon>* >
{
    public:
        SingleSearchFuelCallback(void* callbackEvent):event(callbackEvent){}
        ~SingleSearchFuelCallback(){}

        void Success(SingleSearchResultList<FuelCommon>* res)
        {
            FuelCommon* fuel = NULL;
            int count = 0;

            while((fuel = res->getNext()) != NULL)
            {
                if (fuel->GetFuelDetails() == NULL && fuel->GetFuelSummary() == NULL)
                {
                    CU_FAIL("No results");
                    return;
                }
                else if (fuel->GetFuelDetails() != NULL)
                {
                    CU_ASSERT(fuel->GetFuelDetails()->GetNumFuelProducts() > 0);
                }
                else if (fuel->GetFuelSummary() != NULL)
                {
                    CU_ASSERT_PTR_NOT_NULL(fuel->GetFuelSummary()->GetAverage());
                    CU_ASSERT_PTR_NOT_NULL(fuel->GetFuelSummary()->GetLow());
                }
                count++;
            }
            if (count == 0)
            {
                CU_FAIL("No results");
            }
            SetCallbackCompletedEvent(event);
        }

        void Error(NB_Error err)
        {
            CU_FAIL("Callback recieved error");
            SetCallbackCompletedEvent(event);
        }
        void* GetEvent() { return event; }
    private:
        void* event;
};

// Test Functions ................................................................................

void TestSingleSearchSuggestionServices()
{
    /*PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    void*       event = NULL;

    if (!CreatePalAndContext(&pal, &context))
    {
        CU_FAIL("Context cannot be created");
        return;
    }

    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    event = CreateCallbackCompletedEvent();

    CU_ASSERT_PTR_NOT_NULL_FATAL(event);

    SingleSearch singleSearch(context,
                            "",
                            (char*)TEST_SEARCH_SCHEME,
                            static_cast<NB_POIExtendedConfiguration>(NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantNonProximityPois),
                            NB_SSB_Relevance,
                            NB_SIM_Undefined);
    LatitudeLongitude latLon;
    latLon.SetLatitude(32.80);  //Dallas center
    latLon.SetLongitude(-96.80);
    PageIterator<SuggestionMatch>* suggestions = singleSearch.suggestions((char*)TEST_SEARCH_NAME,
                                                                         TEST_NUMBER_PER_SLICE,
                                                                         &latLon,
                                                                         SS_Carousel);

    SingleSearchSuggestionCallback* callback = new SingleSearchSuggestionCallback(event);

    suggestions->getNext(callback);
    if (!WaitForCallbackCompletedEvent(callback->GetEvent(), 60000))
    {
        goto fail;
    }

    if (callback != NULL)
    {
        delete callback;
        callback = NULL;
    }
    if (suggestions != NULL)
    {
        delete suggestions;
        suggestions = NULL;
    }

    DestroyContext(context);
    PAL_Destroy(pal);
    return;
fail:

    if (callback != NULL)
    {
        delete callback;
        callback = NULL;
    }

    if (suggestions != NULL)
    {
        delete suggestions;
        suggestions = NULL;
    }

    DestroyContext(context);
    PAL_Destroy(pal);
    CU_FAIL("Timeout.");*/
}


void TestSingleSearchPlaceWithCategoriesServices()
{
    /*PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    void*       event = NULL;

    if (!CreatePalAndContext(&pal, &context))
    {
        CU_FAIL("Context cannot be created");
        return;
    }

    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    event = CreateCallbackCompletedEvent();

    CU_ASSERT_PTR_NOT_NULL_FATAL(event);

    SingleSearch singleSearch(context,
                        "",
                        (char*)TEST_SEARCH_SCHEME,
                        NB_EXT_None,
                        NB_SSB_Relevance,
                        NB_SIM_Undefined);

    LatitudeLongitude latLon;
    latLon.SetLatitude(32.80);  //Dallas center
    latLon.SetLongitude(-96.80);
    PageIterator<Place>* place = singleSearch.place(
                        (char*)TEST_SEARCH_NAME,
                        TEST_NUMBER_PER_SLICE,
                        &latLon,
                        SS_Carousel,
                        "AE"); //Restaurants & Bars

    SingleSearchPlaceCallback* callback = new SingleSearchPlaceCallback(event);

    place->getNext(callback);
    if (!WaitForCallbackCompletedEvent(callback->GetEvent(), 10000))
    {
        goto fail;
    }

    place->getNext(callback);
    if (!WaitForCallbackCompletedEvent(callback->GetEvent(), 10000))
    {
        goto fail;
    }

    if (callback != NULL)
    {
        delete callback;
        callback = NULL;
    }

    if (place != NULL)
    {
        delete place;
        place = NULL;
    }

    DestroyContext(context);
    PAL_Destroy(pal);
    return;

fail:

    if (callback != NULL)
    {
        delete callback;
        callback = NULL;
    }

    if (place != NULL)
    {
        delete place;
        place = NULL;
    }

    DestroyContext(context);
    PAL_Destroy(pal);
    CU_FAIL("Timeout.");*/
}

void TestSingleSearchFuelServices()
{
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    void*       event = NULL;

    if (!CreatePalAndContext(&pal, &context))
    {
        CU_FAIL("Context cannot be created");
        return;
    }

    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    event = CreateCallbackCompletedEvent();

    CU_ASSERT_PTR_NOT_NULL_FATAL(event);

    SingleSearch singleSearch(context,
                              "",
                              (char*)TEST_SEARCH_SCHEME,
                              static_cast<NB_POIExtendedConfiguration>(NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantNonProximityPois),
                              NB_SSB_Relevance,
                              NB_SIM_Undefined);

    LatitudeLongitude latLon;
    latLon.SetLatitude(32.80);  //Dallas center
    latLon.SetLongitude(-96.80);
    PageIterator<FuelCommon>* fuel = singleSearch.fuel("",
                            TEST_NUMBER_PER_SLICE,
                            &latLon);

    SingleSearchFuelCallback* callback = new SingleSearchFuelCallback(event);

    fuel->getNext(callback);
    if (!WaitForCallbackCompletedEvent(callback->GetEvent(), 10000))
    {
        goto fail;
    }

    fuel->getNext(callback);
    if (!WaitForCallbackCompletedEvent(callback->GetEvent(), 10000))
    {
        goto fail;
    }

    if (callback != NULL)
    {
        delete callback;
        callback = NULL;
    }

    if (fuel != NULL)
    {
        delete fuel;
        fuel = NULL;
    }

    DestroyContext(context);
    PAL_Destroy(pal);
    return;

fail:

    if (callback != NULL)
    {
        delete callback;
        callback = NULL;
    }

    if (fuel != NULL)
    {
        delete fuel;
        fuel = NULL;
    }

    DestroyContext(context);
    PAL_Destroy(pal);
    CU_FAIL("Timeout.");
}

void TestSingleSearchPlaceForNavigationServices()
{
    NB_Error err = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    void*       event = NULL;

    if (!CreatePalAndContext(&pal, &context))
    {
        CU_FAIL("Context cannot be created");
        return;
    }

    NB_RouteInformation* routeInformation = NULL;

    NB_LatitudeLongitude origin = { 33.607386, -117.695228 };       // 5 & Alicia
    NB_LatitudeLongitude destination = { 33.734283, -117.814528 };  // 5 & Red Hill
    g_CallbackEvent = CreateCallbackCompletedEvent();
    CU_ASSERT_EQUAL(err, NE_OK);

    // Create route to search along
    routeInformation = CreateRouteToSearchAlong(pal, context, &origin, &destination);

    if (routeInformation == NULL)
    {
        DestroyContext(context);
        PAL_Destroy(pal);
        DestroyCallbackCompletedEvent(g_CallbackEvent);
        CU_FAIL("Route Information is NULL");
        return;
    }

    SingleSearch singleSearch(context,
                              Test_OptionsGet()->voiceStyle,
                              (char*)TEST_SEARCH_SCHEME,
                              static_cast<NB_POIExtendedConfiguration>(NB_EXT_WantPremiumPlacement | NB_EXT_WantEnhancedPOI | NB_EXT_WantNonProximityPois),
                              NB_SSB_Relevance,
                              NB_SIM_Undefined);

    LatitudeLongitude latLon;
    latLon.SetLatitude(33.607386); //Laguna Hills
    latLon.SetLongitude(-117.695228);
    PageIterator<Place>* place = singleSearch.place(
                     (char*)TEST_SEARCH_NAME,
                     TEST_NUMBER_PER_SLICE,
                     &latLon, routeInformation);

    event = CreateCallbackCompletedEvent();

    SingleSearchPlaceCallback* callback = new SingleSearchPlaceCallback(event);

    place->getNext(callback);
    if (!WaitForCallbackCompletedEvent(callback->GetEvent(), 60000))
    {
        goto fail;
    }

    if (callback != NULL)
    {
        delete callback;
        callback = NULL;
    }

    if (place != NULL)
    {
        delete place;
        place = NULL;
    }

    DestroyCallbackCompletedEvent(g_CallbackEvent);

    if (routeInformation != NULL)
    {
        NB_RouteInformationDestroy(routeInformation);
    }
    DestroyContext(context);
    PAL_Destroy(pal);
    return;

fail:

    if (callback != NULL)
    {
        delete callback;
        callback = NULL;
    }

    if (place != NULL)
    {
        delete place;
        place = NULL;
    }

    DestroyContext(context);
    PAL_Destroy(pal);
    DestroyCallbackCompletedEvent(g_CallbackEvent);

    if (routeInformation != NULL)
    {
        NB_RouteInformationDestroy(routeInformation);
    }
    CU_FAIL("Timeout");
}

void
RequestHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    Instances* instances = static_cast<Instances*>(userData);

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

        localError = NB_SingleSearchHandlerGetInformation(static_cast<NB_SingleSearchHandler*>(handler), &(instances->information));

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

void RouteHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    if (!up)
    {
        NB_RouteInformation** route = static_cast<NB_RouteInformation**>(userData);
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
            err = NB_RouteHandlerGetRouteInformation((NB_RouteHandler*)handler, route);
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

NB_RouteInformation*
CreateRouteToSearchAlong(PAL_Instance* pal, NB_Context* context, NB_LatitudeLongitude* routeOrigin, NB_LatitudeLongitude* routeDestination)
{
    NB_Error result = NE_OK;

    NB_RouteParameters* routeParameters = 0;
    NB_RouteInformation* routeInformation = 0;
    NB_RouteHandler* routeHandler = 0;

    NB_Place origin = {{0}};
    NB_Place dest = {{0}};
    NB_RouteOptions options = { NB_RouteType_Fastest, NB_RouteAvoid_None, NB_TransportationMode_Car, NB_TrafficType_None, NULL};
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
        guidanceInfo = CreateGuidanceInformation(pal, context);
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

    result = NB_RouteParametersCreateFromPlace(context, &origin, &dest, &options, &config, &routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(routeParameters);

    result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeHandler);
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

/*! @{ */

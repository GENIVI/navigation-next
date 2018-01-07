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

@file     testanalytics.c
@date     02/03/2009
@defgroup testanalytics_h System tests for Analytics functions
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

#include "testanalytics.h"
#include "testnetwork.h"
#include "main.h"
#include "stdlib.h"
#include "platformutil.h"
#include "palclock.h"
#include "palfile.h"
#include "palstdlib.h"

#include "nbcontext.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "nbanalytics.h"
#include "nbanalyticstestprotected.h"
#include "nbsearchinformation.h"

#include "abservermessagehandler.h"
#include "abservermessageinformation.h"
#include "abservermessageparameters.h"

/************************************************************************/
/*                  Private Function declaration                        */
/************************************************************************/

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;
static void* g_TimeoutCallbackEvent = NULL;
static void* g_ConfigReceivedEvent = NULL;
static void* g_ReplyReceivedEvent = NULL;
static void* g_versionCallbackCompletedEvent = NULL;

// System test functions
static void TestAnalyticsCreateObject(void);
static void TestAnalyticsGetConfig(void);
static void TestAnalyticsGpsProbes(void);
static void TestAnalyticsSearchQueryEvent(void);
static void TestAnalyticsSearchDetailEvent(void);
static void TestAnalyticsMapEvent(void);
static void TestAnalyticsPlaceMessageEvent(void);
static void TestAnalyticsCallEvent(void);
static void TestAnalyticsArrivalEvent(void);
static void TestAnalyticsWeburlEvent(void);
static void TestAnalyticsDescriptionEvent(void);
static void TestAnalyticsReviewEvent(void);
static void TestAnalyticsAddFavoritesEvent(void);
static void TestAnalyticsRouteRequestEvent(void);
static void TestAnalyticsRouteReplyEvent(void);
static void TestAnalyticsRouteStateEvent(void);
static void TestAnalyticsFacebookUpdateEvent(void);
static void TestAnalyticsEventsPriority(void);
static void TestAnalyticsEventsSaveAndRestore(void);
static void TestAnalyticsProcess(void);
static void TestAnalyticsForceUpload(void);
static void TestAnalyticsMasterClear(void);
static void TestAnalyticsImpressionEvent(void);
static void TestAnalyticsEnableGpsProbes(void);
static void TestAnalyticsCouponActionEvents(void);
static void TestAnalyticsAddCouponWithoutContext(void);
static void TestAnalyticsAddCouponWithoutCoupon(void);
static void TestAnalyticsAddCouponWithoutStore(void);
static void TestAnalyticsAddCouponWithoutAction(void);
static void TestAnalyticsNavInvocationAppPresent(void);
static void TestAnalyticsNavInvocationAppPresentWithoutContext(void);
static void TestAnalyticsNavInvocationAppPresentWithoutCarrier(void);
static void TestAnalyticsNavInvocationAppPresentWithoutSdkVersion(void);
static void TestAnalyticsNavInvocationAppPresentWithoutPlatform(void);
static void TestAnalyticsNavInvocationUseNavigator(void);
static void TestAnalyticsNavInvocationUseNavigatorWithoutContext(void);
static void TestAnalyticsNavInvocationUseNavigatorWithoutPlace(void);
static void TestAnalyticsNavInvocationUseNavigatorWithoutPlatform(void);
static void TestAnalyticsNavInvocationUseNavigatorWithoutCarrier(void);
static void TestAnalyticsNavInvocationUseNavigatorWithoutSdkVersion(void);
static void TestAnalyticsAddFeedbackEvent(void);
static void TestAnalyticsAddAppErrors(void);

// local functions
static NB_DEF NB_Error CreateCoupon(NB_Coupon** pCoupon);
static NB_DEF NB_Error CouponDestroy(NB_Coupon* pCoupon);
static NB_DEF NB_Error CreateStore(NB_Store** pStore);
static NB_DEF NB_Error StoreDestroy(NB_Store* pStore);

static void ServerMessageStatusCallback(void* handler, NB_NetworkRequestStatus status,
        NB_Error err, uint8 up, int percent, void* pUserData);

static void AnalyticsActivityCallback(NB_Analytics* analytics, void* userData,
        NB_AnalyticsActivity activity);

/*! Add all your test functions here

@return None
*/

void
TestAnalytics_AddTests( CU_pSuite pTestSuite, int level )
{
#define TEST_ANALYTICS_ADD_TEST(test) \
    ADD_TEST(level, TestLevelSmoke, pTestSuite, #test, test)

    // ! Add all your function names here !
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsCreateObject);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsGetConfig);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsAddAppErrors);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsAddFeedbackEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsGpsProbes);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsSearchQueryEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsSearchDetailEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsMapEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsPlaceMessageEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsCallEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsArrivalEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsWeburlEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsDescriptionEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsReviewEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsAddFavoritesEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsRouteRequestEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsRouteReplyEvent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsRouteStateEvent);
    if (Test_OptionsGet()->carrier & TestCarrierVerizon)
    {
        TEST_ANALYTICS_ADD_TEST(TestAnalyticsFacebookUpdateEvent);
        TEST_ANALYTICS_ADD_TEST(TestAnalyticsImpressionEvent);
    }
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsEventsPriority);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsEventsSaveAndRestore);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsProcess);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsForceUpload);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsMasterClear);

    TEST_ANALYTICS_ADD_TEST(TestAnalyticsEnableGpsProbes);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsCouponActionEvents);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsAddCouponWithoutContext);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsAddCouponWithoutCoupon);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsAddCouponWithoutStore);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsAddCouponWithoutAction);

    TEST_ANALYTICS_ADD_TEST(TestAnalyticsNavInvocationAppPresent);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsNavInvocationAppPresentWithoutContext);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsNavInvocationAppPresentWithoutCarrier);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsNavInvocationAppPresentWithoutSdkVersion);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsNavInvocationAppPresentWithoutPlatform);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsNavInvocationUseNavigator);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsNavInvocationUseNavigatorWithoutContext);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsNavInvocationUseNavigatorWithoutPlace);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsNavInvocationUseNavigatorWithoutPlatform);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsNavInvocationUseNavigatorWithoutCarrier);
    TEST_ANALYTICS_ADD_TEST(TestAnalyticsNavInvocationUseNavigatorWithoutSdkVersion);

#undef TEST_ANALYTICS_ADD_TEST
}


/*! Add common initialization code here.

@return 0

@see TestNavigation_SuiteCleanup
*/
int
TestAnalytics_SuiteSetup()
{
    /*! Create event for callback synchronization */
    g_CallbackEvent = CreateCallbackCompletedEvent();
    g_ConfigReceivedEvent = CreateCallbackCompletedEvent();
    g_ReplyReceivedEvent = CreateCallbackCompletedEvent();
    g_TimeoutCallbackEvent = CreateCallbackCompletedEvent();

    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestNavigation_SuiteSetup
*/
int
TestAnalytics_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
    DestroyCallbackCompletedEvent(g_ConfigReceivedEvent);
    DestroyCallbackCompletedEvent(g_ReplyReceivedEvent);
    DestroyCallbackCompletedEvent(g_TimeoutCallbackEvent);

    return 0;
}

NB_DEF NB_Error
CreateCoupon(NB_Coupon** pCoupon)
{
    NB_Coupon* coupon = 0;

    if(!pCoupon)
    {
        return NE_INVAL;
    }

    coupon = nsl_malloc(sizeof(*coupon));
    if (!coupon)
    {
        return NE_NOMEM;
    }
    nsl_memset(coupon, 0, sizeof(*coupon));

    coupon->availability = ( char** )nsl_malloc( 4 * sizeof( char* ) );
    if ( coupon->availability )
    {
        coupon->availability[coupon->availabilityCount++] = nsl_strdup( "local" );
        coupon->availability[coupon->availabilityCount++] = nsl_strdup( "national" );
        coupon->availability[coupon->availabilityCount++] = nsl_strdup( "ecommerce" );
    }
    nsl_strlcpy(coupon->title, "$39 For A 60-Minute Massage Of Your Choice Plus Hair Shampoo And Blow Dry/Style At  Ma\\xc2\\x80S Body Boutique ($124 Value)", sizeof(coupon->title));
    nsl_strlcpy(coupon->id, "116727:f68433d06d2683a8df9431f526920d84", sizeof(coupon->id));
    coupon->description = (char*)nsl_malloc(sizeof("coupon_description_text"));
    if (!coupon->description)
    {
        CouponDestroy(coupon);
        return NE_NOMEM;
    }
    nsl_strlcpy(coupon->description, "coupon_description_text", sizeof("coupon_description_text"));
    coupon->startDate = 993054400;
    coupon->expirationDate = 993254400;
    coupon->discountType = NB_CDTT_Percentage;
    coupon->discountValue = 68.5;
    coupon->buyValue = 400.5;
    coupon->listValue = 1500.5;

    *pCoupon = coupon;

    return NE_OK;
}

NB_DEF NB_Error
CouponDestroy(NB_Coupon* pCoupon)
{
    if(!pCoupon)
    {
        return NE_INVAL;
    }

    if (pCoupon->availability)
    {
        uint32 i = 0;
        for (i = 0; i < pCoupon->availabilityCount; i++)
        {
            if (pCoupon->availability[i])
            {
                nsl_free(pCoupon->availability[i]);
            }
        }
        nsl_free(pCoupon->availability);
    }

    nsl_free(pCoupon->description);

    nsl_free(pCoupon);

    return NE_OK;
}

NB_DEF NB_Error
CreateStore(NB_Store** pStore)
{
    NB_Store *store = 0;

    if(!pStore)
    {
        return NE_INVAL;
    }

    store = nsl_malloc(sizeof(*store));
    if (!store)
    {
        return NE_NOMEM;
    }
    nsl_memset(store, 0, sizeof(*store));

    nsl_strlcpy(store->place.name, "Fremont CA", sizeof(store->place.name));
    store->place.location.type = NB_Location_LatLon;
    store->place.location.latitude  = 37.555629;
    store->place.location.longitude =   -122.051402;
    nsl_strlcpy(store->name, "test_name", sizeof(store->name));
    nsl_strlcpy(store->id, "test_id", sizeof(store->id));

    *pStore = store;

    return NE_OK;
}

NB_DEF NB_Error
StoreDestroy(NB_Store* pStore)
{
    if(!pStore)
    {
        return NE_INVAL;
    }

    nsl_free(pStore);

    return NE_OK;
}

/*! This system test is used to test the network object creation.

Before creating a navigation object we need NB_Network, NB_State and
Pal_Instance objects.  This method tests the creation of these objects.

@return None. CUnit Asserts get called on failures.

@see NavigationRequest
*/
void
TestAnalyticsCreateObject(void)
{
    PAL_Instance*   pal = 0;
    NB_Context*     context = 0;
    NB_Error        result = NE_OK;
    NB_Analytics*   analytics = NULL;

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context))
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        result = NB_AnalyticsCreate(context, NULL, &analytics);
        (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);

        CU_ASSERT_EQUAL(result, NE_OK);
        /* Don't actually do anything */
    }

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);
}

void
TestAnalyticsCreatePiggybackOpportunity(NB_Context* context)
{
    NB_Error result = NE_OK;
    AB_ServerMessageHandler* handler = 0;
    AB_ServerMessageParameters* parameters = 0;
    NB_RequestHandlerCallback callback = { ServerMessageStatusCallback, 0 };

    /* create server message status query to trigger analytic piggy back message */
    CU_ASSERT_EQUAL(AB_ServerMessageHandlerCreate(context, &callback, &handler), NE_OK);
    CU_ASSERT_PTR_NOT_NULL(handler);
    CU_ASSERT_EQUAL(AB_ServerMessageParametersCreateStatus(context, "en", 0, &parameters), NE_OK);


    if (parameters)
    {
        g_versionCallbackCompletedEvent = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL(g_versionCallbackCompletedEvent);

        if (handler)
        {
            result = AB_ServerMessageHandlerStartRequest(handler,
                    parameters);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        CU_ASSERT(WaitForCallbackCompletedEvent(g_versionCallbackCompletedEvent,
                    35000));

        DestroyCallbackCompletedEvent(g_versionCallbackCompletedEvent);
        g_versionCallbackCompletedEvent= NULL;
    }

    if (parameters)
    {
        result = AB_ServerMessageParametersDestroy(parameters);
    }

    if (handler)
    {
        AB_ServerMessageHandlerDestroy(handler);
    }
}

void
TestAnalyticsGetConfig(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);
    
    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsGpsProbes(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint32 navSessionId = PAL_ClockGetGPSTime();

    NB_AnalyticsCallbacks     analyticsCallback;

    NB_GpsLocation            gpsFix = {0};
    NB_Analytics* analytics = NULL;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 2;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* for gps-offset */
    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 0.01;
    gpsFix.longitude  += 0.02;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsSearchQueryEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    const char* categoryCodes[] = {"category1", "category2"};
    uint32 categoryCodeCount = sizeof(categoryCodes) / sizeof(const char *);
    uint32 eventId = 0;

    NB_AnalyticsCallbacks     analyticsCallback;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddSearchQueryEvent(context, "search name",
            "search scheme", categoryCodes, categoryCodeCount,
            "all", &eventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsSearchDetailEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id: search detail";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddSearchDetailEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddSearchDetailEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with empty place id */
    place.id = "";
    place.searchResultsIndex = 0xBAD;
    result = NB_AnalyticsAddSearchDetailEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsImpressionEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id: impression";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddImpressionEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddImpressionEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsMapEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id: map";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

   /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddMapEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddMapEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsPlaceMessageEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id : place message";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddPlaceMessageEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddPlaceMessageEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsCallEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id : call";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddCallEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddCallEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsArrivalEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id : arrival";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddArrivalEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddArrivalEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsWeburlEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id : weburl";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddWebURLEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddWebURLEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsDescriptionEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id : description event test";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddRevealDescriptionEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddRevealDescriptionEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsReviewEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id : review event test";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddRevealReviewEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddRevealReviewEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsAddFavoritesEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id : add-favorites";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddAddFavoritesEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddAddFavoritesEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

static void
TestAnalyticsRouteRequestEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id : route-request";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace origin;
    NB_AnalyticsEventPlace destination;
    uint32 navSessionId = PAL_ClockGetGPSTime();
    const char* reason = "init";
    uint32 eventId = 0;

    nsl_memset(&origin, 0, sizeof(origin));
    origin.isGpsBased = TRUE;
    origin.isPremiumPlacement = TRUE;
    origin.isEnhancedPoi = TRUE;
    origin.id = "route-origin";
    origin.isSearchQueryEventIdPresent = TRUE;
    origin.searchQueryEventId = PAL_ClockGetGPSTime();

    nsl_memset(&destination, 0, sizeof(destination));
    destination.isGpsBased = TRUE;
    destination.isPremiumPlacement = TRUE;
    destination.isEnhancedPoi = TRUE;
    destination.id = "route-destination";
    destination.isSearchQueryEventIdPresent = TRUE;
    destination.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddRouteRequestEvent(context, &origin, &destination,
            reason, navSessionId, &eventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    origin.goldenCookie = &goldenCookie;
    destination.goldenCookie = &goldenCookie;
    navSessionId += 1;
    result = NB_AnalyticsAddRouteRequestEvent(context, &origin, &destination,
            reason, navSessionId, &eventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

static void
TestAnalyticsRouteReplyEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    uint32 navSessionId = PAL_ClockGetGPSTime();
    uint32 routeRequestEventId = 100;
    uint32 duration = 1000;
    double distance = 100000.0;
    uint32 eventId = 0;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddRouteReplyEvent(context, navSessionId,
            routeRequestEventId, duration, distance, &eventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

static void
TestAnalyticsRouteStateEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    uint32 navSessionId = PAL_ClockGetGPSTime();
    uint32 routeRequestEventId = 100;
    uint32 routeReplyEventId = 100;
    uint32 duration = 1000;
    double distance = 100000.0;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddRouteStateEvent(context, navSessionId,
            routeRequestEventId, routeReplyEventId, "recalc", duration,
            distance);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddEnhancedContent(context, NB_AECD_Junction, "USA");
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddEnhancedContent(context, NB_AECD_RealisticSign, "USA");
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddEnhancedContent(context, NB_AECD_CityModels, "USA");
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddRouteStateEvent(context, navSessionId,
            routeRequestEventId, routeReplyEventId, "app-exit", duration,
            distance);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsFacebookUpdateEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    const char* goldenCookieId = "golden cookie id: impression";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddFacebookUpdateEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddFacebookUpdateEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsEventsPriority(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback;

    uint32 navSessionId = PAL_ClockGetGPSTime();
    uint32 routeRequestEventId = 100;
    uint32 routeReplyEventId = 100;
    uint32 duration = 1000;
    double distance = 100000.0;

    const char* categoryCodes[] = {"category1", "category2"};
    uint32 categoryCodeCount = sizeof(categoryCodes) / sizeof(const char *);
    uint32 eventId = 0;

    NB_GpsLocation            gpsFix = {0};

    const char* goldenCookieId = "golden cookie id : add-favorites";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    NB_AnalyticsFeedbackEvent feedback = {0};
    NB_Place poiPlace = {0};
    NB_Location originLocation = {0};
    NB_GpsLocation originGpsLocation = {0};
    NB_Location destinationLocation = {0};
    NB_GpsLocation destinationGpsLocation = {0};
    NB_AnalyticsWantAnalyticsConfig config = {0};

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantGoldCategory = TRUE;
    config.wantPoiCategory = TRUE;
    config.wantRouteTrackingCategory = TRUE;
    config.wantGpsProbesCategory = TRUE;
    config.wantWifiProbesCategory = TRUE;
    config.wantCouponCategory = FALSE;
    config.wantNavInvocationCategory = FALSE;
    config.wantFeedbackCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 2;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* for gps-offset */
    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 0.01;
    gpsFix.longitude  += 0.02;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddSearchQueryEvent(context, "search name",
            "search scheme", categoryCodes, categoryCodeCount,
            "all", &eventId);

    result = NB_AnalyticsAddAddFavoritesEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddAddFavoritesEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddRouteStateEvent(context, navSessionId,
            routeRequestEventId, routeReplyEventId, "recalc", duration,
            distance);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddRouteStateEvent(context, navSessionId,
            routeRequestEventId, routeReplyEventId, "app-exit", duration,
            distance);
    CU_ASSERT_EQUAL(result, NE_OK);

    feedback.navSessionId = PAL_ClockGetGPSTime();
    feedback.routeRequestEventId = 100;
    feedback.searchQueryEventId;

    feedback.screenId = "auto_nav_arrival";
    feedback.issueType = "ADDRESS_INCORRECT";
    feedback.providerId = "NavTaq";

    nsl_strcpy(poiPlace.name, "Mobil");
    poiPlace.phone[0].type = NB_Phone_Primary;
    nsl_strcpy(poiPlace.phone[0].country, "1");
    nsl_strcpy(poiPlace.phone[0].area, "949");
    nsl_strcpy(poiPlace.phone[0].number, "3625531");
    poiPlace.phone[1].type = NB_Phone_Secondary;
    nsl_strcpy(poiPlace.phone[1].country, "1");
    nsl_strcpy(poiPlace.phone[1].area, "949");
    nsl_strcpy(poiPlace.phone[1].number, "6431630");
    poiPlace.numphone = 2;

    poiPlace.numcategory = 1;
    nsl_strcpy(poiPlace.category[0].code, "123");
    nsl_strcpy(poiPlace.category[0].name, "Gas Stations");

    originLocation.latitude = 33.0;
    originLocation.longitude = 117.0;
    nsl_strcpy(originLocation.streetnum, "23431");
    nsl_strcpy(originLocation.street1, "Aliso Viejo Pky");
    nsl_strcpy(originLocation.areaname, "Mobil");
    nsl_strcpy(originLocation.city, "Aliso Viejo");
    originLocation.type = NB_Location_LatLon;

    destinationLocation.latitude = 33.0;
    destinationLocation.longitude = 117.0;
    nsl_strcpy(destinationLocation.streetnum, "26815");
    nsl_strcpy(destinationLocation.street1, "La Paz Rd");
    nsl_strcpy(destinationLocation.areaname, "Mobil");
    nsl_strcpy(destinationLocation.city, "Aliso Viejo");
    destinationLocation.type = NB_Location_LatLon;

    originGpsLocation.status   = PAL_Ok;
    originGpsLocation.valid    = NGV_Latitude | NGV_Longitude;
    originGpsLocation.gpsTime    = PAL_ClockGetGPSTime();
    originGpsLocation.latitude   = 33.58;
    originGpsLocation.longitude  = -117.73;

    destinationGpsLocation.status   = PAL_Ok;
    destinationGpsLocation.valid    = NGV_Latitude | NGV_Longitude;
    destinationGpsLocation.gpsTime    = PAL_ClockGetGPSTime();
    destinationGpsLocation.latitude   = 35.58;
    destinationGpsLocation.longitude  = -117.73;

    feedback.poiPlace = &poiPlace;
    feedback.originGpsLocation = &originGpsLocation;
    feedback.destinationGpsLocation = &destinationGpsLocation;

    result = NB_AnalyticsAddFeedbackEvent(context, &feedback);
    CU_ASSERT_EQUAL(result, NE_OK);

    feedback.originGpsLocation = NULL;
    feedback.destinationGpsLocation = NULL;
    feedback.originLocation = &originLocation;
    feedback.destinationLocation = &destinationLocation;

    result = NB_AnalyticsAddFeedbackEvent(context, &feedback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsEventsSaveAndRestore(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback;

    uint32 navSessionId = PAL_ClockGetGPSTime();
    uint32 routeRequestEventId = 100;
    uint32 routeReplyEventId = 100;
    uint32 duration = 1000;
    double distance = 100000.0;

    const char* categoryCodes[] = {"category1", "category2"};
    uint32 categoryCodeCount = sizeof(categoryCodes) / sizeof(const char *);
    uint32 eventId = 0;

    NB_GpsLocation            gpsFix = {0};

    const char* goldenCookieId = "golden cookie id : add-favorites";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    NB_AnalyticsEventPlace origin;
    NB_AnalyticsEventPlace destination;
    const char* reason = "init";

    NB_AnalyticsFeedbackEvent feedback = {0};
    NB_Place poiPlace = {0};
    NB_Location originLocation = {0};
    NB_GpsLocation originGpsLocation = {0};
    NB_Location destinationLocation = {0};
    NB_GpsLocation destinationGpsLocation = {0};
    NB_AnalyticsWantAnalyticsConfig config = {0};

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantGoldCategory = TRUE;
    config.wantPoiCategory = TRUE;
    config.wantRouteTrackingCategory = TRUE;
    config.wantGpsProbesCategory = TRUE;
    config.wantWifiProbesCategory = TRUE;
    config.wantCouponCategory = FALSE;
    config.wantNavInvocationCategory = FALSE;
    config.wantFeedbackCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 2;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* for gps-offset */
    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 0.01;
    gpsFix.longitude  += 0.02;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddSearchQueryEvent(context, "search name",
            "search scheme", categoryCodes, categoryCodeCount,
            "all", &eventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  search detail */
    place.goldenCookie = NULL;
    result = NB_AnalyticsAddSearchDetailEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddSearchDetailEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add map event */
    place.goldenCookie = NULL;
    result = NB_AnalyticsAddMapEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddMapEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add place message event */
    place.goldenCookie = NULL;
    result = NB_AnalyticsAddPlaceMessageEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddPlaceMessageEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add call event */
    place.goldenCookie = NULL;
    result = NB_AnalyticsAddCallEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddCallEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add favorite event */
    place.goldenCookie = NULL;
    result = NB_AnalyticsAddAddFavoritesEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddAddFavoritesEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add route request event */
    nsl_memset(&origin, 0, sizeof(origin));
    origin.isGpsBased = TRUE;
    origin.isPremiumPlacement = TRUE;
    origin.isEnhancedPoi = TRUE;
    origin.id = "route-origin";
    origin.isSearchQueryEventIdPresent = TRUE;
    origin.searchQueryEventId = PAL_ClockGetGPSTime();

    nsl_memset(&destination, 0, sizeof(destination));
    destination.isGpsBased = TRUE;
    destination.isPremiumPlacement = TRUE;
    destination.isEnhancedPoi = TRUE;
    destination.id = "route-destination";
    destination.isSearchQueryEventIdPresent = TRUE;
    destination.searchQueryEventId = PAL_ClockGetGPSTime();

    result = NB_AnalyticsAddRouteRequestEvent(context, &origin, &destination,
            reason, navSessionId, &routeRequestEventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    origin.goldenCookie = &goldenCookie;
    destination.goldenCookie = &goldenCookie;
    navSessionId += 1;
    result = NB_AnalyticsAddRouteRequestEvent(context, &origin, &destination,
            reason, navSessionId, &routeRequestEventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add route reply event */
    result = NB_AnalyticsAddRouteReplyEvent(context, navSessionId,
            routeRequestEventId, duration, distance, &routeReplyEventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add route state event */
    result = NB_AnalyticsAddRouteStateEvent(context, navSessionId,
            routeRequestEventId, routeReplyEventId, "recalc", duration,
            distance);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddRouteStateEvent(context, navSessionId,
            routeRequestEventId, routeReplyEventId, "app-exit", duration,
            distance);
    CU_ASSERT_EQUAL(result, NE_OK);

    feedback.navSessionId = PAL_ClockGetGPSTime();
    feedback.routeRequestEventId = 100;
    feedback.searchQueryEventId;

    feedback.screenId = "auto_nav_arrival";
    feedback.issueType = "ADDRESS_INCORRECT";
    feedback.providerId = "NavTaq";

    nsl_strcpy(poiPlace.name, "Mobil");
    poiPlace.phone[0].type = NB_Phone_Primary;
    nsl_strcpy(poiPlace.phone[0].country, "1");
    nsl_strcpy(poiPlace.phone[0].area, "949");
    nsl_strcpy(poiPlace.phone[0].number, "3625531");
    poiPlace.phone[1].type = NB_Phone_Secondary;
    nsl_strcpy(poiPlace.phone[1].country, "1");
    nsl_strcpy(poiPlace.phone[1].area, "949");
    nsl_strcpy(poiPlace.phone[1].number, "6431630");
    poiPlace.numphone = 2;

    poiPlace.numcategory = 1;
    nsl_strcpy(poiPlace.category[0].code, "123");
    nsl_strcpy(poiPlace.category[0].name, "Gas Stations");

    originLocation.latitude = 33.0;
    originLocation.longitude = 117.0;
    nsl_strcpy(originLocation.streetnum, "23431");
    nsl_strcpy(originLocation.street1, "Aliso Viejo Pky");
    nsl_strcpy(originLocation.areaname, "Mobil");
    nsl_strcpy(originLocation.city, "Aliso Viejo");
    originLocation.type = NB_Location_LatLon;

    destinationLocation.latitude = 33.0;
    destinationLocation.longitude = 117.0;
    nsl_strcpy(destinationLocation.streetnum, "26815");
    nsl_strcpy(destinationLocation.street1, "La Paz Rd");
    nsl_strcpy(destinationLocation.areaname, "Mobil");
    nsl_strcpy(destinationLocation.city, "Aliso Viejo");
    destinationLocation.type = NB_Location_LatLon;

    originGpsLocation.status   = PAL_Ok;
    originGpsLocation.valid    = NGV_Latitude | NGV_Longitude;
    originGpsLocation.gpsTime    = PAL_ClockGetGPSTime();
    originGpsLocation.latitude   = 33.58;
    originGpsLocation.longitude  = -117.73;

    destinationGpsLocation.status   = PAL_Ok;
    destinationGpsLocation.valid    = NGV_Latitude | NGV_Longitude;
    destinationGpsLocation.gpsTime    = PAL_ClockGetGPSTime();
    destinationGpsLocation.latitude   = 35.58;
    destinationGpsLocation.longitude  = -117.73;

    feedback.poiPlace = &poiPlace;
    feedback.originGpsLocation = &originGpsLocation;
    feedback.destinationGpsLocation = &destinationGpsLocation;

    result = NB_AnalyticsAddFeedbackEvent(context, &feedback);
    CU_ASSERT_EQUAL(result, NE_OK);

    feedback.originGpsLocation = NULL;
    feedback.destinationGpsLocation = NULL;
    feedback.originLocation = &originLocation;
    feedback.destinationLocation = &destinationLocation;

    result = NB_AnalyticsAddFeedbackEvent(context, &feedback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  close the closet to force save events to file */
    DestroyContext(context);
    PAL_Destroy(pal);

    /*  recreate the context and anlytics to restore events */
    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsProcess(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsProcess(context);
    CU_ASSERT_EQUAL(result, NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsForceUpload(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;
    uint32 navSessionId = PAL_ClockGetGPSTime();

    NB_AnalyticsCallbacks     analyticsCallback;

    NB_GpsLocation            gpsFix = {0};

    NB_AnalyticsFeedbackEvent feedback = {0};
    NB_Place poiPlace = {0};
    NB_Location originLocation = {0};
    NB_GpsLocation originGpsLocation = {0};
    NB_Location destinationLocation = {0};
    NB_GpsLocation destinationGpsLocation = {0};
    NB_AnalyticsWantAnalyticsConfig config = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantGoldCategory = TRUE;
    config.wantPoiCategory = TRUE;
    config.wantRouteTrackingCategory = TRUE;
    config.wantGpsProbesCategory = TRUE;
    config.wantWifiProbesCategory = TRUE;
    config.wantCouponCategory = FALSE;
    config.wantNavInvocationCategory = FALSE;
    config.wantFeedbackCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 2;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* for gps-offset */
    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 0.01;
    gpsFix.longitude  += 0.02;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    feedback.navSessionId = PAL_ClockGetGPSTime();
    feedback.routeRequestEventId = 100;
    feedback.searchQueryEventId;

    feedback.screenId = "auto_nav_arrival";
    feedback.issueType = "ADDRESS_INCORRECT";
    feedback.providerId = "NavTaq";

    nsl_strcpy(poiPlace.name, "Mobil");
    poiPlace.phone[0].type = NB_Phone_Primary;
    nsl_strcpy(poiPlace.phone[0].country, "1");
    nsl_strcpy(poiPlace.phone[0].area, "949");
    nsl_strcpy(poiPlace.phone[0].number, "3625531");
    poiPlace.phone[1].type = NB_Phone_Secondary;
    nsl_strcpy(poiPlace.phone[1].country, "1");
    nsl_strcpy(poiPlace.phone[1].area, "949");
    nsl_strcpy(poiPlace.phone[1].number, "6431630");
    poiPlace.numphone = 2;

    poiPlace.numcategory = 1;
    nsl_strcpy(poiPlace.category[0].code, "123");
    nsl_strcpy(poiPlace.category[0].name, "Gas Stations");

    originLocation.latitude = 33.0;
    originLocation.longitude = 117.0;
    nsl_strcpy(originLocation.streetnum, "23431");
    nsl_strcpy(originLocation.street1, "Aliso Viejo Pky");
    nsl_strcpy(originLocation.areaname, "Mobil");
    nsl_strcpy(originLocation.city, "Aliso Viejo");
    originLocation.type = NB_Location_LatLon;

    destinationLocation.latitude = 33.0;
    destinationLocation.longitude = 117.0;
    nsl_strcpy(destinationLocation.streetnum, "26815");
    nsl_strcpy(destinationLocation.street1, "La Paz Rd");
    nsl_strcpy(destinationLocation.areaname, "Mobil");
    nsl_strcpy(destinationLocation.city, "Aliso Viejo");
    destinationLocation.type = NB_Location_LatLon;

    originGpsLocation.status   = PAL_Ok;
    originGpsLocation.valid    = NGV_Latitude | NGV_Longitude;
    originGpsLocation.gpsTime    = PAL_ClockGetGPSTime();
    originGpsLocation.latitude   = 33.58;
    originGpsLocation.longitude  = -117.73;

    destinationGpsLocation.status   = PAL_Ok;
    destinationGpsLocation.valid    = NGV_Latitude | NGV_Longitude;
    destinationGpsLocation.gpsTime    = PAL_ClockGetGPSTime();
    destinationGpsLocation.latitude   = 35.58;
    destinationGpsLocation.longitude  = -117.73;

    feedback.poiPlace = &poiPlace;
    feedback.originGpsLocation = &originGpsLocation;
    feedback.destinationGpsLocation = &destinationGpsLocation;

    result = NB_AnalyticsAddFeedbackEvent(context, &feedback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* force upload */
    NB_AnalyticsForceUpload(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsMasterClear(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback;

    uint32 navSessionId = PAL_ClockGetGPSTime();
    uint32 routeRequestEventId = 100;
    uint32 routeReplyEventId = 100;
    uint32 duration = 1000;
    double distance = 100000.0;

    const char* categoryCodes[] = {"category1", "category2"};
    uint32 categoryCodeCount = sizeof(categoryCodes) / sizeof(const char *);
    uint32 eventId = 0;

    NB_GpsLocation            gpsFix = {0};

    const char* goldenCookieId = "golden cookie id : add-favorites";
    const byte stateData[] = {'a', 'b', 'c', 'd'};
    uint32 stateSize = sizeof(stateData);

    NB_AnalyticsGoldenCookie goldenCookie;

    NB_AnalyticsEventPlace place;

    NB_AnalyticsEventPlace origin;
    NB_AnalyticsEventPlace destination;
    const char* reason = "init";

    NB_AnalyticsFeedbackEvent feedback = {0};
    NB_Place poiPlace = {0};
    NB_Location originLocation = {0};
    NB_GpsLocation originGpsLocation = {0};
    NB_Location destinationLocation = {0};
    NB_GpsLocation destinationGpsLocation = {0};
    NB_AnalyticsWantAnalyticsConfig config = {0};

    nsl_memset(&place, 0, sizeof(place));

    place.isGpsBased = TRUE;
    place.isPremiumPlacement = TRUE;
    place.isEnhancedPoi = TRUE;
    place.id = "6 Liberty";
    place.isSearchQueryEventIdPresent = TRUE;
    place.searchQueryEventId = PAL_ClockGetGPSTime();

    goldenCookie.providerId = goldenCookieId;
    goldenCookie.stateData = stateData;
    goldenCookie.stateSize = stateSize;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantGoldCategory = TRUE;
    config.wantPoiCategory = TRUE;
    config.wantRouteTrackingCategory = TRUE;
    config.wantGpsProbesCategory = TRUE;
    config.wantWifiProbesCategory = TRUE;
    config.wantCouponCategory = FALSE;
    config.wantNavInvocationCategory = FALSE;
    config.wantFeedbackCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 2;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* for gps-offset */
    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 0.01;
    gpsFix.longitude  += 0.02;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddSearchQueryEvent(context, "search name",
            "search scheme", categoryCodes, categoryCodeCount,
            "all", &eventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  search detail */
    place.goldenCookie = NULL;
    result = NB_AnalyticsAddSearchDetailEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddSearchDetailEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add map event */
    place.goldenCookie = NULL;
    result = NB_AnalyticsAddMapEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddMapEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add place message event */
    place.goldenCookie = NULL;
    result = NB_AnalyticsAddPlaceMessageEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddPlaceMessageEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add call event */
    place.goldenCookie = NULL;
    result = NB_AnalyticsAddCallEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddCallEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add favorite event */
    place.goldenCookie = NULL;
    result = NB_AnalyticsAddAddFavoritesEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    place.goldenCookie = &goldenCookie;
    result = NB_AnalyticsAddAddFavoritesEvent(context, &place);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add route request event */
    nsl_memset(&origin, 0, sizeof(origin));
    origin.isGpsBased = TRUE;
    origin.isPremiumPlacement = TRUE;
    origin.isEnhancedPoi = TRUE;
    origin.id = "route-origin";
    origin.isSearchQueryEventIdPresent = TRUE;
    origin.searchQueryEventId = PAL_ClockGetGPSTime();

    nsl_memset(&destination, 0, sizeof(destination));
    destination.isGpsBased = TRUE;
    destination.isPremiumPlacement = TRUE;
    destination.isEnhancedPoi = TRUE;
    destination.id = "route-destination";
    destination.isSearchQueryEventIdPresent = TRUE;
    destination.searchQueryEventId = PAL_ClockGetGPSTime();

    result = NB_AnalyticsAddRouteRequestEvent(context, &origin, &destination,
            reason, navSessionId, &routeRequestEventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* add one with golden cookie */
    origin.goldenCookie = &goldenCookie;
    destination.goldenCookie = &goldenCookie;
    navSessionId += 1;
    result = NB_AnalyticsAddRouteRequestEvent(context, &origin, &destination,
            reason, navSessionId, &routeRequestEventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add route reply event */
    result = NB_AnalyticsAddRouteReplyEvent(context, navSessionId,
            routeRequestEventId, duration, distance, &routeReplyEventId);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  add route state event */
    result = NB_AnalyticsAddRouteStateEvent(context, navSessionId,
            routeRequestEventId, routeReplyEventId, "recalc", duration,
            distance);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddRouteStateEvent(context, navSessionId,
            routeRequestEventId, routeReplyEventId, "app-exit", duration,
            distance);
    CU_ASSERT_EQUAL(result, NE_OK);

    feedback.navSessionId = PAL_ClockGetGPSTime();
    feedback.routeRequestEventId = 100;
    feedback.searchQueryEventId;

    feedback.screenId = "auto_nav_arrival";
    feedback.issueType = "ADDRESS_INCORRECT";
    feedback.providerId = "NavTaq";

    nsl_strcpy(poiPlace.name, "Mobil");
    poiPlace.phone[0].type = NB_Phone_Primary;
    nsl_strcpy(poiPlace.phone[0].country, "1");
    nsl_strcpy(poiPlace.phone[0].area, "949");
    nsl_strcpy(poiPlace.phone[0].number, "3625531");
    poiPlace.phone[1].type = NB_Phone_Secondary;
    nsl_strcpy(poiPlace.phone[1].country, "1");
    nsl_strcpy(poiPlace.phone[1].area, "949");
    nsl_strcpy(poiPlace.phone[1].number, "6431630");
    poiPlace.numphone = 2;

    poiPlace.numcategory = 1;
    nsl_strcpy(poiPlace.category[0].code, "123");
    nsl_strcpy(poiPlace.category[0].name, "Gas Stations");

    originLocation.latitude = 33.0;
    originLocation.longitude = 117.0;
    nsl_strcpy(originLocation.streetnum, "23431");
    nsl_strcpy(originLocation.street1, "Aliso Viejo Pky");
    nsl_strcpy(originLocation.areaname, "Mobil");
    nsl_strcpy(originLocation.city, "Aliso Viejo");
    originLocation.type = NB_Location_LatLon;

    destinationLocation.latitude = 33.0;
    destinationLocation.longitude = 117.0;
    nsl_strcpy(destinationLocation.streetnum, "26815");
    nsl_strcpy(destinationLocation.street1, "La Paz Rd");
    nsl_strcpy(destinationLocation.areaname, "Mobil");
    nsl_strcpy(destinationLocation.city, "Aliso Viejo");
    destinationLocation.type = NB_Location_LatLon;

    originGpsLocation.status   = PAL_Ok;
    originGpsLocation.valid    = NGV_Latitude | NGV_Longitude;
    originGpsLocation.gpsTime    = PAL_ClockGetGPSTime();
    originGpsLocation.latitude   = 33.58;
    originGpsLocation.longitude  = -117.73;

    destinationGpsLocation.status   = PAL_Ok;
    destinationGpsLocation.valid    = NGV_Latitude | NGV_Longitude;
    destinationGpsLocation.gpsTime    = PAL_ClockGetGPSTime();
    destinationGpsLocation.latitude   = 35.58;
    destinationGpsLocation.longitude  = -117.73;

    feedback.poiPlace = &poiPlace;
    feedback.originGpsLocation = &originGpsLocation;
    feedback.destinationGpsLocation = &destinationGpsLocation;

    result = NB_AnalyticsAddFeedbackEvent(context, &feedback);
    CU_ASSERT_EQUAL(result, NE_OK);

    feedback.originGpsLocation = NULL;
    feedback.destinationGpsLocation = NULL;
    feedback.originLocation = &originLocation;
    feedback.destinationLocation = &destinationLocation;

    result = NB_AnalyticsAddFeedbackEvent(context, &feedback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  close the closet to force save events to file */
    DestroyContext(context);
    PAL_Destroy(pal);

    /*  recreate the context and analytics to restore events */
    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* force clear  */
    result = NB_AnalyticsMasterClear(context);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* force upload */
    result = NB_AnalyticsForceUpload(context);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsEnableGpsProbes(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;
    uint32 navSessionId = PAL_ClockGetGPSTime();

    NB_AnalyticsCallbacks     analyticsCallback;

    NB_GpsLocation            gpsFix = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 2;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  disable GPS probes collection, and remove collected GPS probes */
    result = NB_AnalyticsEnableGpsProbes(context, FALSE);
    CU_ASSERT_EQUAL(result, NE_OK);

    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 1;
    gpsFix.longitude  += 1;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    /*  enable GPS probes collection */
    result = NB_AnalyticsEnableGpsProbes(context, TRUE);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* for gps-offset */
    gpsFix.gpsTime += 20;
    gpsFix.latitude   += 0.01;
    gpsFix.longitude  += 0.02;
    result = NB_AnalyticsAddGpsProbe(context, navSessionId, &gpsFix);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsCouponActionEvents(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback = {0};

    NB_Coupon* coupon = 0;
    NB_Store* store = 0;
    NB_AnalyticsWantAnalyticsConfig config = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantCouponCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT_EQUAL(CreateCoupon(&coupon), NE_OK);
    CU_ASSERT_EQUAL(CreateStore(&store), NE_OK);
    CU_ASSERT_PTR_NOT_NULL(coupon);
    CU_ASSERT_PTR_NOT_NULL(store);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    // send all available actions
    result = NB_AnalyticsAddCouponAnalytics(context, coupon, store, "display");
    CU_ASSERT_EQUAL(result, NE_OK);
    result = NB_AnalyticsAddCouponAnalytics(context, coupon, store, "view");
    CU_ASSERT_EQUAL(result, NE_OK);
    result = NB_AnalyticsAddCouponAnalytics(context, coupon, store, "call");
    CU_ASSERT_EQUAL(result, NE_OK);
    result = NB_AnalyticsAddCouponAnalytics(context, coupon, store, "navigate");
    CU_ASSERT_EQUAL(result, NE_OK);
    result = NB_AnalyticsAddCouponAnalytics(context, coupon, store, "click");
    CU_ASSERT_EQUAL(result, NE_OK);
    result = NB_AnalyticsAddCouponAnalytics(context, coupon, store, "clip");
    CU_ASSERT_EQUAL(result, NE_OK);
    result = NB_AnalyticsAddCouponAnalytics(context, coupon, store, "url");
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    CU_ASSERT_EQUAL(CouponDestroy(coupon), NE_OK);
    CU_ASSERT_EQUAL(StoreDestroy(store), NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsAddCouponWithoutContext(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback = {0};

    NB_Coupon *coupon = 0;
    NB_Store *store = 0;
    NB_AnalyticsWantAnalyticsConfig config = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantCouponCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT_EQUAL(CreateCoupon(&coupon), NE_OK);
    CU_ASSERT_EQUAL(CreateStore(&store), NE_OK);
    CU_ASSERT_PTR_NOT_NULL(coupon);
    CU_ASSERT_PTR_NOT_NULL(store);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    // send all available actions
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, store, "display");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, store, "view");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, store, "call");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, store, "navigate");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, store, "click");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, store, "clip");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, store, "url");
    CU_ASSERT_EQUAL(result, NE_INVAL);

    CU_ASSERT_EQUAL(CouponDestroy(coupon), NE_OK);
    CU_ASSERT_EQUAL(StoreDestroy(store), NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsAddCouponWithoutCoupon(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback = {0};

    NB_Store *store = 0;
    NB_AnalyticsWantAnalyticsConfig config = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantCouponCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT_EQUAL(CreateStore(&store), NE_OK);
    CU_ASSERT_PTR_NOT_NULL(store);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    // send all available actions
    result = NB_AnalyticsAddCouponAnalytics(context, NULL, store, "display");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(context, NULL, store, "view");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(context, NULL, store, "call");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(context, NULL, store, "navigate");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(context, NULL, store, "click");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(context, NULL, store, "clip");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(context, NULL, store, "url");
    CU_ASSERT_EQUAL(result, NE_INVAL);

    CU_ASSERT_EQUAL(StoreDestroy(store), NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsAddCouponWithoutStore(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback = {0};

    NB_Coupon *coupon = 0;
    NB_AnalyticsWantAnalyticsConfig config = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantCouponCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT_EQUAL(CreateCoupon(&coupon), NE_OK);
    CU_ASSERT_PTR_NOT_NULL(coupon);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    // send all available actions
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, NULL, "display");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, NULL, "view");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, NULL, "call");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, NULL, "navigate");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, NULL, "click");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, NULL, "clip");
    CU_ASSERT_EQUAL(result, NE_INVAL);
    result = NB_AnalyticsAddCouponAnalytics(NULL, coupon, NULL, "url");
    CU_ASSERT_EQUAL(result, NE_INVAL);

    CU_ASSERT_EQUAL(CouponDestroy(coupon), NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsAddCouponWithoutAction(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback = {0};

    NB_Coupon *coupon = 0;
    NB_Store *store = 0;
    NB_AnalyticsWantAnalyticsConfig config = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantCouponCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT_EQUAL(CreateCoupon(&coupon), NE_OK);
    CU_ASSERT_EQUAL(CreateStore(&store), NE_OK);
    CU_ASSERT_PTR_NOT_NULL(coupon);
    CU_ASSERT_PTR_NOT_NULL(store);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    // send all available actions
    result = NB_AnalyticsAddCouponAnalytics(context, coupon, store, NULL);
    CU_ASSERT_EQUAL(result, NE_INVAL);

    CU_ASSERT_EQUAL(CouponDestroy(coupon), NE_OK);
    CU_ASSERT_EQUAL(StoreDestroy(store), NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}


void
ServerMessageStatusCallback(void* handler, NB_NetworkRequestStatus status,
        NB_Error err, uint8 up, int percent, void* pUserData)
{
    if (!up)
    {
        if (status != NB_NetworkRequestStatus_Progress)
        {
            SetCallbackCompletedEvent(g_versionCallbackCompletedEvent);
        }
    }
}

void
AnalyticsActivityCallback(NB_Analytics* analytics, void* userData,
        NB_AnalyticsActivity activity)
{
    switch (activity)
    {
        case NB_AA_PiggybackOpportunity:
            break;
        case NB_AA_QuerySent:
            break;
        case NB_AA_ReplyReceived:
            SetCallbackCompletedEvent(g_ReplyReceivedEvent);
            break;
        case NB_AA_ConfigReceived:
            SetCallbackCompletedEvent(g_ConfigReceivedEvent);
            break;
        default:
            break;
    }
}

void
TestAnalyticsNavInvocationAppPresent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback = {0};

    nb_boolean appPresent = TRUE;
    const char platform[] = "win32";
    const char carrier[] = "SystemTests";
    const char sdkVersion[] = "1.0.0.0";
    NB_AnalyticsWantAnalyticsConfig config = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantNavInvocationCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddApplicationPresentEvent(context,
        appPresent,
        platform,
        carrier,
        sdkVersion);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsNavInvocationAppPresentWithoutContext(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback = {0};

    nb_boolean appPresent = TRUE;
    const char platform[] = "win32";
    const char carrier[] = "SystemTests";
    const char sdkVersion[] = "1.0.0.0";
    NB_AnalyticsWantAnalyticsConfig config = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantNavInvocationCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddApplicationPresentEvent(NULL,
        appPresent,
        platform,
        carrier,
        sdkVersion);
    CU_ASSERT_EQUAL(result, NE_INVAL);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsNavInvocationAppPresentWithoutCarrier(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback = {0};

    nb_boolean appPresent = TRUE;
    const char platform[] = "win32";
    const char *carrier = NULL;
    const char sdkVersion[] = "1.0.0.0";
    NB_AnalyticsWantAnalyticsConfig config = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantNavInvocationCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddApplicationPresentEvent(context,
        appPresent,
        platform,
        carrier,
        sdkVersion);
    CU_ASSERT_EQUAL(result, NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsNavInvocationAppPresentWithoutSdkVersion(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback = {0};

    nb_boolean appPresent = TRUE;
    const char platform[] = "win32";
    const char carrier[] = "SystemTests";
    const char *sdkVersion = NULL;
    NB_AnalyticsWantAnalyticsConfig config = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantNavInvocationCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddApplicationPresentEvent(context,
        appPresent,
        platform,
        carrier,
        sdkVersion);
    CU_ASSERT_EQUAL(result, NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsNavInvocationAppPresentWithoutPlatform(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks     analyticsCallback = {0};

    nb_boolean appPresent = TRUE;
    const char *platform = NULL;
    const char carrier[] = "SystemTests";
    const char sdkVersion[] = "1.0.0.0";
    NB_AnalyticsWantAnalyticsConfig config = {0};

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantNavInvocationCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddApplicationPresentEvent(context,
        appPresent,
        platform,
        carrier,
        sdkVersion);
    CU_ASSERT_EQUAL(result, NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsNavInvocationUseNavigator(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback = {0};
    NB_AnalyticsWantAnalyticsConfig config = {0};

    NB_Place place = {0};
    NB_RouteType routeType = NB_RouteType_Fastest;
    NB_TransportationMode vehicleType = NB_TransportationMode_Car;
    NB_RouteAvoid routeAvoid = NB_RouteAvoid_None;
    const char platform[] = "win32";
    const char carrier[] = "SystemTests";
    const char sdkVersion[] = "1.0.0.0";

    nsl_strcpy(place.name, "BarPrzyRynku");
    place.location.type = NB_Location_LatLon;
    place.location.latitude  = 54.4128102064;
    place.location.longitude = 18.5909217596;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantNavInvocationCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddUseNavigatorEvent(context,
        &place,
        routeType,
        vehicleType,
        routeAvoid,
        platform,
        carrier,
        sdkVersion);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsNavInvocationUseNavigatorWithoutContext(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback = {0};
    NB_AnalyticsWantAnalyticsConfig config = {0};

    NB_Place place = {0};
    NB_RouteType routeType = NB_RouteType_Fastest;
    NB_TransportationMode vehicleType = NB_TransportationMode_Car;
    NB_RouteAvoid routeAvoid = NB_RouteAvoid_None;
    const char platform[] = "win32";
    const char carrier[] = "SystemTests";
    const char sdkVersion[] = "1.0.0.0";

    nsl_strcpy(place.name, "BarPrzyRynku");
    place.location.type = NB_Location_LatLon;
    place.location.latitude  = 54.4128102064;
    place.location.longitude = 18.5909217596;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantNavInvocationCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddUseNavigatorEvent(NULL,
        &place,
        routeType,
        vehicleType,
        routeAvoid,
        platform,
        carrier,
        sdkVersion);
    CU_ASSERT_EQUAL(result, NE_INVAL);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsNavInvocationUseNavigatorWithoutPlace(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback = {0};
    NB_AnalyticsWantAnalyticsConfig config = {0};

    NB_RouteType routeType = NB_RouteType_Fastest;
    NB_TransportationMode vehicleType = NB_TransportationMode_Car;
    NB_RouteAvoid routeAvoid = NB_RouteAvoid_None;
    const char platform[] = "win32";
    const char carrier[] = "SystemTests";
    const char sdkVersion[] = "1.0.0.0";

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantNavInvocationCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddUseNavigatorEvent(context,
        NULL,
        routeType,
        vehicleType,
        routeAvoid,
        platform,
        carrier,
        sdkVersion);
    CU_ASSERT_EQUAL(result, NE_INVAL);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsNavInvocationUseNavigatorWithoutPlatform(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback = {0};
    NB_AnalyticsWantAnalyticsConfig config = {0};

    NB_Place place = {0};
    NB_RouteType routeType = NB_RouteType_Fastest;
    NB_TransportationMode vehicleType = NB_TransportationMode_Car;
    NB_RouteAvoid routeAvoid = NB_RouteAvoid_None;
    const char *platform = NULL;
    const char carrier[] = "SystemTests";
    const char sdkVersion[] = "1.0.0.0";

    nsl_strcpy(place.name, "BarPrzyRynku");
    place.location.type = NB_Location_LatLon;
    place.location.latitude  = 54.4128102064;
    place.location.longitude = 18.5909217596;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantNavInvocationCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddUseNavigatorEvent(context,
        &place,
        routeType,
        vehicleType,
        routeAvoid,
        platform,
        carrier,
        sdkVersion);
    CU_ASSERT_EQUAL(result, NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsNavInvocationUseNavigatorWithoutCarrier(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback = {0};
    NB_AnalyticsWantAnalyticsConfig config = {0};

    NB_Place place = {0};
    NB_RouteType routeType = NB_RouteType_Fastest;
    NB_TransportationMode vehicleType = NB_TransportationMode_Car;
    NB_RouteAvoid routeAvoid = NB_RouteAvoid_None;
    const char platform[] = "win32";
    const char *carrier = NULL;
    const char sdkVersion[] = "1.0.0.0";

    nsl_strcpy(place.name, "BarPrzyRynku");
    place.location.type = NB_Location_LatLon;
    place.location.latitude  = 54.4128102064;
    place.location.longitude = 18.5909217596;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantNavInvocationCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddUseNavigatorEvent(context,
        &place,
        routeType,
        vehicleType,
        routeAvoid,
        platform,
        carrier,
        sdkVersion);
    CU_ASSERT_EQUAL(result, NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}

void
TestAnalyticsNavInvocationUseNavigatorWithoutSdkVersion(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback = {0};
    NB_AnalyticsWantAnalyticsConfig config = {0};

    NB_Place place = {0};
    NB_RouteType routeType = NB_RouteType_Fastest;
    NB_TransportationMode vehicleType = NB_TransportationMode_Car;
    NB_RouteAvoid routeAvoid = NB_RouteAvoid_None;
    const char platform[] = "win32";
    const char carrier[] = "SystemTests";
    const char *sdkVersion = NULL;

    nsl_strcpy(place.name, "BarPrzyRynku");
    place.location.type = NB_Location_LatLon;
    place.location.latitude  = 54.4128102064;
    place.location.longitude = 18.5909217596;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantNavInvocationCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);
    CU_ASSERT_EQUAL(result, NE_OK);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddUseNavigatorEvent(context,
        &place,
        routeType,
        vehicleType,
        routeAvoid,
        platform,
        carrier,
        sdkVersion);
    CU_ASSERT_EQUAL(result, NE_OK);

    DestroyContext(context);
    PAL_Destroy(pal);
}

static void
TestAnalyticsAddFeedbackEvent(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback;

    NB_AnalyticsFeedbackEvent feedback = {0};
    NB_Place poiPlace = {0};
    NB_Location originLocation = {0};
    NB_GpsLocation originGpsLocation = {0};
    NB_Location destinationLocation = {0};
    NB_GpsLocation destinationGpsLocation = {0};
    NB_AnalyticsWantAnalyticsConfig config = {0};

    feedback.navSessionId = PAL_ClockGetGPSTime();
    feedback.routeRequestEventId = 100;
    feedback.searchQueryEventId;

    feedback.screenId = "auto_nav_arrival";
    feedback.issueType = "ADDRESS_INCORRECT";
    feedback.providerId = "NavTaq";

    nsl_strcpy(poiPlace.name, "Mobil");
    poiPlace.phone[0].type = NB_Phone_Primary;
    nsl_strcpy(poiPlace.phone[0].country, "1");
    nsl_strcpy(poiPlace.phone[0].area, "949");
    nsl_strcpy(poiPlace.phone[0].number, "3625531");
    poiPlace.phone[1].type = NB_Phone_Secondary;
    nsl_strcpy(poiPlace.phone[1].country, "1");
    nsl_strcpy(poiPlace.phone[1].area, "949");
    nsl_strcpy(poiPlace.phone[1].number, "6431630");
    poiPlace.numphone = 2;

    poiPlace.numcategory = 1;
    nsl_strcpy(poiPlace.category[0].code, "123");
    nsl_strcpy(poiPlace.category[0].name, "Gas Stations");

    originLocation.latitude = 33.0;
    originLocation.longitude = 117.0;
    nsl_strcpy(originLocation.streetnum, "23431");
    nsl_strcpy(originLocation.street1, "Aliso Viejo Pky");
    nsl_strcpy(originLocation.areaname, "Mobil");
    nsl_strcpy(originLocation.city, "Aliso Viejo");
    originLocation.type = NB_Location_LatLon;

    destinationLocation.latitude = 33.0;
    destinationLocation.longitude = 117.0;
    nsl_strcpy(destinationLocation.streetnum, "26815");
    nsl_strcpy(destinationLocation.street1, "La Paz Rd");
    nsl_strcpy(destinationLocation.areaname, "Mobil");
    nsl_strcpy(destinationLocation.city, "Aliso Viejo");
    destinationLocation.type = NB_Location_LatLon;

    originGpsLocation.status   = PAL_Ok;
    originGpsLocation.valid    = NGV_Latitude | NGV_Longitude;
    originGpsLocation.gpsTime    = PAL_ClockGetGPSTime();
    originGpsLocation.latitude   = 33.58;
    originGpsLocation.longitude  = -117.73;

    destinationGpsLocation.status   = PAL_Ok;
    destinationGpsLocation.valid    = NGV_Latitude | NGV_Longitude;
    destinationGpsLocation.gpsTime    = PAL_ClockGetGPSTime();
    destinationGpsLocation.latitude   = 35.58;
    destinationGpsLocation.longitude  = -117.73;

    feedback.poiPlace = &poiPlace;
    feedback.originGpsLocation = &originGpsLocation;
    feedback.destinationGpsLocation = &destinationGpsLocation;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantFeedbackCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddFeedbackEvent(context, &feedback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}


static void
TestAnalyticsAddAppErrors(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_Analytics* analytics = NULL;

    NB_AnalyticsCallbacks analyticsCallback = {0};

    NB_AnalyticsWantAnalyticsConfig config = {0};

    uint32 navSessionId = PAL_ClockGetGPSTime();

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    result = NB_AnalyticsCreate(context, NULL, &analytics);
    (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);
    CU_ASSERT_EQUAL(result, NE_OK);

    config.wantAppErrorsCategory = TRUE;
    result = NB_AnalyticsSetWantAnalyticsConfig(context, &config);

    analyticsCallback.activityCallback = AnalyticsActivityCallback;
    analyticsCallback.userData = NULL;

    result = NB_AnalyticsSetCallbacks(context, &analyticsCallback);
    CU_ASSERT_EQUAL(result, NE_OK);

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for config */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    result = NB_AnalyticsAddAppGpsErrorEvent(context, NE_GPS_TIMEOUT, "NE_GPS_TIMEOUT", "The GPS data error", "single", "normal");
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddAppGpsErrorEvent(context, NE_GPS_BUSY, "NE_GPS_BUSY", "The GPS device error", "tracking", "normal");
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddAppGpsErrorEvent(context, NE_GPS_PRIVACY, "NE_GPS_PRIVACY", "The GPS privacy error", "tracking", NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddAppNetworkErrorEvent(context, NESERVER_UNKNOWN, "NESERVER_UNKNOWN", "Failed to write to a network socket [1]", "localhost", "127.0.0.1");
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddAppNetworkErrorEvent(context, NESERVER_UNKNOWN, "NESERVER_UNKNOWN", "Failed to write to a network socket [2]", NULL, "127.0.0.1");
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddAppFileSystemErrorEvent(context, NE_FSYS, "NE_FSYS", "Can't open file - access denied [1]", &navSessionId, "test.txt");
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddAppFileSystemErrorEvent(context, NE_FSYS, "NE_FSYS", "Can't open file - access denied [2]", NULL, "test.txt");
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_AnalyticsAddAppNetworkErrorEvent(context, NE_INVAL, "NE_INVAL", "Failed to write to a network socket", NULL, NULL);
    CU_ASSERT_NOT_EQUAL(result, NE_OK); // NE_INVAL is not network error

    /* create server version query to trigger analytic piggy back message */
    TestAnalyticsCreatePiggybackOpportunity(context);

    /* wait for reply */
    CU_ASSERT(WaitForCallbackCompletedEvent(g_ReplyReceivedEvent, 35000));

    DestroyContext(context);
    PAL_Destroy(pal);
}



/*! @} */


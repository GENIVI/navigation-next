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

    @file     TestDirection.c
    @date     02/13/2009
    @defgroup TestDirection_h System tests for Direction functions
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

#include "main.h"
#include "testdirection.h"
#include "testnetwork.h"
#include "platformutil.h"
#include "nbcontextaccess.h"
#include "nbrouteinformation.h"
#include "nbroutehandler.h"
#include "nbrouteparameters.h"
#include "nbguidanceinformation.h"

static void TestDirectionsRequest1(void);
static void TestDirectionsRequest2(void);
static void TestDirectionsStaticRouting(void);
static void TestDirectionsRouteSelector(void);
static void TestDirectionsRouteSelectorWithLabelPoint(void);
static void TestDirectionsManeuverInfo(void);
static void TestDirectionsRequest4(void);
static void TestDirectionsRequest5(void);
static void TestDirectionsGetImageCode(void);
static void TestDirectionsGetImageCodeLeftDrivingSide(void);
static void TestDirectionsAnnounce(void);
static void TestDirectionsTextEnumerate(void);
static void TestDirectionsPolylineRouteWindow(void);
static void TestDirectionsManeuverTurnMapCenterPoint(void);

// @todo move these tests to the nbservices unit test
#ifdef MOVE_THIS_TO_UNIT_TEST
static void TestDirectionsRequestSignsAndJunctions(void);
static void TestDirectionsRequestCityModel(void);
#endif

/*This is a callback going to be called bt BLUE BOX*/
static void  NavStatus_SummaryText(NB_Font font, nb_color color, const char* txt, nb_boolean newline, void* pUser);

/* Step by step download callback for directions */
static void Steps_DownloadCB(void * pUser, NB_NetworkRequestStatus status, NB_Error err_in, int percent);

static void DownloadRouteCallback(void* pUserData, NB_NetworkRequestStatus status, NB_Error err, int percent);
static void DownloadRouteCallbackNew(void* handler,NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);

static void SetupPlaces (NB_Place* origin, NB_Place* dest);
static void SetupPlacesUK(NB_Place* origin, NB_Place* dest);
static void SetupConfigAndOptions(NB_Context* context, NB_GuidanceInformation* guidanceInfo, NB_RouteOptions* options, NB_RouteConfiguration* config);

static NB_Error GetOriginationDestination(NB_RouteInformation* route);
static NB_Error GetManeuverList(NB_RouteInformation* route);

// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;

/*! Add all your test functions here

@return None
*/

typedef struct TestRouteObj
{
  NB_RouteHandler* routeHandler;
  NB_RouteInformation* route;
} TestRouteObj;

void 
TestDirections_AddTests( CU_pSuite pTestSuite, int level )
{
    // ! Add all your function names here !    
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestDirectionsRequest1", TestDirectionsRequest1);
//    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsStaticRouting", TestDirectionsStaticRouting);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsRequest2", TestDirectionsRequest2);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsRouteSelector", TestDirectionsRouteSelector);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsRouteSelectorWithLabelPoint", TestDirectionsRouteSelectorWithLabelPoint);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsManeuverInfo", TestDirectionsManeuverInfo);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsRequest4", TestDirectionsRequest4);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsRequest5", TestDirectionsRequest5);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsGetImageCode", TestDirectionsGetImageCode);   
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsGetImageCodeLeftDrivingSide", TestDirectionsGetImageCodeLeftDrivingSide);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsAnnounce", TestDirectionsAnnounce);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsTextEnumerate", TestDirectionsTextEnumerate);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsPolylineRouteWindow", TestDirectionsPolylineRouteWindow);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsManeuverTurnMapCenterPoint", TestDirectionsManeuverTurnMapCenterPoint);
#if MOVE_THIS_TO_UNIT_TEST
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsRequestSignsAndJunctions", TestDirectionsRequestSignsAndJunctions);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDirectionsRequestCityModel", TestDirectionsRequestCityModel);
#endif
}


/*! Add common initialization code here.

@return 0

@see TestDirection_SuiteCleanup
*/
int 
TestDirections_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestDirection_SuiteSetup
*/
int 
TestDirections_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
    return 0;
}

void
SetupPlaces (NB_Place* origin, NB_Place* dest)
{
    if (origin)
    {   
        origin->location.type = NB_Location_Address;
        strcpy(origin->location.streetnum, "6");
        strcpy(origin->location.street1, "Liberty");
        strcpy(origin->location.city, "Aliso Viejo");
        strcpy(origin->location.state, "CA");
        strcpy(origin->location.postal, "92656");
        origin->location.latitude = 33.5598;
        origin->location.longitude = -117.7288;
    }

    if (dest)
    {    
        dest->location.type = NB_Location_Address;
        strcpy(dest->location.streetnum, "18872");
        strcpy(dest->location.street1, "Bardeen");
        strcpy(dest->location.city, "Irvine");
        strcpy(dest->location.state, "CA");
        strcpy(dest->location.postal, "92612");
        dest->location.latitude = 33.67035;
        dest->location.longitude = -117.85560;      
    }
}

void
SetupPlacesUK(NB_Place* origin, NB_Place* dest)
{
    if (origin)
    {
        origin->location.type = NB_Location_Address;
        origin->location.latitude = 51.6571;
        origin->location.longitude = -0.49309;
    }

    if (dest)
    {
        dest->location.type = NB_Location_Address;
        dest->location.latitude = 52.4737;
        dest->location.longitude =  -1.90279;
    }
}

NB_GuidanceInformation*
CreateGuidanceInformation (PAL_Instance* pal, NB_Context* context)
{
    NB_GuidanceInformationConfiguration gparams = {0};
    NB_GuidanceInformation* guidanceInfo = 0;
    NB_Error result = NE_OK;

    char*	buf = NULL;
    nb_size size = 0;

    char*	voiceBuf = NULL;
    nb_size voiceBufSize = 0;
    char voiceStyle[100] = {0};
    char*	imagesBuf = NULL;
    nb_size imagesBufSize = 0;
    char*	stepsBuf = NULL;
    nb_size stepsBufSize = 0;

    gparams.imageStyle = "routing-images";
    gparams.stepsStyle = "directions";
    gparams.pronunStyle = Test_OptionsGet()->voiceStyle;

    if ( LoadFile(pal, NULL, GUIDANCE_CONFIG_TPL_DATA, nsl_strlen(GUIDANCE_CONFIG_TPL_DATA)+1, &buf, &size) > 0)
    {
        gparams.datalibBuffer = buf;
        gparams.datalibBufferSize = size;
    }

    nsl_sprintf(voiceStyle, VOICES_TPS, Test_OptionsGet()->currentLanguage);
    if ( LoadFile(pal, NULL, voiceStyle, nsl_strlen(voiceStyle)+1, &voiceBuf, &voiceBufSize) > 0)
    {
        gparams.voiceBuffer = voiceBuf;
        gparams.voiceBufferSize = voiceBufSize;
    }


    if ( LoadFile(pal, NULL, IMAGES_TPS, nsl_strlen(IMAGES_TPS)+1, &imagesBuf, &imagesBufSize) > 0)
    {
        gparams.imagesBuffer = imagesBuf;
        gparams.imagesBufferSize = imagesBufSize;
    }

    if ( LoadFile(pal, NULL, DIRECTIONS_TPS, nsl_strlen(DIRECTIONS_TPS)+1, &stepsBuf, &stepsBufSize) > 0)
    {
        gparams.stepsBuffer = stepsBuf;
        gparams.stepsBufferSize = stepsBufSize;
    }

    result = NB_GuidanceInformationCreate(context, &gparams, &guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

    if (buf)
        nsl_free(buf);

    if (voiceBuf)
        nsl_free(voiceBuf);

    if (imagesBuf)
        nsl_free(imagesBuf);

    if (stepsBuf)
        nsl_free(stepsBuf);

    return guidanceInfo;
}

void
SetupConfigAndOptions(NB_Context* context, NB_GuidanceInformation* guidanceInfo, NB_RouteOptions* options, NB_RouteConfiguration* config)
{        
    uint32 command = 0;
     
    command = NB_GuidanceInformationGetCommandVersion(guidanceInfo);
    CU_ASSERT_NOT_EQUAL(command, 0);

    options->avoid = NB_RouteAvoid_None;
    options->transport = NB_TransportationMode_Car;
    options->type = NB_RouteType_Fastest;
    options->traffic = NB_TrafficType_None;
    options->pronunStyle = Test_OptionsGet()->voiceStyle;

    nsl_memset(config, 0, sizeof(*config));

    config->wantDestinationLocation = TRUE;
    config->wantManeuvers = TRUE;
    config->wantOriginLocation = TRUE;
    config->wantPolyline = TRUE;
    config->wantRouteMap = FALSE;
    config->commandSetVersion = command;    
    config->wantNonGuidanceManeuvers = FALSE;
}

static void 
Steps_DownloadCB(void * pUser, NB_NetworkRequestStatus status, NB_Error err_in, int percent)
{
    NB_Error err = err_in;

    if (err != NE_OK) 
    {
        // Abort and trigger event
        CU_FAIL("Callback returned error\n");
        SetCallbackCompletedEvent(g_CallbackEvent);
        return;
    }
 
    if (status == NB_NetworkRequestStatus_Progress || status == NB_NetworkRequestStatus_Success) {
        LOGOUTPUT(LogLevelLow, ("Steps Download Progress: %d%%\n", percent));
        
        if (status == NB_NetworkRequestStatus_Progress)
            goto exit;
    }

    if (status == NB_NetworkRequestStatus_Success) {
        LOGOUTPUT(LogLevelLow, ("Steps_DownloadCB::NB_NetworkRequestStatus_Success\n"));
    }
    else {

    }

    if (percent == 100) 
    {
        // Trigger main thread
        SetCallbackCompletedEvent(g_CallbackEvent);
        return;
    }

exit:
    return;
}


/*! Callback for the direction text of steps

@return NB_Error. CUnit Asserts get called on failures.
*/
static void
NavStatus_SummaryText(NB_Font font, nb_color color, const char* txt, nb_boolean newline, void* pUser)
{
    //Formatted Text will be here 

//    LOGOUTPUT(LogLevelHigh, ("Text callback: font: %d, color: %x, newline: %d, text: %s\n", font, color, newline, txt));
}

/*! Make a Direction request.

This system test connects to a server. Makes a direction request and then
outputs the results on the standard output.

@return None. CUnit Asserts get called on failures.

@see Steps_DownloadCB
*/
static void 
TestDirectionsRequest1(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    NB_RouteHandler*          routeHandler = 0;
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_Place                  pStart      = {{0}};    
    NB_Place                  pDest       = {{0}};    
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
                  
    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context)) 
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);
        
        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);
        
        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeHandler);

        if (NULL != routeParameters && NULL != routeHandler)
        {   
            if (NE_OK == result)
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest1:: NBI_DirectionsSetup Successful\n"));
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest1:: NBI_DirectionsSetup Failure\n"));
            }
        }
    }
    
    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

/*! Make a Direction request.

This system test connects to a server. Makes a direction request and then
outputs the results on the standard output.

@return None. CUnit Asserts get called on failures.

@see Steps_DownloadCB
*/
static void 
TestDirectionsRequest2(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_Place                  pStart      = {{0}};  
    NB_Place                  pDest       = {{0}}; 
    NB_Place                  pStartRet   = {{0}};
    NB_Place                  pDestRet    = {{0}};
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
    int                       refCount    = 0;
    uint32                    numManeuvers= 0;    
    
    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));
    
    if (CreatePalAndContext(&pal, &context))  
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);            
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2::Callback Successful\n"));                    
                    numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                    NB_RouteInformationGetOrigin(routeObj.route, &pStartRet);
                    NB_RouteInformationGetOrigin(routeObj.route, &pDestRet);                    
                    
                    CU_ASSERT(pDestRet.location.latitude != 0.0);
                    CU_ASSERT(pDestRet.location.longitude != 0.0);                    
                                               
                    // Output request. Empty results are ommited
                    LOGOUTPUT(LogLevelHigh, ("\nDirection Request(Start):\n"));
                    LOGOUTPUT(LogLevelHigh, ("Areaname    = %s \n"    , pStartRet.location.areaname   ));
                    LOGOUTPUT(LogLevelHigh, ("Streetnum   = %s \n"    , pStartRet.location.streetnum  ));
                    LOGOUTPUT(LogLevelHigh, ("Street1     = %s \n"    , pStartRet.location.street1    ));
                    LOGOUTPUT(LogLevelHigh, ("Street2     = %s \n"    , pStartRet.location.street2    ));
                    LOGOUTPUT(LogLevelHigh, ("City        = %s \n"    , pStartRet.location.city       ));
                    LOGOUTPUT(LogLevelHigh, ("County      = %s \n"    , pStartRet.location.county     ));
                    LOGOUTPUT(LogLevelHigh, ("State       = %s \n"    , pStartRet.location.state      ));
                    LOGOUTPUT(LogLevelHigh, ("Postal      = %s \n"    , pStartRet.location.postal     ));
                    LOGOUTPUT(LogLevelHigh, ("Country     = %s \n"    , pStartRet.location.country    ));
                    LOGOUTPUT(LogLevelHigh, ("Airport     = %s \n"    , pStartRet.location.airport    ));

                    if ((pStartRet.location.latitude != 0.0) && (pStartRet.location.longitude))
                    {
                        LOGOUTPUT(LogLevelHigh, ("Start Latitide = %f\n"      , pStartRet.location.latitude   ));
                        LOGOUTPUT(LogLevelHigh, ("Start Longitude = %f\n"     , pStartRet.location.longitude  ));
                    }

                    LOGOUTPUT(LogLevelHigh, ("Type       = %d\n", pStartRet.location.type       ));

                    // Output results. Empty results are ommited
                    LOGOUTPUT(LogLevelHigh, ("\nDirection Results(Destination):\n"));

                    LOGOUTPUT(LogLevelHigh, ("Areaname    = %s \n"   , pDestRet.location.areaname   ));
                    LOGOUTPUT(LogLevelHigh, ("Streetnum   = %s \n"   , pDestRet.location.streetnum  ));
                    LOGOUTPUT(LogLevelHigh, ("Street1     = %s \n"   , pDestRet.location.street1    ));
                    LOGOUTPUT(LogLevelHigh, ("Street2     = %s \n"   , pDestRet.location.street2    ));
                    LOGOUTPUT(LogLevelHigh, ("City        = %s \n"   , pDestRet.location.city       ));
                    LOGOUTPUT(LogLevelHigh, ("County      = %s \n"   , pDestRet.location.county     ));
                    LOGOUTPUT(LogLevelHigh, ("State       = %s \n"   , pDestRet.location.state      ));
                    LOGOUTPUT(LogLevelHigh, ("Postal      = %s \n"   , pDestRet.location.postal     ));
                    LOGOUTPUT(LogLevelHigh, ("Country     = %s \n"   , pDestRet.location.country    ));
                    LOGOUTPUT(LogLevelHigh, ("Airport     = %s \n"   , pDestRet.location.airport    ));

                    if ((pDestRet.location.latitude != 0.0) && (pDestRet.location.longitude != 0.0))
                    {
                        LOGOUTPUT(LogLevelHigh, ("Destination Latitide = %f\n"      , pDestRet.location.latitude   ));
                        LOGOUTPUT(LogLevelHigh, ("Destination Longitude = %f\n"     , pDestRet.location.longitude  ));
                    }                       
                        
                }                
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: NBI_DirectionsSetup Failure\n"));
            }
        }

        //refCount = NBI_DirectionsRelease(pDirections);
        CU_ASSERT_EQUAL(refCount, 0);
        //LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: Reference Count of Directions = %d\n", refCount));
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

static void
TestDirectionsStaticRouting(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = {0};
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_Place                  pStart = {{0}};
    NB_Place                  pDest  = {{0}};
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };
    NB_Error                  result = NE_OK;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);      // US places
        //SetupPlacesUK(&pStart, &pDest);  // UK places

        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        config.wantNonGuidanceManeuvers = TRUE;
        config.wantCountryInformation = TRUE;
        config.wantRealisticSigns = TRUE;
        config.wantJunctionModels = TRUE;
        config.wantCityModels = TRUE;
        //config.wantSpeedCameras = TRUE;

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE)
                {
                    if (routeObj.route)
                    {
                        uint32 numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);

                        LOGOUTPUT(LogLevelHigh, ("TestDirectionsStaticRoute::Callback Successful\n"));

                        GetOriginationDestination(routeObj.route);
                        GetManeuverList(routeObj.route);

                        if (numManeuvers > 0)
                        {
                            uint32 crossStreetCount = 0;
                            uint32 nman = 0;

                            for (nman = 0; nman < numManeuvers; nman++)
                            {
                                nb_boolean bIsStaticRouting = FALSE;

                                result = NB_RouteInformationGetManeuverGuidanceInformation(routeObj.route, nman, &bIsStaticRouting);
                                CU_ASSERT_EQUAL(result, NE_OK);

                                if( bIsStaticRouting )
                                {
                                    result = NB_RouteInformationGetManeuverCrossStreetsCount(routeObj.route, nman, &crossStreetCount);
                                    CU_ASSERT_EQUAL(result, NE_OK);
                                    CU_ASSERT_EQUAL(crossStreetCount, 0);
                                }
                            }
                        }
                    }
                    else
                    {
                        LOGOUTPUT(LogLevelHigh, ("TestDirectionsStaticRoute:: No Route returned\n"));
                        CU_FAIL("CallbackFailure - No route returned");
                    }
                }
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRouteSelector:: Callback Failure\n"));
                CU_FAIL("TimedOut");
            }
        }
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

static void
TestDirectionsRouteSelector(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = {0};
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_Place                  pStart      = {{0}};
    NB_Place                  pDest       = {{0}};
    NB_Place                  pStartRet   = {{0}};
    NB_Place                  pDestRet    = {{0}};
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };
    NB_Error                  result      = NE_OK;
    uint32                    i            = 0;
    uint32                    numManeuvers = 0;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);
        config.wantAlternateRoutes = TRUE;

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE)
                {
                    if (routeObj.route)
                    {
                        uint32 routesCount = 0;
                        LOGOUTPUT(LogLevelHigh, ("TestDirectionsRouteSelector::Callback Successful\n"));
                        NB_RouteInformationGetRoutesCount(routeObj.route, &routesCount);

                        for (i = 0; i < routesCount; i++)
                        {
                            result = NB_RouteInformationSetActiveRoute(routeObj.route, i + 1);
                            CU_ASSERT_EQUAL(result, NE_OK);

                            numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                            CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                            NB_RouteInformationGetOrigin(routeObj.route, &pStartRet);
                            NB_RouteInformationGetOrigin(routeObj.route, &pDestRet);

                            CU_ASSERT(pDestRet.location.latitude != 0.0);
                            CU_ASSERT(pDestRet.location.longitude != 0.0);
                        }
                    }
                    else
                    {
                        LOGOUTPUT(LogLevelHigh, ("TestDirectionsRouteSelector:: Callback Failure\n"));
                        CU_FAIL("CallbackFailure");
                    }
                }


            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRouteSelector:: Callback Failure\n"));
                CU_FAIL("TimedOut");
            }
        }
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

static void
TestDirectionsRouteSelectorWithLabelPoint(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = {0};
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_Place                  pStart      = {{0}};
    NB_Place                  pDest       = {{0}};
    NB_RouteOptions           options;
    NB_LatitudeLongitude      labelPoint  = {0};
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };
    NB_Error                  result      = NE_OK;
    uint32                    i            = 0;
    uint32                    numManeuvers = 0;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);
        config.wantAlternateRoutes = TRUE;
        config.wantLabelPoints = TRUE;

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE)
                {
                    if (routeObj.route)
                    {
                        uint32 routesCount = 0;
                        LOGOUTPUT(LogLevelHigh, ("TestDirectionsRouteSelector::Callback Successful\n"));
                        NB_RouteInformationGetRoutesCount(routeObj.route, &routesCount);

                        for (i = 0; i < routesCount; i++)
                        {
                            result = NB_RouteInformationSetActiveRoute(routeObj.route, i + 1);
                            CU_ASSERT_EQUAL(result, NE_OK);

                            numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                            CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                            result = NB_RouteInformationGetLabelPoint(routeObj.route, &labelPoint);
                            CU_ASSERT_EQUAL(result, NE_OK);

                            CU_ASSERT_NOT_EQUAL(labelPoint.latitude, 0.0);
                            CU_ASSERT_NOT_EQUAL(labelPoint.longitude, 0.0);
                        }
                    }
                    else
                    {
                        LOGOUTPUT(LogLevelHigh, ("TestDirectionsRouteSelector:: Callback Failure\n"));
                        CU_FAIL("CallbackFailure");
                    }
                }


            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRouteSelector:: Callback Failure\n"));
                CU_FAIL("TimedOut");
            }
        }
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

/*! Make a Direction request.

This system test connects to a server. Makes a direction request and then
outputs the results on the standard output.

@return None. CUnit Asserts get called on failures.

@see Steps_DownloadCB
*/
static void 
TestDirectionsManeuverInfo(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_Place                  pStart      = {{0}};    
    NB_Place                  pDest       = {{0}};       
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
    int                       refCount    = 0;
    uint32                    numManeuvers= 0;    
    uint32 nIndex = 0;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);            
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2::Callback Successful\n"));                    
                    numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                    for(nIndex = 0; nIndex < numManeuvers; nIndex++)
                    {
                        #define TEXT_SIZE 128
                        double		            latitude            = 0.0;
                        double		            longitude           = 0.0;
                        double                  heading             = 0.0;          
                        char                    primary[TEXT_SIZE]  = {0};
                        char                    secondary[TEXT_SIZE]= {0};                      
                        char                    current[TEXT_SIZE]  = {0};      
                        double                  distance            = 0.0;
                        //NB_ManeuverTrafficInfo traffic              = {0};

                        NB_RouteInformationGetTurnInformation(routeObj.route, nIndex, NULL, primary, TEXT_SIZE, secondary, TEXT_SIZE, current, TEXT_SIZE, &distance, 0);

                        /*
                        result = NBI_DirectionsGetManueverInfo(pDirections, 
                            nIndex, 
                            &latitude, 
                            &longitude, 
                            &heading,
                            primary, 
                            TEXT_SIZE, 
                            secondary, 
                            TEXT_SIZE, 
                            current, 
                            TEXT_SIZE, 
                            &distance, 
                            &traffic);
                        CU_ASSERT_EQUAL(result, NE_OK);
                        */

                        LOGOUTPUT(LogLevelHigh, ("Maneuver Info\n"));
                        LOGOUTPUT(LogLevelHigh, ("Lat: %f, Long: %f, Heading: %f, Distance: %f\n", latitude, longitude, heading, distance));
                        LOGOUTPUT(LogLevelHigh, ("Primary: %s\n", primary));
                        LOGOUTPUT(LogLevelHigh, ("Secondary: %s\n", secondary));
                        LOGOUTPUT(LogLevelHigh, ("Current: %s\n", current));
                    }

                    //numManeuvers = 2; //Current Maneuver is 2
                    //bRet = NBI_DirectionsGetManeuverDisplay(pDirections, numManeuvers, &dispm, NULL);
                    //CU_ASSERT_EQUAL(bRet, TRUE);
                    //CU_ASSERT_NOT_EQUAL(numManeuvers, numManeuvers+1);
                 }   
                         
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: NBI_DirectionsSetup Failure\n"));
            }
        }

        //refCount = NBI_DirectionsRelease(pDirections);
        CU_ASSERT_EQUAL(refCount, 0);
        //LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: Reference Count of Directions = %d\n", refCount));
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

/*! Make a Direction request.

This system test connects to a server. Makes a direction request and then
outputs the results on the standard output.

@return None. CUnit Asserts get called on failures.

@see DirectionsRequest
*/
static void 
TestDirectionsRequest4(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_Place                  pStart      = {{0}};    
    NB_Place                  pDest       = {{0}};   
    NB_Place                  pStartRet   = {{0}};
    NB_Place                  pDestRet    = {{0}};
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
    int                       refCount    = 0;
    uint32                    numManeuvers= 0;    
    uint32 i=0;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);            
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2::Callback Successful\n"));                    
                    numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);
                    
                    for (i=0; i<numManeuvers; i++)
                    {
                        result = NB_RouteInformationEnumerateManeuverText(routeObj.route, guidanceInfo, NB_NTT_Primary, NB_NAU_Miles, i, NavStatus_SummaryText, routeObj.route);
                        CU_ASSERT_EQUAL(result, NE_OK);
                    }

                    NB_RouteInformationGetOrigin(routeObj.route, &pStartRet);
                    NB_RouteInformationGetOrigin(routeObj.route, &pDestRet);                    

                    CU_ASSERT(pDestRet.location.latitude != 0.0);
                    CU_ASSERT(pDestRet.location.longitude != 0.0);                    

                    // Output request. Empty results are ommited
                    LOGOUTPUT(LogLevelHigh, ("\nDirection Request(Start):\n"));
                    LOGOUTPUT(LogLevelHigh, ("Areaname    = %s \n"    , pStartRet.location.areaname   ));
                    LOGOUTPUT(LogLevelHigh, ("Streetnum   = %s \n"    , pStartRet.location.streetnum  ));
                    LOGOUTPUT(LogLevelHigh, ("Street1     = %s \n"    , pStartRet.location.street1    ));
                    LOGOUTPUT(LogLevelHigh, ("Street2     = %s \n"    , pStartRet.location.street2    ));
                    LOGOUTPUT(LogLevelHigh, ("City        = %s \n"    , pStartRet.location.city       ));
                    LOGOUTPUT(LogLevelHigh, ("County      = %s \n"    , pStartRet.location.county     ));
                    LOGOUTPUT(LogLevelHigh, ("State       = %s \n"    , pStartRet.location.state      ));
                    LOGOUTPUT(LogLevelHigh, ("Postal      = %s \n"    , pStartRet.location.postal     ));
                    LOGOUTPUT(LogLevelHigh, ("Country     = %s \n"    , pStartRet.location.country    ));
                    LOGOUTPUT(LogLevelHigh, ("Airport     = %s \n"    , pStartRet.location.airport    ));

                    if ((pStartRet.location.latitude != 0.0) && (pStartRet.location.longitude))
                    {
                        LOGOUTPUT(LogLevelHigh, ("Start Latitide = %f\n"      , pStartRet.location.latitude   ));
                        LOGOUTPUT(LogLevelHigh, ("Start Longitude = %f\n"     , pStartRet.location.longitude  ));
                    }

                    LOGOUTPUT(LogLevelHigh, ("Type       = %d\n", pStartRet.location.type       ));

                    // Output results. Empty results are ommited
                    LOGOUTPUT(LogLevelHigh, ("\nDirection Results(Destination):\n"));

                    LOGOUTPUT(LogLevelHigh, ("Areaname    = %s \n"   , pDestRet.location.areaname   ));
                    LOGOUTPUT(LogLevelHigh, ("Streetnum   = %s \n"   , pDestRet.location.streetnum  ));
                    LOGOUTPUT(LogLevelHigh, ("Street1     = %s \n"   , pDestRet.location.street1    ));
                    LOGOUTPUT(LogLevelHigh, ("Street2     = %s \n"   , pDestRet.location.street2    ));
                    LOGOUTPUT(LogLevelHigh, ("City        = %s \n"   , pDestRet.location.city       ));
                    LOGOUTPUT(LogLevelHigh, ("County      = %s \n"   , pDestRet.location.county     ));
                    LOGOUTPUT(LogLevelHigh, ("State       = %s \n"   , pDestRet.location.state      ));
                    LOGOUTPUT(LogLevelHigh, ("Postal      = %s \n"   , pDestRet.location.postal     ));
                    LOGOUTPUT(LogLevelHigh, ("Country     = %s \n"   , pDestRet.location.country    ));
                    LOGOUTPUT(LogLevelHigh, ("Airport     = %s \n"   , pDestRet.location.airport    ));

                    if ((pDestRet.location.latitude != 0.0) && (pDestRet.location.longitude != 0.0))
                    {
                        LOGOUTPUT(LogLevelHigh, ("Destination Latitide = %f\n"      , pDestRet.location.latitude   ));
                        LOGOUTPUT(LogLevelHigh, ("Destination Longitude = %f\n"     , pDestRet.location.longitude  ));
                    }                       

                }                
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: NBI_DirectionsSetup Failure\n"));
            }
        }

        //refCount = NBI_DirectionsRelease(pDirections);
        CU_ASSERT_EQUAL(refCount, 0);
        //LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: Reference Count of Directions = %d\n", refCount));
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

/*! Make a Direction request.

This system test connects to a server. Makes a direction request and then
outputs the results on the standard output.

@return None. CUnit Asserts get called on failures.

@see Steps_DownloadCB
*/
static void 
TestDirectionsRequest5(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_Place                  pStart      = {{0}};    
    NB_Place                  pDest       = {{0}};       
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
    int                       refCount    = 0;
    uint32                    numManeuvers= 0;    

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);            
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2::Callback Successful\n"));                    
                    numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                    /*
                    result = NBI_DirectionsGetRouteBounds(pDirections, NAV_MANEUVER_NONE, NAV_MANEUVER_NONE, &slat, &slat2, &slon, &slon2);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    LOGOUTPUT(LogLevelHigh, ("DirectionsRequest::Lat = %f\n", slat));
                    LOGOUTPUT(LogLevelHigh, ("DirectionsRequest::Lon = %f\n", slon));
                    LOGOUTPUT(LogLevelHigh, ("DirectionsRequest::Lat2 = %f\n", slat2));
                    LOGOUTPUT(LogLevelHigh, ("DirectionsRequest::Lon2 = %f\n", slon2));

                    rResult = NBI_DirectionsGetRouteError(pDirections);
                    */

                }                
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: NBI_DirectionsSetup Failure\n"));
            }
        }

        //refCount = NBI_DirectionsRelease(pDirections);
        CU_ASSERT_EQUAL(refCount, 0);
        //LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: Reference Count of Directions = %d\n", refCount));
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test getting the image turn code

    @return None. CUnit Asserts get called on failures.
*/
static void 
TestDirectionsGetImageCode(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_Place                  pStart      = {{0}};    
    NB_Place                  pDest       = {{0}};       
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
    int                       refCount    = 0;
    uint32                    numManeuvers= 0;    
    uint32                    i           = 0;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);            
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2::Callback Successful\n"));                    
                    numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                    LOGOUTPUT(LogLevelHigh, ("Maneuver Count: %d\n", numManeuvers));
                    // For all maneuvers
                    for (i = 0; i < numManeuvers; i++)
                    {
                        #define IMAGE_LENGTH 40
                        char imageCode[IMAGE_LENGTH] = {0};

                        result = NB_RouteInformationGetTurnImage(routeObj.route, guidanceInfo, i, NB_NMP_Turn, NB_NAU_Miles, imageCode, IMAGE_LENGTH);

                        CU_ASSERT_EQUAL(result, NE_OK);
                        CU_ASSERT(nsl_strlen(imageCode) > 0);
                        if (result == NE_OK)
                        {
                            LOGOUTPUT(LogLevelHigh, ("Image Code[%d]: %s\n", i, imageCode));
                        }
                    }
                }      
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: NBI_DirectionsSetup Failure\n"));
            }
        }

        //refCount = NBI_DirectionsRelease(pDirections);
        CU_ASSERT_EQUAL(refCount, 0);
        //LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: Reference Count of Directions = %d\n", refCount));
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test getting the image turn codes with left driving side

    @return None. CUnit Asserts get called on failures.
*/
static void 
TestDirectionsGetImageCodeLeftDrivingSide(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_Place                  origin      = {{0}};    
    NB_Place                  dest        = {{0}};       
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
    uint32                    numManeuvers= 0;
    uint32                    i           = 0;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        origin.location.type = NB_Location_Address;
        strcpy(origin.location.streetnum, "3");
        strcpy(origin.location.street1, "Rushes Rd");
        strcpy(origin.location.city, "Petersfield");
        strcpy(origin.location.state, "Hampshire");
        strcpy(origin.location.postal, "GU32 2");
        strcpy(origin.location.country, "UK");
        origin.location.latitude = 51.008461;
        origin.location.longitude = -0.945743;

        dest.location.type = NB_Location_Address;
        strcpy(dest.location.streetnum, "16");
        strcpy(dest.location.street1, "Kimbers");
        strcpy(dest.location.city, "Petersfield");
        strcpy(dest.location.state, "Hampshire");
        strcpy(dest.location.postal, "GU32 2");
        strcpy(dest.location.country, "UK");
        dest.location.latitude = 51.0091;
        dest.location.longitude = -0.941504;

        SetupConfigAndOptions(context, guidanceInfo, &options, &config);
        config.wantCountryInformation = TRUE;
        config.wantEnterCountryManeuvers = TRUE;
        config.wantEnterRoundaboutManeuvers = TRUE;

        result = NB_RouteParametersCreateFromPlace(context, &origin, &dest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);            
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2::Callback Successful\n"));                    
                    numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                    LOGOUTPUT(LogLevelHigh, ("Maneuver Count: %d\n", numManeuvers));
                    // For all maneuvers
                    for (i = 0; i < numManeuvers; i++)
                    {
                        #define IMAGE_LENGTH 40
                        const char* countryCode = 0;
                        char imageCode[IMAGE_LENGTH] = {0};

                        result = NB_RouteInformationGetTurnCountryCode(routeObj.route, i, &countryCode);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        result = NB_RouteInformationGetTurnImage(routeObj.route, guidanceInfo, i, NB_NMP_Turn, NB_NAU_Miles, imageCode, IMAGE_LENGTH);

                        CU_ASSERT_EQUAL(result, NE_OK);
                        CU_ASSERT(nsl_strlen(imageCode) > 0);
                        if (result == NE_OK)
                        {
                            LOGOUTPUT(LogLevelHigh, ("Image Code[%d]: %s\n", i, imageCode));
                        }
                    }
                }
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsGetImageCodeLeftDrivingSide Failure\n"));
            }
        }
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test announce functionality

    @return None. CUnit Asserts get called on failures.
*/
static void 
TestDirectionsAnnounce(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_Place                  start      = {{0}};    
    NB_Place                  dest       = {{0}};       
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
    uint32                    numManeuvers= 0;
    uint32                    i           = 0;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        // Request route beyond route window to test switch to 
        // short form when pronun file key is not provided
        start.location.type = NB_Location_Address;
        strcpy(start.location.streetnum, "6");
        strcpy(start.location.street1, "Liberty");
        strcpy(start.location.city, "Aliso Viejo");
        strcpy(start.location.state, "CA");
        strcpy(start.location.postal, "92656");
        start.location.latitude = 33.55961648;
        start.location.longitude = -117.7295304;

        dest.location.type = NB_Location_Address;
        strcpy(dest.location.streetnum, "601");
        strcpy(dest.location.street1, "Firestone Road");
        strcpy(dest.location.city, "Santa Barbara");
        strcpy(dest.location.state, "CA");
        strcpy(dest.location.postal, "93117");
        dest.location.latitude = 34.42611;
        dest.location.longitude = -119.84028;

        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        // Request first five pronun files to test switch to
        // short form  when pronun file is not in voice cache
        config.maxPronunFiles = 5;

        result = NB_RouteParametersCreateFromPlace(context, &start, &dest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                    LOGOUTPUT(LogLevelHigh, ("Maneuver Count: %d\n", numManeuvers));

                    // For all maneuvers
                    for (i = 0; i < numManeuvers; i++)
                    {
                        NB_GuidanceMessage* message = NULL;
                        const char* code = NULL;
                        int num = 0;

                        result = NB_RouteInformationGetTurnCountryCode(routeObj.route, i, &code);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        result = NB_RouteInformationGetTurnAnnouncement(routeObj.route, guidanceInfo, i, NB_NMP_Turn, NB_NAS_Lookahead, NB_NAT_Street, NB_NAU_Miles, &message);
                        CU_ASSERT_EQUAL(result, NE_OK);
                        CU_ASSERT_PTR_NOT_NULL(message);

                        if (result == NE_OK && message)
                        {
                            for (num = 0; num < NB_GuidanceMessageGetCodeCount(message); num++)
                            {
                                code = NB_GuidanceMessageGetCode(message, num);

                                CU_ASSERT_PTR_NOT_NULL(code);
                                if (code)
                                {
                                    CU_ASSERT(nsl_strlen(code) > 0);
                                    LOGOUTPUT(LogLevelHigh, ("%d: Voice Code[%d]: %s\n", i, num, code));
                                }
                            }

                            NB_GuidanceMessageDestroy(message);
                        }
                    }
                }      
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsAnnounce Failure\n"));
            }
        }
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test polyline route window

    @return None. CUnit Asserts get called on failures.
*/
static void 
TestDirectionsPolylineRouteWindow(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_Place                  start      = {{0}};    
    NB_Place                  dest       = {{0}};       
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
    uint32                    numManeuvers= 0;
    uint32                    i           = 0;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        // Request route beyond polyline route window to test NB_RouteInformationIsManeuverWithinRouteWindow
        start.location.type = NB_Location_Address;
        strcpy(start.location.streetnum, "6");
        strcpy(start.location.street1, "Liberty");
        strcpy(start.location.city, "Aliso Viejo");
        strcpy(start.location.state, "CA");
        strcpy(start.location.postal, "92656");
        start.location.latitude = 33.55961648;
        start.location.longitude = -117.7295304;

        dest.location.type = NB_Location_Address;
        strcpy(dest.location.streetnum, "");
        strcpy(dest.location.street1, "");
        strcpy(dest.location.city, "Chicago");
        strcpy(dest.location.state, "IL");
        strcpy(dest.location.postal, "60666");
        dest.location.latitude = 41.9786;
        dest.location.longitude = -87.9048;

        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        result = NB_RouteParametersCreateFromPlace(context, &start, &dest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    double totalDistance = 0;

                    numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                    LOGOUTPUT(LogLevelHigh, ("Maneuver Count: %d\n", numManeuvers));

                    // For all maneuvers
                    for (i = 0; i < numManeuvers; i++)
                    {
                        #define TEST_BUFFER_SIZE 40

                        char primaryNameBuffer[TEST_BUFFER_SIZE];
                        char secondaryNameBuffer[TEST_BUFFER_SIZE];
                        char currentNameBuffer[TEST_BUFFER_SIZE];

                        NB_LatitudeLongitude turnPoint = { 0 };
                        double distance = 0;

                        nb_boolean inRouteWindow = NB_RouteInformationIsManeuverWithinRouteWindow(routeObj.route, i);

                        result = NB_RouteInformationGetTurnInformation(
                            routeObj.route,
                            i,
                            &turnPoint,
                            primaryNameBuffer, sizeof(primaryNameBuffer),
                            secondaryNameBuffer, sizeof(secondaryNameBuffer),
                            currentNameBuffer, sizeof(currentNameBuffer),
                            &distance,
                            FALSE);

                        totalDistance += distance;

                        LOGOUTPUT(LogLevelHigh, ("Maneuver %d: route window: %d, %9.1f: %s to %s\n",
                                    i, inRouteWindow, totalDistance, currentNameBuffer, primaryNameBuffer));
                    }
                }
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsPolylineRouteWindow Failure\n"));
            }
        }
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test getting maneuver turn map center points for look ahead

    @return None. CUnit Asserts get called on failures.
*/
static void 
TestDirectionsManeuverTurnMapCenterPoint(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_Place                  pStart      = {{0}};    
    NB_Place                  pDest       = {{0}};       
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
    int                       refCount    = 0;
    uint32                    numManeuvers= 0;    
    uint32                    i           = 0;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                    LOGOUTPUT(LogLevelHigh, ("Maneuver Count: %d\n", numManeuvers));

                    // For all maneuvers
                    for (i = 0; i < numManeuvers; i++)
                    {
                        NB_LatitudeLongitude centerLatLon = { 0 };
                        double heading = 0;

                        result = NB_RouteInformationGetManeuverTurnMapCenterPoint(routeObj.route, i, &centerLatLon, &heading);

                        CU_ASSERT_EQUAL(result, NE_OK);
                        if (result == NE_OK)
                        {
                            LOGOUTPUT(LogLevelHigh, ("Maneuver %d: center lat/lon: (%f, %f)  heading %f\n",
                                    i, centerLatLon.latitude, centerLatLon.longitude, heading));
                        }
                    }
                }      
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsManeuverTurnMapCenterPoint failure\n"));
            }
        }

        CU_ASSERT_EQUAL(refCount, 0);
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

#if MOVE_THIS_TO_UNIT_TEST

/*! Test realistic signs and junctions in route reply

    @return None. CUnit Asserts get called on failures.
*/
static void 
TestDirectionsRequestSignsAndJunctions(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_Place                  start      = {{0}};    
    NB_Place                  dest       = {{0}};       
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
    uint32                    numManeuvers= 0;
    uint32                    i           = 0;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        // TODO: enhanced content not working with default nav,global servlet
        NB_TargetMapping targets[] = {{ "nav", "nav,usa" }};
        result = NB_ContextSetTargetMappings(context, targets, sizeof(targets) / sizeof(targets[0]));

        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        start.location.type = NB_Location_Address;
        strcpy(start.location.streetnum, "6");
        strcpy(start.location.street1, "Liberty");
        strcpy(start.location.city, "Aliso Viejo");
        strcpy(start.location.state, "CA");
        strcpy(start.location.postal, "92656");
        start.location.latitude = 33.55961648;
        start.location.longitude = -117.7295304;

        dest.location.type = NB_Location_Address;
        strcpy(dest.location.streetnum, "");
        strcpy(dest.location.street1, "");
        strcpy(dest.location.city, "Chicago");
        strcpy(dest.location.state, "IL");
        strcpy(dest.location.postal, "60666");
        dest.location.latitude = 41.9786;
        dest.location.longitude = -87.9048;

        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        config.wantRealisticSigns = TRUE;
        config.wantJunctionModels = TRUE;
        config.wantCityModels = TRUE;

        result = NB_RouteParametersCreateFromPlace(context, &start, &dest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    uint32 contentRegionCount = 0;
                    uint32 contentRegionIndex = 0;

                    numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                    LOGOUTPUT(LogLevelHigh, ("Maneuver Count: %d\n", numManeuvers));

                    // For all maneuvers
                    for (i = 0; i < numManeuvers; i++)
                    {
                        #define TEST_BUFFER_SIZE 40

                        char primaryNameBuffer[TEST_BUFFER_SIZE];
                        char secondaryNameBuffer[TEST_BUFFER_SIZE];
                        char currentNameBuffer[TEST_BUFFER_SIZE];

                        NB_LatitudeLongitude turnPoint = { 0 };
                        double distance = 0;
                        uint32 signCount = 0;
                        uint32 signIndex = 0;

                        result = NB_RouteInformationGetTurnInformation(
                            routeObj.route,
                            i,
                            &turnPoint,
                            primaryNameBuffer, sizeof(primaryNameBuffer),
                            secondaryNameBuffer, sizeof(secondaryNameBuffer),
                            currentNameBuffer, sizeof(currentNameBuffer),
                            &distance,
                            FALSE);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        result = NB_RouteInformationGetManeuverRealisticSignCount(routeObj.route, i, &signCount);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        LOGOUTPUT(LogLevelHigh, ("Maneuver %d: %d signs, %s to %s\n", i, signCount, currentNameBuffer, primaryNameBuffer));
                        for (signIndex = 0; signIndex < signCount; signIndex++)
                        {
                            NB_RouteRealisticSign sign = { 0 };

                            result = NB_RouteInformationGetManeuverRealisticSign(routeObj.route, i, signIndex, &sign);
                            CU_ASSERT_EQUAL(result, NE_OK);

                            LOGOUTPUT(LogLevelHigh, ("    Sign %d: id '%s' portrait '%s' type '%s' dataset '%s' version '%s' pos %f\n",
                                        signIndex, sign.signId, sign.signPortraitId, sign.signFileType, sign.datasetId, sign.version, sign.positionMeters));
                        }
                    }

                    // For all content regions
                    contentRegionCount = NB_RouteInformationGetContentRegionCount(routeObj.route);
                    CU_ASSERT_EQUAL(result, NE_OK);

                    LOGOUTPUT(LogLevelHigh, ("\nContent regions: %d\n", contentRegionCount));
                    for (contentRegionIndex = 0; contentRegionIndex < contentRegionCount; contentRegionIndex++)
                    {
                        NB_RouteContentRegion contentRegion = { 0 };
                        result = NB_RouteInformationGetContentRegion(routeObj.route, contentRegionIndex, &contentRegion);
                        CU_ASSERT_EQUAL(result, NE_OK);
                        LOGOUTPUT(LogLevelHigh, ("    Region %d: type %d id '%s' dataset '%s' version '%s' start %d (%f) end %d (%f)\n",
                            contentRegionIndex, contentRegion.type, contentRegion.regionId, contentRegion.datasetId, contentRegion.version, 
                            contentRegion.startManeuver, contentRegion.startManeuverOffset, contentRegion.endManeuver, contentRegion.endManeuverOffset));
                    }
                }
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequestEnhancedContent Failure\n"));
            }
        }
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Test city model in route reply

    @return None. CUnit Asserts get called on failures.
*/
static void 
TestDirectionsRequestCityModel(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_Place                  start      = {{0}};    
    NB_Place                  dest       = {{0}};       
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        // TODO: enhanced content not working with default nav,global servlet
        NB_TargetMapping targets[] = {{ "nav", "nav,usa" }};
        result = NB_ContextSetTargetMappings(context, targets, sizeof(targets) / sizeof(targets[0]));

        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        start.location.type = NB_Location_LatLon;
        strcpy(start.location.streetnum, "");
        strcpy(start.location.street1, "");
        strcpy(start.location.city, "Los Angeles");
        strcpy(start.location.state, "CA");
        strcpy(start.location.postal, "");
        start.location.latitude = 34.0543824434;
        start.location.longitude = -118.253499269;

        dest.location.type = NB_Location_LatLon;
        strcpy(dest.location.streetnum, "");
        strcpy(dest.location.street1, "");
        strcpy(dest.location.city, "Los Angeles");
        strcpy(dest.location.state, "CA");
        strcpy(dest.location.postal, "");
        dest.location.latitude = 34.043546319;
        dest.location.longitude = -118.26533854;

        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        config.wantCityModels = TRUE;

        result = NB_RouteParametersCreateFromPlace(context, &start, &dest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    uint32 contentRegionCount = 0;
                    uint32 contentRegionIndex = 0;

                    // For all content regions
                    contentRegionCount = NB_RouteInformationGetContentRegionCount(routeObj.route);
                    CU_ASSERT_EQUAL(result, NE_OK);

                    LOGOUTPUT(LogLevelHigh, ("\nContent regions: %d\n", contentRegionCount));
                    for (contentRegionIndex = 0; contentRegionIndex < contentRegionCount; contentRegionIndex++)
                    {
                        NB_RouteContentRegion contentRegion = { 0 };
                        result = NB_RouteInformationGetContentRegion(routeObj.route, contentRegionIndex, &contentRegion);
                        CU_ASSERT_EQUAL(result, NE_OK);
                        LOGOUTPUT(LogLevelHigh, ("    Region %d: type %d id '%s' dataset '%s' version '%s' start %d (%f) end %d (%f)\n",
                            contentRegionIndex, contentRegion.type, contentRegion.regionId, contentRegion.datasetId, contentRegion.version, 
                            contentRegion.startManeuver, contentRegion.startManeuverOffset, contentRegion.endManeuver, contentRegion.endManeuverOffset));
                    }
                }
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequestEnhancedContent Failure\n"));
            }
        }
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}
#endif

void GuidanceInformationTextCallback(NB_Font font, nb_color color, const char* text, nb_boolean newline, void* userData)
{
    LOGOUTPUT(LogLevelHigh, (text));
    if (newline)
    {
        LOGOUTPUT(LogLevelHigh, ("\n"));
    }
}

/*! Test enumerate text functionality

    @return None. CUnit Asserts get called on failures.
*/
static void 
TestDirectionsTextEnumerate(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_RouteParameters*       routeParameters = 0;
    TestRouteObj              routeObj = { 0 };
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_Place                  pStart      = {{0}};
    NB_Place                  pDest       = {{0}};
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };    
    NB_Error                  result      = NE_OK;
    uint32                    numManeuvers= 0;    
    uint32 nIndex = 0;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        if (NULL != routeParameters && NULL != routeObj.routeHandler)
        {
            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);            
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {                
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                {
                    LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2::Callback Successful\n"));                    
                    numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
                    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

                    for(nIndex = 0; nIndex < numManeuvers; nIndex++)
                    {
                        NB_RouteInformationEnumerateManeuverText(routeObj.route, guidanceInfo, NB_NTT_Primary, NB_NAU_Miles, nIndex, GuidanceInformationTextCallback, NULL);
                        LOGOUTPUT(LogLevelHigh, ("\n"));

                    }
                    NB_RouteInformationEnumerateArrivalText(routeObj.route, guidanceInfo, NB_NTT_Primary, NB_NAU_Miles, GuidanceInformationTextCallback, NULL);
                 }
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: NBI_DirectionsSetup Failure\n"));
            }
        }
    }

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


void 
DownloadRouteCallbackNew(
                         void* handler,              /*!< Handler invoking the callback */
                         NB_NetworkRequestStatus status,    /*!< Status of download request */
                         NB_Error err,               /*!< error state */
                         uint8 up,                   /*!< Non-zero is query upload, zero is query download */
                         int percent,                /*!< Download progress */
                         void* pUserData)            /*!< Optional user data provided */
{
    if (!up)
    {        
        DownloadRouteCallback(pUserData, status, err, percent);
    }
}


void 
DownloadRouteCallback(void* pUserData,            /*!< Optional user data provided */
                      NB_NetworkRequestStatus status,    /*!< Status of download request */
                      NB_Error err,               /*!< error state */
                      int percent)                /*!< Download progress */
{
    TestRouteObj* routeObj = (TestRouteObj*) pUserData;
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
        {
            NB_RouteHandlerGetRouteInformation(routeObj->routeHandler, &routeObj->route);  
        }                  
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
    return;
}

static NB_Error GetOriginationDestination(NB_RouteInformation* route)
{
    NB_Error  result = NE_INVAL;
    NB_Place  origination;
    NB_Place  destination;

    if( route == NULL )
    {
        return result;
    }

    nsl_memset(&origination, 0, sizeof(NB_Place));
    nsl_memset(&destination, 0, sizeof(NB_Place));

    result = NB_RouteInformationGetOrigin(route, &origination);
    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT(origination.location.latitude != 0.0);
    CU_ASSERT(origination.location.longitude != 0.0);

    result = NB_RouteInformationGetDestination(route, &destination);
    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT(destination.location.latitude != 0.0);
    CU_ASSERT(destination.location.longitude != 0.0);

    LOGOUTPUT(LogLevelHigh, ("\n origination - areaname   = %s", origination.location.areaname));
    LOGOUTPUT(LogLevelHigh, ("\n               streetnum  = %s", origination.location.streetnum));
    LOGOUTPUT(LogLevelHigh, ("\n               streetaddr = %s", origination.location.street1));
    LOGOUTPUT(LogLevelHigh, ("\n               city       = %s", origination.location.city));
    LOGOUTPUT(LogLevelHigh, ("\n               county     = %s",origination.location.county));
    LOGOUTPUT(LogLevelHigh, ("\n               state      = %s", origination.location.state));
    LOGOUTPUT(LogLevelHigh, ("\n               postal     = %s", origination.location.postal));
    LOGOUTPUT(LogLevelHigh, ("\n               country    = %s", origination.location.country));
    LOGOUTPUT(LogLevelHigh, ("\n               airport    = %s", origination.location.airport));
    LOGOUTPUT(LogLevelHigh, ("\n               freeform   = %s", origination.location.freeform));
    LOGOUTPUT(LogLevelHigh, ("\n               type       = %d", origination.location.type));
    LOGOUTPUT(LogLevelHigh, ("\n               lat        = %f", origination.location.latitude));
    LOGOUTPUT(LogLevelHigh, ("\n               lon        = %f\n", origination.location.longitude));

    LOGOUTPUT(LogLevelHigh, ("\n destination - areaname   = %s", destination.location.areaname));
    LOGOUTPUT(LogLevelHigh, ("\n               streetnum  = %s", destination.location.streetnum));
    LOGOUTPUT(LogLevelHigh, ("\n               streetaddr = %s", destination.location.street1));
    LOGOUTPUT(LogLevelHigh, ("\n               city       = %s", destination.location.city));
    LOGOUTPUT(LogLevelHigh, ("\n               county     = %s",destination.location.county));
    LOGOUTPUT(LogLevelHigh, ("\n               state      = %s", destination.location.state));
    LOGOUTPUT(LogLevelHigh, ("\n               postal     = %s", destination.location.postal));
    LOGOUTPUT(LogLevelHigh, ("\n               country    = %s", destination.location.country));
    LOGOUTPUT(LogLevelHigh, ("\n               airport    = %s", destination.location.airport));
    LOGOUTPUT(LogLevelHigh, ("\n               freeform   = %s", destination.location.freeform));
    LOGOUTPUT(LogLevelHigh, ("\n               type       = %d", destination.location.type));
    LOGOUTPUT(LogLevelHigh, ("\n               lat        = %f", destination.location.latitude));
    LOGOUTPUT(LogLevelHigh, ("\n               lon        = %f\n", destination.location.longitude));

    return result;
}

static NB_Error GetManeuverList(NB_RouteInformation* route)
{
    NB_Error  result = NE_INVAL;
    uint32    routeTime = 0;
    uint32    routeDelay = 0;
    double    routeDistance = 0.0;
    uint32    nIndex = 0;
    uint32    numManeuvers = 0;

    if( route == NULL )
    {
        return result;
    }

    numManeuvers = NB_RouteInformationGetManeuverCount(route);
    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

    LOGOUTPUT(LogLevelHigh, ("   ===== Number of Maneuver: %d ======\n\n", numManeuvers));

    result = NB_RouteInformationGetSummaryInformation(route, &routeTime, &routeDelay, &routeDistance);
    CU_ASSERT_EQUAL(result, NE_OK);
    if( NE_OK == result )
    {
        LOGOUTPUT(LogLevelHigh, ("   Route Length:      %f [%f miles]\n", routeDistance, routeDistance/1600.0));
        LOGOUTPUT(LogLevelHigh, ("   Route Travel Time: %u [%u h %u m]\n", routeTime, routeTime/3600, (routeTime%3600)/60));
        LOGOUTPUT(LogLevelHigh, ("   Route Delay  Time: %u\n", routeDelay));
    }

    for (nIndex = 0; nIndex < numManeuvers; nIndex++)
    {
        #define TEXT_SIZE 128
        char                 primary[TEXT_SIZE]  = {0};
        char                 secondary[TEXT_SIZE]= {0};
        char                 current[TEXT_SIZE]  = {0};
        double               distance            = 0.0;
        NB_LatitudeLongitude centerLatLon        = { 0 };
        nb_boolean           guidanceDisabled    = FALSE;

        result = NB_RouteInformationGetTurnInformation(route, nIndex, &centerLatLon, primary, TEXT_SIZE, secondary, TEXT_SIZE, current, TEXT_SIZE, &distance, 0);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_RouteInformationGetManeuverGuidanceInformation(route, nIndex, &guidanceDisabled);
        CU_ASSERT_EQUAL(result, NE_OK);

        if( result == NE_OK && guidanceDisabled )
        {
            LOGOUTPUT(LogLevelHigh, ("\n Maneuver [%d] Turn Info - Guidance is disabled \n", nIndex));
        }
        else
        {
            LOGOUTPUT(LogLevelHigh, ("\n Maneuver [%d] Turn Info \n", nIndex));
        }
        LOGOUTPUT(LogLevelHigh, ("   lon/lat (%f, %f) Distance: %f miles \n",
                                 centerLatLon.longitude, centerLatLon.latitude, distance/1600.0));

        LOGOUTPUT(LogLevelHigh, ("   Primary:   %s\n", primary));
        LOGOUTPUT(LogLevelHigh, ("   Secondary: %s\n", secondary));
        LOGOUTPUT(LogLevelHigh, ("   Current:   %s\n", current));
    }

    return result;
}

/*! @} */

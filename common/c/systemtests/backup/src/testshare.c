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

@file     testshare.c
@date     03/19/2009
@defgroup testshare_h System tests for AB_Share functions

*/
/*
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

#include "testshare.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"

#include "nbcontext.h"
#include "absharehandler.h"

// Local Functions ...........................................................

// All tests. Add all tests to the TestPlaceMessage_AddTests function
#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
static void TestShareSendPlaceMessage(void);
#endif

static void TestShareSendPlaceMessageNew(void);
static void TestShareSendPlaceMessageWithBannerId(void);
static void TestShareSendShareMessageToFacebookWithAddress(void);
static void TestShareSendShareMessageToFacebookWithAirport(void);
static void TestShareSendShareMessageToFacebookWithLatLon(void);
static void TestShareSendTextMessage(void);

// Subfunctions of tests
#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
static void DownloadCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);
#endif

static void SendPlaceMessage(const char* fromMdn, const char* fromName, const char* message, const char** to, int toCount, const NB_Place* place, const char* bannerId);
static void SendShareMessage(const char* fromMdn, const char* fromName, const char* message, const char** to, int toCount, const NB_Place* place);
static void SendTextMessage(const char* fromMdn, const char* fromName, const char* message, const char** to, int toCount);

static void SendMessageCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
static void LogSendMessageResults(AB_ShareInformation* info, const char* testName, const char** expectedRecipients, uint32 expectedRecipientCount);

// Constants .................................................................


// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;


/*! Add all your test functions here

@return None
*/
void 
TestPlaceMessage_AddTests( CU_pSuite pTestSuite, int level )
{
    // ! Add all your function names here !
#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestShareSendPlaceMessage", TestShareSendPlaceMessage);
#endif
    
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestShareSendPlaceMessageNew", TestShareSendPlaceMessageNew);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestShareSendPlaceMessageWithBannerId", TestShareSendPlaceMessageWithBannerId);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestShareSendShareMessageToFacebookWithAddress", TestShareSendShareMessageToFacebookWithAddress);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestShareSendShareMessageToFacebookWithAirport", TestShareSendShareMessageToFacebookWithAirport);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestShareSendShareMessageToFacebookWithLatLon", TestShareSendShareMessageToFacebookWithLatLon);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestShareSendTextMessage", TestShareSendTextMessage);
};


/*! Add common initialization code here.

@return 0

@see TestPlaceMessage_SuiteCleanup
*/
int 
TestPlaceMessage_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestPlaceMessage_SuiteSetup
*/
int 
TestPlaceMessage_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
    return 0;
}

#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
void 
TestShareSendPlaceMessage(void)
{
    NB_Error err = NE_OK;
    
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_ShareParameters* params = 0;
        AB_ShareHandler* handler = 0;

        NB_RequestHandlerCallback callback = { DownloadCallback, 0 };

        char* fromMdn = "9493790319";
        char* fromName = "Samsung Omnia";
        char* message = "This is a test...";
        char* toEmail = "nobody@networksinmotion.com";
        char* toPhone = "9493790319";

        NB_Place place = {{0}};
        nsl_strcpy(place.name, "NIM");
        place.location.type = NB_Location_Address;
        nsl_strcpy(place.location.street1, "6 Liberty");
        nsl_strcpy(place.location.city, "Aliso Viejo");
        nsl_strcpy(place.location.state, "CA");
        
        err = AB_ShareParametersCreateMessage(context, fromMdn, fromName, toEmail, message, &place, &params);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        err = AB_ShareParametersAddAdditionalRecipient(params, toPhone);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ShareHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        err = AB_ShareHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        if (!WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
        {
            CU_ASSERT(FALSE);
        }

        err = AB_ShareHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        err = AB_ShareParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }
    
    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


void 
DownloadCallback(
    void* handler,              /*!< Handler invoking the callback */
    NB_NetworkRequestStatus status,    /*!< Status of download request */
    NB_Error err,               /*!< error state */
    uint8 up,                   /*!< Non-zero is query upload, zero is query download */
    int percent,                /*!< Download progress */
    void* pUserData)            /*!< Optional user data provided */
{
    if (!up)
    {
        if (err != NE_OK) {

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
                AB_ShareInformation* info = 0;
                NB_Error err = NE_OK;
                
                AB_SharePlaceMessageStatus status = ASPMS_UnknownError;
                uint32 count = 0;
                uint32 index = 0;

                err = err ? err : AB_ShareHandlerGetShareInformation(handler, &info);
                CU_ASSERT_EQUAL(err, NE_OK);
                
                err = err ? err : AB_ShareInformationGetResultCount(info, &count);
                CU_ASSERT_EQUAL(err, NE_OK);
                
                if (!err) 
                {
                    for (index = 0; index < count; index++)
                    {
                        char to[80];
                        char id[80];
                        *to = 0;
                        *id = 0;
                        
                        err = AB_ShareInformationGetResult(info, index, &status, to, sizeof(to), id, sizeof(id));
                        CU_ASSERT_EQUAL(err, NE_OK);
                        CU_ASSERT_EQUAL(status, ASPMS_Success);
                        CU_ASSERT_STRING_NOT_EQUAL(to, "");
                        CU_ASSERT_STRING_NOT_EQUAL(id, "");
                    }
                }
                
                AB_ShareInformationDestroy(info);
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
        SetCallbackCompletedEvent(g_CallbackEvent);
    }
}
#endif


void
TestShareSendPlaceMessageNew(void)
{
    char* fromMdn = "9493790319";
    char* fromName = "Samsung Omnia";
    char* message = "This is a test...";
    const char* to[] = {"nobody@networksinmotion.com", "9493790319"};
    const int toCount = sizeof(to) / sizeof(to[0]);

    NB_Place place = {{0}};
    nsl_strcpy(place.name, "NIM");
    place.location.type = NB_Location_Address;
    nsl_strcpy(place.location.areaname, "Liberty Plaza");
    nsl_strcpy(place.location.street1, "6B Liberty");
    nsl_strcpy(place.location.street2, "Second Floor");
    nsl_strcpy(place.location.city, "Aliso Viejo");
    nsl_strcpy(place.location.state, "CA");
    nsl_strcpy(place.location.postal, "92656");
    nsl_strcpy(place.location.country, "USA");

    SendPlaceMessage(fromMdn, fromName, message, to, toCount, &place, NULL);
}


void
TestShareSendPlaceMessageWithBannerId(void)
{
    char* fromMdn = "9493790319";
    char* fromName = "Samsung Omnia";
    char* message = "This is a test...";
    const char* to[] = {"nobody@networksinmotion.com", "9493790319"};
    const int toCount = sizeof(to) / sizeof(to[0]);

    NB_Place place = {{0}};
    nsl_strcpy(place.name, "NIM");
    place.location.type = NB_Location_Address;
    nsl_strcpy(place.location.areaname, "Liberty Plaza");
    nsl_strcpy(place.location.street1, "6B Liberty");
    nsl_strcpy(place.location.street2, "Second Floor");
    nsl_strcpy(place.location.city, "Aliso Viejo");
    nsl_strcpy(place.location.state, "CA");
    nsl_strcpy(place.location.postal, "92656");
    nsl_strcpy(place.location.country, "USA");

    SendPlaceMessage(fromMdn, fromName, message, to, toCount, &place, "mobile");
}


void 
TestShareSendShareMessageToFacebookWithAddress(void)
{
    const char* fromMdn = "9493790319";
    const char* fromName = "";
    const char* message = "This is a place message sent to facebook with an address...";
    const char* to[] = {"facebook"};
    const int toCount = sizeof(to) / sizeof(to[0]);

    NB_Place place = {{0}};
    nsl_strcpy(place.name, "NIM");
    place.location.type = NB_Location_Address;
    nsl_strcpy(place.location.areaname, "Liberty Plaza");
    nsl_strcpy(place.location.street1, "6B Liberty");
    nsl_strcpy(place.location.street2, "Second Floor");
    nsl_strcpy(place.location.city, "Aliso Viejo");
    nsl_strcpy(place.location.state, "CA");
    nsl_strcpy(place.location.postal, "92656");
    nsl_strcpy(place.location.country, "USA");

    SendShareMessage(fromMdn, fromName, message, to, toCount, &place);
}


void 
TestShareSendShareMessageToFacebookWithAirport(void)
{
    const char* fromMdn = "9493790319";
    const char* fromName = "";
    const char* message = "This is a place message sent to facebook with an airport...";
    const char* to[] = {"facebook"};
    const int toCount = sizeof(to) / sizeof(to[0]);

    NB_Place place = {{0}};
    nsl_strcpy(place.name, "NIM");
    place.location.type = NB_Location_Airport;
    nsl_strcpy(place.location.airport, "SNA");

    SendShareMessage(fromMdn, fromName, message, to, toCount, &place);
}


void 
TestShareSendShareMessageToFacebookWithLatLon(void)
{
    const char* fromMdn = "9493790319";
    const char* fromName = "";
    const char* message = "This is a place message sent to facebook with lat lon...";
    const char* to[] = {"facebook"};
    const int toCount = sizeof(to) / sizeof(to[0]);

    NB_Place place = {{0}};
    nsl_strcpy(place.name, "NIM");
    place.location.type = NB_Location_LatLon;
    place.location.latitude = 33.5;
    place.location.longitude = -117.5;

    SendShareMessage(fromMdn, fromName, message, to, toCount, &place);
}


void
TestShareSendTextMessage(void)
{
    const char* fromMdn = "9493790319";
    const char* fromName = "";
    const char* message = "This is a text message...";
    const char* to[] = {"9493790319"};
    const int toCount = sizeof(to) / sizeof(to[0]);

    SendTextMessage(fromMdn, fromName, message, to, toCount);
}


void
SendPlaceMessage(const char* fromMdn, const char* fromName, const char* message, const char** to, int toCount, const NB_Place* place, const char* bannerId)
{
    if (toCount)
    {
        NB_Error err = NE_OK;

        PAL_Instance* pal = 0;
        NB_Context* context = 0;

        uint8 rc = CreatePalAndContext(&pal, &context);
        if (rc)
        {
            AB_ShareParameters* params = 0;
            AB_ShareHandler* handler = 0;
            AB_ShareInformation* information = 0;

            NB_RequestHandlerCallback callback = { SendMessageCallback, 0 };
            int n = 0;

            if (bannerId && *bannerId)
            {
                err = AB_ShareParametersCreatePlaceMessageWithBannerId(context, fromMdn, fromName, to[n], message, place, bannerId, &params);
            }
            else
            {
                err = AB_ShareParametersCreatePlaceMessage(context, fromMdn, fromName, to[n], message, place, &params);
            }
            CU_ASSERT_EQUAL(err, NE_OK);

            while (++n < toCount)
            {
                err = AB_ShareParametersAddAdditionalRecipient(params, to[n]);
                CU_ASSERT_EQUAL(err, NE_OK);
            }
            
            callback.callbackData = &information;
            err = AB_ShareHandlerCreate(context, &callback, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_ShareHandlerStartRequest(handler, params);
            CU_ASSERT_EQUAL(err, NE_OK);

            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
            {
                CU_ASSERT_NOT_EQUAL(information, 0);
                if (information)
                {
                    LogSendMessageResults(information, "SendPlaceMessage", to, toCount);
                }
            }
            else
            {
                CU_ASSERT(FALSE);
            }

            err = AB_ShareInformationDestroy(information);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_ShareHandlerDestroy(handler);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_ShareParametersDestroy(params);
            CU_ASSERT_EQUAL(err, NE_OK);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

void
SendShareMessage(const char* fromMdn, const char* fromName, const char* message, const char** to, int toCount, const NB_Place* place)
{
    if (toCount)
    {
        NB_Error err = NE_OK;

        PAL_Instance* pal = 0;
        NB_Context* context = 0;

        uint8 rc = CreatePalAndContext(&pal, &context);
        if (rc)
        {
            AB_ShareParameters* params = 0;
            AB_ShareHandler* handler = 0;
            AB_ShareInformation* information = 0;

            NB_RequestHandlerCallback callback = { SendMessageCallback, 0 };
            int n = 0;

            err = AB_ShareParametersCreateShareMessage(context, fromMdn, fromName, to[n], message, place, &params);
            CU_ASSERT_EQUAL(err, NE_OK);

            while (++n < toCount)
            {
                err = AB_ShareParametersAddAdditionalRecipient(params, to[n]);
                CU_ASSERT_EQUAL(err, NE_OK);
            }

            callback.callbackData = &information;
            err = AB_ShareHandlerCreate(context, &callback, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_ShareHandlerStartRequest(handler, params);
            CU_ASSERT_EQUAL(err, NE_OK);

            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
            {
                CU_ASSERT_NOT_EQUAL(information, 0);
                if (information)
                {
                    LogSendMessageResults(information, "SendShareMessage", to, toCount);
                }
            }
            else
            {
                CU_ASSERT(FALSE);
            }

            err = AB_ShareInformationDestroy(information);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_ShareHandlerDestroy(handler);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_ShareParametersDestroy(params);
            CU_ASSERT_EQUAL(err, NE_OK);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

void
SendTextMessage(const char* fromMdn, const char* fromName, const char* message, const char** to, int toCount)
{
    if (toCount)
    {
        NB_Error err = NE_OK;

        PAL_Instance* pal = 0;
        NB_Context* context = 0;

        uint8 rc = CreatePalAndContext(&pal, &context);
        if (rc)
        {
            AB_ShareParameters* params = 0;
            AB_ShareHandler* handler = 0;
            AB_ShareInformation* information = 0;

            NB_RequestHandlerCallback callback = { SendMessageCallback, 0 };
            int n = 0;

            err = AB_ShareParametersCreateTextMessage(context, fromMdn, fromName, to[n], message, &params);
            CU_ASSERT_EQUAL(err, NE_OK);

            while (++n < toCount)
            {
                err = AB_ShareParametersAddAdditionalRecipient(params, to[n]);
                CU_ASSERT_EQUAL(err, NE_OK);
            }

            callback.callbackData = &information;
            err = AB_ShareHandlerCreate(context, &callback, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_ShareHandlerStartRequest(handler, params);
            CU_ASSERT_EQUAL(err, NE_OK);

            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
            {
                CU_ASSERT_NOT_EQUAL(information, 0);
                if (information)
                {
                    LogSendMessageResults(information, "SendTextMessage", to, toCount);
                }
            }
            else
            {
                CU_ASSERT(FALSE);
            }

            err = AB_ShareInformationDestroy(information);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_ShareHandlerDestroy(handler);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_ShareParametersDestroy(params);
            CU_ASSERT_EQUAL(err, NE_OK);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

void 
SendMessageCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    if (!up)
    {
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
            // create info from request and store pointer back using userData
            AB_ShareInformation** pInfo = (AB_ShareInformation**)userData;
            err = AB_ShareHandlerGetShareInformation(handler, pInfo);
            CU_ASSERT_EQUAL(err, NE_OK);
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
        SetCallbackCompletedEvent(g_CallbackEvent);
    }
}

void
LogSendMessageResults(AB_ShareInformation* info, const char* testName, const char** expectedRecipients, uint32 expectedRecipientCount)
{
    NB_Error err = NE_OK;

    uint32 n = 0;
    uint32 m = 0;
    uint32 count = 0;
    uint32 errorCode = 0;
    nb_boolean found = FALSE;
    char errorMessage[80] = {0};
    char to[80] = {0};
    char id[80] = {0};
        
    err = AB_ShareInformationGetResultCount(info, &count);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT_EQUAL(count, expectedRecipientCount);

    LOGOUTPUT(LogLevelHigh, ("\n\t%s result count = %d\n", testName, count));

    if (!err)
    {
        for (n = 0; n < count; n++)
        {
            err = AB_ShareInformationGetResultEx(info, n, &errorCode, errorMessage, sizeof(errorMessage), to, sizeof(to), id, sizeof(id));
            CU_ASSERT_EQUAL(err, NE_OK);
            if (!err)
            {
                LOGOUTPUT(LogLevelHigh, ("\tRecipient %d: %s\terror %d %s\tmessage id %s\n", n, to, errorCode, errorMessage, id));
            }
            
            // check if returned recipient was requested
            found = FALSE;
            for (m = 0; m < expectedRecipientCount; m++)
            {
                if (!nsl_strcmp(to, expectedRecipients[m]))
                {
                    found = TRUE;
                    break;
                }
            }
            if (!found)
            {
                LOGOUTPUT(LogLevelHigh, ("\tReturned recipient '%s' not requested\n", to));
                CU_FAIL("returned recipient was not requested");
            }
        }
        
        // check if all requested recipients were returned
        for (n = 0; n < expectedRecipientCount; n++)
        {
            found = FALSE;
            for (m = 0; m < count; m++)
            {
                err = AB_ShareInformationGetResultEx(info, m, &errorCode, errorMessage, sizeof(errorMessage), to, sizeof(to), id, sizeof(id));
                CU_ASSERT_EQUAL(err, NE_OK);
                if (!err && !nsl_strcmp(expectedRecipients[n], to))
                {
                    found = TRUE;
                    break;
                }
            }
            if (!found)
            {
                LOGOUTPUT(LogLevelHigh, ("\tRequested recipient '%s' not returned\n", expectedRecipients[n]));
                CU_FAIL("requested recipient was not returned")
            }
        }
    }
}

/*! @} */

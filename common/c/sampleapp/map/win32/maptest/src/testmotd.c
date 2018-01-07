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

@file     TestMotd.c
@date     03/19/2009
@defgroup TestMotd_h System tests for AB_Message functions

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

#include "testmotd.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"

#include "nbcontext.h"
#include "abservermessagehandler.h"



// Local Functions ...........................................................

// All tests. Add all tests to the TestMotd_AddTests function
static void TestServerMessageStatus(void);
static void TestServerMessageQuery(void);
static void TestServerMessageConfirm(void);
static void TestServerMessageReview(void);
static void TestServerMessageQueryConfirm(void);

static void LogServerMessage(const char* text, AB_ServerMessageDetail* detail);

// Subfunctions of tests
static void ServerMessageCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);

// Constants .................................................................


// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;

// Message id and action
static char* g_messageId = 0;
static char* g_messageAction = 0;

/*! Add all your test functions here

@return None
*/
void 
TestMotd_AddTests( CU_pSuite pTestSuite, int level )
{
    // N.B. The tests must be run in this order: Status, Query, Confirm and Review
    // Confirm and Review depend on previous test results to work properly
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestServerMessageStatus", TestServerMessageStatus);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestServerMessageQuery", TestServerMessageQuery);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestServerMessageConfirm", TestServerMessageConfirm);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestServerMessageReview", TestServerMessageReview);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestServerMessageQueryConfirm", TestServerMessageQueryConfirm);
};


/*! Add common initialization code here.

@return 0

@see TestMotd_SuiteCleanup
*/
int 
TestMotd_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
	return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestMotd_SuiteSetup
*/
int 
TestMotd_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
	return 0;
}

void
TestServerMessageStatus(void)
{
    NB_Error err = NE_OK;
    
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_ServerMessageParameters* params = 0;
        AB_ServerMessageHandler* handler = 0;
        AB_ServerMessageInformation* info = 0;

        NB_RequestHandlerCallback callback = { ServerMessageCallback, 0 };

        err = AB_ServerMessageParametersCreateStatus(context, "en", 0, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        callback.callbackData = &info;
        err = AB_ServerMessageHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ServerMessageHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 30000))
        {
            const char* modifier = "";

            if (!AB_ServerMessageInformationGetMessagePending(info))
            {
                modifier = "no ";
            }
            LOGOUTPUT(LogLevelMedium, ("\nmessage-status-reply: %smessage(s) pending\n", modifier));

            err = AB_ServerMessageInformationDestroy(info);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_ServerMessageHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ServerMessageParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}

void
TestServerMessageQuery(void)
{
    NB_Error err = NE_OK;
    
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_ServerMessageParameters* params = 0;
        AB_ServerMessageHandler* handler = 0;
        AB_ServerMessageInformation* info = 0;

        NB_RequestHandlerCallback callback = { ServerMessageCallback, 0 };

        err = AB_ServerMessageParametersCreateQuery(context, "en", 0, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        callback.callbackData = &info;
        err = AB_ServerMessageHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ServerMessageHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 30000))
        {
            AB_ServerMessageDetail* serverMessageDetail = 0;

            err = AB_ServerMessageInformationEnumerateInitialize(info, 0, ASMEO_NoSorting);
            CU_ASSERT_EQUAL(err, NE_OK);
            
            while (AB_ServerMessageInformationEnumerateNext(info, &serverMessageDetail))
            {
                CU_ASSERT_NOT_EQUAL(serverMessageDetail->id, NULL);

                // If this is a EULA, save the id to use on confirm test
                if (nsl_stricmp(serverMessageDetail->type, "eula") == 0)
                {
                    if (!g_messageId && !g_messageAction)
                    {
                        g_messageId = nsl_strdup(serverMessageDetail->id);
                        g_messageAction = nsl_strdup(serverMessageDetail->accept_text);
                    }
                }

                LogServerMessage("TestServerMessageQuery", serverMessageDetail);
            }

            err = AB_ServerMessageInformationDestroy(info);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_ServerMessageHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ServerMessageParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}

void
TestServerMessageConfirm(void)
{
    NB_Error err = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = 0;
    
    if (!g_messageId)
    {
        LOGOUTPUT(LogLevelHigh, ("\nNo message id to confirm\n"));
        return;
    }

    if (!g_messageAction)
    {
        LOGOUTPUT(LogLevelHigh, ("\nNo confirm action\n"));
        return;
    }

    rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_ServerMessageParameters* params = 0;
        AB_ServerMessageHandler* handler = 0;
        AB_ServerMessageInformation* info = 0;

        NB_RequestHandlerCallback callback = { ServerMessageCallback, 0 };

        // Confirm the EULA that was seen before
        err = AB_ServerMessageParametersCreateConfirm(context, g_messageId, g_messageAction, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        callback.callbackData = &info;
        err = AB_ServerMessageHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ServerMessageHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 30000))
        {
            err = AB_ServerMessageInformationDestroy(info);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_ServerMessageHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ServerMessageParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);

    nsl_free(g_messageAction);
    g_messageAction = 0;
}


void
TestServerMessageReview(void)
{
    NB_Error err = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = 0;
    
    if (!g_messageId)
    {
        LOGOUTPUT(LogLevelHigh, ("No message id to review\n"));
        return;
    }

    rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_ServerMessageParameters* params = 0;
        AB_ServerMessageHandler* handler = 0;
        AB_ServerMessageInformation* info = 0;

        NB_RequestHandlerCallback callback = { ServerMessageCallback, 0 };

        // Review the EULA that was confirmed earlier
        err = AB_ServerMessageParametersCreateReview(context, "eula", &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        callback.callbackData = &info;
        err = AB_ServerMessageHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ServerMessageHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 30000))
        {
            AB_ServerMessageDetail* serverMessageDetail = 0;
            int count = 0;

            err = AB_ServerMessageInformationEnumerateInitialize(info, 0, ASMEO_NoSorting);
            CU_ASSERT_EQUAL(err, NE_OK);
            
            while (AB_ServerMessageInformationEnumerateNext(info, &serverMessageDetail))
            {
                // Make sure the id that comes back is the one confirmed
                CU_ASSERT_TRUE(nsl_strcmp(serverMessageDetail->id, g_messageId) == 0);
                count++;

                LogServerMessage("TestServerMessageReview", serverMessageDetail);
            }

            // Should have received one confirmed message
            CU_ASSERT_EQUAL(count, 1);

            err = AB_ServerMessageInformationDestroy(info);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_ServerMessageHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ServerMessageParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);

    nsl_free(g_messageId);
    g_messageId = 0;
}

void
TestServerMessageQueryConfirm(void)
{
    NB_Error err = NE_OK;

    PAL_Instance* pal = 0;
    NB_Context* context = 0;

    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_ServerMessageParameters* params = 0;
        AB_ServerMessageHandler* handler = 0;
        AB_ServerMessageInformation* info = 0;

        NB_RequestHandlerCallback callback = { ServerMessageCallback, 0 };

        err = AB_ServerMessageParametersCreateQuery(context, "en", 0, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        callback.callbackData = &info;
        err = AB_ServerMessageHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ServerMessageHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 30000))
        {
            AB_ServerMessageInformation* queryInfo = info;
            AB_ServerMessageDetail* serverMessageDetail = 0;

            err = AB_ServerMessageInformationEnumerateInitialize(queryInfo, 0, ASMEO_NoSorting);
            CU_ASSERT_EQUAL(err, NE_OK);

            while (AB_ServerMessageInformationEnumerateNext(queryInfo, &serverMessageDetail))
            {
                CU_ASSERT_NOT_EQUAL(serverMessageDetail->id, NULL);

                // If this is a EULA, confirm it...
                if (nsl_stricmp(serverMessageDetail->type, "eula") == 0)
                {
                    AB_ServerMessageParameters* confirmParams = 0;

                    err = AB_ServerMessageParametersCreateConfirm(context, serverMessageDetail->id, serverMessageDetail->accept_text, &confirmParams);
                    CU_ASSERT_EQUAL(err, NE_OK);

                    info = 0;
                    err = AB_ServerMessageHandlerStartRequest(handler, confirmParams);
                    CU_ASSERT_EQUAL(err, NE_OK);

                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, 30000))
                    {
                        err = AB_ServerMessageInformationDestroy(info);
                        CU_ASSERT_EQUAL(err, NE_OK);
                    }
                    else
                    {
                        CU_ASSERT(FALSE);
                    }
                    err = AB_ServerMessageParametersDestroy(confirmParams);
                    CU_ASSERT_EQUAL(err, NE_OK);
                }
            }

            err = AB_ServerMessageInformationDestroy(queryInfo);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_ServerMessageHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ServerMessageParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


void 
ServerMessageCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
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
                AB_ServerMessageInformation** pInfo = (AB_ServerMessageInformation**)userData;
                err = AB_ServerMessageHandlerGetInformation(handler, pInfo);
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

const char* GetFrequencyUnitText(AB_ServerMessageNagUnit unit)
{
    const char* text = 0;

    switch (unit)
    {
    case ASNU_Days:
        text = "day(s)";
        break;

    case ASNU_Seconds:
        text = "second(s)";
        break;

    case ASNU_Uses:
        text = "use(s)";
        break;

    default:
        text = "unknown unit";
        break;
    }

    return text;
}

void LogServerMessage(const char* text, AB_ServerMessageDetail* detail)
{
    LOGOUTPUT(LogLevelMedium, ("\n%s message\n", text));
    LOGOUTPUT(LogLevelMedium, ("   timestamp:       %d\n", detail->time));
    LOGOUTPUT(LogLevelMedium, ("   id:              %s\n", detail->id));
    LOGOUTPUT(LogLevelMedium, ("   type:            %s\n", detail->type));
    LOGOUTPUT(LogLevelMedium, ("   language:        %s\n", detail->language));
    LOGOUTPUT(LogLevelMedium, ("   title:           %.20s\n", detail->title));
    LOGOUTPUT(LogLevelMedium, ("   text:            %.20s\n", detail->text));
    LOGOUTPUT(LogLevelMedium, ("   accept:          %.20s\n", detail->accept_text));
    LOGOUTPUT(LogLevelMedium, ("   center:          %.20s\n", detail->center_text));
    LOGOUTPUT(LogLevelMedium, ("   decline:         %.20s\n", detail->decline_text));
    LOGOUTPUT(LogLevelMedium, ("   url:             %.20s\n", detail->url ? detail->url : "(null)"));
    LOGOUTPUT(LogLevelMedium, ("   confirm:         %s\n", detail->confirm ? "true" : "false"));
    LOGOUTPUT(LogLevelMedium, ("   exit-on-decline: %s\n", detail->exit_on_decline ? "true" : "false"));
    LOGOUTPUT(LogLevelMedium, ("   frequency:       %d %s\n", detail->message_nag.freq_count, GetFrequencyUnitText(detail->message_nag.freq_unit)));
    LOGOUTPUT(LogLevelMedium, ("   expire:          %d %s\n", detail->message_nag.expire_count, GetFrequencyUnitText(detail->message_nag.expire_unit)));
    LOGOUTPUT(LogLevelMedium, ("   expire date:     %d\n", detail->message_nag.expire_date));
}


/*! @} */

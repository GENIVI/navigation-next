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

@file     TestProfile.c
@date     03/23/2009
@defgroup TestProfile_h System tests for AB_Profile functions

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

#include "testprofile.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"

#include "nbcontext.h"
#include "abprofilehandler.h"


// Local Functions ...........................................................

// All tests. Add all tests to the TestProfile_AddTests function

static void TestProfilePrivacy(void);
static void TestProfileProbes(void);
static void TestProfileFacebookActivation(void);
static void TestProfileFacebookActivationUrl(void);
static void TestProfileIlapUserId(void);

// Subfunctions of tests
static void ProfileCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);

static void CheckProfileSetGetAndDefault(const char* key, const char* defaultValue, const char* nonDefaultValue);


// Constants .................................................................

static int PROFILE_VERSION = 1;

static char* PRIVACY_KEY = "privacy";
static char* PRIVACY_DEFAULT = "allow";
static char* PRIVACY_NON_DEFAULT = "deny";

static char* PROBES_PARTICIPATION_KEY = "probes_participation";
static char* PROBES_PARTICIPATION_DEFAULT = "deny";
static char* PROBES_PARTICIPATION_NON_DEFAULT = "allow";

static char* FACEBOOK_ACTIVATION_KEY = "fb:activation-code";
static char* FACEBOOK_ACTIVATION_CODE = "ZZZZZZZ"; // "XLY3N8P";

static char* FACEBOOK_ACTIVATION_URL_KEY = "fb:activation-code-url";

static char* ILAP_USERID_KEY = "ilap:userid";

// Variables .................................................................

struct TestProfileCallbackData
{
    void*                           event;          /*!< Event to signal when callback complete */
    AB_ProfileInformation*          information;    /*!< Whether or not request was successful */
};



/*! Add all your test functions here

@return None
*/
void 
TestProfile_AddTests(CU_pSuite pTestSuite, int level)
{
    Test_Options* options = Test_OptionsGet();

    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestProfilePrivacy", TestProfilePrivacy);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestProfileProbes", TestProfileProbes);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestProfileFacebookActivation", TestProfileFacebookActivation);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestProfileFacebookActivationUrl", TestProfileFacebookActivationUrl);

    if (options->carrier & TestCarrierVerizon)
    {
        ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestProfileIlapUserId", TestProfileIlapUserId);
    }
};


/*! Add common initialization code here.

@return 0

@see TestProfile_SuiteCleanup
*/
int 
TestProfile_SuiteSetup()
{
	return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestProfile_SuiteSetup
*/
int 
TestProfile_SuiteCleanup()
{
	return 0;
}


void
TestProfilePrivacy(void)
{
    CheckProfileSetGetAndDefault(PRIVACY_KEY, PRIVACY_DEFAULT, PRIVACY_NON_DEFAULT);
}


void
TestProfileProbes(void)
{
    CheckProfileSetGetAndDefault(PROBES_PARTICIPATION_KEY, PROBES_PARTICIPATION_DEFAULT, PROBES_PARTICIPATION_NON_DEFAULT);
}


static void TestProfileFacebookActivation(void)
{
    NB_Error err = NE_OK;

    PAL_Instance* pal = 0;
    NB_Context* context = 0;

    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_ProfileHandler* handler = 0;
        AB_ProfileParameters* params = 0;

        NB_RequestHandlerCallback callback = { ProfileCallback, 0 };
        struct TestProfileCallbackData callbackData = { 0 };

        callbackData.event = CreateCallbackCompletedEvent();
        callback.callbackData = &callbackData;
        err = AB_ProfileHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        // Test setting profile to its non-default value....

        err = AB_ProfileParametersCreate(context, PROFILE_VERSION, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileParametersAddSetStringValue(params, FACEBOOK_ACTIVATION_KEY, FACEBOOK_ACTIVATION_CODE);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(callbackData.event, 30000))
        {
            uint32 code = 0;
            const char* description = 0;

            // an error should be returned, unless the test machine has an MDN that is waiting to be Facebook activated
            code = AB_ProfileInformationGetError(callbackData.information, &description); 
            CU_ASSERT_NOT_EQUAL(code, 0);
            CU_ASSERT_NOT_EQUAL(description, 0);

            err = AB_ProfileInformationDestroy(callbackData.information);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_ProfileParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        DestroyCallbackCompletedEvent(callbackData.event);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


static void TestProfileFacebookActivationUrl(void)
{
    NB_Error err = NE_OK;

    PAL_Instance* pal = 0;
    NB_Context* context = 0;

    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_ProfileHandler* handler = 0;
        AB_ProfileParameters* params = 0;

        NB_RequestHandlerCallback callback = { ProfileCallback, 0 };
        struct TestProfileCallbackData callbackData = { 0 };

        callbackData.event = CreateCallbackCompletedEvent();
        callback.callbackData = &callbackData;
        err = AB_ProfileHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileParametersCreate(context, PROFILE_VERSION, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileParametersAddGetValue(params, FACEBOOK_ACTIVATION_URL_KEY);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(callbackData.event, 30000))
        {
            uint32 code = 0;
            const char* description = NULL;
            const char* url = NULL;

            code = AB_ProfileInformationGetError(callbackData.information, &description); 
            CU_ASSERT_EQUAL(code, 0);
            CU_ASSERT_EQUAL(description, NULL);

            err = AB_ProfileInformationGetStringValue(callbackData.information, FACEBOOK_ACTIVATION_URL_KEY, &url);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_ProfileInformationDestroy(callbackData.information);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_ProfileParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        DestroyCallbackCompletedEvent(callbackData.event);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


void
CheckProfileSetGetAndDefault(const char* key, const char* defaultValue, const char* nonDefaultValue)
{
    NB_Error err = NE_OK;

    PAL_Instance* pal = 0;
    NB_Context* context = 0;

    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_ProfileHandler* handler = 0;
        AB_ProfileParameters* params = 0;

        NB_RequestHandlerCallback callback = { ProfileCallback, 0 };
        struct TestProfileCallbackData callbackData = { 0 };

        callbackData.event = CreateCallbackCompletedEvent();
        callback.callbackData = &callbackData;
        err = AB_ProfileHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        // Test setting profile to its non-default value....

        err = AB_ProfileParametersCreate(context, PROFILE_VERSION, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileParametersAddSetStringValue(params, key, nonDefaultValue);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        err = AB_ProfileParametersAddGetValue(params, key);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(callbackData.event, 30000))
        {
            uint32 code = 0;
            const char* description = 0;
            const char* returnedValue = 0;

            // no error should be returned
            code = AB_ProfileInformationGetError(callbackData.information, &description);
            CU_ASSERT_EQUAL(code, 0);
            CU_ASSERT_EQUAL(description, 0);

            // value returned should equal set value (the non-default value)
            err = AB_ProfileInformationGetStringValue(callbackData.information, key, &returnedValue);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (returnedValue)
            {
                CU_ASSERT_EQUAL(nsl_strcmp(returnedValue, nonDefaultValue), 0);
            }
            else
            {
                CU_FAIL("NULL profile value returned");
            }

            err = AB_ProfileInformationDestroy(callbackData.information);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_ProfileParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);

        // Test setting profile value to its default value

        ResetCallbackCompletedEvent(callbackData.event);
        callbackData.information = 0;

        err = AB_ProfileParametersCreate(context, PROFILE_VERSION, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileParametersAddSetToDefaults(params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileParametersAddGetValue(params, key);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(callbackData.event, 30000))
        {
            uint32 code = 0;
            const char* description = 0;
            const char* returnedValue = 0;

            // no error should be returned
            code = AB_ProfileInformationGetError(callbackData.information, &description);
            CU_ASSERT_EQUAL(code, 0);
            CU_ASSERT_EQUAL(description, 0);

            // value returned should equal the default value
            err = AB_ProfileInformationGetStringValue(callbackData.information, key, &returnedValue);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (returnedValue)
            {
                CU_ASSERT_EQUAL(nsl_strcmp(returnedValue, defaultValue), 0);
            }
            else
            {
                CU_FAIL("NULL profile value returned");
            }


            err = AB_ProfileInformationDestroy(callbackData.information);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_ProfileParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        DestroyCallbackCompletedEvent(callbackData.event);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


void 
ProfileCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    struct TestProfileCallbackData* data = userData;

    if (!up)
    {
        if (err != NE_OK)
        {
            // Abort and trigger event
            CU_FAIL("Callback returned error");
            SetCallbackCompletedEvent(data->event);
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
                err = AB_ProfileHandlerGetInformation(handler, &data->information);
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
        SetCallbackCompletedEvent(data->event);
    }
}

void TestProfileIlapUserId(void)
{
    char* userId = 0;
    NB_Error err = TestProfile_GetIlapUserId(&userId);

    CU_ASSERT_EQUAL(err, NE_OK);

    if (!err)
    {
        LOGOUTPUT(LogLevelLow, ("ILAP user-id: %s\n", userId));
        nsl_free(userId);
    }
}


NB_Error TestProfile_GetIlapUserId(char** userId)
{
    NB_Error err = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;

    if (!userId)
    {
        return NE_INVAL;
    }
    *userId = 0;

    if (CreatePalAndContext(&pal, &context))
    {
        AB_ProfileHandler* handler = 0;
        AB_ProfileParameters* params = 0;

        NB_RequestHandlerCallback callback = { ProfileCallback, 0 };
        struct TestProfileCallbackData callbackData = { 0 };

        callbackData.event = CreateCallbackCompletedEvent();
        callback.callbackData = &callbackData;

        err = AB_ProfileHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileParametersCreate(context, PROFILE_VERSION, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileParametersAddGetValue(params, ILAP_USERID_KEY);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(callbackData.event, 30000))
        {
            const char* id = 0;

            err = AB_ProfileInformationGetStringValue(callbackData.information, ILAP_USERID_KEY, &id);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(userId);

            if (!err)
            {
                *userId = nsl_strdup(id);
            }

            err = AB_ProfileInformationDestroy(callbackData.information);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_ProfileParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_ProfileHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        DestroyCallbackCompletedEvent(callbackData.event);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);

    return err;
}


/*! @} */

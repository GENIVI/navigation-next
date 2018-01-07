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

@file     TestLicense.h
@date     05/14/2009
@defgroup TestLicense System tests for AB_License functions

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

#include "testlicense.h"
#include "testnetwork.h"
#include "main.h"
#include "nbcontext.h"
#include "ablicensehandler.h"

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;
static uint32 timestamp=1;
/************************************************************************/
/*                  Private Function declaration                        */
/************************************************************************/ 

// System test functions
static void TestVerizonLicenseCreate(void);
static void TestAtlasbookLicenseCreate(void);

static void CreateRequest(AB_LicenseAction action, const char* vendor, const char* subscriberKey, const char* productId, const char* licenseKey);
static void DownloadCallback(void* handler, NB_NetworkRequestStatus status,NB_Error err, uint8 up, int percent,void* pUserData);

/*! Add all your test functions here

@return None
*/
void TestLicense_AddTests(CU_pSuite pTestSuite, int level)
{
    // ! Add all your function names here !
    if ((Test_OptionsGet()->carrier & TestCarrierVerizon) != 0)
    {
        ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestVerizonLicenseCreate", TestVerizonLicenseCreate);
    }
    else if ((Test_OptionsGet()->carrier & TestCarrierAtlasbook) != 0)
    {
        ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestAtlasbookLicenseCreate", TestAtlasbookLicenseCreate);
    }
}

/*! Add common initialization code here.

@return 0

@see TestLicense_SuiteSetup
*/
int TestLicense_SuiteSetup(void)
{
    g_CallbackEvent = CreateCallbackCompletedEvent(); /*! Create event for callback synchronization */
    return 0;
}

/*! Add common cleanup code here.

@return 0

@see TestLicense_SuiteCleanup
*/
int TestLicense_SuiteCleanup(void)
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
    return 0;
}

static void TestVerizonLicenseCreate(void)
{
    AB_LicenseAction action = AB_LicenseAction_Create;
    const char* vendor = "verizon";
    const char* subscriberKey = "VZ:MDN";
    const char* productId = "VZNV5";
    const char* licenseKey = "";
    CreateRequest(action, vendor, subscriberKey, productId, licenseKey);
    //once license is created you can use the license to try update and delete options
}

static void TestAtlasbookLicenseCreate(void)
{
    AB_LicenseAction action = AB_LicenseAction_Create;
    const char* vendor = "nimdbs";
    const char* subscriberKey = "AB:CLTKEY";
    const char* productId = "ANBV5";
    const char* licenseKey = "";
    CreateRequest(action, vendor, subscriberKey, productId, licenseKey);
    //once license is created you can use the license to try update and delete options
}

void CreateRequest(AB_LicenseAction action, const char* vendor, const char* subscriberKey, const char* productId, const char* licenseKey)
{
    NB_Error err = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);

    if (rc)
    {
        AB_LicenseParameters* params = 0;
        AB_LicenseHandler* handler = 0;
        int licIndex = 0;
        

        NB_RequestHandlerCallback callback = { DownloadCallback, 0 };

        err = AB_LicenseParametersCreate(context, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        AB_LicenseParametersSetRequest(params, "1", "", vendor, "", "USA", subscriberKey, action);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_LicenseParametersCreateLicense(params, licenseKey, productId, "1", &licIndex);
        CU_ASSERT_EQUAL(err, NE_OK);

        //bundle name and type will be setup per server definition
        err = AB_LicenseParametersAddBundle(params, licIndex, "VNF", "r");
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_LicenseParametersSetLanguage(params, Test_OptionsGet()->currentLanguage);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_LicenseHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_LicenseHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (!WaitForCallbackCompletedEvent(g_CallbackEvent, 45000))
        {
            CU_ASSERT(FALSE);
        }

        err = AB_LicenseHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_LicenseParametersDestroy(params);
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
    AB_LicenseStatus statusCode = 0;
    const char* requestId = NULL;
    const char* transactionId = NULL;
    const char* key = NULL;
    const char* productId = NULL;
    const char* licenseRequestId = NULL;
    int licenseCount = 0;
    int i = 0;
    const char* errorDescription = NULL;

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
                AB_LicenseInformation* info = 0;

                err = err ? err : AB_LicenseHandlerGetLicenseInformation(handler, &info);
                CU_ASSERT_EQUAL(err, NE_OK);
                if (!err)
                {
                    err = AB_LicenseInformationGetStatus(info, &statusCode);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    LOGOUTPUT(LogLevelMedium, ("\nStatus Code = %d", statusCode));

                    if (statusCode != AB_LicenseStatus_Success)
                    {
                        err = AB_LicenseInformationGetError(info, &errorDescription);
                        CU_ASSERT_EQUAL(err, NE_OK);
                        LOGOUTPUT(LogLevelMedium, ("\nError description = %s", errorDescription));
                    }

                    err = err ? err : AB_LicenseInformationGetRequestId(info, &requestId);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    LOGOUTPUT(LogLevelMedium, ("\nRequestId = %s", requestId));

                    err = err ? err : AB_LicenseInformationGetTransactionId(info, &transactionId);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    LOGOUTPUT(LogLevelMedium, ("\nTransactionId = %s", transactionId));

                    //get license
                    err = AB_LicenseInformationGetLicenseCount (info, &licenseCount);
                    CU_ASSERT_EQUAL(err, NE_OK);

                    for (i = 0; i < licenseCount; i++)
                    {
                        err = AB_LicenseInformationGetLicenseDetail (info, i, &key, &productId, &licenseRequestId, &statusCode);
                        CU_ASSERT_EQUAL(err, NE_OK);

                        LOGOUTPUT(LogLevelMedium, ("\nLicense Key = %s", key));
                        LOGOUTPUT(LogLevelMedium, ("\nLicense Product Id = %s", productId));
                        LOGOUTPUT(LogLevelMedium, ("\nLicense Request Id = %s", licenseRequestId));
                        LOGOUTPUT(LogLevelMedium, ("\nLicense Status Code = %d", statusCode));
                    }

                    AB_LicenseInformationDestroy(info);
                }
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


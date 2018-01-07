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

@file     TestSubscription.h
@date     05/14/2009
@defgroup TestSubscription System tests for AB_Subscription functions

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

#include "testsubscription.h"
#include "testnetwork.h"
#include "main.h"
#include "nbcontext.h"
#include "abauthenticationhandler.h"

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;
static uint32 timestamp=1;
/************************************************************************/
/*                  Private Function declaration                        */
/************************************************************************/ 

// System test functions
static void TestSubscriptionListFeatureRequest(void);
static void TestSubscriptionBindLicenseRequest(void);
static void TestSubscriptionAddFeatureRequest(void);
static void TestSubscriptionRemoveFeatureRequest(void);
static void TestSubscriptionOptinResponseRequest(void);

void CreateRequest(AB_Authentication_RequestType RequestType, char* LicenseKey, char* addFeatureName, char* removeFeatureName);
void DownloadCallback(void* handler, NB_NetworkRequestStatus status,NB_Error err, uint8 up, int percent,void* pUserData);

void PrintMessage(AB_MessageDetail* pMsg);
void DestroyMessageDetail(AB_MessageDetail* pMessageDetail);
void PrintClientMessage (AB_ClientStoredMessageDetail* pMsg);
static NB_Error FormattedTextCallback(NB_Font font, nb_color color, const char* text, uint8 newline, const char* href, const char* hreftext, void* userData);
static void LogBundleOption (AB_BundleOption* bundle);
static void LogPriceOption (AB_PriceOption* price);
/*! Add all your test functions here

@return None
*/
void TestSubscription_AddTests(CU_pSuite pTestSuite, int level)
{
    // ! Add all your function names here !
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSubscriptionBindLicenseRequest", TestSubscriptionBindLicenseRequest);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestSubscriptionListFeatureRequest", TestSubscriptionListFeatureRequest);
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSubscriptionAddFeatureRequest", TestSubscriptionAddFeatureRequest);
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSubscriptionRemoveFeatureRequest", TestSubscriptionRemoveFeatureRequest);
}

/*! Add common initialization code here.

@return 0

@see TestSubscription_SuiteSetup
*/
int TestSubscription_SuiteSetup(void)
{
    g_CallbackEvent = CreateCallbackCompletedEvent(); /*! Create event for callback synchronization */
    return 0;
}

/*! Add common cleanup code here.

@return 0

@see TestSubscription_SuiteCleanup
*/
int TestSubscription_SuiteCleanup(void)
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
    return 0;
}

static void TestSubscriptionListFeatureRequest(void)
{
    CreateRequest(AB_Authentication_List_Feature, NULL, NULL, NULL);
}

static void TestSubscriptionBindLicenseRequest(void)
{
    CreateRequest(AB_Authentication_Bind_License,"ABNV5-428-MJHD", NULL, NULL);
}

static void TestSubscriptionAddFeatureRequest(void)
{
    //CreateRequest(AB_Authentication_Add_Feature, NULL, "VZNAV", NULL);
}
static void TestSubscriptionRemoveFeatureRequest(void)
{
    //CreateRequest(AB_Authentication_Remove_Feature, NULL, NULL, "TREK");        
}

static void TestSubscriptionOptinResponseRequest(void)
{
    CreateRequest(AB_Authentication_Save_Optin_Response, NULL, NULL, "TREK");        
}

void CreateRequest(AB_Authentication_RequestType RequestType, char* LicenseKey, char* addFeatureName, char* removeFeatureName)
{
    NB_Error err = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);

    if (rc)
    {
        AB_AuthenticationParameters* params = 0;
        AB_AuthenticationHandler* handler = 0;

        NB_RequestHandlerCallback callback = { DownloadCallback, 0 };

        err = AB_AuthenticationParametersCreate(context, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        AB_AuthenticationParametersSetRequest(params, RequestType);
        CU_ASSERT_EQUAL(err, NE_OK);

        if(LicenseKey != NULL)
        {
            err = AB_AuthenticationParametersSetLicenseKey(params, LicenseKey);
            CU_ASSERT_EQUAL(err, NE_OK);
        }

        if(addFeatureName != NULL)
        {
            err = AB_AuthenticationParametersSetAddFeatureName(params, addFeatureName);
            CU_ASSERT_EQUAL(err, NE_OK);
        }

        if(removeFeatureName != NULL)
        {
            err = AB_AuthenticationParametersSetRemoveFeatureName(params, removeFeatureName);
            CU_ASSERT_EQUAL(err, NE_OK);
        }

        err = AB_AuthenticationParametersSetLanguage(params, Test_OptionsGet()->currentLanguage);

        err = AB_AuthenticationParametersSetWantLicenseMessage (params, TRUE, timestamp);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_AuthenticationParametersSetWantPurchaseMessage (params, TRUE, timestamp);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_AuthenticationParametersSetWantRegion (params, TRUE);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_AuthenticationHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_AuthenticationHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (!WaitForCallbackCompletedEvent(g_CallbackEvent, 45000))
        {
            CU_ASSERT(FALSE);
        }

        err = AB_AuthenticationHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_AuthenticationParametersDestroy(params);
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
    int32 statusCode = 0;
    int featureCount = 0;
    int i = 0, j = 0;
    AB_MessageDetail* pMsg  = NULL;
    AB_FeatureDetail* featureDetail = NULL;
    AB_ClientStoredMessageDetail* clientMessage = NULL;
    int messageCount =0;
    AB_BundleOption bundle;
    AB_PriceOption price;
    AB_SubscribedMessage* subscribedMessage = NULL;

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
                AB_AuthenticationInformation* info = 0;
                NB_Error err = NE_OK;

                err = err ? err : AB_AuthenticationHandlerGetAuthenticationInformation(handler, &info);
                CU_ASSERT_EQUAL(err, NE_OK);
                if (!err)
                {
                    err = err ? err : AB_AuthenticationInformationFeatureCount(info, &featureCount);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    LOGOUTPUT(LogLevelMedium, ("\nFeature Count = %d", featureCount));

                    for(i =0; i < featureCount; i++)
                    {
                        err = AB_AuthenticationInformationGetFeatureResult(info, i, &featureDetail);
                        CU_ASSERT_EQUAL(err, NE_OK);
                        if (!err)
                        {
                            LOGOUTPUT(LogLevelMedium, ("\nFeature Name = %s, EndDate = %d, Feature Type = %s", featureDetail->name, featureDetail->endDate, featureDetail->type));
                            AB_AuthenticationInformationGetFeatureResultDestroy(featureDetail);                
                        }
                    }

                    err = AB_AuthenticationInformationGetStatusCode(info, &statusCode);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    LOGOUTPUT(LogLevelMedium, ("\nStatus Code = %d\n", statusCode));

                    //get client stored message
                    err = AB_AuthenticationInformationGetClientStoredMessageDetail(info, &clientMessage);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    if (!err && clientMessage)
                    {
                        PrintClientMessage(clientMessage);  
                        timestamp = clientMessage->timestamp;

                        AB_AuthenticationInformationGetClientStoredMessageText(info, &FormattedTextCallback, NULL);

                        //get purchase bundle information
                        for (i=0; i<clientMessage->bundle_count; i++)
                        {
                            err = err ? err : AB_AuthenticationInformationGetPurchaseBundle(info, i, &bundle);
                            CU_ASSERT_EQUAL(err, NE_OK);

                            LogBundleOption(&bundle);
                            
                            for(j=0; j< bundle.price_option_count; j++)
                            {
                                //get bundle pricing information
                                AB_AuthenticationInformationGetPurchaseBundlePricing(info, i, j, &price);
                                CU_ASSERT_EQUAL(err, NE_OK);

                                LogPriceOption(&price);
                            }
                        }

                        AB_AuthenticationInformationDestroyClientStoredMessageDetail(clientMessage);
                    }

                    //get messages
                    err = AB_AuthenticationInformationGetMessageCount (info, &messageCount);
                    CU_ASSERT_EQUAL(err, NE_OK);

                    for (i = 0; i<messageCount; i++)
                    {
                        err = AB_AuthenticationInformationGetMessageDetail (info, i, &pMsg);
                        CU_ASSERT_EQUAL(err, NE_OK);

                        AB_AuthenticationInformationGetMessageText(info, i, &FormattedTextCallback, NULL);

                        PrintMessage (pMsg);
                        AB_AuthenticationInformationDestroyMessageDetail(pMsg); 
                    }

                    //get subscribed message
                    subscribedMessage = nsl_malloc(sizeof(*subscribedMessage));
                    nsl_memset(subscribedMessage, 0, sizeof(*subscribedMessage));

                    err = AB_AuthenticationInformationGetSubscribedMessage(info, subscribedMessage);
                    CU_ASSERT_EQUAL(err, NE_OK);

                    if (!err && subscribedMessage)
                    {
                        timestamp = subscribedMessage->ts;

                        AB_AuthenticationInformationGetSubscribedMessageText(info, &FormattedTextCallback, NULL);

                        //get purchase bundle information
                        for (i=0; i<subscribedMessage->bundle_count; i++)
                        {
                            err = err ? err : AB_AuthenticationInformationGetSubscribedBundle(info, i, &bundle);
                            CU_ASSERT_EQUAL(err, NE_OK);

                            LogBundleOption(&bundle);
                            
                            for(j=0; j< bundle.price_option_count; j++)
                            {
                                //get bundle pricing information
                                AB_AuthenticationInformationGetSubscribedBundlePricing(info, i, j, &price);
                                CU_ASSERT_EQUAL(err, NE_OK);

                                LogPriceOption(&price);
                            }
                        }
                    }

                    nsl_free(subscribedMessage);

                    AB_AuthenticationInformationDestroy(info);
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

void PrintMessage(AB_MessageDetail* pMsg)
{
    LOGOUTPUT(LogLevelHigh, ("\n\n******* Message Details *******\n"));

    if(pMsg->id)
    {
        LOGOUTPUT(LogLevelHigh, ("\nMessage ID: %s", pMsg->id));
    }
    if(pMsg->title)
    {
        LOGOUTPUT(LogLevelHigh, ("\nTitle: %s", pMsg->title));
    }
    if(pMsg->type)
    {
        LOGOUTPUT(LogLevelHigh, ("\nType: %s", pMsg->type));
    }
    if(pMsg->language)
    {
        LOGOUTPUT(LogLevelHigh, ("\nLanguage: %s", pMsg->language));
    }
    if(pMsg->accept_text)
    {
        LOGOUTPUT(LogLevelHigh, ("\nAccepted Text: %s", pMsg->accept_text));
    }
    if(pMsg->center_text)
    {
        LOGOUTPUT(LogLevelHigh, ("\nCenter Text: %s", pMsg->center_text));
    }
    if(pMsg->decline_text)
    {
        LOGOUTPUT(LogLevelHigh, ("\nDecline Text: %s", pMsg->decline_text));
    }
    if(pMsg->url)
    {
        LOGOUTPUT(LogLevelHigh, ("\nURL: %s", pMsg->url));
    }

    LOGOUTPUT(LogLevelHigh, ("\nMessage Confirm: %s", pMsg->confirm ? "TRUE" : "FALSE"));

    LOGOUTPUT(LogLevelHigh, ("\nTime: %d\n", pMsg->time));

}

void PrintClientMessage (AB_ClientStoredMessageDetail* pMsg)
{
    LOGOUTPUT(LogLevelHigh, ("\n\n******* Message Details *******\n"));

    if(pMsg->type)
    {
        LOGOUTPUT(LogLevelHigh, ("\nMessage ID: %s\n", pMsg->type));
    }

    if(pMsg->language)
    {
        LOGOUTPUT(LogLevelHigh, ("\nMessage ID: %s\n", pMsg->language));
    }

    if(pMsg->timestamp)
    {
        LOGOUTPUT(LogLevelHigh, ("\nMessage ID: %d\n", pMsg->timestamp));
    }
}

void LogBundleOption (AB_BundleOption* bundle)
{
    LOGOUTPUT(LogLevelMedium, ("\n Bundle Name ........%s" , bundle->name)); 
    LOGOUTPUT(LogLevelMedium, ("\n description ........%s" , bundle->description)); 
    LOGOUTPUT(LogLevelMedium, ("\n title ..............%s" , bundle->title));
    LOGOUTPUT(LogLevelMedium, ("\n feature codes ......%s" , bundle->feature_codes));
    LOGOUTPUT(LogLevelMedium, ("\n price option count ........%d" , bundle->price_option_count)); 
}

void LogPriceOption(AB_PriceOption* price)
{
    LOGOUTPUT(LogLevelMedium, ("\n Price data.......%s", price->data));
    LOGOUTPUT(LogLevelMedium, ("....type............%s", price->type));
    LOGOUTPUT(LogLevelMedium, ("....recommendedoption...%d", price->is_recommended_option));
    LOGOUTPUT(LogLevelMedium, ("....end date............%u", price->enddate));

}

NB_Error
FormattedTextCallback(NB_Font font,       /*!< Font used for returned text */
                      nb_color color,     /*!< Font color for returned text */
                      const char* text,   /*!< Returned text */
                      uint8 newline,      /*!< TRUE if a new line should be added. */
                      const char* href,
                      const char* hreftext,
                      void* userData      /*!< User data passed in NB_SearchInformationGetFormattedContentText() */
                      )
{
    // The text field always has to be valid
    //CU_ASSERT_PTR_NOT_NULL(text);

    if (newline)
    {
        LOGOUTPUT(LogLevelHigh, ("\n"));
    }
    else if(text)
    {
        // Output the event details
        LOGOUTPUT(LogLevelHigh, ("      Content Details (font:%d, color:%d): %s\n", font, color, text));
    }
    else if(href)
    {
        // Output the event details
        LOGOUTPUT(LogLevelHigh, ("      Content Details %s %s\n", href, hreftext));
    }


    return NE_OK;
}

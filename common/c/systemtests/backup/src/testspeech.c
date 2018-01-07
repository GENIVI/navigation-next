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

@file     testspeech.c
@date     03/19/2009
@defgroup TestSpeech_h System tests for AB_Asr functions

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

#include "testspeech.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"

#include "palclock.h"
#include "palfile.h"

#include "nbcontext.h"
#include "abspeechrecognitionhandler.h"
#include "abspeechstatisticshandler.h"
#include "abqalog.h"


// Local Functions ...........................................................

// All tests. Add all tests to the TestSpeech_AddTests function
static void TestSpeechRecognition(void);
static void TestSpeechRecognitionPlace(void);
static void TestSpeechRecognitionExtended(void);
static void TestSpeechRecognitionFullAddress(void);


// Common test struct and functions ..........................................

struct SpeechTestParameters
{
    uint32 options;
    const char* audioFileName;
    const char* utteranceFieldType;
    const char* utteranceDataFormat;
    const char* utteranceText;
    const char* textData;
    const char* textFieldType;
    const char* textFieldSource;
    uint32 expectedLocationCount;
    uint32 expectedPlaceCount;
};

static void SpeechRecognitionTest(struct SpeechTestParameters* testParams);
static void SpeechStatisticsTest(PAL_Instance* pal, NB_Context* context, struct SpeechTestParameters* params, const char* interactionID, const char* validDataId);

static void SpeechCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);


// Constants .................................................................


// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_SpeechRecognitionCallbackEvent = NULL;


/*! Add all your test functions here

@return None
*/
void 
TestSpeech_AddTests( CU_pSuite pTestSuite, int level )
{
    // ! Add all your function names here !
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestSpeechRecognition", TestSpeechRecognition);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestSpeechRecognitionPlace", TestSpeechRecognitionPlace);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestSpeechRecognitionExtended", TestSpeechRecognitionExtended);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestSpeechRecognitionFullAddress", TestSpeechRecognitionFullAddress);
};

/*! Add common initialization code here.

@return 0

@see TestSpeech_SuiteCleanup
*/
int 
TestSpeech_SuiteSetup()
{
    // Create event for callback synchronization
    g_SpeechRecognitionCallbackEvent = CreateCallbackCompletedEvent();
	return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestSpeech_SuiteSetup
*/
int 
TestSpeech_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_SpeechRecognitionCallbackEvent);
	return 0;
}


void
TestSpeechRecognition(void)
{
    struct SpeechTestParameters params = { 0 };

    params.options = 0;
    params.audioFileName = "95014.qcp";
    params.utteranceFieldType = "address-postalcode";
    params.utteranceDataFormat ="qcp";
    params.utteranceText = "95014";
    params.textData = "Cordova Rd";
    params.textFieldType = "address-street";
    params.textFieldSource = 0;
    params.expectedLocationCount = 1;
    params.expectedPlaceCount = 0;

    SpeechRecognitionTest(&params);
}


void
TestSpeechRecognitionPlace(void)
{
    struct SpeechTestParameters params = { 0 };

    params.options = 0;
    params.audioFileName = "Starbucks.qcp";
    params.utteranceFieldType = "local-search-poi";
    params.utteranceDataFormat ="qcp";
    params.utteranceText = "Starbucks";
    params.textData = 0;
    params.textFieldType = 0;
    params.textFieldSource = 0;
    params.expectedLocationCount = 0;
    params.expectedPlaceCount = 1;

    SpeechRecognitionTest(&params);
}


void
TestSpeechRecognitionExtended(void)
{
    struct SpeechTestParameters params = { 0 };

    params.options = AB_ASR_WantAccuracy | AB_ASR_WantExtendedCodes;
    params.audioFileName = "95014.qcp";
    params.utteranceFieldType = "address-postalcode";
    params.utteranceDataFormat ="qcp";
    params.utteranceText = "95014";
    params.textData = "Cordova Rd";
    params.textFieldType = "address-street";
    params.textFieldSource = "usertext";
    params.expectedLocationCount = 1;
    params.expectedPlaceCount = 0;

    SpeechRecognitionTest(&params);
}


void
TestSpeechRecognitionFullAddress(void)
{
    struct SpeechTestParameters params = { 0 };

    params.options = 0;
    params.audioFileName = "full_6liberty.wav";
    params.utteranceFieldType = "address-full";
    params.utteranceDataFormat ="wav";
    params.utteranceText = "6 Liberty, Aliso Viejo, CA, 92656";
    params.textData = 0;
    params.textFieldType = 0;
    params.textFieldSource = 0;
    params.expectedLocationCount = 1;
    params.expectedPlaceCount = 0;

    SpeechRecognitionTest(&params);
}


void
SpeechRecognitionTest(struct SpeechTestParameters* params)
{
    NB_Error err = NE_OK;

    PAL_Instance* pal = 0;
    NB_Context* context = 0;

    nb_boolean rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_SpeechRecognitionParameters* recoParams = 0;
        AB_SpeechRecognitionHandler* recoHandler = 0;
        AB_SpeechRecognitionInformation* recoInfo = 0;

        NB_RequestHandlerCallback callback = { SpeechCallback, 0 };

        byte* audioData = NULL;
        uint32 audioDataSize = 0;

        PAL_Error result = PAL_Ok;

        // test QA log for speech record start
        AB_QaLogSpeechRecognitionRecordStart(context, params->utteranceFieldType);

        result = PlatformLoadFile(pal, params->audioFileName, &audioData, &audioDataSize);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_NOT_EQUAL(audioData, NULL);

        // test QA log for speech record stop
        AB_QaLogSpeechRecognitionRecordStop(context, params->utteranceFieldType, audioDataSize);

        if (audioData)
        {
            NB_GpsLocation gpsFix = { 0 };
            gpsFix.status   = PAL_Ok;
            gpsFix.valid    = NGV_Latitude | NGV_Longitude;
            gpsFix.gpsTime    = PAL_ClockGetGPSTime();
            gpsFix.latitude   = 33.58;
            gpsFix.longitude  = -117.73;

            err = AB_SpeechRecognitionParametersCreate(context, &gpsFix, 0, &recoParams);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = err ? err : AB_SpeechRecognitionParametersSetConfigurationOptions(recoParams, params->options);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = err ? err : AB_SpeechRecognitionParametersAddUtterance(recoParams, params->utteranceFieldType, "", params->utteranceDataFormat, audioData, audioDataSize);
            CU_ASSERT_EQUAL(err, NE_OK);

            if (params->textData)
            {
                if (params->textFieldSource)
                {
                    err = err ? err : AB_SpeechRecognitionParametersAddTextFromSource(recoParams, params->textFieldType, params->textData, params->textFieldSource);
                    CU_ASSERT_EQUAL(err, NE_OK);
                }
                else
                {
                    err = err ? err : AB_SpeechRecognitionParametersAddText(recoParams, params->textFieldType, params->textData);
                    CU_ASSERT_EQUAL(err, NE_OK);
                }
            }
            
            callback.callbackData = &recoInfo;
            err = AB_SpeechRecognitionHandlerCreate(context, &callback, &recoHandler);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_SpeechRecognitionHandlerStartRequest(recoHandler, recoParams);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (WaitForCallbackCompletedEvent(g_SpeechRecognitionCallbackEvent, 60000))
            {
                CU_ASSERT_NOT_EQUAL(recoInfo, 0);
                if (recoInfo)
                {
                    AB_SpeechRecognitionResults results = {0};

                    NB_Location location = {{0}};
                    NB_Place place = {{0}};
                    double distance = 0.0;
                    const char* dataId = 0;
                    const char* validDataId = 0;
                    uint32 n = 0;

                    err = err ? err : AB_SpeechRecognitionInformationGetRecognizeResults(recoInfo, &results);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    CU_ASSERT_EQUAL(results.completionCode, 0);
                    CU_ASSERT_EQUAL(results.dataIdCount, 1);
                    CU_ASSERT_EQUAL(results.locationMatchCount, params->expectedLocationCount);
                    CU_ASSERT_EQUAL(results.placeMatchCount, params->expectedPlaceCount);

                    for (n = 0; n <= results.locationMatchCount; n++)
                    {
                        err = AB_SpeechRecognitionInformationGetRecognizedLocation(recoInfo, n, &location);
                        CU_ASSERT_EQUAL(err, n < results.locationMatchCount ? NE_OK : NE_NOENT);
                    }

                    for (n = 0; n <= results.placeMatchCount; n++)
                    {
                        err = AB_SpeechRecognitionInformationGetRecognizedPlace(recoInfo, n, &place, &distance);
                        CU_ASSERT_EQUAL(err, n < results.placeMatchCount ? NE_OK : NE_NOENT);
                    }

                    for (n = 0; n <= results.dataIdCount; n++)
                    {
                        err = AB_SpeechRecognitionInformationGetSavedDataId(recoInfo, n, &dataId);
                        CU_ASSERT_EQUAL(err, n < results.dataIdCount ? NE_OK : NE_NOENT);

                        // save dataId for statistics
                        if (!n)
                        {
                            validDataId = dataId;
                        }
                    }

                    SpeechStatisticsTest(pal, context, params, results.interactionID, validDataId);

                    err = AB_SpeechRecognitionInformationDestroy(recoInfo);
                    CU_ASSERT_EQUAL(err, NE_OK);
                }
            }
            else
            {
                CU_ASSERT(FALSE);
            }

            err = AB_SpeechRecognitionHandlerDestroy(recoHandler);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_SpeechRecognitionParametersDestroy(recoParams);
            CU_ASSERT_EQUAL(err, NE_OK);

            nsl_free(audioData);
        }
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


void
SpeechStatisticsTest(PAL_Instance* pal, NB_Context* context, struct SpeechTestParameters* params, const char* interactionID, const char* validDataId)
{
    NB_Error err = NE_OK;

    AB_SpeechStatisticsParameters* statsParams = 0;
    AB_SpeechStatisticsHandler* statsHandler = 0;

    NB_RequestHandlerCallback callback = { SpeechCallback, 0 };

    if (pal && context && params)
    {
        err = AB_SpeechStatisticsParametersCreate(context, interactionID, &statsParams);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (params->textData)
        {
            if (params->textFieldSource)
            {
                err = err ? err : AB_SpeechStatisticsParametersAddTextFromSource(statsParams, params->textFieldType, params->textData, params->textFieldSource);
                CU_ASSERT_EQUAL(err, NE_OK);
            }
            else
            {
                err = err ? err : AB_SpeechStatisticsParametersAddText(statsParams, params->textFieldType, params->textData);
                CU_ASSERT_EQUAL(err, NE_OK);
            }
        }

        err = err ? err : AB_SpeechStatisticsParametersAddUtteranceFromDataStore(statsParams, params->utteranceFieldType, params->utteranceText, params->utteranceDataFormat, validDataId);
        CU_ASSERT_EQUAL(err, NE_OK);

        callback.callbackData = 0;
        err = err ? err : AB_SpeechStatisticsHandlerCreate(context, &callback, &statsHandler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = err ? err : AB_SpeechStatisticsHandlerStartRequest(statsHandler, statsParams);
        CU_ASSERT_EQUAL(err, NE_OK);

        CU_ASSERT_EQUAL(WaitForCallbackCompletedEvent(g_SpeechRecognitionCallbackEvent, 60000), TRUE);

        err = AB_SpeechStatisticsHandlerDestroy(statsHandler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_SpeechStatisticsParametersDestroy(statsParams);
        CU_ASSERT_EQUAL(err, NE_OK);
    }
    else
    {
        CU_ASSERT(FALSE);
    }
}


void
SpeechCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    if (!up)
    {
        if (err != NE_OK)
        {
            // Abort and trigger event
            CU_FAIL("Callback returned error");
            SetCallbackCompletedEvent(g_SpeechRecognitionCallbackEvent);
            return;
        }

        switch (status)
        {
            case NB_NetworkRequestStatus_Progress:
                // not complete yet... return
                return;
            case NB_NetworkRequestStatus_Success:
                if (userData)
                {
                    err = AB_SpeechRecognitionHandlerGetInformation(handler, userData);
                }
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
        SetCallbackCompletedEvent(g_SpeechRecognitionCallbackEvent);
    }
}


/*! @} */

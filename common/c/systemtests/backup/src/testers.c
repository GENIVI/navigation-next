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

@file testers.c
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

#include "testers.h"
#include "testnetwork.h"
#include "main.h"
#include "abershandler.h"
#include "abersinformation.h"
#include "abersparameters.h"

static void ErsHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData);

static void TestErs(void);
static void TestErsNoGps(void);

static void DoErsRequest(NB_Context* context, AB_ErsParameters* parameters);


struct TestErsCallbackData
{
    void*                           event;          /*!< Event to signal when callback complete */
    nb_boolean                      succeeded;      /*!< Whether or not request was successful */
};


void TestErs_AddTests(CU_pSuite pTestSuite, int level)
{
    Test_Options* options = Test_OptionsGet();
    if (options->carrier & TestCarrierVerizon)
    {
        ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestErs", TestErs);
        ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestErsNoGps", TestErsNoGps);
    }
};


int TestErs_SuiteSetup()
{
	return 0;
}


int TestErs_SuiteCleanup()
{
	return 0;
}

void TestErs(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_ErsParameters* parameters = 0;
        NB_Phone phone = { 0 };
        NB_GpsLocation location = { 0 };

        phone.type = NB_Phone_Primary;
        nsl_strcpy(phone.country, "USA");
        nsl_strcpy(phone.area, "949");
        nsl_strcpy(phone.number, "4533405");

        location.gpsTime = 933968318;
        location.latitude  = 33.55961648;
        location.longitude = -117.7295304;
        location.valid = NGV_Latitude | NGV_Longitude;

        CU_ASSERT_EQUAL(AB_ErsParametersCreate(context, 0, &phone, &location, &parameters), NE_OK);
        if (parameters)
        {
            DoErsRequest(context, parameters);
            AB_ErsParametersDestroy(parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


void TestErsNoGps(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_ErsParameters* parameters = 0;
        NB_Phone phone = { 0 };

        phone.type = NB_Phone_Primary;
        nsl_strcpy(phone.country, "USA");
        nsl_strcpy(phone.area, "949");
        nsl_strcpy(phone.number, "4533405");

        CU_ASSERT_EQUAL(AB_ErsParametersCreate(context, 0, &phone, 0, &parameters), NE_OK);
        if (parameters)
        {
            DoErsRequest(context, parameters);
            AB_ErsParametersDestroy(parameters);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


void DoErsRequest(NB_Context* context, AB_ErsParameters* parameters)
{
    AB_ErsHandler* handler = 0;
    NB_RequestHandlerCallback callback = { 0 };
    struct TestErsCallbackData callbackData = { 0 };

    callback.callback = ErsHandlerCallback;
    callback.callbackData = &callbackData;

    callbackData.event = CreateCallbackCompletedEvent();
    CU_ASSERT_PTR_NOT_NULL(callbackData.event);

    CU_ASSERT_EQUAL(AB_ErsHandlerCreate(context, &callback, &handler), NE_OK);
    if (handler)
    {
        CU_ASSERT_EQUAL(AB_ErsHandlerStartRequest(handler, parameters), NE_OK);

        CU_ASSERT(WaitForCallbackCompletedEvent(callbackData.event, 35000));
        CU_ASSERT_TRUE(callbackData.succeeded);

        CU_ASSERT_EQUAL(AB_ErsHandlerDestroy(handler), NE_OK);
    }

    DestroyCallbackCompletedEvent(callbackData.event);
}


void ErsHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    struct TestErsCallbackData* data = userData;
    data->succeeded = FALSE;

    if (status == NB_NetworkRequestStatus_Success)
    {
        AB_ErsInformation* information = 0;

        CU_ASSERT_EQUAL(AB_ErsHandlerGetErsInformation(handler, &information), NE_OK);
        if (information)
        {
            uint32 count = 0;
            uint32 index = 0;

            data->succeeded = TRUE;

            CU_ASSERT_EQUAL(AB_ErsInformationGetContactCount(information, &count), NE_OK);
            if (count)
            {
                for (index = 0; index < count; index++)
                {
                    AB_ErsContact* contact = 0;
                    CU_ASSERT_EQUAL(AB_ErsInformationGetContact(information, index, &contact), NE_OK);
                    if (contact)
                    {
                        LOGOUTPUT(LogLevelLow, ("Name: %s, phone = %s (%s)%s, type = %d", contact->name, contact->phone.country, contact->phone.area, contact->phone.number, contact->phone.type));

                        AB_ErsContactDestroy(contact);
                    }
                }
            }
            else
            {
                LOGOUTPUT(LogLevelLow, ("No contacts returned\n"));
            }

            AB_ErsInformationDestroy(information);
        }
    }

    if (status != NB_NetworkRequestStatus_Progress)
    {
        CU_ASSERT_EQUAL(status, NB_NetworkRequestStatus_Success);
        SetCallbackCompletedEvent(data->event);
    }
}


/*! @} */

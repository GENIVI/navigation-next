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

@file     TestDataStore.c
@date     03/19/2009
@defgroup TestDataStore_h System tests for AB_DataStore functions

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

#include "testdatastore.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"

#include "palfile.h"

#include "nbcontext.h"
#include "abdatastorehandler.h"


// Local Functions ...........................................................

// All tests. Add all tests to the TestDataStore_AddTests function
static void TestDataStoreStoreRetrieveDelete(void);
static void TestDataStoreInvalidId(void);

// Subfunctions of tests
static void DataStoreCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);

static void RetrieveStoredDataStore(NB_Context* context, char* id, byte* expectedData, nb_size expectedDataSize);
static void DeleteStoredDataStore(NB_Context* context, char* id);

// Constants .................................................................

// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_DataStoreCallbackEvent = NULL;


/*! Add all your test functions here

@return None
*/
void 
TestDataStore_AddTests( CU_pSuite pTestSuite, int level )
{
	// ! Add all your function names here !
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestDataStoreStoreRetrieveDelete", TestDataStoreStoreRetrieveDelete);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestDataStoreInvalidId", TestDataStoreInvalidId);
}


/*! Add common initialization code here.

@return 0

@see TestDataStore_SuiteCleanup
*/
int 
TestDataStore_SuiteSetup()
{
    // Create event for callback synchronization
    g_DataStoreCallbackEvent = CreateCallbackCompletedEvent();
	return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestDataStore_SuiteSetup
*/
int 
TestDataStore_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_DataStoreCallbackEvent);
	return 0;
}

void
TestDataStoreStoreRetrieveDelete(void)
{
    NB_Error err = NE_OK;
    
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_DataStoreParameters* params = 0;
        AB_DataStoreHandler* handler = 0;
        AB_DataStoreInformation* info = 0;
        
        NB_RequestHandlerCallback callback = { DataStoreCallback, 0 };
        
        byte* pTestData = 0;
        uint32 testDataSize = 0;
        
        PAL_Error result = PlatformLoadFile(pal, "6Liberty.qcp", &pTestData, &testDataSize);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_NOT_EQUAL(pTestData, NULL);
        
        if (pTestData)
        {
            err = AB_DataStoreParametersCreate(context, &params);
            CU_ASSERT_EQUAL(err, NE_OK);
            
            err = AB_DataStoreParametersAddStore(params, pTestData, testDataSize);
            CU_ASSERT_EQUAL(err, NE_OK);
            
            err = AB_DataStoreParametersAddStore(params, pTestData, testDataSize >> 1);
            CU_ASSERT_EQUAL(err, NE_OK);
            
            err = AB_DataStoreParametersAddStore(params, pTestData, testDataSize >> 2);
            CU_ASSERT_EQUAL(err, NE_OK);
            
            callback.callbackData = &info;
            err = AB_DataStoreHandlerCreate(context, &callback, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_DataStoreHandlerStartRequest(handler, params);
            CU_ASSERT_EQUAL(err, NE_OK);
            
            if (WaitForCallbackCompletedEvent(g_DataStoreCallbackEvent, 30000))
            {
                CU_ASSERT_NOT_EQUAL(info, 0);
                if (info)
                {
                    uint32 resultCount = 0;
                    uint32 completionCode = 0;
                    uint32 n = 0;
                    
                    err = AB_DataStoreInformationGetResultCount(info, &resultCount, &completionCode);
                    CU_ASSERT_EQUAL(err, 0);
                    CU_ASSERT_EQUAL(resultCount, 3);
                    CU_ASSERT_EQUAL(completionCode, 0);
                    
                    for (n = 0; n < resultCount; n++)
                    {
                        byte* data = 0;
                        nb_size dataSize = 0;
                        err = AB_DataStoreInformationGetResult(info, n, &data, &dataSize);
                        CU_ASSERT_EQUAL(err, NE_OK);
                        CU_ASSERT_NOT_EQUAL(data, 0);
                        CU_ASSERT_NOT_EQUAL(dataSize, 0);
                        
                        if (!err && data)
                        {
                            RetrieveStoredDataStore(context, (char*)data, pTestData, testDataSize >> n);
                            DeleteStoredDataStore(context, (char*)data);
                        }
                        
                        nsl_free(data);
                    }
                    
                    err = AB_DataStoreInformationDestroy(info);
                }
            }
            else
            {
                CU_ASSERT(FALSE);
            }

            err = AB_DataStoreHandlerDestroy(handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            
            err = AB_DataStoreParametersDestroy(params);
            CU_ASSERT_EQUAL(err, NE_OK);

            nsl_free(pTestData);
        }
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}

void
TestDataStoreInvalidId(void)
{
    NB_Error err = NE_OK;
    
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_DataStoreParameters* params = 0;
        AB_DataStoreHandler* handler = 0;
        AB_DataStoreInformation* info = 0;

        NB_RequestHandlerCallback callback = { DataStoreCallback, 0 };
        
        char* INVALID_ID = "BOGUS.DATASTORE.ID";
        
        err = AB_DataStoreParametersCreate(context, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_DataStoreParametersAddRetrieve(params, INVALID_ID);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_DataStoreParametersAddDelete(params, INVALID_ID);
        CU_ASSERT_EQUAL(err, NE_OK);

        callback.callbackData = &info;
        err = AB_DataStoreHandlerCreate(context, &callback, &handler);

        err = AB_DataStoreHandlerStartRequest(handler, params);
        
        if (WaitForCallbackCompletedEvent(g_DataStoreCallbackEvent, 30000))
        {
            CU_ASSERT_NOT_EQUAL(info, 0);
            if (info)
            {
                uint32 resultCount = 0;
                uint32 completionCode = 0;
                uint32 n = 0;
                
                err = AB_DataStoreInformationGetResultCount(info, &resultCount, &completionCode);
                CU_ASSERT_EQUAL(err, 0);
                CU_ASSERT_EQUAL(resultCount, 2);
                CU_ASSERT_EQUAL(completionCode, 1);
                
                for (n = 0; n < resultCount; n++)
                {
                    byte* data = 0;
                    nb_size dataSize = 0;
                    err = AB_DataStoreInformationGetResult(info, n, &data, &dataSize);
                    if (err == NE_OK)           // delete returns id regardless if it exists
                    {
                        CU_ASSERT_NOT_EQUAL(data, 0);
                        CU_ASSERT_NOT_EQUAL(dataSize, 0);
                        nsl_free(data);
                    }
                    else if (err == NE_NOENT)   // retrieve unsuccessful
                    {
                        CU_ASSERT_EQUAL(data, 0);
                        CU_ASSERT_EQUAL(dataSize, 0);
                    }
                    else
                    {
                        CU_ASSERT(FALSE);
                    }
                }

                err = AB_DataStoreInformationDestroy(info);
            }
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_DataStoreHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        err = AB_DataStoreParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


void
RetrieveStoredDataStore(NB_Context* context, char* id, byte* expectedData, nb_size expectedDataSize)
{
    NB_Error err = NE_OK;
    
    AB_DataStoreParameters* params = 0;
    AB_DataStoreHandler* handler = 0;
    AB_DataStoreInformation* info = 0;

    NB_RequestHandlerCallback callback = { DataStoreCallback, 0 };
    
    err = AB_DataStoreParametersCreate(context, &params);
    CU_ASSERT_EQUAL(err, NE_OK);
    
    err = AB_DataStoreParametersAddRetrieve(params, id);
    CU_ASSERT_EQUAL(err, NE_OK);

    callback.callbackData = &info;
    err = AB_DataStoreHandlerCreate(context, &callback, &handler);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = AB_DataStoreHandlerStartRequest(handler, params);
    CU_ASSERT_EQUAL(err, NE_OK);
    
    if (WaitForCallbackCompletedEvent(g_DataStoreCallbackEvent, 30000))
    {
        CU_ASSERT_NOT_EQUAL(info, 0);
        if (info)
        {
            uint32 resultCount = 0;
            uint32 completionCode = 0;

            byte* data = 0;
            nb_size dataSize = 0;
            
            err = AB_DataStoreInformationGetResultCount(info, &resultCount, &completionCode);
            CU_ASSERT_EQUAL(err, 0);
            CU_ASSERT_EQUAL(resultCount, 1);
            CU_ASSERT_EQUAL(completionCode, 0);
            
            err = AB_DataStoreInformationGetResult(info, 0, &data, &dataSize);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_NOT_EQUAL(data, 0);
            CU_ASSERT_EQUAL(dataSize, expectedDataSize);
            
            if (data)
            {
                CU_ASSERT_EQUAL(nsl_memcmp(data, expectedData, expectedDataSize), 0);
                nsl_free(data);
            }

            err = AB_DataStoreInformationDestroy(info);
        }
    }
    else
    {
        CU_ASSERT(FALSE);
    }

    err = AB_DataStoreHandlerDestroy(handler);
    CU_ASSERT_EQUAL(err, NE_OK);
    
    err = AB_DataStoreParametersDestroy(params);
    CU_ASSERT_EQUAL(err, NE_OK);
}

void
DeleteStoredDataStore(NB_Context* context, char* id)
{
    NB_Error err = NE_OK;
    
    AB_DataStoreParameters* params = 0;
    AB_DataStoreHandler* handler = 0;
    AB_DataStoreInformation* info = 0;

    NB_RequestHandlerCallback callback = { DataStoreCallback, 0 };
    
    err = AB_DataStoreParametersCreate(context, &params);
    CU_ASSERT_EQUAL(err, NE_OK);
    
    err = AB_DataStoreParametersAddDelete(params, id);
    CU_ASSERT_EQUAL(err, NE_OK);

    callback.callbackData = &info;
    err = AB_DataStoreHandlerCreate(context, &callback, &handler);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = AB_DataStoreHandlerStartRequest(handler, params);
    CU_ASSERT_EQUAL(err, NE_OK);
    
    if (WaitForCallbackCompletedEvent(g_DataStoreCallbackEvent, 30000))
    {
        CU_ASSERT_NOT_EQUAL(info, 0);
        if (info)
        {
            uint32 resultCount = 0;
            uint32 completionCode = 0;

            byte* data = 0;
            nb_size dataSize = 0;
            
            err = AB_DataStoreInformationGetResultCount(info, &resultCount, &completionCode);
            CU_ASSERT_EQUAL(err, 0);
            CU_ASSERT_EQUAL(resultCount, 1);
            CU_ASSERT_EQUAL(completionCode, 0);
            
            err = AB_DataStoreInformationGetResult(info, 0, &data, &dataSize);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_NOT_EQUAL(data, 0);
            
            if (data)
            {
                CU_ASSERT_EQUAL(nsl_memcmp(data, id, nsl_strlen(id)), 0);
                nsl_free(data);
            }

            err = AB_DataStoreInformationDestroy(info);
        }
    }
    else
    {
        CU_ASSERT(FALSE);
    }

    err = AB_DataStoreHandlerDestroy(handler);
    CU_ASSERT_EQUAL(err, NE_OK);
    
    err = AB_DataStoreParametersDestroy(params);
    CU_ASSERT_EQUAL(err, NE_OK);}

void 
DataStoreCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    if (!up)
    {
        if (err != NE_OK)
        {
            // Abort and trigger event
            CU_FAIL("Callback returned error");
            SetCallbackCompletedEvent(g_DataStoreCallbackEvent);
            return;
        }

        switch (status)
        {
            case NB_NetworkRequestStatus_Progress:
                // not complete yet... return
                return;
                
            case NB_NetworkRequestStatus_Success:
            {
                AB_DataStoreInformation* info = 0;
                err = AB_DataStoreHandlerGetInformation(handler, &info);
                CU_ASSERT_EQUAL(err, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(info);
                if (userData)
                {
                    *(AB_DataStoreInformation**)userData = info;
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
        SetCallbackCompletedEvent(g_DataStoreCallbackEvent);
    }
}

/*! @} */

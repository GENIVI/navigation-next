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

    @file     TestSync.c
    @date     03/19/2009
    @defgroup TestSync_h System tests for AB_PlaceSync functions

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

#include "testsync.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"
#include "csltypes.h"

#include "nbcontext.h"
#include "absynchronizationstatushandler.h"
#include "absynchronizationhandler.h"

// Local Functions ...........................................................

// All tests. Add all tests to the TestSync_AddTests function
static void TestSynchronizationStatus(void);
static void TestSynchronization(void);

static void TestSynchronizationFavorites(void);


// Subfunctions of tests
static void RequestSynchronizationStatusCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);
static void RequestSynchronizationCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);


// Constants .................................................................

#define FAVORITES_DB_NAME       "favorites"
#define PLACE_INBOX_DB_NAME     "placeinbox"
#define PLACE_OUTBOX_DB_NAME    "placeoutbox"
#define RECENTS_DB_NAME         "recents"


// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;


/*! Add all your test functions here

@return None
*/
void 
TestSync_AddTests( CU_pSuite pTestSuite, int level )
{
    // ! Add all your function names here !
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestSynchronizationStatus", TestSynchronizationStatus);

    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSynchronization", TestSynchronization);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSynchronizationFavorites", TestSynchronizationFavorites);
};


/*! Add common initialization code here.

@return 0

@see TestSync_SuiteCleanup
*/
int 
TestSync_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
	return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestSync_SuiteSetup
*/
int 
TestSync_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
	return 0;
}


void
TestSynchronizationStatus(void)
{
    NB_Error err = NE_OK;
    
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_SynchronizationStatusParameters* params = 0;
        AB_SynchronizationStatusHandler* handler = 0;
        AB_SynchronizationStatusInformation* info = 0;

        NB_RequestHandlerCallback callback = { RequestSynchronizationStatusCallback, 0 };
        err = AB_SynchronizationStatusParametersCreate(context, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_SynchronizationStatusParametersAddDatabase(params, RECENTS_DB_NAME);
        CU_ASSERT_EQUAL(err, NE_OK);
        err = AB_SynchronizationStatusParametersAddDatabase(params, FAVORITES_DB_NAME);
        CU_ASSERT_EQUAL(err, NE_OK);
        err = AB_SynchronizationStatusParametersAddDatabase(params, PLACE_INBOX_DB_NAME);
        CU_ASSERT_EQUAL(err, NE_OK);
        err = AB_SynchronizationStatusParametersAddDatabase(params, PLACE_OUTBOX_DB_NAME);
        CU_ASSERT_EQUAL(err, NE_OK);

        callback.callbackData = &info;
        err = AB_SynchronizationStatusHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        err = AB_SynchronizationStatusHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
        {
            uint32 generationId = 0;

            LOGOUTPUT(LogLevelHigh, ("\n  Synchronization status information:\n"));

            err = AB_SynchronizationStatusInformationGetDatabaseServerGenerationId(info, RECENTS_DB_NAME, &generationId);
            CU_ASSERT_EQUAL(err, NE_OK);
            LOGOUTPUT(LogLevelHigh, ("    '%s' generation ID: %u\n", RECENTS_DB_NAME, generationId));

            err = AB_SynchronizationStatusInformationGetDatabaseServerGenerationId(info, FAVORITES_DB_NAME, &generationId);
            CU_ASSERT_EQUAL(err, NE_OK);
            LOGOUTPUT(LogLevelHigh, ("    '%s' generation ID: %u\n", FAVORITES_DB_NAME, generationId));

            err = AB_SynchronizationStatusInformationGetDatabaseServerGenerationId(info, PLACE_INBOX_DB_NAME, &generationId);
            CU_ASSERT_EQUAL(err, NE_OK);
            LOGOUTPUT(LogLevelHigh, ("    '%s' generation ID: %u\n", PLACE_INBOX_DB_NAME, generationId));

            err = AB_SynchronizationStatusInformationGetDatabaseServerGenerationId(info, PLACE_OUTBOX_DB_NAME, &generationId);
            CU_ASSERT_EQUAL(err, NE_OK);
            LOGOUTPUT(LogLevelHigh, ("    '%s' generation ID: %u\n", PLACE_OUTBOX_DB_NAME, generationId));

            err = AB_SynchronizationStatusInformationDestroy(info);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_SynchronizationStatusHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        err = AB_SynchronizationStatusParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }
    
    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


void
TestSynchronization(void)
{
    NB_Error err = NE_OK;
    
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_SynchronizationParameters* params = 0;
        AB_SynchronizationHandler* handler = 0;
        AB_SynchronizationInformation* info = 0; 

        AB_SynchronizationOperation so = {{ 0 }};
        uint32 localOperationCount = 0;

        NB_RequestHandlerCallback callback = { RequestSynchronizationCallback, 0 };
        err = AB_SynchronizationParametersCreate(context, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_SynchronizationParamatersSetDatabaseData(params, RECENTS_DB_NAME, 0, 0);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        memset(&so, 0, sizeof(AB_SynchronizationOperation));
        so.place.location.latitude = 33.5;
        so.place.location.longitude = -117.5;
        nsl_strcpy(so.place.name, "Somewhere");
        so.placeValid = TRUE;
        so.operationType = ASOT_Add;
        nsl_strcpy(so.databaseName, RECENTS_DB_NAME);

        err = AB_SynchronizationParametersAddLocalOperation(params, &so);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_SynchronizationParametersGetLocalOperationCount(params, RECENTS_DB_NAME, ASOT_All, &localOperationCount);
        CU_ASSERT_EQUAL(err, NE_OK);
        CU_ASSERT_EQUAL(localOperationCount, 1);
        
        callback.callbackData = &info;
        err = AB_SynchronizationHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        err = AB_SynchronizationHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 30000))
        {
            uint32 index = 0;
            
            uint32 serverCount = 0;
            uint32 databaseId = 0;
            uint32 generationId = 0;
            
            nb_boolean syncErrorValid = FALSE;
            AB_SynchronizationError syncErrorCode = ASE_None;
            uint32 syncErrorValue = 0;
            
            uint32 opCount = 0;
            
            AB_SynchronizationOperation op = {{ 0 }};

            err = AB_SynchronizationInformationGetServerCount(info, RECENTS_DB_NAME, &serverCount);
            CU_ASSERT_EQUAL(err, NE_OK);
            
            err = AB_SynchronizationInformationGetDatabaseData(info, RECENTS_DB_NAME, &generationId, &databaseId);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_SynchronizationInformationGetServerError(info, RECENTS_DB_NAME, &syncErrorValid, &syncErrorCode, &syncErrorValue);
            CU_ASSERT_EQUAL(err, NE_OK);
       
            err = AB_SynchronizationInformationGetServerOperationCount(info, RECENTS_DB_NAME, ASOT_Add, &opCount);
            CU_ASSERT_EQUAL(err, NE_OK);
            
            for (index = 0; index < opCount; index++)
            {
                err = AB_SynchronizationInformationGetServerOperation(info, RECENTS_DB_NAME, ASOT_Add, index, &op);
                CU_ASSERT_EQUAL(err, NE_OK);
            }
            
            AB_SynchronizationInformationDestroy(info);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_SynchronizationHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        err = AB_SynchronizationParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }
    
    DestroyContext(context);
    PAL_DestroyInstance(pal);
}

struct SyncTestDataItem
{
    AB_PlaceID localId;
    AB_PlaceID serverId;
    const char* placeName;
    const char* placeCategoryCode;
    const char* placeStreet;
    const char* placeCity;
    const char* placeState;
    const char* placeZip;
};

static struct SyncTestDataItem favorites[] =
{
    { 1, 20179, "TeleCommunication Systems", "",  "6 Liberty",    "Aliso Viejo",  "CA",  "92656" },
    { 2, 20180, "Home",               "",  "8 Justice",    "Aliso Viejo",  "CA",  "92656" },
    { 3, 20181, "Bogus",              "",  "8 Justice",    "Aliso Viejo",  "CA",  "92656" },
    { 4, 20176, "TeleCommunication Systems", "",  "6 Liberty",    "Aliso Viejo",  "CA",  "99999" },
    { 5, 20177, "Home",               "",  "8 Justice",    "Aliso Viejo",  "CA",  "99999" },
    { 6, 20178, "Bogus",              "",  "8 Justice",    "Aliso Viejo",  "CA",  "99999" },
};
const uint32 favoritesCount = (sizeof(favorites) / sizeof(favorites[0]));

void
PopulatePlaceSynchronizationOperation(AB_SynchronizationOperation* pso, struct SyncTestDataItem* pTestItem, AB_SynchronizationOperationType opType, const char* dbName)
{
    nsl_memset(pso, 0, sizeof(AB_SynchronizationOperation));

    pso->operationType = opType;

    nsl_strcpy(pso->databaseName, dbName);
    pso->localId = pTestItem->localId;
    pso->serverId = pTestItem->serverId;

    nsl_strcpy(pso->place.name, pTestItem->placeName);

    nsl_strcpy(pso->place.category[0].code, pTestItem->placeCategoryCode);
    pso->place.numcategory = 1;

    pso->place.location.latitude = 33.5;
    pso->place.location.longitude = -117.5;
    nsl_strcpy(pso->place.location.street1, pTestItem->placeStreet);
    nsl_strcpy(pso->place.location.city, pTestItem->placeCity);
    nsl_strcpy(pso->place.location.state, pTestItem->placeState);
    nsl_strcpy(pso->place.location.postal, pTestItem->placeZip);
    pso->place.location.type = NB_Location_Address;

    pso->placeValid = TRUE;
}

const char*
StringOpType(AB_SynchronizationOperationType opType)
{
    const char* stringOpType = "unknown";

    switch (opType)
    {
        case ASOT_Add:              stringOpType = "add";               break;
        case ASOT_AssignId:         stringOpType = "assign id";         break;
        case ASOT_Delete:           stringOpType = "delete";            break;
        case ASOT_Modify:           stringOpType = "modify";            break;
        case ASOT_ModifyTimeStamp:  stringOpType = "modify time stamp"; break;
    }

    return stringOpType;
}

void
DumpSynchronizationOperations(AB_SynchronizationInformation* info, const char* database, AB_SynchronizationOperationType opType)
{
    NB_Error err = NE_OK;
    uint32 opCount = 0;
    uint32 index = 0;

    AB_SynchronizationOperation so = {{0}};

    err = AB_SynchronizationInformationGetServerOperationCount(info, database, opType, &opCount);
    CU_ASSERT_EQUAL(err, NE_OK);

    LOGOUTPUT(LogLevelHigh, ("\n%s %s operations: %d\n", database, StringOpType(opType), opCount));

    for (index = 0; index < opCount; index++)
    {
        err = AB_SynchronizationInformationGetServerOperation(info, database, opType, index, &so);
        CU_ASSERT_EQUAL(err, NE_OK);

        LOGOUTPUT(LogLevelHigh, ("  %d: local: %lld  server: %lld  category: %s  '%s': '%s', '%s', '%s'. '%s'\n",
            index, so.localId, so.serverId, so.place.category[0].code,
            so.place.name, so.place.location.street1, so.place.location.city, so.place.location.state, so.place.location.postal));
    }
}


void
TestSynchronizationFavorites(void)
{
    NB_Error err = NE_OK;

    PAL_Instance* pal = 0;
    NB_Context* context = 0;

    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_SynchronizationHandler* handler = 0;

        AB_SynchronizationParameters* params = 0;
        AB_SynchronizationInformation* info = 0; 

        AB_SynchronizationOperation so = {{ 0 }};
        uint32 localOperationCount = 0;
        uint32 serverOperationCount = 0;
        uint32 opIndex = 0;

        NB_RequestHandlerCallback callback = { RequestSynchronizationCallback, 0 };
        err = AB_SynchronizationParametersCreate(context, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_SynchronizationParamatersSetDatabaseData(params, FAVORITES_DB_NAME, 0, 0);
        CU_ASSERT_EQUAL(err, NE_OK);

        for (opIndex = 0; opIndex < favoritesCount; opIndex++)
        {
            PopulatePlaceSynchronizationOperation(&so, &favorites[opIndex], ASOT_Add, FAVORITES_DB_NAME);
            err = AB_SynchronizationParametersAddLocalOperation(params, &so);
            CU_ASSERT_EQUAL(err, NE_OK);
        }

        err = AB_SynchronizationParametersGetLocalOperationCount(params, FAVORITES_DB_NAME, ASOT_All, &localOperationCount);
        CU_ASSERT_EQUAL(err, NE_OK);
        CU_ASSERT_EQUAL(localOperationCount, favoritesCount);

        callback.callbackData = &info;
        err = AB_SynchronizationHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_SynchronizationHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);

        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 30000))
        {
            uint32 serverCount = 0;
            uint32 databaseId = 0;
            uint32 generationId = 0;

            nb_boolean syncErrorValid = FALSE;
            AB_SynchronizationError syncErrorCode = ASE_None;
            uint32 syncErrorValue = 0;

            err = AB_SynchronizationInformationGetServerCount(info, FAVORITES_DB_NAME, &serverCount);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_SynchronizationInformationGetDatabaseData(info, FAVORITES_DB_NAME, &generationId, &databaseId);
            CU_ASSERT_EQUAL(err, NE_OK);

            err = AB_SynchronizationInformationGetServerError(info, FAVORITES_DB_NAME, &syncErrorValid, &syncErrorCode, &syncErrorValue);
            CU_ASSERT_EQUAL(err, NE_OK);

            DumpSynchronizationOperations(info, FAVORITES_DB_NAME, ASOT_Add);
            DumpSynchronizationOperations(info, FAVORITES_DB_NAME, ASOT_AssignId);
            DumpSynchronizationOperations(info, FAVORITES_DB_NAME, ASOT_Modify);
            DumpSynchronizationOperations(info, FAVORITES_DB_NAME, ASOT_Delete);
            DumpSynchronizationOperations(info, FAVORITES_DB_NAME, ASOT_ModifyTimeStamp);

            err = AB_SynchronizationInformationGetServerOperationCount(info, FAVORITES_DB_NAME, ASOT_AssignId, &serverOperationCount);
            CU_ASSERT_EQUAL(err, NE_OK);

            for (opIndex = 0; opIndex < serverOperationCount; opIndex++)
            {
                uint32 itemIndex = 0;

                err = AB_SynchronizationInformationGetServerOperation(info, FAVORITES_DB_NAME, ASOT_AssignId, opIndex, &so);
                CU_ASSERT_EQUAL(err, NE_OK);

                for (itemIndex = 0; itemIndex < favoritesCount; itemIndex++)
                {
                    if (favorites[itemIndex].localId == so.localId)
                    {
                        favorites[itemIndex].serverId = so.serverId;
                        break;
                    }
                }
                CU_ASSERT_NOT_EQUAL(itemIndex, favoritesCount);
            }

            AB_SynchronizationInformationDestroy(info);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_SynchronizationHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_SynchronizationParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


void 
RequestSynchronizationCallback(
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
                // create info from request and store pointer back using userData
                AB_SynchronizationInformation** pInfo = (AB_SynchronizationInformation**)pUserData;
                err = AB_SynchronizationHandlerGetInformation(handler, pInfo);
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
RequestSynchronizationStatusCallback(
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
                // create info from request and store pointer back using userData
                AB_SynchronizationStatusInformation** pInfo = (AB_SynchronizationStatusInformation**)pUserData;
                err = AB_SynchronizationStatusHandlerGetInformation(handler, pInfo);
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


/*! @} */

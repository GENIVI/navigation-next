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

@file     TestFileset.c
@date     03/19/2009
@defgroup TestFileset_h System tests for AB_Fileset functions

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

#include "testfileset.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"

#include "palfile.h"

#include "nbcontext.h"
#include "abfilesethandler.h"
#include "abfilesetstatushandler.h"

// Local Functions ...........................................................

// All tests. Add all tests to the TestFileset_AddTests function
static void TestFileSetStatus(void);
static void TestFileSet(void);

// Subfunctions of tests

static void FileSetStatusCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
static void FileSetCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);


// Constants .................................................................

//static const char* OBFUSCATE_KEY = ";lk a'df9u043fG RT890ajga034j dfnmvo8y hgDSfbno90*SDVJs8N vsdPPOI&Bvn89 dfzdb79zdbh ad79&^&*^NLLSsdfhbo zduvn s05y w'zSIDGF:oF SDJKBF";

static const char* SERVER_FILESET_NAME = "msg-banners-en-gb-240x268";
//static const char* CLIENT_FILESET_FILE_NAME = "n13.dat";
//static const char* EXTRACT_FROM_FILESET_FILE_NAME = "320x186-411connect.bmp";
//static const char* EXTRACTED_LOCAL_FILE_NAME = "320x186-411connect-local.bmp";


// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;


/*! Add all your test functions here

@return None
*/
void 
TestFileset_AddTests( CU_pSuite pTestSuite, int level )
{
	// ! Add all your function names here !
	ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestFileSetStatus", TestFileSetStatus);
	ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestFileSet", TestFileSet);
};

/*! Add common initialization code here.

@return 0

@see TestFileset_SuiteCleanup
*/
int 
TestFileset_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
	return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestFileset_SuiteSetup
*/
int 
TestFileset_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
	return 0;
}


void
TestFileSetStatus(void)
{
    NB_Error err = NE_OK;
    
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_FileSetStatusParameters* params = 0;
        AB_FileSetStatusHandler* handler = 0;
        AB_FileSetStatusInformation* info = 0;

        NB_RequestHandlerCallback callback = { FileSetStatusCallback, 0 };
        
        nb_boolean inProgress = FALSE;
        
        err = AB_FileSetStatusParametersCreate(context, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_FileSetStatusParametersAddFileSet(params, SERVER_FILESET_NAME);
        CU_ASSERT_EQUAL(err, NE_OK);

        callback.callbackData = &info;
        err = AB_FileSetStatusHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_FileSetStatusHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        inProgress = AB_FileSetStatusHandlerIsRequestInProgress(handler);
        CU_ASSERT_EQUAL(inProgress, TRUE);

        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 30000))
        {
            uint32 fileSetStatusCount = 0;
            uint32 fileSetIndex = 0;
            
            // callback should have created and stored info pointer if successful
            CU_ASSERT_NOT_EQUAL(info, 0);

            err = AB_FileSetStatusInformationGetCount(info, &fileSetStatusCount);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_EQUAL(fileSetStatusCount, 1);
 
            for (fileSetIndex = 0; fileSetIndex < fileSetStatusCount; fileSetIndex++)
            {
                char fileSetName[80] = {0};
                uint64 fileSetTimeStamp = 0;
                
                err = AB_FileSetStatusInformationGetStatus(info, 0, fileSetName, sizeof(fileSetName), &fileSetTimeStamp);
                CU_ASSERT_EQUAL(err, NE_OK);
                CU_ASSERT_NOT_EQUAL(fileSetTimeStamp, 0);
            }           

            err = AB_FileSetStatusInformationDestroy(info);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_FileSetStatusHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_FileSetStatusParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


void
TestFileSet(void)
{
    NB_Error err = NE_OK;
    
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        AB_FileSetParameters* params = 0;
        AB_FileSetHandler* handler = 0;
        AB_FileSetInformation* info = 0;

        NB_RequestHandlerCallback callback = { FileSetCallback, 0 };
        
        nb_boolean inProgress = FALSE;
        
        err = AB_FileSetParametersCreate(context, &params);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_FileSetParametersAddFileSet(params, SERVER_FILESET_NAME);
        CU_ASSERT_EQUAL(err, NE_OK);

        callback.callbackData = &info;
        err = AB_FileSetHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_FileSetHandlerStartRequest(handler, params);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        inProgress = AB_FileSetHandlerIsRequestInProgress(handler);
        CU_ASSERT_EQUAL(inProgress, TRUE);

        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 30000))
        {
            uint32 fileSetCount = 0;
            uint32 fileSetIndex = 0;
            
            // callback should have created and stored info pointer if successful
            CU_ASSERT_NOT_EQUAL(info, 0);

            err = AB_FileSetInformationGetCount(info, &fileSetCount);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_EQUAL(fileSetCount, 1);
 
            for (fileSetIndex = 0; fileSetIndex < fileSetCount; fileSetIndex++)
            {
                char fileSetName[80] = {0};
                uint64 fileSetTimeStamp = 0;
                uint32 fileSetFileCount = 0;
                uint32 fileIndex = 0;
                
                err = AB_FileSetInformationGetStatus(info, 0, fileSetName, sizeof(fileSetName) - 1, &fileSetTimeStamp, &fileSetFileCount);
                
                for (fileIndex = 0; fileIndex < fileSetFileCount; fileIndex++)
                {
                    char fileName[80] = {0};
                    byte* fileData = 0;
                    nb_size fileDataSize = 0;
                    
                    err = AB_FileSetInformationGetFileData(info, fileSetIndex, fileIndex, fileName, sizeof(fileName), &fileData, &fileDataSize);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    CU_ASSERT_NOT_EQUAL(fileName[0], 0);
                    CU_ASSERT_NOT_EQUAL(fileData, 0);
                    CU_ASSERT_NOT_EQUAL(fileDataSize, 0);
                }
            }           

            err = AB_FileSetInformationDestroy(info);
            CU_ASSERT_EQUAL(err, NE_OK);
        }
        else
        {
            CU_ASSERT(FALSE);
        }

        err = AB_FileSetHandlerDestroy(handler);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = AB_FileSetParametersDestroy(params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


void 
FileSetStatusCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
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
                AB_FileSetStatusInformation** pInfo = (AB_FileSetStatusInformation**)userData;
                err = AB_FileSetStatusHandlerGetInformation(handler, pInfo);
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
FileSetCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
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
                AB_FileSetInformation** pInfo = (AB_FileSetInformation**)userData;
                err = AB_FileSetHandlerGetInformation(handler, pInfo);
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



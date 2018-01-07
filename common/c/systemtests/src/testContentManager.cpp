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

    @file     TestContentManager.h
    @defgroup TestContentManager_h System tests for Content functions
*/
/*
    See description in header file.

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

#include "OnboardContentManager.h"
extern "C"
{
#include "main.h"
#include "testnetwork.h"
#include "platformutil.h"
#include "paltimer.h"
#include "palfile.h"
#include "palunzip.h"
#include "testContentManager.h"
}

using namespace nbcommon;
// Local Constants ...........................................................
const uint32 TEST_MAX_PATH_LENGTH = 256;

const uint32 TEST_SCREEN_WIDTH = 780;
const uint32 TEST_SCREEN_HEIGHT = 585;

const string TEST_COUNTRY  = "USA";
const string TEST_NBGM_VERSION = "1";

static int downloadprgress = -1;

// directories for test data
#define MAP_REGIONS_PATH                            "MapData"
static bool ISSTARTED = false;

// Variables .................................................................

// Event gets triggered from the download callback.
class RegionalMapDataListenerImpl;
class DownloadStatusListenerImpl;
static void* g_CallbackEvent = NULL;
static shared_ptr<OnboardContentManager> contentManager;
static RegionalMapDataListenerImpl* mapDataListener = NULL;
static DownloadStatusListenerImpl* downloadStatusListener = NULL;

class RegionalMapDataListenerImpl : public RegionalMapDataListener
{
public:
    RegionalMapDataListenerImpl(){};
    void OnError(NB_Error error);
    void OnAvailableRegions(const std::vector<RegionalInformationPtr>& regions);
};

void RegionalMapDataListenerImpl::OnError(NB_Error error)
{

}

void RegionalMapDataListenerImpl::OnAvailableRegions(const std::vector<RegionalInformationPtr>& regions)
{
    if (!ISSTARTED)
    {
        contentManager->RequestRegionalData("USA-OR+WA");
        //contentManager->RequestRegionalData("USA-CA");
        ISSTARTED = true;
    }
}

class DownloadStatusListenerImpl : public DownloadStatusListener
{
public:
    DownloadStatusListenerImpl(){};
    void OnStatusChanged(const std::string& regionalId,
                                 NB_EnhancedContentDataType dataType,
                                 RegionDownloadStatus newStatus);

    void OnError(const std::string& regionalId, NB_Error error);

    void OnProgress(const std::string& regionalId, int percentage);
};

void DownloadStatusListenerImpl::OnStatusChanged(const std::string& regionalId,
                                                 NB_EnhancedContentDataType dataType,
                                                 RegionDownloadStatus newStatus)
{
    if (newStatus == RDS_Compelete)
    {
        printf(" %s download and uncompressed complate.\n", regionalId.c_str());
    }
}

void DownloadStatusListenerImpl::OnError(const std::string& regionalId, NB_Error error)
{
}

void DownloadStatusListenerImpl::OnProgress(const std::string& regionalId, int percentage)
{
    if (downloadprgress != percentage)
    {
        printf(" %s download percentage: %d\n", regionalId.c_str(),percentage);
        downloadprgress = percentage;
    }
}

// Local Functions ...........................................................

static void TestContentCheckAvailableRegions(void);
static void TimerCallbackForPauseDownload(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static void TimerCallbackForResumeDownload(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static void TimerCallbackForRemovingAllStoredData(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);

/*! Add all your test functions here

@return None
*/
extern "C" void TestContent_AddTests( CU_pSuite pTestSuite, int level )
{
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestContentStartMetadataSynchronization", TestContentCheckAvailableRegions);
}


/*! Add common initialization code here.

@return 0

@see TestDirection_SuiteCleanup
*/
extern "C" int TestContent_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestDirection_SuiteSetup
*/
extern "C" int TestContent_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
    return 0;
}


/*! Test content manager create and initial start sync

    @return None. CUnit Asserts get called on failures.
*/
void TestContentCheckAvailableRegions(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))  
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);
        NB_Error result = NE_OK;
        string basePath(GetBasePath());
        string clientGuid("1234567890");
        string language(Test_OptionsGet()->currentLanguage);
        string mapdataPath = basePath + PATH_DELIMITER +  MAP_REGIONS_PATH;

        OnboardContentManagerConfigPtr config(new OnboardContentManagerConfig(
                                                    TEST_SCREEN_WIDTH,
                                                    TEST_SCREEN_HEIGHT,
                                                    TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT,
                                                    TEST_COUNTRY,
                                                    language,
                                                    TEST_NBGM_VERSION,
                                                    mapdataPath,
                                                    basePath,
                                                    clientGuid,
                                                    NB_ECDT_MAP_ALL));
        contentManager = OnboardContentManager::CreateOnboardContentManager(context,config);

        mapDataListener = new RegionalMapDataListenerImpl();
        downloadStatusListener = new DownloadStatusListenerImpl();

        contentManager->RegisterDownloadStatusListener(downloadStatusListener);
        contentManager->RegisterRegionalMapDataListener(mapDataListener);
        
        result = contentManager->CheckAvaliableRegions();
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            CU_ASSERT_EQUAL(PAL_TimerSet(pal, 1200000, TimerCallbackForRemovingAllStoredData, contentManager.get()), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_TimerSet(pal, 180000, TimerCallbackForPauseDownload, contentManager.get()), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_TimerSet(pal, 240000, TimerCallbackForResumeDownload, contentManager.get()), PAL_Ok);
            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 3600000))
            {
                // TODO: check results
            }
            else
            {
                CU_FAIL("NB_EnhancedContentManagerStartSynchronization() failed");
            }
            CU_ASSERT_EQUAL(PAL_TimerCancel(pal, TimerCallbackForRemovingAllStoredData, contentManager.get()), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_TimerCancel(pal, TimerCallbackForPauseDownload, contentManager.get()), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_TimerCancel(pal, TimerCallbackForResumeDownload, contentManager.get()), PAL_Ok);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

void TimerCallbackForPauseDownload(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    OnboardContentManager* manager = (OnboardContentManager*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    result = manager->PauseDownload("USA-OR+WA", NB_ECD0_Preload);
    CU_ASSERT_EQUAL(result, NE_OK);
}
void TimerCallbackForResumeDownload(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    OnboardContentManager* manager = (OnboardContentManager*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    result = manager->ResumeDownload("USA-OR+WA", NB_ECD0_Preload);
    CU_ASSERT_EQUAL(result, NE_OK);
}
void TimerCallbackForRemovingAllStoredData(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    NB_Error result = NE_OK;
    OnboardContentManager* manager = (OnboardContentManager*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    result = manager->RemoveRegionalData("-1");
    CU_ASSERT_EQUAL(result, NE_OK);

    // Cancel data download successfully
    //SetCallbackCompletedEvent(g_CallbackEvent);
}
/*! @} */

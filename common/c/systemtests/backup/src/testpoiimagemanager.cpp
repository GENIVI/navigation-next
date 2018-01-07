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
   @file        testpoiimagemanager.cpp

   Implementation of unit tests for PoiImageManager.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of Networks In
   Motion is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

--------------------------------------------------------------------------*/

/*! @{ */
#include "testpoiimagemanager.h"

extern "C"
{
#include "main.h"
#include "nbcontext.h"
#include "nbsearchinformation.h"
#include "nbsinglesearchhandler.h"
#include "nbsinglesearchinformation.h"
#include "nbsinglesearchparameters.h"
#include "palfile.h"
#include "paltimer.h"
#include "platformutil.h"
#include "testnetwork.h"
}

#include "poiimagemanager.h"
#include "poiimagelistener.h"
#include "poiimageidentifier.h"

using namespace nbsearch;


// Local Constants .............................................................................

static const char* LARGE_IMAGE           = "LargeImage";
static const char* TEST_SEARCH_NAME      = "Friday";
static const char* TEST_SEARCH_SCHEME    = "tcs-single-search";
static const char* THUMBNAIL_IMAGE       = "ThumbNail";
static const char* TEST_CACHE_PATH       = "TestPoiImageManager";
static const int   MAX_ITEMS_IN_CACHE    = 5;
static const int   TEST_NUMBER_PER_SLICE = 64;
static const char* OUTPUT_FILE           = "TestPoi_%s_%d.png";
static int         saved_index           = 0;
static int         MaxRequests           = 0;

/*! Test class for PoiImageListener. */
class TestPoiImageListener : public PoiImageListener {
public:

    // Refer to base class for more documentation.
    TestPoiImageListener(PAL_Instance* pal, void *event);
    virtual ~TestPoiImageListener();

    virtual void
    ImageDownloaded(NB_Error result, const PoiImageIdentifierPtr identifier);

    virtual void
    ImageRemoved(const PoiImageIdentifierPtr identifier);

    void
    SetImageManager(PoiImageManager* manager);

private:
    // Private functions .......................................................................
    NB_Error SaveToFile(void* data, uint32 length, bool isThumbnail);

    // Private variables .......................................................................
    void*            m_event;
    PAL_Instance*    m_pal;
    PoiImageManager* m_imageManager;
    bool             m_itemRemoved;
};

/*! Test for PoiInstance. */
class TestPoiInstance
{
public:
    /*! Default constructor.  */
    TestPoiInstance();
    virtual ~TestPoiInstance();

    NB_Error Initialize();

    NB_Error StartSingleSearch();

    NB_Error StartDownloadSingleSearchImages(bool isThumbnail = TRUE,
                                             bool waitEvent   = TRUE
                                             );
    NB_Error CancelDownloading();
    NB_Error ClearCaches();
private:

    // Private functions .......................................................................
    static void
    RequestHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up,
                           int percent, void* userData);
    void Reset();
    NB_Error DoRequest();
    bool IsInstancesValid();

    // Private variables .......................................................................

    PAL_Instance* m_pal;                            /*!< Pointer to PAL instance */
    NB_Context*   m_context;                        /*!< Pointer to current context */
    void*         m_event;                          /*!< Callback completed event */

    PoiImageManager*      m_imageManager;           /*!<  POI image Manager  */
    TestPoiImageListener* m_listener;               /*!<  POI Image Listener  */

    NB_SingleSearchHandler*     m_handler;          /*!< Single search handler instance */
    NB_SingleSearchInformation* m_information;      /*!< Single search information instance */
    NB_SingleSearchParameters*  m_parameters;       /*!< Single search parameters */
};

/*! Collection of all instances used for the unit tests */
TestPoiImageListener::TestPoiImageListener(PAL_Instance* pal, void *event)
{
    m_pal         = pal;
    m_event       = event;
    m_itemRemoved = false;
}

TestPoiImageListener::~TestPoiImageListener()
{
}

/*! Dump content into file.

    @return NB_OK if succeeded, or other value otherwise.
*/
NB_Error
TestPoiImageListener::SaveToFile(void* data,      /*!< Data to be dumped. */
                                 uint32 length,   /*!< Length of data */
                                 bool isThumbnail /*!< Flag is to indicate if this is a thumbnail  */
                                 )
{

    PAL_Error palError = PAL_Ok;

    PAL_File* file = NULL;

    char filename[256];
    nsl_memset(filename, 0, 256);
    SPRINTF(filename, OUTPUT_FILE, isThumbnail ? THUMBNAIL_IMAGE : LARGE_IMAGE, ++saved_index);

    PAL_FileRemove(m_pal, filename);

    palError = PAL_FileOpen(m_pal, filename, PFM_Create, &file);

    CU_ASSERT_EQUAL(palError, PAL_Ok);
    if (palError != PAL_Ok)
    {
        return NE_FSYS;
    }

    if (length > 0)
    {
        uint32 bytesWritten = 0;
        uint32 position = 0;

        do {
            palError = PAL_FileWrite(file, (uint8*)data, length, &bytesWritten);
            CU_ASSERT_EQUAL(palError, PAL_Ok);
            if (palError != PAL_Ok)
            {
                return NE_FSYS;
            }

            position += bytesWritten;

            palError = PAL_FileSetPosition(file, PFSO_Start, position);
            CU_ASSERT_EQUAL(palError, PAL_Ok);
            if (palError != PAL_Ok)
            {
                return NE_FSYS;
            }
        } while (bytesWritten < length);
    }

    palError = PAL_FileClose(file);
    CU_ASSERT_EQUAL(palError, PAL_Ok);
    if (palError != PAL_Ok)
    {
        return NE_FSYS;
    }

    return NE_OK;
}

/*! Set image manager of this Listener.

    @return None.
*/
void TestPoiImageListener::SetImageManager(PoiImageManager* manager /*!<  POI Image manager instance.  */
                                          )
{
    m_imageManager = manager;
}

/*! Refer to base class. */
void TestPoiImageListener::ImageDownloaded(NB_Error result,
                                           const PoiImageIdentifierPtr identifier)
{
    CU_ASSERT_PTR_NOT_NULL(m_imageManager);
    CU_ASSERT(identifier != NULL);
    if (m_imageManager == NULL)
    {
        CU_FAIL("ImageManager is NULL!\n");
        return;
    }

    if (!identifier)
    {
        CU_FAIL("\n\tInvalid identifier!\n");
        return;
    }

    if (result == NE_OK) // Download finished normally.
    {
        nbcommon::DataStreamPtr binData = m_imageManager->GetImageData(identifier);
        uint32 size = binData->GetDataSize();
        CU_ASSERT_NOT_EQUAL(size, 0);
        if (size == 0)
        {
            CU_FAIL("\nSize of downloaded data is 0!\n")
            return;
        }
        void* buffer = nsl_malloc(size);
        nsl_memset(buffer, 0, size);
        binData->GetData((uint8*)buffer, 0, size);
        CU_ASSERT_PTR_NOT_NULL(buffer);
        if (buffer == NULL)
        {
            CU_FAIL("\nBinary data points to NULL!\n")
            return;
        }
        nsl_free(buffer);
    }
    else // Download error.
    {
        printf("\n\tDownload error, url: %s.\n", identifier->GetUrl()->c_str());
    }

    int remained = m_imageManager->GetPendingItemNumber();
    if (remained == 0)
    {
        if (MAX_ITEMS_IN_CACHE && MaxRequests > MAX_ITEMS_IN_CACHE)
        {
            if (m_itemRemoved)
            {
                CU_PASS("TestImageRemoved()");
            }
            else
            {
                CU_FAIL("TestImageRemoved()");
            }
        }
        SetCallbackCompletedEvent(m_event);
    }
}

void TestPoiImageListener::ImageRemoved(const PoiImageIdentifierPtr identifier)
{
    m_itemRemoved = true;
}

/*! TestPoiInstance constructor */
TestPoiInstance::TestPoiInstance()
{
    m_context             = NULL;
    m_event               = NULL;
    m_handler             = NULL;
    m_imageManager        = NULL;
    m_information         = NULL;
    m_listener            = NULL;
    m_pal                 = NULL;
    m_parameters          = NULL;
}

/*! TestPoiInstance destructor */
TestPoiInstance::~TestPoiInstance()
{
    Reset();
}

/*! Initialize a 'TestPoiInstance' object

    @return NE_OK if success,
            NE_EXIST if this object has already been initialized,
            NE_NOTINIT if this object is not properly initialized.
*/
NB_Error
TestPoiInstance::Initialize()
{
    NB_Error error = NE_OK;

    // Check if this 'TestPoiInstance' object is not properly initialized.
    if (m_pal || m_context || m_event || m_imageManager ||
        m_listener || m_handler || m_information)
    {
        return NE_NOTINIT;
    }

    // Create a PAL and context.
    if (!CreatePalAndContext(&m_pal, &m_context))
    {
        Reset();
        return NE_NOTINIT;
    }
    CU_ASSERT_PTR_NOT_NULL(m_pal);
    CU_ASSERT_PTR_NOT_NULL(m_context);

    // Create an event.
    m_event = CreateCallbackCompletedEvent();
    CU_ASSERT_PTR_NOT_NULL(m_event);
    if (!m_event)
    {
        Reset();
        return NE_NOTINIT;
    }

    NB_RequestHandlerCallback callback;
    callback.callback = &RequestHandlerCallback;
    callback.callbackData = this;
    error = NB_SingleSearchHandlerCreate(m_context, &callback, &m_handler);

    CU_ASSERT_EQUAL(error, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(m_handler);

    // Create a PoiImageListener object.
    m_listener = new TestPoiImageListener(m_pal, m_event);
    CU_ASSERT_PTR_NOT_NULL(m_listener);
    if (m_listener == NULL)
    {
        Reset();
        return error;
    }

    PoiImageManagerConfiguration* config = new PoiImageManagerConfiguration;

    // A rather small CacheSize is set here to make 'ImageRemoved()' possible to be called.

    config->m_thumbnailImageCacheSize = MAX_ITEMS_IN_CACHE;
    config->m_largeImageCacheSize     = 0;
    config->m_cachePath               = TEST_CACHE_PATH;

    error = PoiImageManager::CreateInstance(m_context, PoiImageListenerPtr(m_listener), PoiImageManagerConfigPtr(config),
                                            &m_imageManager);
    CU_ASSERT_EQUAL(error, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(m_imageManager);
    if (error != NE_OK || m_imageManager == NULL)
    {
        Reset();
        return error;
    }

    m_listener->SetImageManager(m_imageManager);
    return error;
}

/*! Reset a 'TestPoiInstance' object to be uninitialized

    Destroy the PAL, context, event and 'HttpDownloadManager' object in the 'TestPoiInstance' object.

    @return None
*/
void
TestPoiInstance::Reset()
{

    if (m_event)
    {
        DestroyCallbackCompletedEvent(m_event);
        m_event = NULL;
    }

    if (m_imageManager)
    {
        PoiImageManager::DestroyInstance(m_imageManager);
        m_imageManager = NULL;
    }

    if (m_handler)
    {
        NB_SingleSearchHandlerDestroy(m_handler);
        m_handler = NULL;
    }

    if (m_parameters)
    {
        NB_SingleSearchParametersDestroy(m_parameters);
        m_parameters = NULL;
    }

    if (m_information)
    {
        NB_SingleSearchInformationDestroy(m_information);
        m_information = NULL;
    }


    if (m_context)
    {
        DestroyContext(m_context);
        m_context = NULL;
    }

    if (m_pal)
    {
        PAL_Destroy(m_pal);
        m_pal = NULL;
    }
}

/*! Callback for all search requests

    @return None
*/
void TestPoiInstance::RequestHandlerCallback(void * handler, NB_RequestStatus status,
                                           NB_Error err, nb_boolean up, int percent,
                                           void * userData)
{

    TestPoiInstance* me = (TestPoiInstance*)(userData);
    CU_ASSERT_PTR_NOT_NULL(me);

    me->m_handler = (NB_SingleSearchHandler*) handler;

    if (err != NE_OK)
    {
        // Abort and trigger event
        CU_FAIL("Callback returned error.");
        printf("\n\tCallback error = %d, status = %d\n", err, status);
        SetCallbackCompletedEvent(me->m_event);
        return;
    }

    if (percent != 100)
    {
        // The status should be set to progress
        CU_ASSERT_EQUAL(status, NB_NetworkRequestStatus_Progress);
    }
    else
    {
        NB_Error localError = NE_OK;

        if (me->m_information)
        {
            NB_SingleSearchInformationDestroy(me->m_information);
            me->m_information = NULL;
        }

        localError = NB_SingleSearchHandlerGetInformation((NB_SingleSearchHandler*)handler, &(me->m_information));

        CU_ASSERT_EQUAL(localError, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(me->m_information);
        if (localError != NE_OK || !(me->m_information))
        {
            // Abort and trigger event
            CU_FAIL("Failed to retrieve search information");
            SetCallbackCompletedEvent(me->m_event);
            return;
        }

        CU_ASSERT_EQUAL(status, NB_NetworkRequestStatus_Success);

        // Trigger main thread
        SetCallbackCompletedEvent(me->m_event);
    }
}


/*! Test simple search of result style of single search

    @return None. CUnit Asserts get called on failures.
*/
NB_Error TestPoiInstance::StartSingleSearch()
{
    NB_Error error = NE_OK;
    int config = NB_EXT_WantPremiumPlacement |  \
        NB_EXT_WantEnhancedPOI | NB_EXT_WantNonProximityPois;

    NB_SearchRegion region;
    nsl_memset(&region, 0, sizeof(region));

    NB_ImageSize imageSize = {0};

    // Set center to New York
    region.type = NB_ST_Center;
    region.center.latitude = 40.47;
    region.center.longitude = -73.58;

    error = NB_SingleSearchParametersCreateByResultStyleType(m_context,
                                                              &region,
                                                              TEST_SEARCH_NAME,
                                                              TEST_SEARCH_SCHEME,
                                                              TEST_NUMBER_PER_SLICE,
                                                              (NB_POIExtendedConfiguration)config,
                                                              NULL,
                                                              NB_SRST_SingleSearch,
                                                              &m_parameters);
    CU_ASSERT_EQUAL(error, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(m_parameters);
    if (error != NE_OK)
    {
        return error;
    }

    error = NB_SingleSearchParametersAddSearchFilterKeyValue(m_parameters, "category", "AE");
    CU_ASSERT_PTR_NOT_NULL(m_parameters);
    if (error != NE_OK)
    {
        return error;
    }

    // Add an extended image specification for premium placements
    imageSize.width = 64;
    imageSize.height = 64;
    error = NB_SingleSearchParametersAddExtendedImage(m_parameters,
                                                       NB_EXT_WantPremiumPlacement, NB_IF_PNG,
                                                       &imageSize, 96);
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return error;
    }

    error = NB_SingleSearchParametersSetSource(m_parameters, NB_SS_Place);
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return error;
    }

    error = NB_SingleSearchParametersSetSource(m_parameters, NB_SS_None);
    CU_ASSERT_EQUAL(error, NE_INVAL);

    // Test to get search source.
    NB_SearchSource searchSource = NB_SS_None;

    error = NB_SingleSearchParametersGetSource(m_parameters, &searchSource);
    CU_ASSERT((error == NE_OK) && (searchSource == NB_SS_Place));
    if (error != NE_OK)
    {
        return NE_INVAL;
    }

    error = DoRequest();
    CU_ASSERT_EQUAL(error, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(m_information);

    return error;
}

/*! Do a search request

    Gets called from all tests. Returns the search information once the download has finished.

    @return NE_OK if success
*/
NB_Error
TestPoiInstance::DoRequest()
{
    NB_Error error = NE_OK;

    if (!m_parameters || !IsInstancesValid())
    {
        return NE_INVAL;
    }

    error = NB_SingleSearchHandlerStartRequest(m_handler, m_parameters);
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return error;
    }

    if (!WaitForCallbackCompletedEvent(m_event, 60000))
    {
        CU_FAIL("Download timeout.");
        return NE_UNEXPECTED;
    }

    CU_ASSERT_PTR_NOT_NULL(m_information);
    if (!(m_information))
    {
        return NE_UNEXPECTED;
    }

    return error;
}

/*! Helper function to check whether TestPoiInstance is valid or not.

    @return TRUE if valid, or FALSE otherwise.
*/
bool TestPoiInstance::IsInstancesValid()
{
    return (bool) (m_pal && m_context && m_handler && m_event);
}

/*! Cancel a download.

    @return NE_OK if succeed.
*/
NB_Error TestPoiInstance::CancelDownloading()
{
    return m_imageManager->CancelDownloading();
}

/*! Clear cache.

  @return NE_OK if succeed.
*/
NB_Error TestPoiInstance::ClearCaches()
{
    return m_imageManager->ClearAllCache();
}

/*! Starts to download images for single search.

    @return NE_OK if succeeded.
*/
NB_Error TestPoiInstance::StartDownloadSingleSearchImages(bool isThumbnail,
                                                          bool waitEvent)
{
    CU_ASSERT_PTR_NOT_NULL(m_information);
    if (m_information == NULL)
    {
        return NE_UNEXPECTED;
    }

    NB_Error error = NE_OK;
    if (isThumbnail)
    {
        error = m_imageManager->DownloadSingleSearchThumbnailImages(m_information);
    }
    else // We are not sure whether which poi has URL, so loop to find one.
    {
        uint32 count = 0;
        NB_SearchResultStyleType type = NB_SRST_None;
        error = NB_SingleSearchInformationGetResultStyleType(m_information, &type);
        if ((error == NE_OK) && (type == NB_SRST_Geocode || type == NB_SRST_SingleSearch))
            error = NB_SingleSearchInformationGetResultCount(m_information, &count);

        if (error != NE_OK)
        {
            return error;
        }

        uint32     index = 0;
        bool found = FALSE;
        for (; index < count; index++) {
            const char* url = NB_SingleSearchInformationGetPoiImageUrl(m_information, index, FALSE);
            if (url) // OK, we find one.
            {
                found = TRUE;
                error = m_imageManager->DownloadSingleSearchImagesForPoi(m_information, index);
            }
        }
        if (!found)
        {
            CU_PASS("Can not get url for Large image.\n");
            waitEvent = FALSE;
        }
    }

    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return error;
    }

    MaxRequests = m_imageManager->GetPendingItemNumber();
    if (MaxRequests > 0 && waitEvent == TRUE)
    {
        if (!WaitForCallbackCompletedEvent(m_event, 60000))
        {
            CU_FAIL("Download timeout.");
            return NE_UNEXPECTED;
        }
    }
    return NE_OK;
}

TestPoiInstance *instance = NULL;

/*! Helper function to cleanup resources after a test.

  @return none.
*/
void CLEANUP()
{
    if (instance != NULL)
    {
        delete instance;
    }
    instance = NULL;
}

/*! Helper function to allocate resources before a real test.

    @return none.
*/
void PREPARE()
{
    CLEANUP();
    instance = new TestPoiInstance();
}

// Test Functions ................................................................................

/*! Test initializing a 'HttpDownloadManager' object

    @return None. CUnit Asserts get called on failures.
*/
void
TestInstanceInitialization()
{
    PREPARE();
    NB_Error error = instance->Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);
    CLEANUP();
}

/*! Start test to download thumbnail images for SingleSearch.

    @return None.
*/
void
TestDownloadSingleSearchThumbnailImages()
{
    PREPARE();
    instance->Initialize();
    NB_Error error = instance->StartSingleSearch();
    CU_ASSERT_EQUAL(error, NE_OK);
    error = instance->StartDownloadSingleSearchImages(TRUE);

    // Download them again, images should be returned from cache this time.
    error = instance->StartDownloadSingleSearchImages(TRUE);
    CU_ASSERT_EQUAL(error, NE_OK);
    CLEANUP();
}

/*! Test to start download Images for specific POI.

    @return None.
*/
void
TestDownloadSingleSearchImagesForPoi()
{
    PREPARE();
    instance->Initialize();
    NB_Error error = instance->StartSingleSearch();
    CU_ASSERT_EQUAL(error, NE_OK);
    error = instance->StartDownloadSingleSearchImages(FALSE);
    CU_ASSERT_EQUAL(error, NE_OK);
    CLEANUP();
    CU_ASSERT_EQUAL(error, NE_OK);
}

/*! Test to download thumbnails for Non SingleSearch.

    @return None.
*/
void TestDownloadSearchThumbnailImages()
{
}

/*! Test to download image of specific POI for Non SingleSearch.

    @return None.
*/
void TestDownloadSearchImagesForPoi()
{

}

/*! Test cancel downloading.

    @@return None.
*/
void TestCancelDownloading()
{
    PREPARE();
    instance->Initialize();
    NB_Error error = instance->StartSingleSearch();
    CU_ASSERT_EQUAL(error, NE_OK);

    error = instance->StartDownloadSingleSearchImages(TRUE, FALSE);
    CU_ASSERT_EQUAL(error, NE_OK);

    error = instance->CancelDownloading();
    CU_ASSERT_EQUAL(error, NE_OK);

    CLEANUP();
    CU_ASSERT_EQUAL(error, NE_OK);
}

/*! Test cancel downloading.

  @@return None.
*/
void TestClearCache()
{
    PREPARE();
    instance->Initialize();
    NB_Error error = instance->StartSingleSearch();
    CU_ASSERT_EQUAL(error, NE_OK);

    error = instance->StartDownloadSingleSearchImages(TRUE, FALSE);
    CU_ASSERT_EQUAL(error, NE_OK);

    error = instance->CancelDownloading();
    CU_ASSERT_EQUAL(error, NE_OK);

    error = instance->ClearCaches();
    CU_ASSERT_EQUAL(error, NE_OK);

    CLEANUP();
    CU_ASSERT_EQUAL(error, NE_OK);
}

/*! Add all your test functions here

    @return None.
*/
void
TestPoiImageManager_AddAllTests(CU_pSuite pTestSuite, int level)
{
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestInstanceInitialization",
             TestInstanceInitialization);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDownloadSingleSearchThumbnailImages",
             TestDownloadSingleSearchThumbnailImages);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestDownloadSingleSearchImagesForPoi",
             TestDownloadSingleSearchImagesForPoi);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestCancelDownloading",
             TestCancelDownloading);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestClearCache",
             TestClearCache);
};

/*! Add common initialization code here

    @return 0
*/
int
TestPoiImageManager_SuiteSetup()
{
    return 0;
}

/*! Add common cleanup code here

    @return 0
*/
int
TestPoiImageManager_SuiteCleanup()
{
    return 0;
}

/*! @} */

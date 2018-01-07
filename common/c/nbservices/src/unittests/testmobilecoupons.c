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

    @file     testmobilecoupons.c
    @date     04/21/2011
    @defgroup TestMobileCoupons_h System tests for Mobile Coupons functions
*/
/*
    See description in header file.

    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems, Inc. is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "testmobilecoupons.h"
#include "networkutil.h"
#include "main.h"
#include "platformutil.h"
#include "palmath.h"
#include "nbcontext.h"
#include "nbstorerequesthandler.h"
#include "nbcouponrequesthandler.h"
#include "nbcouponloadimagehandler.h"
#include "nbcategorylisthandler.h"
#include "nbsearchfilter.h"

// debug logs
//#define PRINT_DEBUG_LOGS

// Types ...........................................................
typedef enum MobileCouponsRequestType_
{
    MC_Invalid = 0,
    MC_StoreRequest,
    MC_CouponRequest,
    MC_LoadImage,
    MC_CategoryList

} MobileCouponsRequestType;

typedef struct RequestData_
{
    MobileCouponsRequestType type;
    void* information;
} RequestData;

// Local Functions ...........................................................

// All tests. Add all tests to the TestMobileCoupons_AddTests function
static void TestMobileCouponsStoreRequest(void);
static void TestMobileCouponsCouponRequest(void);
static void TestMobileCouponsLoadImage(void);
static void TestMobileCouponsCategoryList(void);

static void RequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);
static NB_Error GetInformation(void* handler, void* userData);
static void PrintRequestStatusToLog(void* userData, uint8 up, int percent);

// Constants .................................................................

// How much should we download
static const int NUMBER_PER_SLICE = 4;
static const int SLICES_TO_DOWNLOAD = 2;
static const int MAX_IMAGE_SIZE = 256;

#define STATUS_MAX_LENGTH       50
#define DESCRIPTION_MAX_LENGTH  50
#define CLIPPING_ID_MAX_LENGTH  50
#define REQUEST_TIMEOUT         20000

// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;
static RequestData g_RequestData = { 0 };

#ifdef PRINT_DEBUG_LOGS
static nb_boolean g_DebugLogsEnabled = TRUE;
#else
static nb_boolean g_DebugLogsEnabled = FALSE;
#endif

void TestMobileCoupons_AddAllTests(CU_pSuite pTestSuite)
{
    CU_add_test(pTestSuite, "TestMobileCouponsStoreRequest", TestMobileCouponsStoreRequest);
    CU_add_test(pTestSuite, "TestMobileCouponsCouponRequest", TestMobileCouponsCouponRequest);
    CU_add_test(pTestSuite, "TestMobileCouponsLoadImage", TestMobileCouponsLoadImage);
    CU_add_test(pTestSuite, "TestMobileCouponsCategoryList", TestMobileCouponsCategoryList);
};


/*! Add common initialization code here.

@return 0

@see TestMobileCoupons_SuiteCleanup
*/
int TestMobileCoupons_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestMobileCoupons_SuiteSetup
*/
int TestMobileCoupons_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
    return 0;
}

/*! Test Mobile Coupons Store Request
*/
void TestMobileCouponsStoreRequest(void)
{
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    NB_StoreRequestParameters* parameters = NULL;
    NB_Error err = NE_OK;
    int sliceIndex = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);

    if (!rc)
    {
        CU_FAIL("CreatePalAndContext() failed");
        return;
    }

    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; ++sliceIndex)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            NB_CouponSearchRegion searchRegion = {0};
            searchRegion.type = NB_CSRT_Center;
            searchRegion.center.latitude = 40.47;
            searchRegion.center.longitude = -73.58;

            err = NB_StoreRequestParametersCreate(context, &searchRegion, NUMBER_PER_SLICE, &parameters);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(parameters);
        }
        else if (g_RequestData.information != NULL)
        {
            err = NB_StoreRequestParametersCreateIteration(context, g_RequestData.information, NB_IterationCommand_Next, &parameters);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(parameters);

            NB_StoreRequestInformationDestroy(g_RequestData.information);
            g_RequestData.information = NULL;
        }

        if (err == NE_OK)
        {
            NB_RequestHandlerCallback callback = { RequestHandlerCallback, &g_RequestData };
            NB_StoreRequestHandler* handler = NULL;

            g_RequestData.type = MC_StoreRequest;

            err = NB_StoreRequestHandlerCreate(context, &callback, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (err == NE_OK && handler != NULL)
            {
                err = NB_StoreRequestHandlerStartRequest(handler, parameters);
                if (err == NE_OK)
                {
                    if (!WaitForCallbackCompletedEvent(g_CallbackEvent, REQUEST_TIMEOUT))
                    {
                        err = NE_UNEXPECTED;
                    }
                }

                CU_ASSERT_EQUAL(err, NE_OK);
                if (err == NE_OK)
                {
                    int32 count = 0, index = 0;
                    NB_Store* store = NULL;

                    (void)NB_StoreRequestInformationGetResultCount(g_RequestData.information, &count);
                    for (index = 0; (err == NE_OK) && (index < count); ++index)
                    {
                        err = NB_StoreRequestInformationGetStore(g_RequestData.information, index, &store);
                        if (err == NE_OK)
                        {
                            if (g_DebugLogsEnabled == TRUE)
                            {
                                PRINTF("\nStore #%d:", index);
                                PRINTF("\nName: %d", store->name);
                                PRINTF("\nId: %s", store->id);
                                PRINTF("\nAddress: %s", store->place.location.street1);
                                PRINTF("\nCount: %d", store->count);

                                PRINTF("\nimageUrl: %s", store->imageUrl);
                                PRINTF("\nsmallThumbUrl: %s", store->smallThumbUrl);
                                PRINTF("\nmediumThumbUrl: %s", store->mediumThumbUrl);
                                PRINTF("\nlargeThumbUrl: %s", store->largeThumbUrl);

                                if (store->categories != NULL && store->categoriesNumber > 0)
                                {
                                    PRINTF("\nCategory: %s", store->categories[0]);
                                }
                                PRINTF("\nCategory num: %d", store->categoriesNumber);
                                if (store->subCategories != NULL && store->subCategoriesNumber > 0)
                                {
                                    PRINTF("\nSub-Category: %s", store->subCategories[0]);
                                }
                                PRINTF("\nSub-Category num: %d", store->subCategoriesNumber);
                                if (store->broadCategories != NULL && store->broadCategoriesNumber > 0)
                                {
                                    PRINTF("\nBroad-Category: %s", store->broadCategories[0]);
                                }
                                PRINTF("\nBroad-Category num: %d", store->broadCategoriesNumber);

                                PRINTF("\n\n");
                            }
                            NB_StoreRequestInformationFreeStoreData(store);
                        }
                    }
                }

                (void)NB_StoreRequestHandlerDestroy(handler);
            }

            g_RequestData.type = MC_Invalid;
            (void)NB_StoreRequestParametersDestroy(parameters);
            parameters = NULL;
        }
    }

    NB_StoreRequestInformationDestroy(g_RequestData.information);
    g_RequestData.information = NULL;

    (void)NB_ContextDestroy(context);
    PAL_Destroy(pal);
}

/*! Test Mobile Coupons Coupon Request
*/
void TestMobileCouponsCouponRequest(void)
{
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    NB_CouponRequestParameters* parameters = NULL;
    NB_Error err = NE_OK;
    int sliceIndex = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    nb_boolean hasMoreResult = FALSE;
    nb_boolean hasMoreResultOnPreviousIteration = FALSE;
    nb_boolean flag = FALSE;

    if (!rc)
    {
        CU_FAIL("CreatePalAndContext() failed");
        return;
    }

    for (sliceIndex = 0; sliceIndex < SLICES_TO_DOWNLOAD; ++sliceIndex)
    {
        // If it is the first slice then we need to create our initial parameter object
        if (sliceIndex == 0)
        {
            NB_CouponSearchRegion searchRegion = {0};
            NB_SearchFilter* searchFilter = NULL;
            searchRegion.type = NB_CSRT_Center;
            searchRegion.center.latitude = 40.47;
            searchRegion.center.longitude = -73.58;

            err = NB_CouponRequestParametersCreate(context, &searchRegion, NUMBER_PER_SLICE, &parameters);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(parameters);
            err = NB_SearchFilterCreate(context, &searchFilter);
            CU_ASSERT_EQUAL(err, NE_OK);
            err = NB_SearchFilterAdd(searchFilter, "return", "deals" );
            CU_ASSERT_EQUAL(err, NE_OK);
            err = NB_CouponRequestParametersSetSearchFilter( parameters, searchFilter );
            CU_ASSERT_EQUAL(err, NE_OK);
            err = NB_SearchFilterDestroy( searchFilter );
            CU_ASSERT_EQUAL(err, NE_OK);

        }
        else if (g_RequestData.information != NULL)
        {
            err = NB_CouponRequestParametersCreateIteration(context, g_RequestData.information, NB_IterationCommand_Next, &parameters);
            CU_ASSERT_EQUAL(err, NE_OK);
            CU_ASSERT_PTR_NOT_NULL(parameters);

            NB_CouponRequestInformationDestroy(g_RequestData.information);
            g_RequestData.information = NULL;
        }

        if (err == NE_OK)
        {
            NB_RequestHandlerCallback callback = { RequestHandlerCallback, &g_RequestData };
            NB_CouponRequestHandler* handler = NULL;

            g_RequestData.type = MC_CouponRequest;

            err =  NB_CouponRequestHandlerCreate(context, &callback, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (err == NE_OK && handler != NULL)
            {
                err = NB_CouponRequestHandlerStartRequest(handler, parameters);
                if (err == NE_OK)
                {
                    if (!WaitForCallbackCompletedEvent(g_CallbackEvent, REQUEST_TIMEOUT))
                    {
                        err = NE_UNEXPECTED;
                    }
                }

                CU_ASSERT_EQUAL(err, NE_OK);
                if (err == NE_OK)
                {
                    int32 count = 0, index = 0;
                    NB_StoreCoupons* storeCoupon = NULL;

                    (void)NB_CouponRequestInformationGetResultCount(g_RequestData.information, &count);
                    for (index = 0; (err == NE_OK) && (index < count); ++index)
                    {
                        err = NB_CouponRequestInformationGetStoreCoupon(g_RequestData.information, index, &storeCoupon);
                        if ( err == NE_OK )
                        {
                            if ( g_DebugLogsEnabled == TRUE )
                            {
                                PRINTF("\nStoreCoupon #%d:", index);
                                PRINTF("\nStoreId: %s", storeCoupon->store.id);
                                PRINTF("\nStoreName: %d", storeCoupon->store.name);
                                PRINTF("\nStoreAddress: %s\n\n", storeCoupon->store.place.location.street1);
                                PRINTF("\nCouponsNumber: %d", storeCoupon->couponsNumber );
                                if ( storeCoupon->couponsNumber > 0 && storeCoupon->coupons != NULL )
                                {
                                    PRINTF("\n\tCoupon #0:");
                                    PRINTF("\n\tAvailabilityNumber: %d", storeCoupon->coupons[0].availabilityCount);
                                    if ( storeCoupon->coupons[0].availabilityCount > 0 &&
                                         storeCoupon->coupons[0].availability != NULL )
                                    {
                                        PRINTF("\n\tAvailability: %s", storeCoupon->coupons[0].availability[0]);
                                    }
                                    PRINTF("\n\tAcquisitionNumber: %d", storeCoupon->coupons[0].acquisitionCount);
                                    if ( storeCoupon->coupons[0].acquisitionCount > 0 &&
                                         storeCoupon->coupons[0].acquisition != NULL )
                                    {
                                        PRINTF("\n\tAcquisition: %s", storeCoupon->coupons[0].acquisition[0]);
                                    }
                                    PRINTF("\n\tTitle: %s", storeCoupon->coupons[0].title);
                                    PRINTF("\n\tDescription: %s", storeCoupon->coupons[0].description);
                                    PRINTF("\n\tConditions: %s", storeCoupon->coupons[0].conditions);
                                    PRINTF("\n\tID: %s", storeCoupon->coupons[0].id);
                                    PRINTF("\n\tStartDate: %d", storeCoupon->coupons[0].startDate);
                                    PRINTF("\n\tExpirationDate: %d", storeCoupon->coupons[0].expirationDate);
                                    PRINTF("\n\tCurrency: %s", storeCoupon->coupons[0].currency);
                                    PRINTF("\n\tDiscountValue: %f", storeCoupon->coupons[0].discountValue);
                                    PRINTF("\n\tBuyValue: %f", storeCoupon->coupons[0].buyValue);
                                    PRINTF("\n\tListValue: %f", storeCoupon->coupons[0].listValue);
                                    PRINTF("\n\tCouponCode: %s", storeCoupon->coupons[0].couponCode);
                                    PRINTF("\n\tDealUrl: %s", storeCoupon->coupons[0].dealUrl);
                                    PRINTF("\n\tImageUrl: %s", storeCoupon->coupons[0].imageUrl);
                                    PRINTF("\n\tSmallThumbUrl: %s", storeCoupon->coupons[0].smallThumbUrl);
                                    PRINTF("\n\tMediumThumbUrl: %s", storeCoupon->coupons[0].mediumThumbUrl);
                                    PRINTF("\n\tLargeThumbUrl: %s", storeCoupon->coupons[0].largeThumbUrl);
                                }

                            }
                            NB_CouponRequestInformationFreeStoreCouponData( storeCoupon );
                        }
                    }

                    NB_CouponRequestInformationHasMoreResults(g_RequestData.information, &hasMoreResult);

                    if (count > 0)
                    {
                        if (sliceIndex > 0)
                        {
                            CU_ASSERT(hasMoreResultOnPreviousIteration);
                        }
                    }
                    else
                    {
                        if (sliceIndex == 0)
                        {
                            CU_ASSERT(!hasMoreResult);
                        }
                        else
                        {
                            CU_ASSERT(!hasMoreResultOnPreviousIteration);
                        }
                        flag = TRUE;
                    }

                    hasMoreResultOnPreviousIteration = hasMoreResult;
                }

                (void)NB_CouponRequestHandlerDestroy(handler);
                handler = NULL;
            }

            g_RequestData.type = MC_Invalid;
            (void)NB_CouponRequestParametersDestroy(parameters);
            parameters = NULL;
            if ( flag )
            {
                break;
            }
        }
    }

    (void)NB_CouponRequestParametersDestroy(parameters);
    parameters = NULL;
    NB_CouponRequestInformationDestroy(g_RequestData.information);
    g_RequestData.information = NULL;

    (void)NB_ContextDestroy(context);
    PAL_Destroy(pal);
}

void TestMobileCouponsCategoryList(void)
{
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_CategoryListParameters* parameters = NULL;
        NB_CouponSearchRegion searchRegion = {0};

        searchRegion.type = NB_CSRT_Center;
        searchRegion.center.latitude = 40.47;
        searchRegion.center.longitude = -73.58;
        // actually I don't know correct board-category and category names
        err = NB_CategoryListParametersCreate(context, &searchRegion, NULL, "Computing & Electronics", &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (err == NE_OK)
        {
            NB_RequestHandlerCallback callback = { RequestHandlerCallback, &g_RequestData };
            NB_CategoryListHandler* handler = NULL;

            g_RequestData.type = MC_CategoryList;

            err = NB_CategoryListHandlerCreate(context, &callback, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (err == NE_OK && handler != NULL)
            {
                err = NB_CategoryListHandlerStartRequest(handler, parameters);
                if (err == NE_OK)
                {
                    if (!WaitForCallbackCompletedEvent(g_CallbackEvent, REQUEST_TIMEOUT))
                    {
                        err = NE_UNEXPECTED;
                    }
                }

                CU_ASSERT_EQUAL(err, NE_OK);
                if (err == NE_OK)
                {
                    int32 index = 0, count =0;
                    char* category = NULL;

                    err = NB_CategoryListInformationGetBroadCategoryCount(g_RequestData.information, &count);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    for (index = 0; (err == NE_OK) && (index < count); ++index)
                    {
                        category = NULL;
                        err = NB_CategoryListInformationGetBroadCategoryItem(g_RequestData.information, index, &category);
                        if ( err == NE_OK )
                        {
                            if ( g_DebugLogsEnabled == TRUE )
                            {
                                PRINTF("\nBroadCategory #%d - %s:", index, category);
                            }
                        }
                    }

                    err = NB_CategoryListInformationGetSubCategoryCount(g_RequestData.information, &count);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    for (index = 0; (err == NE_OK) && (index < count); ++index)
                    {
                        category = NULL;
                        err = NB_CategoryListInformationGetSubCategoryItem(g_RequestData.information, index, &category);
                        if ( err == NE_OK )
                        {
                            if ( g_DebugLogsEnabled == TRUE )
                            {
                                PRINTF("\nSubCategory #%d - %s:", index, category);
                            }
                        }
                    }

                    err = NB_CategoryListInformationGetCategoryCount(g_RequestData.information, &count);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    for (index = 0; (err == NE_OK) && (index < count); ++index)
                    {
                        category = NULL;
                        err = NB_CategoryListInformationGetCategoryItem(g_RequestData.information, index, &category);
                        if ( err == NE_OK )
                        {
                            if ( g_DebugLogsEnabled == TRUE )
                            {
                                PRINTF("\nCategory #%d - %s:", index, category);
                            }
                        }
                    }

                    (void)NB_CategoryListInformationDestroy(g_RequestData.information);
                    g_RequestData.information = NULL;
                }

                (void)NB_CategoryListHandlerDestroy(handler);
            }

            g_RequestData.type = MC_Invalid;
            (void)NB_CategoryListParametersDestroy(parameters);
        }

        (void)NB_ContextDestroy(context);
        PAL_Destroy(pal);
    }
    else
    {
        CU_FAIL("CreatePalAndContext() failed");
    }
}

void TestMobileCouponsLoadImage(void)
{
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_CouponLoadImageParameters* parameters = NULL;

        err = NB_CouponLoadImageParametersCreate(context, "http://cdn-dev.mphoria.com/offers/images/bb_1230458_100.jpg", &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (err == NE_OK)
        {
            NB_RequestHandlerCallback callback = { RequestHandlerCallback, &g_RequestData };
            NB_CouponLoadImageHandler* handler = NULL;

            g_RequestData.type = MC_LoadImage;

            err = NB_CouponLoadImageHandlerCreate(context, &callback, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (err == NE_OK && handler != NULL)
            {
                err = NB_CouponLoadImageHandlerStartRequest(handler, parameters);
                if (err == NE_OK)
                {
                    if (!WaitForCallbackCompletedEvent(g_CallbackEvent, REQUEST_TIMEOUT))
                    {
                        err = NE_UNEXPECTED;
                    }
                }

                CU_ASSERT_EQUAL(err, NE_OK);
                if (err == NE_OK)
                {
                    uint8* image = NULL;
                    uint32 imageSize = 0;

                    err = NB_CouponLoadImageInformationGetImage(g_RequestData.information, &image, &imageSize);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    if (err == NE_OK && g_DebugLogsEnabled == TRUE)
                    {
                        PRINTF("\nImage size: %d\n\n", imageSize);
                    }
                    (void)NB_CouponLoadImageInformationDestroy(g_RequestData.information);
                    g_RequestData.information = NULL;
                }

                (void)NB_CouponLoadImageHandlerDestroy(handler);
            }

            g_RequestData.type = MC_Invalid;
            (void)NB_CouponLoadImageParametersDestroy(parameters);
        }

        (void)NB_ContextDestroy(context);
        PAL_Destroy(pal);
    }
    else
    {
        CU_FAIL("CreatePalAndContext() failed");
    }
}


/*! Returns request information for various mobile coupon requests.

@param handler Request handler
@param userData User data provided to request handler

@return Error code
*/
NB_Error GetInformation(void* handler, void* userData)
{
    NB_Error err = NE_NOENT;
    RequestData* requestData = (RequestData*)userData;
    if (handler == NULL || userData == NULL)
    {
        return NE_INVAL;
    }

    switch (requestData->type)
    {
        case MC_StoreRequest:
            err = NB_StoreRequestHandlerGetInformation(handler, (NB_StoreRequestInformation**)(&(requestData->information)));
            break;

        case MC_CouponRequest:
            err = NB_CouponRequestHandlerGetInformation(handler, (NB_CouponRequestInformation**)(&(requestData->information)));
            break;

        case MC_LoadImage:
            err = NB_CouponLoadImageHandlerGetInformation(handler, (NB_CouponLoadImageInformation**)(&(requestData->information)));
            break;

        case MC_CategoryList:
            err = NB_CategoryListHandlerGetInformation(handler, (NB_CategoryListInformation**)(&(requestData->information)));
            break;

        default:
            requestData->information = NULL;
            break;
    }
    return err;
}

/*! Common request handler callback

Request handler callback.  Reports progress, gets information on success and
signals that the request has completed

@param handler Handler invoking the callback
@param status Request status
@param err Error state
@param up Non-zero if progress report is for server query; zero for server reply
@param percent Percent of data transferred up or down
@param userData User data provided to request handler
*/
void RequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err,
                            uint8 up, int percent, void* userData)
{
    if (!up)
    {
        if (err != NE_OK)
        {
            CU_FAIL("Callback returned error");
            if (userData != NULL)
            {
                ((RequestData*)userData)->information = 0;
            }
            SetCallbackCompletedEvent(g_CallbackEvent);
        }
        else if (percent == 100)
        {
            NB_Error err = GetInformation(handler, userData);
            CU_ASSERT_EQUAL(err, NE_OK);
            SetCallbackCompletedEvent(g_CallbackEvent);
        }
    }
}



/*! @} */

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

    @file TestSingleSearchProtocol.cpp
    @defgroup TestSingleSearchProtocol_cpp system tests for single search functions

    See description in header file.
*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

extern "C"
{
    #include "platformutil.h"
    #include "networkutil.h"
    #include "nbcontext.h"
}

#include "CUnit.h"
#include "TestSingleSearchProtocol.h"
#include "Handler.h"
#include "NBProtocolSingleSearchSourceInformation.h"
#include "NBProtocolSingleSearchSourceParameters.h"
#include "smartpointer.h"
#include "TpsAtlasbookProtocol.h"
#include "TpsElement.h"
#include "NBProtocolSingleSearchSourceInformationSerializer.h"
#include "NBProtocolSingleSearchSourceParametersSerializer.h"


// Local Constants ...............................................................................

#define TEST_NUMBER_PER_SLICE       10

#define TEST_LATITUDE               33.60095
#define TEST_LONGITUDE              -117.69163
#define TEST_SEARCH_SCHEME          "tcs-single-search-2"
#define TEST_SEARCH_NAME            "KFC"

using namespace std;
using namespace nbmap;
using namespace protocol;

// Local Types ...................................................................................
const static string SINGLE_SEARCH_RESULT_TYLE = "single-search";
const static string SUGGEST_SEARCH_RESULT_TYLE = "suggest";

/*! Collection of all instances used for the unit tests */
typedef struct
{
    PAL_Instance*               pal;                /*!< Pointer to PAL */
    NB_Context*                 context;            /*!< Pointer to current context */

    /*!< Single search handler instance */
    shared_ptr<protocol::Handler<protocol::SingleSearchSourceParameters,
                                 protocol::SingleSearchSourceInformation> > handler; 

    void*                       event;              /*!< Event gets triggered from the download callback */

} Instances;

static Instances g_instances = {0};

/*! SearchAsyncCallback

    Refer to AsyncCallbackWithRequest for more description.
 */
class SearchAsyncCallback : public nbmap::AsyncCallbackWithRequest<protocol::SingleSearchSourceParametersSharedPtr,
                                                                          protocol::SingleSearchSourceInformationSharedPtr>
{
public:
    SearchAsyncCallback() {}
    virtual ~SearchAsyncCallback() {}
    void Success(protocol::SingleSearchSourceParametersSharedPtr request,
                 protocol::SingleSearchSourceInformationSharedPtr response);
    void Error(protocol::SingleSearchSourceParametersSharedPtr request, NB_Error error);
    bool Progress(int percentage) { return 0; }

    /*! Reset this call back to invalid state */
    void Reset() {}
};

/*! SuggestionAsyncCallback

    Refer to AsyncCallbackWithRequest for more description.
 */
class SuggestionAsyncCallback : public nbmap::AsyncCallbackWithRequest<protocol::SingleSearchSourceParametersSharedPtr,
                                                                       protocol::SingleSearchSourceInformationSharedPtr>
{
public:
    SuggestionAsyncCallback() {}
    virtual ~SuggestionAsyncCallback() {}
    void Success(protocol::SingleSearchSourceParametersSharedPtr request,
                 protocol::SingleSearchSourceInformationSharedPtr response);
    void Error(protocol::SingleSearchSourceParametersSharedPtr request, NB_Error error);
    bool Progress(int percentage) { return 0; }

    /*! Reset this call back to invalid state */
    void Reset() {}
};


// Test Functions ................................................................................

static void TestSingleSearchSimpleSearchProtocol(void);
static void TestSingleSearchSuggestionSearchProtocol(void);


// Helper Functions ..............................................................................
static nb_boolean CreateInstances(Instances* instances);
static void DestroyInstances(Instances* instances);
static SingleSearchSourceParametersSharedPtr CreateSearchParameter(string resultStyleName);
static NB_Error DoRequest(Instances* instances, SingleSearchSourceParametersSharedPtr parameters);
static NB_Error DoSuggestRequest(Instances* instances, SingleSearchSourceParametersSharedPtr parameters);

// Public Functions ..............................................................................

/*! Add all your test functions here

    @return None
*/
void 
TestSingleSearchProtocol_AddAllTests(CU_pSuite pTestSuite)
{
    // Add all your function names here
    CU_add_test(pTestSuite, "TestSingleSearchSimpleSearchProtocol", &TestSingleSearchSimpleSearchProtocol);
    CU_add_test(pTestSuite, "TestSingleSearchSuggestionSearchProtocol", &TestSingleSearchSuggestionSearchProtocol);
}

/*! Add common initialization code here.

    @return 0

    @see TestSingleSearch_SuiteCleanup
*/
int
TestSingleSearchProtocol_SuiteSetup()
{
    // Nothing to do

    return 0;
}

/*! Add common cleanup code here.

    @return 0

    @see TestSingleSearch_SuiteSetup
*/
int
TestSingleSearchProtocol_SuiteCleanup()
{
    // Nothing to do

    return 0;
}


// Test Functions ................................................................................

/*! Test simple search of result style of single search

    @return None. CUnit Asserts get called on failures.
*/
void
TestSingleSearchSimpleSearchProtocol()
{
    if (CreateInstances(&g_instances))
    {
        NB_Error result = NE_OK;

        SingleSearchSourceParametersSharedPtr parameters = CreateSearchParameter(SINGLE_SEARCH_RESULT_TYLE);

        result = DoRequest(&g_instances, parameters);

        CU_ASSERT_EQUAL(result, NE_OK);

        if (parameters)
        {
            parameters.reset();
        }

        DestroyInstances(&g_instances);
    }
}

/*! Test simple search of result style of suggestion search and search by a suggestion

    @return None. CUnit Asserts get called on failures.
*/
void
TestSingleSearchSuggestionSearchProtocol()
{
    if (CreateInstances(&g_instances))
    {
        NB_Error result = NE_OK;

        SingleSearchSourceParametersSharedPtr parameters = CreateSearchParameter(SUGGEST_SEARCH_RESULT_TYLE);

        result = DoSuggestRequest(&g_instances, parameters);

        CU_ASSERT_EQUAL(result, NE_OK);
        if (parameters)
        {
            parameters.reset();
        }

        DestroyInstances(&g_instances);
    }
}

void SearchAsyncCallback::Success(SingleSearchSourceParametersSharedPtr request,
                                  SingleSearchSourceInformationSharedPtr response)
{
    shared_ptr<std::vector<shared_ptr<ProxMatch> > > proxMatchs = response->GetProxMatchArray();
    CU_ASSERT_PTR_NOT_NULL(proxMatchs.get());
    int resultCount = proxMatchs->size(); 
    CU_ASSERT(resultCount > 0);
    vector<shared_ptr<ProxMatch> >::iterator begin = proxMatchs->begin();
    while (begin != proxMatchs->end())
    {
        shared_ptr<ProxMatch> proxMatch = *begin;
        CU_ASSERT_PTR_NOT_NULL(proxMatch.get());
        CU_ASSERT(proxMatch->GetDistance() > 0);
        shared_ptr<Place> palce = proxMatch->GetPlace();
        CU_ASSERT_PTR_NOT_NULL(palce.get());
        CU_ASSERT(!palce->GetName()->empty());
        shared_ptr<Location> location = palce->GetLocation();
        CU_ASSERT_PTR_NOT_NULL(location.get());

        ++begin;
    }

    // Trigger main thread
    SetCallbackCompletedEvent(g_instances.event);
}

void SearchAsyncCallback::Error(SingleSearchSourceParametersSharedPtr request, NB_Error error)
{
    // Trigger main thread
    SetCallbackCompletedEvent(g_instances.event);
    CU_ASSERT_EQUAL(error, NE_OK);
}

void SuggestionAsyncCallback::Success(SingleSearchSourceParametersSharedPtr request,
                                      SingleSearchSourceInformationSharedPtr response)
{
    // Trigger main thread

    shared_ptr<std::vector<shared_ptr<SuggestMatch> > > suggestMatchs = response->GetSuggestMatchArray();
    CU_ASSERT_PTR_NOT_NULL(suggestMatchs.get());
    int resultCount = suggestMatchs->size(); 
    CU_ASSERT(resultCount > 0);
    vector<shared_ptr<SuggestMatch> >::iterator begin = suggestMatchs->begin();
    while (begin != suggestMatchs->end())
    {
        shared_ptr<SuggestMatch> suggestMatch = *begin;
        CU_ASSERT_PTR_NOT_NULL(suggestMatch.get());
        CU_ASSERT(!suggestMatch->GetLine1()->empty());
        shared_ptr<SearchFilter> searchFilter = suggestMatch->GetSearchFilter();
        CU_ASSERT_PTR_NOT_NULL(searchFilter.get());
        CU_ASSERT(nsl_strncmp(searchFilter->GetResultStyle()->GetKey()->c_str(), SINGLE_SEARCH_RESULT_TYLE.c_str(), nsl_strlen(SINGLE_SEARCH_RESULT_TYLE.c_str())) == 0);
        CU_ASSERT(searchFilter->GetSearchKeyArray()->size() > 0);
        ++begin;
    }

    SetCallbackCompletedEvent(g_instances.event);
}

void SuggestionAsyncCallback::Error(SingleSearchSourceParametersSharedPtr request, NB_Error error)
{
    // Trigger main thread
    SetCallbackCompletedEvent(g_instances.event);
    CU_ASSERT_EQUAL(error, NE_OK);
}

// Helper Functions ..............................................................................

/*! Create PAL, context, single search handler and event for callback

    Shared by all unit tests.

    @return TRUE on success, FALSE otherwise
*/
nb_boolean
CreateInstances(Instances* instances                /*!< The Instances object to create */
                )
{
    NB_Error result = NE_OK;

    if (!instances)
    {
        return FALSE;
    }

    if (!CreatePalAndContext(&instances->pal, &instances->context))
    {
        DestroyInstances(instances);
        return FALSE;
    }

    CU_ASSERT_PTR_NOT_NULL(instances->pal);
    CU_ASSERT_PTR_NOT_NULL(instances->context);

    instances->event = CreateCallbackCompletedEvent();
    if (!instances->event)
    {
        DestroyInstances(instances);
        return FALSE;
    }

    CU_ASSERT_PTR_NOT_NULL(instances->event);

    TpsAtlasbookProtocol tpsAtlasbookProtocol(instances->context);
    instances->handler = tpsAtlasbookProtocol.GetSingleSearchSourceHandler();

    CU_ASSERT_PTR_NOT_NULL(instances->handler.get());

    if (result != NE_OK)
    {
        DestroyInstances(instances);
        return FALSE;
    }


    return TRUE;
}

/*! Destroy PAL, context, single search handler and event for callback

    Shared by all unit tests.

    @return None
*/
void
DestroyInstances(Instances* instances               /*!< The Instances object to destroy */
                 )
{
    if (instances->handler)
    {
        instances->handler.reset();
    }

    if (instances->context)
    {
        NB_ContextDestroy(instances->context);
    }

    if (instances->pal)
    {
        PAL_Destroy(instances->pal);
    }

    if (instances->event)
    {
        DestroyCallbackCompletedEvent(instances->event);
    }
}

/*! Do a search request

    Gets called from all tests. Returns the search information once the download has finished.

    @return NE_OK if success
    @see RequestHandlerCallback
*/
NB_Error
DoRequest(Instances* instances,                     /*!< The Instances object */
          SingleSearchSourceParametersSharedPtr parameters    /*!< The parameters used for the search request */
          )
{
    NB_Error result = NE_OK;

    //Create AsyncCallback for search Request.
    shared_ptr<SearchAsyncCallback> handlerCallback(new SearchAsyncCallback());
    if (!handlerCallback)
    {
        result = NE_NOMEM;
        CU_ASSERT_EQUAL(result, NE_OK);
        return result;
    }

    instances->handler->Request(parameters, handlerCallback);

    if (!WaitForCallbackCompletedEvent(instances->event, 60000))
    {
        CU_FAIL("Download timeout.");
        return NE_UNEXPECTED;
    }

    return result;
}

/*! Do a suggest request

    Gets called from all tests. Returns the search information once the download has finished.

    @return NE_OK if success
    @see RequestHandlerCallback
*/
NB_Error
DoSuggestRequest(Instances* instances,                     /*!< The Instances object */
                 SingleSearchSourceParametersSharedPtr parameters    /*!< The parameters used for the search request */
                )
{
    NB_Error result = NE_OK;

    //Create AsyncCallback for search Request.
    shared_ptr<SuggestionAsyncCallback> handlerCallback(new SuggestionAsyncCallback());
    if (!handlerCallback)
    {
        result = NE_NOMEM;
        CU_ASSERT_EQUAL(result, NE_OK);
        return result;
    }

    instances->handler->Request(parameters, handlerCallback);

    if (!WaitForCallbackCompletedEvent(instances->event, 60000))
    {
        CU_FAIL("Download timeout.");
        return NE_UNEXPECTED;
    }

    return result;
}

SingleSearchSourceParametersSharedPtr
CreateSearchParameter(string resultStyleName)
{
    SingleSearchSourceParametersSharedPtr parameters(new SingleSearchSourceParameters());

    CU_ASSERT_PTR_NOT_NULL(parameters.get());
    shared_ptr<string> scheme(new string(TEST_SEARCH_SCHEME));
    CU_ASSERT_PTR_NOT_NULL(scheme.get());
    if (!scheme)
    {
        DestroyInstances(&g_instances);
        return SingleSearchSourceParametersSharedPtr();
    }
    parameters->SetDataScheme(scheme);

    //position temp hard code for testing
    PositionSharedPtr position(new Position);
    position->SetVariant(shared_ptr<string>(new std::string("point")));
    protocol::PointSharedPtr point(new Point);
    point->SetLat(TEST_LATITUDE);
    point->SetLon(TEST_LONGITUDE);
    position->SetPoint(point);

    parameters->GetPositionArray()->push_back(position);

    //iter-command hard code temp
    shared_ptr<IterCommand> iterCommand(new IterCommand);
    shared_ptr<string> command(new string("start"));
    if (!iterCommand || !command)
    {
        DestroyInstances(&g_instances);
        parameters.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    iterCommand->SetCommand(command);
    iterCommand->SetObjectCount(TEST_NUMBER_PER_SLICE);

    parameters->SetIterCommand(iterCommand);

    //search-filter hard code temp
    SearchFilterSharedPtr searchFilter(new SearchFilter);
    ResultStyleSharedPtr resultStyle(new ResultStyle);
    PairSharedPtr pair(new Pair);
    shared_ptr<string> resultStyleKey(new string(resultStyleName));
    shared_ptr<string> name(new string("name"));
    shared_ptr<string> value(new string(TEST_SEARCH_NAME));
    if (!searchFilter || !resultStyle || !pair
        || !resultStyleKey || !name || !value)
    {
        DestroyInstances(&g_instances);
        parameters.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    resultStyle->SetKey(resultStyleKey);
    pair->SetKey(name);
    pair->SetValue(value);
    searchFilter->GetSearchKeyArray()->push_back(pair);
    searchFilter->SetResultStyle(resultStyle);
    parameters->SetSearchFilter(searchFilter);

    shared_ptr<ContentType> contentType(new ContentType);
    contentType->SetType(shared_ptr<string>(new string("poi")));
    shared_ptr<ContentType> contentType1(new ContentType);
    contentType1->SetType(shared_ptr<string>(new string("address")));
    shared_ptr<ContentType> contentType2(new ContentType);
    contentType2->SetType(shared_ptr<string>(new string("movie")));
    shared_ptr<WantContentTypes> wantContentTypes(new WantContentTypes);
    wantContentTypes->GetContentTypeArray()->push_back(contentType);
    wantContentTypes->GetContentTypeArray()->push_back(contentType1);
    wantContentTypes->GetContentTypeArray()->push_back(contentType2);

    return parameters;
}

/*! @} */

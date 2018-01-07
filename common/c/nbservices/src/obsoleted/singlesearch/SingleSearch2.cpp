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

    @file       nbsinglesearch.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "SingleSearch2.h"
#include "SingleSearchPrivate2.h"
#include "SSCommonTypesProtected.h"
#include "nbsuggestmatch.h"
#include "nbsinglesearchparameters.h"
#include "cslutil.h"
#include <cmath>
#include <limits>

/*! Default value of search scheme */
#define DEFAULT_SEARCH_SCHEME   "tcs-single-search"

#define SET_STRING(class_member, param)        \
    if (class_member) {                        \
        nsl_free(class_member);                \
        class_member = NULL;  }                \
    if (param) {                               \
        class_member = nsl_strdup(param); }

#define CHECK_AND_DELETE(param)                \
    if (NULL != param) {                       \
    delete param;                              \
    param = NULL;  }                           \

#define CHECK_AND_DELETE_CHAR(param)           \
    if (NULL != param) {                       \
    nsl_free(param);                           \
    param = NULL;  }                           \

#define DEFAULT_POI_SEARCH_WIDTH_METERS         400
#define DEFAULT_POI_SEARCH_LENGTH_METERS        5000

#define POI_CONTENT_KEY_CUISINES            "cuisines"
#define POI_CONTENT_KEY_DESCRIPTION         "description"
#define POI_CONTENT_KEY_FEATURES            "features"
#define POI_CONTENT_KEY_HOURSOFOPERATION    "hours-of-operation"
#define POI_CONTENT_KEY_PARKING             "parking"
#define POI_CONTENT_KEY_PRICE               "price"
#define POI_CONTENT_KEY_PAYEMENTMETHOD      "payment-method"
#define POI_CONTENT_KEY_RESERVATIONS        "reservations"
#define POI_CONTENT_KEY_SITEURL             "site-url"
#define POI_CONTENT_KEY_SPECIALFEATURES     "special-features"
#define POI_CONTENT_KEY_THUMBNAILPHOTOURL   "thumbnail-photo-url"


using namespace nbcommon2;

// Converters
SuggestionMatch* ConvertSuggestMatchStructToClassLocal(NB_SuggestMatch* suggestMatch);
Place* ConvertPlaceStructToClassLocal(NB_Place* place, NB_BoundingBox* boundingBox, NB_HoursOfOperation *operationalHours, NB_SingleSearchInformation* info);
void AppendExtendedPlaceToClassPlaceLocal(NB_ExtendedPlace* place, Place* placeClass);
FuelCommon* ConvertFuelDetailsStructToClassLocal(NB_FuelDetails* fuelDetails);
FuelCommon* ConvertFuelSummaryStructToClassLocal(NB_FuelSummary* fuelSummary);
NB_SearchSource ConvertSingleSearchSourceToSearchSourceLocal(SingleSearchSource source);
void ConvertSuggestMatchClassToStructLocal(SuggestionMatch* suggestMatch, NB_SuggestMatch* nbSuggestMath);
NB_Error GetBoundingboxLocal(NB_SingleSearchInformation* information, NB_BoundingBox* boundingBox);

//SingleSearch implementation
SingleSearch2::SingleSearch2()
{
    m_impl = NULL;
}

SingleSearch2::SingleSearch2(NB_Context* context,
                    char *language,
                    char *searchScheme,
                    NB_POIExtendedConfiguration extendedConfiguration,
                    NB_SearchSortBy sortBy,
                    NB_SearchInputMethod inputMethod)
{
    m_impl = new SingleSearchImpl2(context, language, searchScheme, extendedConfiguration, sortBy, inputMethod);
}

SingleSearch2::~SingleSearch2()
{
    if (m_impl)
    {
        delete m_impl;
    }
}

PageIterator2<SuggestionMatch>*
SingleSearch2::suggestions(const Criteria* criteria, uint sliceSize)
{
    NB_Error err = NE_OK;
    if (criteria)
    {
        if (criteria->isSearchQueryValid() && criteria->isSearchPointValid())
        {
            LatitudeLongitude latLon = criteria->searchPoint();
            LatitudeLongitude orgin = criteria->currentLocation();
            NB_SearchRegion searchRegion;
            NB_Location m_origin ={0};
            searchRegion.type = NB_ST_Center;
            searchRegion.center.latitude = latLon.GetLatitude();
            searchRegion.center.longitude = latLon.GetLongitude();

            if( orgin.GetLatitude() == INVALID_LATLON || orgin.GetLongitude() == INVALID_LATLON ||
                !criteria->isCurrentLocationValid())
            {
                m_origin.latitude = latLon.GetLatitude();
                m_origin.longitude = latLon.GetLongitude();
            }
            else
            {
                m_origin.latitude = orgin.GetLatitude();
                m_origin.longitude = orgin.GetLongitude();
            }

            std::string language;
            if(criteria->isLanguageValid())
            {
            	language = criteria->language();
            }
            else
            {
            	language = m_impl->m_language;
            }

            NB_SingleSearchParameters* parameters = NULL;
            err =  NB_SingleSearchParametersCreateByResultStyleType(m_impl->m_context,
                                                                &searchRegion,
                                                                criteria->searchQuery().c_str(),
                                                                m_impl->m_searchScheme,
                                                                sliceSize,
                                                                m_impl->m_extendedConfiguration,
                                                                language.c_str(),
                                                                NB_SRST_Suggestion,
                                                                &parameters);

            err = NB_SingleSearchParametersSetSource(parameters, ConvertSingleSearchSourceToSearchSourceLocal(criteria->singleSearchSource()));
            err = NB_SingleSearchParametersSetOrigin(parameters, &m_origin);
            if (err == NE_OK)
            {
                return new PageIteratorImpl2<SuggestionMatch>(m_impl->m_context,
                    parameters,
                    SSR_SuggestionMatch);
            }
        }
    }
    return new PageIteratorImpl2<SuggestionMatch>(NULL, NULL);
}

PageIterator2<Place>*
SingleSearch2::place(const Criteria* criteria, uint sliceSize)
{
    NB_Error err = NE_OK;
    if(criteria)
    {
        if (criteria->isSearchPointValid())
        {
            LatitudeLongitude latLon = criteria->searchPoint();
            LatitudeLongitude orgin = criteria->currentLocation();
            NB_SearchRegion searchRegion;
            NB_Location m_origin ={0};
            searchRegion.type = NB_ST_Center;
            searchRegion.center.latitude = latLon.GetLatitude();
            searchRegion.center.longitude = latLon.GetLongitude();

            if( orgin.GetLatitude() == INVALID_LATLON || orgin.GetLongitude() == INVALID_LATLON ||
                !criteria->isCurrentLocationValid())
            {
                m_origin.latitude = latLon.GetLatitude();
                m_origin.longitude = latLon.GetLongitude();
            }
            else
            {
                m_origin.latitude = orgin.GetLatitude();
                m_origin.longitude = orgin.GetLongitude();
            }

            std::string language;
            if(criteria->isLanguageValid())
            {
            	language = criteria->language();
            }
            else
            {
            	language = m_impl->m_language;
            }

            searchRegion.type = NB_ST_Center;
            NB_SingleSearchParameters* parameters = NULL;
            err =  NB_SingleSearchParametersCreateByResultStyleType(m_impl->m_context,
                                                                &searchRegion,
                                                                criteria->searchQuery().c_str(),
                                                                m_impl->m_searchScheme,
                                                                sliceSize,
                                                                m_impl->m_extendedConfiguration,
                                                                language.c_str(),
                                                                NB_SRST_SingleSearch,
                                                                &parameters);

            NB_SingleSearchParametersSetSource(parameters, ConvertSingleSearchSourceToSearchSourceLocal(criteria->singleSearchSource()));
            NB_SingleSearchParametersSetOrigin(parameters, &m_origin);
            if(!criteria->categories().empty())
            {
                NB_SingleSearchParametersAddSearchFilterKeyValue(parameters, "category", criteria->categories().front().c_str());
            }
            if (err == NE_OK)
            {
                return new PageIteratorImpl2<Place>(m_impl->m_context, parameters, SSR_Place);
            }
        }
    }
    return new PageIteratorImpl2<Place>(NULL, NULL);
}

PageIterator2<Place>*
SingleSearch2::placeBySuggestion(const Criteria* criteria, uint sliceSize)
{
    NB_Error err = NE_OK;
    if(criteria)
    {
        if (!criteria->suggestionCookie().empty() && criteria->isSearchPointValid())
        {
            LatitudeLongitude latLon = criteria->searchPoint();
            LatitudeLongitude orgin = criteria->currentLocation();
            char* serializedData = (char *)nsl_malloc(criteria->suggestionCookie().size() + 1);
            if (serializedData != NULL)
            {
                int len = decode_base64(criteria->suggestionCookie().c_str(), serializedData);
                if (len != -1 && len >= 4)
                {
                    NB_SearchRegion searchRegion;
                    searchRegion.type = NB_ST_Center;
                    NB_Location m_origin ={0};
                    NB_SingleSearchParameters* parameters = NULL;

                    if( orgin.GetLatitude() == INVALID_LATLON || orgin.GetLongitude() == INVALID_LATLON ||
                        !criteria->isCurrentLocationValid())
                    {
                        m_origin.latitude = latLon.GetLatitude();
                        m_origin.longitude = latLon.GetLongitude();
                    }
                    else
                    {
                        m_origin.latitude = orgin.GetLatitude();
                        m_origin.longitude = orgin.GetLongitude();
                    }

                    searchRegion.center.latitude = latLon.GetLatitude();
                    searchRegion.center.longitude = latLon.GetLongitude();

                    std::string language;
                    if(criteria->isLanguageValid())
                    {
                    	language = criteria->language();
                    }
                    else
                    {
                    	language = m_impl->m_language;
                    }


                    NB_SuggestMatch* suggestMatch = NULL;
                    NB_SuggestMatchCreateBySerializedData(m_impl->m_context, (uint8*)serializedData, len, &suggestMatch);
                    if (suggestMatch != NULL)
                    {
                        err =  NB_SingleSearchParametersCreateBySuggestion(m_impl->m_context,
                                                                            &searchRegion,
                                                                            criteria->searchQuery().c_str(),
                                                                            m_impl->m_searchScheme,
                                                                            sliceSize,
                                                                            m_impl->m_extendedConfiguration,
                                                                            language.c_str(),
                                                                            suggestMatch,
                                                                            &parameters);
                        NB_SuggestMatchDestroy(suggestMatch);
                    }

                    if(!criteria->categories().empty())
                    {
                        NB_SingleSearchParametersAddSearchFilterKeyValue(parameters, "category", criteria->categories().front().c_str());
                    }

                    NB_SingleSearchParametersSetSource(parameters, ConvertSingleSearchSourceToSearchSourceLocal(criteria->singleSearchSource()));
                    NB_SingleSearchParametersSetOrigin(parameters, &m_origin);

                    if (err == NE_OK)
                    {
                    	nsl_free(serializedData);
                        return new PageIteratorImpl2<Place>(m_impl->m_context, parameters, SSR_Place);
                    }
                }
                nsl_free(serializedData);
            }
        }
    }
    return new PageIteratorImpl2<Place>(NULL, NULL);
}
//For navigation
PageIterator2<Place>*
SingleSearch2::place(const Criteria* criteria, uint sliceSize, NB_RouteInformation* route)
{
    NB_Error err = NE_OK;
    if (criteria)
    {
        if (criteria->isSearchQueryValid() && criteria->isSearchPointValid() && route)
        {
            LatitudeLongitude latLon = criteria->searchPoint();
            NB_SearchRegion searchRegion;
            searchRegion.center.latitude = latLon.GetLatitude();
            searchRegion.center.longitude = latLon.GetLongitude();
            searchRegion.width = DEFAULT_POI_SEARCH_WIDTH_METERS;
            searchRegion.distance = DEFAULT_POI_SEARCH_LENGTH_METERS;
            searchRegion.route = route;
            searchRegion.type = NB_ST_RouteAndCenter;

            std::string language;
            if(criteria->isLanguageValid())
            {
            	language = criteria->language();
            }
            else
            {
            	language = m_impl->m_language;
            }

            NB_SingleSearchParameters* parameters = NULL;
            err =  NB_SingleSearchParametersCreateByResultStyleType(m_impl->m_context,
                                                                &searchRegion,
                                                                criteria->searchQuery().c_str(),
                                                                m_impl->m_searchScheme,
                                                                sliceSize,
                                                                m_impl->m_extendedConfiguration,
                                                                language.c_str(),
                                                                NB_SRST_SingleSearch,
                                                                &parameters);
            if (err == NE_OK)
            {
                return new PageIteratorImpl2<Place>(m_impl->m_context, parameters, SSR_Place);
            }
        }
    }
    return new PageIteratorImpl2<Place>(NULL, NULL);
}

PageIterator2<FuelCommon>*
SingleSearch2::fuel(const Criteria* criteria, uint sliceSize)
{
    NB_Error err = NE_OK;
    if (criteria)
    {
        if (criteria->isSearchQueryValid() && criteria->isSearchPointValid())
        {
            LatitudeLongitude latLon = criteria->searchPoint();

            NB_SearchRegion searchRegion;
            searchRegion.center.latitude = latLon.GetLatitude();
            searchRegion.center.longitude = latLon.GetLongitude();
            searchRegion.type = NB_ST_Center;

            std::string language;
            if(criteria->isLanguageValid())
            {
            	language = criteria->language();
            }
            else
            {
            	language = m_impl->m_language;
            }

            NB_SingleSearchParameters* parameters = NULL;
            err =  NB_SingleSearchParametersCreateByResultStyleType(m_impl->m_context,
                                                                &searchRegion,
                                                                criteria->searchQuery().c_str(),
                                                                m_impl->m_searchScheme,
                                                                sliceSize,
                                                                m_impl->m_extendedConfiguration,
                                                                language.c_str(),
                                                                NB_SRST_SingleSearch,
                                                                &parameters);
            NB_SingleSearchParametersAddSearchFilterKeyValue(parameters, "category", "ACC");
            if (err == NE_OK)
            {
                return new PageIteratorImpl2<FuelCommon>(m_impl->m_context, parameters, SSR_Fuel);
            }
        }
    }
    return new PageIteratorImpl2<FuelCommon>(NULL, NULL);
}

/////////////////////////////////////////////////////////////////////
//SingleSearchImpl implementation
SingleSearchImpl2::SingleSearchImpl2(NB_Context *context,
                                   char *language,
                                   char *searchScheme,
                                   NB_POIExtendedConfiguration extendedConfiguration,
                                   NB_SearchSortBy sortBy,
                                   NB_SearchInputMethod inputMethod) : SingleSearch2()
{
    m_context = context;
    m_language = NULL;
    m_searchScheme = NULL;

    if (NULL != language)
    {
        SET_STRING(m_language, language);
    }
    if (NULL != searchScheme)
    {
        SET_STRING(m_searchScheme, searchScheme);
    }
    else
    {
        SET_STRING(m_searchScheme, DEFAULT_SEARCH_SCHEME);
    }

    m_extendedConfiguration = extendedConfiguration;
    m_sortBy = sortBy;
    m_inputMethod = inputMethod;
}

SingleSearchImpl2::~SingleSearchImpl2()
{
    CHECK_AND_DELETE_CHAR(m_language);
    CHECK_AND_DELETE_CHAR(m_searchScheme);
}

////////////////////////////////////////////////////////////////////
// PageIterator implementation
template <class T>
void PageIteratorImpl2<T>::SingleSearch_StaticCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    PageIteratorImpl2<T>* pThis = static_cast<PageIteratorImpl2<T>*>(userData);
    pThis->SingleSearch_Callback(status, err, percent);
}

template <class T>
void PageIteratorImpl2<T>::SingleSearch_Callback(NB_RequestStatus status, NB_Error err, int percent)
{
    if (NE_OK != err)
    {
        m_userCallback->Error(err);
        return;
    }

    if (status == NB_NetworkRequestStatus_Progress)
    {
        m_userCallback->Progress(percent);
    }
    else if (status == NB_NetworkRequestStatus_Success)
    {
        NB_SearchResultStyleType resultStyleType;
        err = NB_SingleSearchHandlerGetInformation(m_handler, &m_information);

        if (NE_OK != err)
        {
            m_userCallback->Error(err);
            return;
        }

        err = NB_SingleSearchInformationGetResultStyleType(m_information, &resultStyleType);

        if (NE_OK != err)
        {
            m_userCallback->Error(err);
            return;
        }

        switch (m_resultType)
        {
            case SSR_SuggestionMatch:
            {
                SingleSearch_SuggestionMatchCallback();
                break;
            }
            case SSR_Place:
            {
                SingleSearch_PlaceCallback();
                break;
            }
            case SSR_Fuel:
            {
                SingleSearch_FuelCallback();
                break;
            }
            case SSR_None:
            default:
            {
                //This situation is possible if you didn't pass any result type to PageIterator constructor
                m_userCallback->Error(NE_INVAL);
                break;
            }
        }
    }
    else if (status == NB_NetworkRequestStatus_Canceled)
    {
        m_userCallback->Error(NE_CANCELLED);
    }
    else
    {
        m_userCallback->Error(NE_NET);
        return;
    }
}

template <class T>
void PageIteratorImpl2<T>::SingleSearch_SuggestionMatchCallback()
{
    NB_Error err = NE_OK;
    uint32 resultCount = 0;
    err = NB_SingleSearchInformationGetResultCount(m_information, &resultCount);
    NB_SuggestMatch* suggestMatch = NULL;
    std::vector<T*> resultsVector;
    int offset = m_requestOffset;
    //todo:: lock result list here if getNext/getData is not called in CCC thread
    typename vector<T*>::reverse_iterator currentResultIter = m_resultList.rbegin();
    for (uint32 i = m_resultList.size(); i> offset; i--)
    {
        resultsVector.insert(resultsVector.begin(), *currentResultIter);
        currentResultIter++;
    }
    for (uint32 resultIndex = 0; resultIndex < resultCount; resultIndex++)
    {
        err = NB_SingleSearchInformationGetSuggestMatch(m_information,
                                                        resultIndex,
                                                        &suggestMatch);

        if (NE_OK != err)
        {
            m_userCallback->Error(err);
            return;
        }

        if (suggestMatch == NULL)
        {
            m_userCallback->Error(NE_INVAL);
        }

        T* convertedClass = reinterpret_cast<T*>(ConvertSuggestMatchStructToClassLocal(suggestMatch));
        NB_SuggestMatchDestroy(suggestMatch);
        suggestMatch = NULL;
        //todo:: lock result list here if getNext/getData is not called in CCC thread
        m_resultList.push_back(convertedClass);
        if (m_resultList.size() > offset)
        {
            resultsVector.push_back(convertedClass);
        }
    }

    m_hasResults = TRUE;
    SingleSearchResultListImpl2<T>* vec = new SingleSearchResultListImpl2<T>(resultsVector);
    m_userCallback->Success(vec);
    delete vec;
}
template <class T>
void PageIteratorImpl2<T>::SingleSearch_PlaceCallback()
{
    NB_Error err = NE_OK;
    uint32 resultCount = 0;
    err = NB_SingleSearchInformationGetResultCount(m_information, &resultCount);
    std::vector<T*> resultsVector;
    int offset = m_requestOffset;
    //todo:: lock result list here if getNext/getData is not called in CCC thread
    typename vector<T*>::reverse_iterator currentResultIter = m_resultList.rbegin();
    for (uint32 i = m_resultList.size(); i> offset; i--)
    {
        resultsVector.insert(resultsVector.begin(), *currentResultIter);
        currentResultIter++;
    }
    for (uint32 resultIndex = 0; resultIndex < resultCount; resultIndex++)
    {
        NB_SearchResultType resultType;
        NB_SingleSearchInformationGetResultType(m_information, resultIndex, &resultType);
        if (resultType == NB_SRT_Place)
        {
            NB_Place place = {0};
            NB_BoundingBox boundingBox = {0};
            double distance = 0.0;
            NB_ExtendedPlace* extPlace = NULL;
            err = NB_SingleSearchInformationGetPlace(m_information,
                                                     resultIndex,
                                                     &place,
                                                     &distance, &extPlace);
            if (NE_OK != err)
            {
                m_userCallback->Error(err);
                return;
            }

            err =  GetBoundingboxLocal(m_information, &boundingBox);

            NB_HoursOfOperation operationalHours = {0};
            NB_SingleSearchInformationGetHoursOfOperation(m_information, resultIndex, &operationalHours);

            T* convertedClass = reinterpret_cast<T*>(ConvertPlaceStructToClassLocal(&place, &boundingBox, &operationalHours, m_information));
            AppendExtendedPlaceToClassPlaceLocal(extPlace, reinterpret_cast<Place*>(convertedClass));

            NB_SingleSearchInformationFreeExtendedPlaces(m_information);
            NB_HoursOfOperationDestroy(&operationalHours);

            //todo:: lock result list here if getNext/getData is not called in CCC thread
            m_resultList.push_back(convertedClass);
            if (m_resultList.size() > offset)
            {
                resultsVector.push_back(convertedClass);
            }
        }
    }



    m_hasResults = TRUE;
    SingleSearchResultListImpl2<T>* vec = new SingleSearchResultListImpl2<T>(resultsVector);
    m_userCallback->Success(vec);
    delete vec;
}

template <class T>
void PageIteratorImpl2<T>::SingleSearch_FuelCallback()
{
    NB_Error err = NE_OK;
    uint32 resultCount = 0;
    err = NB_SingleSearchInformationGetResultCount(m_information, &resultCount);
    std::vector<T*> resultsVector;
    int offset = m_requestOffset;
    //todo:: lock result list here if getNext/getData is not called in CCC thread
    typename vector<T*>::reverse_iterator currentResultIter = m_resultList.rbegin();
    for (uint32 i = m_resultList.size(); i> offset; i--)
    {
        resultsVector.insert(resultsVector.begin(), *currentResultIter);
        currentResultIter++;
    }
    for (uint32 resultIndex = 0; resultIndex < resultCount; resultIndex++)
    {
        NB_SearchResultType resultType;
        NB_SingleSearchInformationGetResultType(m_information, resultIndex, &resultType);
        if (resultType & NB_SRT_FuelDetails)
        {
            NB_FuelDetails fuelDetails = {0};

            err = NB_SingleSearchInformationGetFuelDetails(m_information,
                                                        resultIndex,
                                                        &fuelDetails);
            if(NE_OK != err)
            {
                m_userCallback->Error(err);
                return;
            }


            T* convertedClass = reinterpret_cast<T*>(ConvertFuelDetailsStructToClassLocal(&fuelDetails));
            resultsVector.push_back(convertedClass);
        }
        else if (resultType & NB_SRT_FuelSummary)
        {
            NB_FuelSummary fuelSummary = {0};
            err = NB_SingleSearchInformationGetFuelSummary(m_information,
                                                        &fuelSummary);
            if(NE_OK != err)
            {
                m_userCallback->Error(err);
                return;
            }

            T* convertedClass = reinterpret_cast<T*>(ConvertFuelSummaryStructToClassLocal(&fuelSummary));
            //todo:: lock result list here if getNext/getData is not called in CCC thread
            m_resultList.push_back(convertedClass);
            if (m_resultList.size() > offset)
            {
                resultsVector.push_back(convertedClass);
            }
        }
    }

    m_hasResults = TRUE;
    SingleSearchResultListImpl2<T>* vec = new SingleSearchResultListImpl2<T>(resultsVector);
    m_userCallback->Success(vec);
    delete vec;
}

////////////////////////////////////////////////////////////////////
// PageIterator implementation
template <class T>
PageIteratorImpl2<T>::PageIteratorImpl2(NB_Context* context,
                                      NB_SingleSearchParameters* parameters)
{
    m_information = NULL;
    m_handler = NULL;
    m_userCallback = NULL;

    m_context = context;
    m_parameters = parameters;
    m_hasResults = false;
    m_resultType = SSR_None;
    m_requestOffset = 0;
}
template <class T>
PageIteratorImpl2<T>::PageIteratorImpl2(NB_Context* context,
                     NB_SingleSearchParameters* parameters,
                     SingleSearchResultType resultType)
{
    m_information = NULL;
    m_handler = NULL;
    m_userCallback = NULL;

    m_context = context;
    m_parameters = parameters;
    m_hasResults = false;
    m_resultType = resultType;
    m_requestOffset = 0;
}

template <class T>
PageIteratorImpl2<T>::~PageIteratorImpl2()
{
    if (NULL != m_handler)
    {
        NB_SingleSearchHandlerDestroy(m_handler);
        m_handler = NULL;
    }

    if (NULL != m_parameters)
    {
        NB_SingleSearchParametersDestroy(m_parameters);
        m_parameters = NULL;
    }

    if (NULL != m_information)
    {
        NB_SingleSearchInformationDestroy(m_information);
        m_information = NULL;
    }

    if (!m_resultList.empty())
    {
        for( typename std::vector<T*>::iterator iter = m_resultList.begin(); iter != m_resultList.end(); ++iter)
        {
            delete *iter;
        }
        m_resultList.clear();
    }
}

template <class T>
void PageIteratorImpl2<T>::getData(int offset, int count, AsyncCallback<SingleSearchResultList2<T>* >* callback)
{
    NB_Error err = NE_OK;

    if (NULL == callback)
    {
        return;
    }

    int lastIndex = offset + count - 1;
    if (lastIndex < 0)
    {
        callback->Error(NE_INVAL);
        return;
    }
    //todo: lock resultlist here if this function is not called in CCC thread
    int resultListSize = m_resultList.size();
    if (lastIndex < resultListSize) // cache hit
    {
        vector<T*> resultsVector;
        typename vector<T*>::iterator iter = m_resultList.begin();
        for (int i = 0; i <= lastIndex; i++)
        {
            if (i>= offset)
            {
                resultsVector.push_back(*iter);
            }
            iter++;
        }
        SingleSearchResultListImpl2<T>* vec = new SingleSearchResultListImpl2<T>(resultsVector);
        callback->Success(vec);
        delete vec;
        return;
    }

    //start a new request
    if (!m_parameters || !m_context)
    {
        callback->Error(NE_NOTINIT);
        return;
    }

    m_userCallback = callback;
    if (false == m_hasResults)
    {
        NB_RequestHandlerCallback networkCallback = {SingleSearch_StaticCallback, this};

        err = NB_SingleSearchHandlerCreate(m_context, &networkCallback, &m_handler);

        if (NE_OK != err)
        {
            callback->Error(err);
            return;
        }
    }
    else
    {
        if (NULL != m_parameters)
        {
            NB_SingleSearchParametersDestroy(m_parameters);
            m_parameters = NULL;
        }

        err = NB_SingleSearchParametersCreateByIteration(m_context, m_information, NB_IterationCommand_Next, &m_parameters);
        err = err ? err : NB_SingleSearchParametersSetSearchInputMethod(m_parameters, NB_SIM_More);

        if (NULL != m_information)
        {
            NB_SingleSearchInformationDestroy(m_information);
            m_information = NULL;
        }

        if (NE_OK != err)
        {
            callback->Error(err);
            return;
        }
    }

    NB_SingleSearchParametersSetSliceSize(m_parameters, lastIndex - m_resultList.size() + 1);
    err = NB_SingleSearchHandlerStartRequest(m_handler, m_parameters);

    if (NE_OK != err)
    {
        callback->Error(err);
        return;
    }
    m_requestOffset = offset;
}
////////////////////////////////////////////////////////////
// SingleSearchResultListImpl implementation
template <class T>
SingleSearchResultListImpl2<T>::SingleSearchResultListImpl2(std::vector<T*> list)
{
    m_position = 0;
    m_resultList = list;
}

template <class T>
SingleSearchResultListImpl2<T>::~SingleSearchResultListImpl2()
{

}

template <class T>
T* SingleSearchResultListImpl2<T>::getNext()
{
    if (m_position == m_resultList.size())
    {
        return NULL;
    }

    return m_resultList.at(m_position++);
}

template <class T>
T* SingleSearchResultListImpl2<T>::getPrev()
{
    if (m_position == 0)
    {
        return NULL;
    }

    return m_resultList.at(--m_position);
}

/////////////////////////////////////////////////////////////////////
// Convertres implementation
FuelCommon* ConvertFuelDetailsStructToClassLocal(NB_FuelDetails* fuelDetails)
{
    FuelCommonImpl* fuelCommon = new FuelCommonImpl();

    FuelDetailsImpl fuelDetailsClass;
    FuelTypeImpl fuelType;
    PriceImpl price;
    FuelProductImpl fuelProduct;

    fuelDetailsClass.SetNumFuelProducts(fuelDetails->num_fuelproducts);
    for (int i = 0; i < fuelDetails->num_fuelproducts; i++)
    {
        fuelType.SetCode(fuelDetails->fuel_product[i].fuel_type.code);
        fuelType.SetProductName(fuelDetails->fuel_product[i].fuel_type.product_name);
        fuelType.SetTypeName(fuelDetails->fuel_product[i].fuel_type.type_name);


        price.SetCurrency(fuelDetails->fuel_product[i].price.currency);
        price.SetModTime(fuelDetails->fuel_product[i].price.modtime);
        price.SetValue(fuelDetails->fuel_product[i].price.value);


        fuelProduct.SetFuelType(&fuelType);
        fuelProduct.SetPrice(&price);
        fuelProduct.SetUnits(fuelDetails->fuel_product[i].units);


        fuelDetailsClass.SetFuelProduct(i, &fuelProduct);
    }

    fuelCommon->SetFuelDetails(&fuelDetailsClass);
    return static_cast<FuelCommon*>(fuelCommon);
}
FuelCommon* ConvertFuelSummaryStructToClassLocal(NB_FuelSummary* fuelSummary)
{
    FuelCommonImpl* fuelCommon = new FuelCommonImpl();
    FuelSummaryImpl fuelSummaryClass;

    FuelProductImpl fuelProduct;
    FuelTypeImpl fuelType;
    PriceImpl price;
//Set Average
    fuelType.SetCode(fuelSummary->average.fuel_type.code);
    fuelType.SetProductName(fuelSummary->average.fuel_type.product_name);
    fuelType.SetTypeName(fuelSummary->average.fuel_type.type_name);

    price.SetCurrency(fuelSummary->average.price.currency);
    price.SetModTime(fuelSummary->average.price.modtime);
    price.SetValue(fuelSummary->average.price.value);

    fuelProduct.SetFuelType(&fuelType);
    fuelProduct.SetPrice(&price);
    fuelProduct.SetUnits(fuelSummary->average.units);

    fuelSummaryClass.SetAverage(&fuelProduct);

//Set Low
    fuelType.SetCode(fuelSummary->low.fuel_type.code);
    fuelType.SetProductName(fuelSummary->low.fuel_type.product_name);
    fuelType.SetTypeName(fuelSummary->low.fuel_type.type_name);

    price.SetCurrency(fuelSummary->low.price.currency);
    price.SetModTime(fuelSummary->low.price.modtime);
    price.SetValue(fuelSummary->low.price.value);

    fuelProduct.SetFuelType(&fuelType);
    fuelProduct.SetPrice(&price);
    fuelProduct.SetUnits(fuelSummary->low.units);

    fuelSummaryClass.SetLow(&fuelProduct);

    fuelCommon->SetFuelSummary(&fuelSummaryClass);
    return static_cast<FuelCommon*>(fuelCommon);
}

typedef std::map<CategoryPtrWrapper, std::vector<CategoryPtrWrapper> > CategoryMap;
class CategoryDic
{
public:
    CategoryDic(NB_SingleSearchInformation* info): m_info(info) {}

    CategoryMap make(NB_Place const& place) const {
        CategoryMap cmap;

        for (int i = 0; i < place.numcategory; ++i)
        {
            NB_Category nb_parent_category;
            nsl_memset(&nb_parent_category, 0, sizeof(nb_parent_category));

            CategoryPtrWrapper current_category_wrapper(new CategoryImpl(place.category[i].code, place.category[i].name));

            NB_Error err = NB_SingleSearchInformationGetParentCategory(m_info, place.category[i].code, &nb_parent_category);

            switch(err)
            {
                case NE_OK:
                {
                    std::vector<CategoryPtrWrapper> subcategories;
                    subcategories.push_back(current_category_wrapper);

                    CategoryPtrWrapper parent_category_wrapper(new CategoryImpl(nb_parent_category.code, nb_parent_category.name));
                    std::pair<CategoryMap::iterator, bool> inserted_item = cmap.insert(std::make_pair(parent_category_wrapper, subcategories));
                    if (!inserted_item.second) {
                        inserted_item.first->second.push_back(current_category_wrapper);
                    }

                    break;
                }
                case NE_NOENT: // If parent not found, make current category be a key to an empty sub-categories.
                {
                    // Here it's not necessary to check whether this call is successful or not,
                    // because there may be several places sharing the same category, one key-value pair is enough.
                    cmap.insert(std::make_pair(current_category_wrapper, std::vector<CategoryPtrWrapper>()));

                    break;
                }
                default:
                    break;
            }
        }

        return cmap;
    }

private:
    NB_SingleSearchInformation* m_info;
};

Place* ConvertPlaceStructToClassLocal(NB_Place* place, NB_BoundingBox* boundingBox, NB_HoursOfOperation* operationalHours, NB_SingleSearchInformation* info)
{
    PlaceImpl* placeClass = new PlaceImpl();

    CategoryDic category_dic(info);
    std::map<CategoryPtrWrapper, std::vector<CategoryPtrWrapper> > categories = category_dic.make(*place);

    std::vector<TimeRangeImpl*> timeRangeVector;
    for (int i = 0; i< operationalHours->timeRangeCount; i++)
    {
        TimeRangeImpl* timeRange = new TimeRangeImpl();
        uint32 startTime = operationalHours->timeRangeArray[i].startTime;
        uint32 endTime = operationalHours->timeRangeArray[i].endTime;
        timeRange->SetStartHour(floor(startTime/3600.0));
        timeRange->SetStartMinute((startTime/60)%60);
        timeRange->SetEndHour(floor(endTime/3600.0));
        timeRange->SetEndMinute((endTime/60)%60);
        timeRange->SetDayOfWeek(operationalHours->timeRangeArray[i].dayOfWeek);

        timeRangeVector.push_back(timeRange);
    }

    LatitudeLongitude coords;
    coords.SetLatitude(place->location.latitude);
    coords.SetLongitude(place->location.longitude);

    LocationImpl loc;
    loc.SetAirport(place->location.airport);
    loc.SetCity(place->location.city);
    loc.SetCoordinates(&coords);

    loc.SetCountry(place->location.country);
    loc.SetCounty(place->location.county);
    loc.SetCrossStreet(place->location.street2);
    loc.SetName(place->location.areaname);
    loc.SetNumber(place->location.streetnum);
    loc.SetState(place->location.state);
    loc.SetStreet(place->location.street1);
    loc.SetType((Location::LocationType)place->location.type);
    loc.SetZipCode(place->location.postal);
    loc.SetAirport(place->location.airport);
    loc.SetCountryName(place->location.country_name);

    ExtendedAddressImpl extendedAddress;
    extendedAddress.SetFirstLine(place->location.extendedAddress.addressLine1);
    extendedAddress.SetSecondLine(place->location.extendedAddress.addressLine2);
    extendedAddress.SetThirdLine(place->location.extendedAddress.addressLine3);
    extendedAddress.SetFourthLine(place->location.extendedAddress.addressLine4);
    extendedAddress.SetFifthLine(place->location.extendedAddress.addressLine5);

    loc.SetExtendedAddress(&extendedAddress);

    CompactAddressImpl compactAddress;
    compactAddress.SetFirstLine(place->location.compactAddress.addressLine1);
    compactAddress.SetSecondLine(place->location.compactAddress.addressLine2);
    compactAddress.SetLineSeparator(place->location.compactAddress.lineSeparator);

    loc.SetCompactAddress(&compactAddress);

    std::vector<PhoneImpl*> phones;
    for (int i = 0; i< place->numphone; i++)
    {
        PhoneImpl* phone = new PhoneImpl();
        phone->SetArea(place->phone[i].area);
        phone->SetCountry(place->phone[i].country);
        phone->SetNumber(place->phone[i].number);
        phone->SetPhoneType((Phone::PhoneType)place->phone[i].type);
        phone->SetFormattedNumber(place->phone[i].formattedText);
        phones.push_back(phone);
    }

    LatitudeLongitude topLeftCoords;
    topLeftCoords.SetLatitude(boundingBox->topLeft.latitude);
    topLeftCoords.SetLongitude(boundingBox->topLeft.longitude);

    LatitudeLongitude bottomRightCoords;
    bottomRightCoords.SetLatitude(boundingBox->bottomRight.latitude);
    bottomRightCoords.SetLongitude(boundingBox->bottomRight.longitude);

    BoundingBoxImpl boundingBoxImpl;
    boundingBoxImpl.SetTopLeftCoordinates(&topLeftCoords);
    boundingBoxImpl.SetBottomRightCoordinates(&bottomRightCoords);


    placeClass->SetLocation(&loc);
    placeClass->SetCategories(categories);
    placeClass->SetName(place->name);
    placeClass->SetTimeRanges(timeRangeVector);
    placeClass->SetUtcOffset(operationalHours->utcOffset);
    placeClass->SetUtcOffsetValidityFlag(operationalHours->hasUtcOffset);
//    placeClass->SetNumCategory(place->numcategory);
//    placeClass->SetNumPhone(place->numphone);
    placeClass->SetPhoneNumbers(phones);
    placeClass->SetBoundingBox(&boundingBoxImpl);

    return static_cast<Place*>(placeClass);
}
void AppendExtendedPlaceToClassPlaceLocal(NB_ExtendedPlace* place, Place* placeClass)
{
    PlaceImpl* placeImpl = static_cast<PlaceImpl*>(placeClass);
    if (!placeImpl || !place)
    {
        return;
    }

    placeImpl->SetAverageRating(place->overallRating.averageRating);
    placeImpl->SetRatingCount((double)place->overallRating.ratingCount);
    //todo::convert poicontend to key value pair
    //placeImpl->SetKeyValues(LS_POIKeys key, const std::string& value);
    int count = place->poiContentCount;
    NB_POIContent* poiContent = place->poiContent;
    for(int i = 0; i < count; i++)
    {
        std::string key = poiContent[i].key;
        LS_POIKeys enumKey;
        if (key == POI_CONTENT_KEY_CUISINES)
        {
            enumKey = LS_PK_cuisines;
        }
        else if (key == POI_CONTENT_KEY_DESCRIPTION)
        {
            enumKey = LS_PK_description;
        }
        else if (key == POI_CONTENT_KEY_FEATURES)
        {
            enumKey = LS_PK_features;
        }
        else if (key == POI_CONTENT_KEY_HOURSOFOPERATION)
        {
            enumKey = LS_PK_hoursOfOperation;
        }
        else if (key == POI_CONTENT_KEY_PARKING)
        {
            enumKey = LS_PK_parking;
        }
        else if (key == POI_CONTENT_KEY_PRICE)
        {
            enumKey = LS_PK_price;
        }
        else if (key == POI_CONTENT_KEY_PAYEMENTMETHOD)
        {
            enumKey = LS_PK_paymentMethod;
        }
        else if (key == POI_CONTENT_KEY_RESERVATIONS)
        {
            enumKey = LS_PK_reservations;
        }
        else if (key == POI_CONTENT_KEY_SITEURL)
        {
            enumKey = LS_PK_siteUrl;
        }
        else if (key == POI_CONTENT_KEY_SPECIALFEATURES)
        {
            enumKey = LS_PK_specialFeatures;
        }
        else if (key == POI_CONTENT_KEY_THUMBNAILPHOTOURL)
        {
            enumKey = LS_PK_thumbnailPhotoUrl;
        }
        else
        {
            continue;
        }

        std::string value = poiContent[i].value;
        placeImpl->SetKeyValues(enumKey, value);
    }
}

SuggestionMatch* ConvertSuggestMatchStructToClassLocal(NB_SuggestMatch* suggestMatch)
{
    SuggestionMatchImpl* suggestMatchClass = new SuggestionMatchImpl();

    NB_SuggestMatchData suggestMatchData = {0};
    NB_Error err = NE_OK;

    err = NB_SuggestMatchGetDataToDisplay(suggestMatch, &suggestMatchData);

    if (NE_OK != err)
    {
        return NULL;
    }

    suggestMatchClass->SetFirstLine(suggestMatchData.line1);
    suggestMatchClass->SetSecondLine(suggestMatchData.line2);
    suggestMatchClass->SetThirdLine(suggestMatchData.line3);
    suggestMatchClass->SetMatchType(suggestMatchData.matchType);
    suggestMatchClass->SetDistance(suggestMatchData.distance);


    char *serializedData = NULL;
    int len = 0;
    NB_SuggestMatchSerializeLite(suggestMatch, (uint8**)(&serializedData), (nb_size*)(&len));

    if (serializedData != NULL)
    {
        char* suggestionCookie = NULL;
        encode_base64(serializedData, len, &suggestionCookie);
        if (suggestionCookie != NULL)
        {
            suggestMatchClass->SetSuggestionCookie(suggestionCookie);
            nsl_free(suggestionCookie);
        }
        nsl_free(serializedData);
    }
    return static_cast<SuggestionMatch*>(suggestMatchClass);
}

NB_SearchSource ConvertSingleSearchSourceToSearchSourceLocal(SingleSearchSource source)
{
    NB_SearchSource retval = NB_SS_None;

    switch (source)
    {
        case (SS_User):
            retval = NB_SS_User;
            break;
        case SS_Address:
            retval = NB_SS_Address;
            break;
        case SS_Carousel:
            retval = NB_SS_Carousel;
            break;
        case SS_Map:
            retval = NB_SS_Map;
            break;
        case SS_Place:
            retval = NB_SS_Place;
            break;
        default:
            retval = NB_SS_None;
            break;
    }
    return retval;
}

NB_Error GetBoundingboxLocal(NB_SingleSearchInformation* information, NB_BoundingBox* boundingBox)
{
	NB_Error err = NE_OK;

	if(!boundingBox)
	{
		return NE_INVAL;
	}

	nsl_memset(boundingBox, 0, sizeof(*boundingBox));

	err = NB_SingleSearchInformationGetDataBox(information, 0,
          &boundingBox->topLeft.latitude, &boundingBox->topLeft.longitude,
          &boundingBox->bottomRight.latitude, &boundingBox->bottomRight.longitude);

	if(boundingBox->topLeft.latitude == 0 && boundingBox->topLeft.longitude == 0 && boundingBox->bottomRight.latitude == 0 && boundingBox->bottomRight.longitude == 0)
	{
		boundingBox->topLeft.latitude = numeric_limits<double>::quiet_NaN();
		boundingBox->topLeft.longitude = numeric_limits<double>::quiet_NaN();
		boundingBox->bottomRight.latitude = numeric_limits<double>::quiet_NaN();
		boundingBox->bottomRight.longitude = numeric_limits<double>::quiet_NaN();
	}

    return err;
}


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

#include "SingleSearch.h"
#include "SingleSearchPrivate.h"
#include "CommonTypesProtected.h"
#include "nbsuggestmatch.h"
#include "nbsinglesearchparameters.h"
#include "cslutil.h"
#include <limits>

/*! Default value of search scheme */
const char* DEFAULT_SEARCH_SCHEME =  "tcs-single-search";

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

using namespace nbcommon;

// Converters
SuggestionMatch* ConvertSuggestMatchStructToClass(NB_SuggestMatch* suggestMatch);
Place* ConvertPlaceStructToClass(NB_Place* place, NB_BoundingBox* boundingBox);
FuelCommon* ConvertFuelDetailsStructToClass(NB_FuelDetails* fuelDetails);
FuelCommon* ConvertFuelSummaryStructToClass(NB_FuelSummary* fuelSummary);
NB_SearchSource ConvertSingleSearchSourceToSearchSource(SingleSearchSource source);
void ConvertSuggestMatchClassToStruct(SuggestionMatch* suggestMatch, NB_SuggestMatch* nbSuggestMath);
NB_Error GetBoundingbox(NB_SingleSearchInformation* information, NB_BoundingBox* boundingBox);

//SingleSearch implementation
SingleSearch::SingleSearch()
{
    m_impl = NULL;
}

SingleSearch::SingleSearch(NB_Context* context,
                    char *language,
                    char *searchScheme,
                    NB_POIExtendedConfiguration extendedConfiguration,
                    NB_SearchSortBy sortBy,
                    NB_SearchInputMethod inputMethod)
{
    m_impl = new SingleSearchImpl(context, language, searchScheme, extendedConfiguration, sortBy, inputMethod);
}

SingleSearch::~SingleSearch()
{
    if (m_impl)
    {
        delete m_impl;
    }
}

PageIterator<SuggestionMatch>*
SingleSearch::suggestions(char *searchQuery,
                          int count,
                          LatitudeLongitude* latLon,
                          SingleSearchSource singleSearchSource,
                          LatitudeLongitude* orgin)
{
    NB_Error err = NE_OK;

    if (searchQuery && latLon)
    {
        NB_SearchRegion searchRegion;
        NB_Location m_origin;
        nsl_memset(&m_origin, 0, sizeof(m_origin));
        searchRegion.type = NB_ST_Center;
        searchRegion.center.latitude = latLon->GetLatitude();
        searchRegion.center.longitude = latLon->GetLongitude();

        if( orgin->GetLatitude() == INVALID_LATLON || orgin->GetLongitude() == INVALID_LATLON)
		{
			m_origin.latitude = latLon->GetLatitude();
			m_origin.longitude = latLon->GetLongitude();
		}
		else
		{
			m_origin.latitude = orgin->GetLatitude();
			m_origin.longitude = orgin->GetLongitude();
		}

        NB_SingleSearchParameters* parameters = NULL;
        err =  NB_SingleSearchParametersCreateByResultStyleType(m_impl->m_context,
                                                            &searchRegion,
                                                            searchQuery,
                                                            m_impl->m_searchScheme,
                                                            count,
                                                            m_impl->m_extendedConfiguration,
                                                            m_impl->m_language,
                                                            NB_SRST_Suggestion,
                                                            &parameters);

        err = NB_SingleSearchParametersSetSource(parameters, ConvertSingleSearchSourceToSearchSource(singleSearchSource));
        err = NB_SingleSearchParametersSetOrigin(parameters, &m_origin);
        if (err == NE_OK)
        {
            return new PageIteratorImpl<SuggestionMatch>(m_impl->m_context,
                parameters,
                SSR_SuggestionMatch);
        }
    }

    return new PageIteratorImpl<SuggestionMatch>(NULL, NULL);
}

PageIterator<Place>*
SingleSearch::place(char* searchQuery,
                   int count,
                   LatitudeLongitude* latLon,
                   SingleSearchSource singleSearchSource,
                   const char* category,
                   LatitudeLongitude* orgin)
{
    NB_Error err = NE_OK;

    if (searchQuery && latLon)
    {
        NB_SearchRegion searchRegion;
        NB_Location m_origin;
        nsl_memset(&m_origin, 0, sizeof(m_origin));
        searchRegion.center.latitude = latLon->GetLatitude();
        searchRegion.center.longitude = latLon->GetLongitude();

		if( orgin->GetLatitude() == INVALID_LATLON || orgin->GetLongitude() == INVALID_LATLON)
		{
			m_origin.latitude = latLon->GetLatitude();
			m_origin.longitude = latLon->GetLongitude();
		}
		else
		{
			m_origin.latitude = orgin->GetLatitude();
			m_origin.longitude = orgin->GetLongitude();
		}

        searchRegion.type = NB_ST_Center;
        NB_SingleSearchParameters* parameters = NULL;
        err =  NB_SingleSearchParametersCreateByResultStyleType(m_impl->m_context,
                                                            &searchRegion,
                                                            searchQuery,
                                                            m_impl->m_searchScheme,
                                                            count,
                                                            m_impl->m_extendedConfiguration,
                                                            m_impl->m_language,
                                                            NB_SRST_SingleSearch,
                                                            &parameters);

        NB_SingleSearchParametersSetSource(parameters, ConvertSingleSearchSourceToSearchSource(singleSearchSource));
        NB_SingleSearchParametersSetOrigin(parameters, &m_origin);
        NB_SingleSearchParametersAddSearchFilterKeyValue(parameters, "category", category);

        if (err == NE_OK)
        {
            return new PageIteratorImpl<Place>(m_impl->m_context, parameters, SSR_Place);
        }
    }

    return new PageIteratorImpl<Place>(NULL, NULL);
}

PageIterator<Place>*
SingleSearch::placeBySuggestion(char* searchQuery,
				   int count,
                   LatitudeLongitude* latLon,
                   const char* suggestionCookie,
                   SingleSearchSource singleSearchSource,
                   const char* category,
                   LatitudeLongitude* orgin)
{
    NB_Error err = NE_OK;

    if (suggestionCookie && latLon)
    {
        char* serializedData = (char *)nsl_malloc(nsl_strlen(suggestionCookie)+1);
        if (serializedData != NULL)
        {
            int len = decode_base64(suggestionCookie, serializedData);
            if (len != -1)
            {
                NB_SearchRegion searchRegion;
                searchRegion.type = NB_ST_Center;
                NB_Location m_origin;
                NB_SingleSearchParameters* parameters = NULL;
                nsl_memset(&m_origin, 0, sizeof(m_origin));

				if( orgin->GetLatitude() == INVALID_LATLON || orgin->GetLongitude() == INVALID_LATLON)
				{
					m_origin.latitude = latLon->GetLatitude();
					m_origin.longitude = latLon->GetLongitude();
				}
				else
				{
					m_origin.latitude = orgin->GetLatitude();
					m_origin.longitude = orgin->GetLongitude();
				}

                searchRegion.center.latitude = latLon->GetLatitude();
                searchRegion.center.longitude = latLon->GetLongitude();

                NB_SuggestMatch* suggestMatch = NULL;
                NB_SuggestMatchCreateBySerializedData(m_impl->m_context, (uint8*)serializedData, len, &suggestMatch);
                if (suggestMatch != NULL)
                {
                    err =  NB_SingleSearchParametersCreateBySuggestion(m_impl->m_context,
                                                                        &searchRegion,
                                                                        searchQuery,
                                                                        m_impl->m_searchScheme,
                                                                        count,
                                                                        m_impl->m_extendedConfiguration,
                                                                        m_impl->m_language,
                                                                        suggestMatch,
                                                                        &parameters);
                    nsl_free(suggestMatch);
                }

                if (category)
                {
                    NB_SingleSearchParametersAddSearchFilterKeyValue(parameters, "category", category);
                }

                NB_SingleSearchParametersSetSource(parameters, ConvertSingleSearchSourceToSearchSource(singleSearchSource));
                NB_SingleSearchParametersSetOrigin(parameters, &m_origin);

                if (err == NE_OK)
                {
                    return new PageIteratorImpl<Place>(m_impl->m_context, parameters, SSR_Place);
                }
            }
            nsl_free(serializedData);
        }
    }

    return new PageIteratorImpl<Place>(NULL, NULL);
}
//For navigation
PageIterator<Place>*
SingleSearch::place(char *searchQuery,
                    int count,
                    LatitudeLongitude* latLon,
                    NB_RouteInformation* route)
{
    NB_Error err = NE_OK;

    if (searchQuery && latLon && route)
    {
        NB_SearchRegion searchRegion;
        searchRegion.center.latitude = latLon->GetLatitude();
        searchRegion.center.longitude = latLon->GetLongitude();
        searchRegion.width = DEFAULT_POI_SEARCH_WIDTH_METERS;
        searchRegion.distance = DEFAULT_POI_SEARCH_LENGTH_METERS;
        searchRegion.route = route;
        searchRegion.type = NB_ST_RouteAndCenter;

        NB_SingleSearchParameters* parameters = NULL;
        err =  NB_SingleSearchParametersCreateByResultStyleType(m_impl->m_context,
                                                            &searchRegion,
                                                            searchQuery,
                                                            m_impl->m_searchScheme,
                                                            count,
                                                            m_impl->m_extendedConfiguration,
                                                            m_impl->m_language,
                                                            NB_SRST_SingleSearch,
                                                            &parameters);
        if (err == NE_OK)
        {
            return new PageIteratorImpl<Place>(m_impl->m_context, parameters, SSR_Place);
        }
    }

    return new PageIteratorImpl<Place>(NULL, NULL);
}

PageIterator<FuelCommon>*
SingleSearch::fuel(char *searchQuery,
                    int count,
                    LatitudeLongitude* latLon)
{
    NB_Error err = NE_OK;

    if (searchQuery && latLon)
    {
        NB_SearchRegion searchRegion;
        searchRegion.center.latitude = latLon->GetLatitude();
        searchRegion.center.longitude = latLon->GetLongitude();
        searchRegion.type = NB_ST_Center;

        NB_SingleSearchParameters* parameters = NULL;
        err =  NB_SingleSearchParametersCreateByResultStyleType(m_impl->m_context,
                                                            &searchRegion,
                                                            searchQuery,
                                                            m_impl->m_searchScheme,
                                                            count,
                                                            m_impl->m_extendedConfiguration,
                                                            m_impl->m_language,
                                                            NB_SRST_SingleSearch,
                                                            &parameters);
        NB_SingleSearchParametersAddSearchFilterKeyValue(parameters, "category", "ACC");
        if (err == NE_OK)
        {
            return new PageIteratorImpl<FuelCommon>(m_impl->m_context, parameters, SSR_Fuel);
        }
    }

    return new PageIteratorImpl<FuelCommon>(NULL, NULL);
}

/////////////////////////////////////////////////////////////////////
//SingleSearchImpl implementation
SingleSearchImpl::SingleSearchImpl(NB_Context *context,
                                   char *language,
                                   char *searchScheme,
                                   NB_POIExtendedConfiguration extendedConfiguration,
                                   NB_SearchSortBy sortBy,
                                   NB_SearchInputMethod inputMethod) : SingleSearch()
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

SingleSearchImpl::~SingleSearchImpl()
{
    CHECK_AND_DELETE_CHAR(m_language);
    CHECK_AND_DELETE_CHAR(m_searchScheme);
}

////////////////////////////////////////////////////////////////////
// PageIterator implementation
template <class T>
void PageIteratorImpl<T>::SingleSearch_StaticCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    PageIteratorImpl<T>* pThis = static_cast<PageIteratorImpl<T>*>(userData);
    pThis->SingleSearch_Callback(status, err, percent);
}

template <class T>
void PageIteratorImpl<T>::SingleSearch_Callback(NB_RequestStatus status, NB_Error err, int percent)
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
void PageIteratorImpl<T>::SingleSearch_SuggestionMatchCallback()
{
    NB_Error err = NE_OK;
    uint32 resultCount = 0;
    err = NB_SingleSearchInformationGetResultCount(m_information, &resultCount);
    NB_SuggestMatch* suggestMatch = NULL;
    std::vector<T*> resultsVector;

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

        T* convertedClass = reinterpret_cast<T*>(ConvertSuggestMatchStructToClass(suggestMatch));
        NB_SuggestMatchDestroy(suggestMatch);
        suggestMatch = NULL;
        resultsVector.push_back(convertedClass);
    }

    m_hasResults = TRUE;
    SingleSearchResultListImpl<T>* vec = new SingleSearchResultListImpl<T>(resultsVector);
    m_userCallback->Success(vec);
    delete vec;
}
template <class T>
void PageIteratorImpl<T>::SingleSearch_PlaceCallback()
{
    NB_Error err = NE_OK;
    uint32 resultCount = 0;
    err = NB_SingleSearchInformationGetResultCount(m_information, &resultCount);
    std::vector<T*> resultsVector;

    for (uint32 resultIndex = 0; resultIndex < resultCount; resultIndex++)
    {
        NB_SearchResultType resultType;
        NB_SingleSearchInformationGetResultType(m_information, resultIndex, &resultType);
        if (resultType == NB_SRT_Place)
        {
                NB_Place place;
                nsl_memset(&place, 0, sizeof(place));
                NB_BoundingBox boundingBox;
                double distance = 0.0;
                NB_ExtendedPlace* extPlace = NULL;
                nsl_memset(&boundingBox, 0, sizeof(boundingBox));
                err = NB_SingleSearchInformationGetPlace(m_information,
                                                         resultIndex,
                                                         &place,
                                                         &distance, &extPlace);
                if (NE_OK != err)
                {
                    m_userCallback->Error(err);
                    return;
                }

                NB_SingleSearchInformationFreeExtendedPlaces(m_information);

                err =  GetBoundingbox(m_information, &boundingBox);

                T* convertedClass = reinterpret_cast<T*>(ConvertPlaceStructToClass(&place, &boundingBox));
                resultsVector.push_back(convertedClass);
        }
    }



    m_hasResults = TRUE;
    SingleSearchResultListImpl<T>* vec = new SingleSearchResultListImpl<T>(resultsVector);
    m_userCallback->Success(vec);
    delete vec;
}

template <class T>
void PageIteratorImpl<T>::SingleSearch_FuelCallback()
{
    NB_Error err = NE_OK;
    uint32 resultCount = 0;
    err = NB_SingleSearchInformationGetResultCount(m_information, &resultCount);
    std::vector<T*> resultsVector;

    for (uint32 resultIndex = 0; resultIndex < resultCount; resultIndex++)
    {
        NB_SearchResultType resultType;
        NB_SingleSearchInformationGetResultType(m_information, resultIndex, &resultType);
        if (resultType & NB_SRT_FuelDetails)
        {
            NB_FuelDetails fuelDetails;
            nsl_memset(&fuelDetails, 0, sizeof(fuelDetails));

            err = NB_SingleSearchInformationGetFuelDetails(m_information,
                                                        resultIndex,
                                                        &fuelDetails);
            if(NE_OK != err)
            {
                m_userCallback->Error(err);
                return;
            }


            T* convertedClass = reinterpret_cast<T*>(ConvertFuelDetailsStructToClass(&fuelDetails));
            resultsVector.push_back(convertedClass);
        }
        else if (resultType & NB_SRT_FuelSummary)
        {
            NB_FuelSummary fuelSummary;
            nsl_memset(&fuelSummary, 0, sizeof(fuelSummary));
            err = NB_SingleSearchInformationGetFuelSummary(m_information,
                                                        &fuelSummary);
            if(NE_OK != err)
            {
                m_userCallback->Error(err);
                return;
            }

            T* convertedClass = reinterpret_cast<T*>(ConvertFuelSummaryStructToClass(&fuelSummary));
            resultsVector.push_back(convertedClass);
        }
    }

    m_hasResults = TRUE;
    SingleSearchResultListImpl<T>* vec = new SingleSearchResultListImpl<T>(resultsVector);
    m_userCallback->Success(vec);
    delete vec;
}

////////////////////////////////////////////////////////////////////
// PageIterator implementation
template <class T>
PageIteratorImpl<T>::PageIteratorImpl(NB_Context* context,
                                      NB_SingleSearchParameters* parameters)
{
    m_information = NULL;
    m_handler = NULL;
    m_userCallback = NULL;

    m_context = context;
    m_parameters = parameters;
    m_hasResults = false;
    m_resultType = SSR_None;
}
template <class T>
PageIteratorImpl<T>::PageIteratorImpl(NB_Context* context,
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
}

template <class T>
PageIteratorImpl<T>::~PageIteratorImpl()
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
}

template <class T>
void PageIteratorImpl<T>::getNext(AsyncCallback<SingleSearchResultList<T>* >* callback)
{
    NB_Error err = NE_OK;

    if (NULL == callback)
    {
        return;
    }

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

    err = NB_SingleSearchHandlerStartRequest(m_handler, m_parameters);

    if (NE_OK != err)
    {
        callback->Error(err);
        return;
    }
}

////////////////////////////////////////////////////////////
// SingleSearchResultListImpl implementation
template <class T>
SingleSearchResultListImpl<T>::SingleSearchResultListImpl(std::vector<T*> list)
{
    m_position = 0;
    m_resultList = list;
}

template <class T>
SingleSearchResultListImpl<T>::~SingleSearchResultListImpl()
{
    for(size_t i = 0; i < m_resultList.size(); i++)
    {
        T* obj = m_resultList.at(i);
        delete obj;
    }
}

template <class T>
T* SingleSearchResultListImpl<T>::getNext()
{
    if (m_position == m_resultList.size())
    {
        return NULL;
    }

    return m_resultList.at(m_position++);
}

template <class T>
T* SingleSearchResultListImpl<T>::getPrev()
{
    if (m_position == 0)
    {
        return NULL;
    }

    return m_resultList.at(--m_position);
}

/////////////////////////////////////////////////////////////////////
// Convertres implementation
FuelCommon* ConvertFuelDetailsStructToClass(NB_FuelDetails* fuelDetails)
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
FuelCommon* ConvertFuelSummaryStructToClass(NB_FuelSummary* fuelSummary)
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
Place* ConvertPlaceStructToClass(NB_Place* place, NB_BoundingBox* boundingBox)
{
    PlaceImpl* placeClass = new PlaceImpl();

    CategoryImpl category;
    category.SetCode(place->category->code);
    category.SetName(place->category->name);

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

    PhoneImpl phone;
    phone.SetArea(place->phone->area);
    phone.SetCountry(place->phone->country);
    phone.SetNumber(place->phone->number);
    phone.SetPhoneType((Phone::PhoneType)place->phone->type);

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
    placeClass->SetCategory(&category);
    placeClass->SetName(place->name);
    placeClass->SetNumCategory(place->numcategory);
    placeClass->SetNumPhone(place->numphone);
    placeClass->SetPhone(&phone);
    placeClass->SetBoundingBox(&boundingBoxImpl);




    return static_cast<Place*>(placeClass);
}

SuggestionMatch* ConvertSuggestMatchStructToClass(NB_SuggestMatch* suggestMatch)
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

NB_SearchSource ConvertSingleSearchSourceToSearchSource(SingleSearchSource source)
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

NB_Error GetBoundingbox(NB_SingleSearchInformation* information, NB_BoundingBox* boundingBox)
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


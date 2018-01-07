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

    @file     SingleSearchPrivate2.h
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
#ifndef _SINGLE_SEARCH_PRIVATE2_H_
#define _SINGLE_SEARCH_PRIVATE2_H_


#include "SingleSearch2.h"
#include "AsyncCallback.h"
#include <vector>
using namespace std;
typedef enum
{
    SSR_None = 0,
    SSR_SuggestionMatch,
    SSR_Place,
    SSR_Fuel,
} SingleSearchResultType;

template <class T>
class SingleSearchResultListImpl2 : public SingleSearchResultList2<T>
{
public:
    SingleSearchResultListImpl2(vector<T*> list);
    ~SingleSearchResultListImpl2();

    T* getNext();
    T* getPrev();

private:
    uint32         m_position;
    vector<T*> m_resultList; /*! List of received results */
};

template <class T>
class PageIteratorImpl2 : public PageIterator2<T>
{
public:
    PageIteratorImpl2(NB_Context* context,
                     NB_SingleSearchParameters* parameters);
    PageIteratorImpl2(NB_Context* context,
                     NB_SingleSearchParameters* parameters,
                     SingleSearchResultType resultType);
    ~PageIteratorImpl2();

    void getData(int offset, int count, AsyncCallback<SingleSearchResultList2<T>* >* callback);

private:
    NB_Context*                     m_context;
    NB_SingleSearchParameters*      m_parameters;     /*!< Single search parameters instance */
    NB_SingleSearchInformation*     m_information;    /*!< Single search information instance */
    NB_SingleSearchHandler*         m_handler;        /*!< Single search handler instance */
    bool                            m_hasResults;     /*!< 'True' means that we already have some received results */
    SingleSearchResultType          m_resultType;     /*!< Type of  information which we should retrieve */
    vector<T*>                      m_resultList;     /*!< Cached results*/
    int                             m_requestOffset;
    AsyncCallback<SingleSearchResultList2<T>* >* m_userCallback;

    static void SingleSearch_StaticCallback(
                     void* handler,
                     NB_RequestStatus status,
                     NB_Error err,
                     nb_boolean up,
                     int percent,
                     void* userData);

    void SingleSearch_Callback(
                     NB_RequestStatus status,
                     NB_Error err,
                     int percent);
    void SingleSearch_SuggestionMatchCallback();
    void SingleSearch_PlaceCallback();
    void SingleSearch_FuelCallback();
};

class SingleSearchImpl2 : public SingleSearch2
{
public:
    SingleSearchImpl2(NB_Context* context,
                     char* language,
                     char* searchScheme,
                     NB_POIExtendedConfiguration extendedConfiguration,
                     NB_SearchSortBy sortBy,
                     NB_SearchInputMethod inputMethod);

    ~SingleSearchImpl2();

    NB_Context* m_context;
    char* m_language;                                       /*!< Optional. Specifies the language for the request. If not specified US English is default */
    char* m_searchScheme;                                   /*!< The search scheme used for the request. Can be left blank */
    NB_POIExtendedConfiguration m_extendedConfiguration;    /*!< Specifies the extended configuration for the request */
    NB_SearchSortBy m_sortBy;                               /*!< The desired result sort order */
    NB_SearchInputMethod m_inputMethod;                     /*!< The search input method */
};

#endif

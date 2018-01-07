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

    @file     SingleSearch.h
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

#ifndef NBSINGLESEARCH_H
#define NBSINGLESEARCH_H

#include "AsyncCallback.h"
#include "CommonTypes.h"

extern "C"
{
#include "nbsinglesearchinformation.h"
#include "nbsinglesearchparameters.h"
#include "nbsinglesearchhandler.h"
}

using namespace nbcommon;
using namespace nbmap;

/*! SingleSearchResultList<T> used to get items from received SingleSearch information */
template <class T>
class SingleSearchResultList
{
public:
// Public functions
    /*! SingleSearchResultList destructor */
    virtual ~SingleSearchResultList(){};

    /*! Get next item from result list

        @return single result item
    */
    virtual T* getNext() = 0;

    /*! Get previous item from result list

        @return single result item
    */
    virtual T* getPrev() = 0;
};

/*! PageIterator<T> used to start and get results SingleSearch requests */
template <class T>
class PageIterator
{
public:
// Public functions
    /*! PageIterator destructor */
    virtual ~PageIterator(){};

    /*! Create NB_SingleSearchHandler and start new request */
    virtual void getNext(AsyncCallback<SingleSearchResultList<T>* >* callback) = 0;
};

class SingleSearchImpl;
/*! SingleSearch used to manage SingleSearch requests */
class SingleSearch
{
public:
//Public functions
    /*! SingleSearch constructor */
    SingleSearch(NB_Context* context,
                 char* language,
                 char* searchScheme,
                 NB_POIExtendedConfiguration extendedConfiguration,
                 NB_SearchSortBy sortBy,
                 NB_SearchInputMethod inputMethod);

    /*! SingleSearch destructor */
    ~SingleSearch();

    /*! Create SingleSearch parameters with NB_SRST_Suggestion result style type

        @return PageIterator<SuggestionMatch> item which contain parameters for request
    */
    PageIterator<SuggestionMatch>* suggestions(
                 char* searchQuery,
                 int count,
                 LatitudeLongitude* latLon,
                 SingleSearchSource singleSearchSource,
                 LatitudeLongitude* orgin);


    /*! Create SingleSearch parameters for SSB

        @return PageIterator<SuggestionMatch> item which contain parameters for request
    */
	PageIterator<Place>* place(
				 char* searchQuery,
				 int count,
				 LatitudeLongitude* latLon,
				 SingleSearchSource singleSearchSource,
				 const char* category,
				 LatitudeLongitude* orgin);

    /*! Create SingleSearch parameters for SSB

            @return PageIterator<SuggestionMatch> item which contain parameters for request
        */
	PageIterator<Place>* placeBySuggestion(
				 char* searchQuery,
				 int count,
				 LatitudeLongitude* latLon,
				 const char* suggestionCookie,
				 SingleSearchSource singleSearchSource,
				 const char* category,
				 LatitudeLongitude* orgin);

    /*! Create SingleSearch parameters for navigation with NB_SRST_SingleSearch result style type.
        Information from request with such parameters will contain inforamtion about places.

        @return PageIterator<Place> item which contain parameters for request
    */
    PageIterator<Place>* place(
                 char* searchQuery,
                 int count,
                 LatitudeLongitude* latLon,
                 NB_RouteInformation* route);

    /*! Create SingleSearch parameters for navigation with NB_SRST_SingleSearch result style type
        Information from request with such parameters will contain inforamtion about fuel.

        @return PageIterator<Place> item which contain parameters for request
    */
    PageIterator<FuelCommon>* fuel(
        char* searchQuery,
        int count,
        LatitudeLongitude* latLon);

protected:
    SingleSearch();
private:
    SingleSearchImpl*  m_impl;
};

#endif

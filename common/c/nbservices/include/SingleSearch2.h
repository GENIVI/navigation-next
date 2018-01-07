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

    @file     SingleSearch2.h
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

#ifndef NBSINGLESEARCH2_H
#define NBSINGLESEARCH2_H

#include "AsyncCallback.h"
#include "SSCommonTypes.h"

extern "C"
{
#include "nbsinglesearchinformation.h"
#include "nbsinglesearchparameters.h"
#include "nbsinglesearchhandler.h"
}

using namespace nbcommon2;
using namespace nbmap;

/*! SingleSearchResultList<T> used to get items from received SingleSearch information */
template <class T>
class SingleSearchResultList2
{
public:
// Public functions
    /*! SingleSearchResultList destructor */
    virtual ~SingleSearchResultList2(){};

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
class PageIterator2
{
public:
// Public functions
    /*! PageIterator destructor */
    virtual ~PageIterator2(){};

    /*! Get data by offset and count, will start new request if needed*/
    virtual void getData(int offset, int count, AsyncCallback<SingleSearchResultList2<T>* >* callback) = 0;

};

class SingleSearchImpl2;
/*! SingleSearch used to manage SingleSearch requests */
class SingleSearch2
{
public:
    class Criteria
    {
    public:
        virtual ~Criteria(){};
        virtual const std::string& searchQuery() const = 0;
        virtual const bool isSearchQueryValid() const = 0;
        virtual const LatitudeLongitude searchPoint() const = 0;
        virtual const bool isSearchPointValid() const = 0;
        virtual const LatitudeLongitude currentLocation() const = 0;
        virtual const bool isCurrentLocationValid() const = 0;
        virtual const std::vector<std::string>& categories() const = 0;
        virtual const std::string& suggestionCookie() const = 0;
        virtual const bool isSuggestionCookieValid() const = 0;
        virtual const std::string& language() const = 0;
        virtual const bool isLanguageValid() const = 0;
        virtual const NB_SearchSortBy sortBy() const = 0;
        virtual const SingleSearchSource singleSearchSource() const = 0;
    };
//Public functions
    /*! SingleSearch constructor */
    SingleSearch2(NB_Context* context,
                 char* language,
                 char* searchScheme,
                 NB_POIExtendedConfiguration extendedConfiguration,
                 NB_SearchSortBy sortBy,
                 NB_SearchInputMethod inputMethod);

    /*! SingleSearch destructor */
    ~SingleSearch2();

    /*! Create SingleSearch parameters with NB_SRST_Suggestion result style type

        @return PageIterator<SuggestionMatch> item which contain parameters for request
    */
    PageIterator2<SuggestionMatch>* suggestions(const Criteria* criteria, uint sliceSize);


    /*! Create SingleSearch parameters for SSB

        @return PageIterator<SuggestionMatch> item which contain parameters for request
    */
	PageIterator2<Place>* place(const Criteria* criteria, uint sliceSize);


    /*! Create SingleSearch parameters for SSB

            @return PageIterator<SSSuggestionMatch> item which contain parameters for request
        */
	PageIterator2<Place>* placeBySuggestion(const Criteria* criteria, uint sliceSize);

    /*! Create SingleSearch parameters for navigation with NB_SRST_SingleSearch result style type.
        Information from request with such parameters will contain inforamtion about places.

        @return PageIterator<Place> item which contain parameters for request
    */
    PageIterator2<Place>* place(const Criteria* criteria, uint sliceSize, NB_RouteInformation* route);

    /*! Create SingleSearch parameters for navigation with NB_SRST_SingleSearch result style type
        Information from request with such parameters will contain inforamtion about fuel.

        @return PageIterator<Place> item which contain parameters for request
    */
    PageIterator2<FuelCommon>* fuel(const Criteria* criteria, uint sliceSize);

    /*
     * (char *searchQuery,
                          int count,
                          LatitudeLongitude* latLon,
                          SingleSearchSource singleSearchSource,
                          LatitudeLongitude* orgin,
     Char * language,
     NB_SearchSortBy m_sortBy)
     *
     */
protected:
    SingleSearch2();
private:
    SingleSearchImpl2*  m_impl;
};

#endif

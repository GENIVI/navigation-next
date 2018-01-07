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
    @file     SingleSearchRequest.h
    @defgroup nbsearch
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __SINGLESEARCHREQUEST_H__
#define __SINGLESEARCHREQUEST_H__

#include "nbcontext.h"
#include "SingleSearchConfiguration.h"
#include "StaticPoi.h"

namespace nbsearch
{

class SingleSearchParameters;
class SingleSearchRequestPrivate;
class SingleSearchListener;
class SingleSearchConfiguration;
class SingleSearchInformation;
class LatLonPoint;
class SearchFilter;

/*! The movie id or theater id.
*/
typedef enum
{
    SSRIT_None,
    SSRIT_Movie,   /*!< This is used for movie-id. */
    SSRIT_Theater  /*!< This is used for theater-id. */
} SingleSearchResultIdType;

/*! The type for movie showing.
 */
typedef enum
{
    MST_None,
    MST_NowInTheater,      /*!< This is used for movie list explore now in theater. */
    MST_OpeningThisWeek,   /*!< This is used for movie list explore opening this week. */
    MST_ComingSoon         /*!< This is used for movie list explore coming soon. */
} MovieShowingType;

/*!
 * Performs single search.
 *
 */
class SingleSearchRequest
{
public:

    /*!
     * Creates single search request object.
     * Use this constructor to initialize search request that does not use suggestion filters.
     *
     * @param context
     * @param keyword String keyword, used for matching.
     * @param categoryCode Category code used to filter the search results.
     * @param LatLonPoint
     *            The position around which to search.
     *            Can't be null. Center of region can't be null.
     * @param LatLonPoint
     *            The position of map center.
     *            Can be null.
     * @param listener Callback for results.
     * @param configuration configuration of the search request.
     */
    SingleSearchRequest(NB_Context* context,
                        const char* keyWord,
                        const char* categoryCode,
                        const LatLonPoint* currentPoint,
                        const LatLonPoint* searchPoint,
                        const std::shared_ptr<SingleSearchListener>& listener,
                        const SingleSearchConfiguration& configuration);

    /*!
     * Creates single search request object.
     * Use this constructor to initialize search request that use search filters.
     *
     * @param context
     * @param searchFilter
     * @param LatLonPoint
     *            The position around which to search.
     *            Can't be null. Center of region can't be null.
     * @param LatLonPoint
     *            The position of map center.
     *            Can be null.
     * @param listener Callback for results.
     * @param configuration configuration of the search request.
     */
    SingleSearchRequest(NB_Context* context,
                        const SearchFilter* searchFilter,
                        const LatLonPoint* currentPoint,
                        const LatLonPoint* searchPoint,
                        const std::shared_ptr<SingleSearchListener>& listener,
                        const SingleSearchConfiguration& configuration);

    /*!
     * Constructor for getting next or previous result set.
     *
     * @param context
     * @param request returned from previous POI request. SingleSearchRequest will not take ownership of the request parameter.
     * @param information returned from previous POI request.
     * @param listener Callback for results.
     * @param direction prev or next.
     */
    SingleSearchRequest(NB_Context* context,
                        SingleSearchRequest* request,
                        const SingleSearchInformation* information,
                        const std::shared_ptr<SingleSearchListener>& listener,
                        SearchDirection direction);

    /*!
     * Constructor only for movie search.
     * @param context
     * @param showType "openging this week", "now playing", "Coming soon"
     * @param LatLonPoint
     *            The position around which to search.
     *            Can't be null. Center of region can't be null.
     * @param LatLonPoint
     *            The position of map center.
     *            Can be null.
     * @param listener Callback for results.
     * @param configuration configuration of the search request.
     */
    SingleSearchRequest(NB_Context* context,
                        MovieShowingType showType,
                        const LatLonPoint* currentPoint,
                        const LatLonPoint* searchPoint,
                        const std::shared_ptr<SingleSearchListener>& listener,
                        const SingleSearchConfiguration& configuration);

    /*!
     * Constructor only for movie/theater detail changed time.
     * @param context
     * @param idType "movie-id", "theater-id"
     * @param id which should be pointing to the movie/theater id string, like "CSC_567894223"
     * @param start time: utc time which unit is second.
     * @param LatLonPoint
     *            The position around which to search.
     *            Can't be null. Center of region can't be null.
     * @param LatLonPoint
     *            The position of map center.
     *            Can be null.
     * @param listener Callback for results.
     * @param configuration configuration of the search request.
     */
    SingleSearchRequest(NB_Context* context,
                        SingleSearchResultIdType idType,
                        const char* id,
                        uint32 startTime,
                        const LatLonPoint* currentPoint,
                        const LatLonPoint* searchPoint,
                        const std::shared_ptr<SingleSearchListener>& listener,
                        const SingleSearchConfiguration& configuration);

    /*!
     * Constructor for static poi search.
     * @param context
     * @param static poi instance
     * @param LatLonPoint
     *            The position around which to search.
     *            Can't be null. Center of region can't be null.
     * @param LatLonPoint
     *            The position of map center.
     *            Can be null.
     * @param listener Callback for results.
     * @param configuration configuration of the search request.
     */
    SingleSearchRequest(NB_Context* context,
                        const nbcommon::StaticPoi* staticPoi,
                        const LatLonPoint* currentPoint,
                        const LatLonPoint* searchPoint,
                        const std::shared_ptr<SingleSearchListener>& listener,
                        const SingleSearchConfiguration& configuration);

    /*!
     * Consutructor for weather pin layer search.
     * @param context
     * @param box
     *        contains bounding rectangle of the tiled area to be populated with weather conditions.
     * @param listener Callback for results.
     * @param configuration configuration of the search request.
     */
    SingleSearchRequest(NB_Context* context,
                        const BoundingBox& box,
                        const std::shared_ptr<SingleSearchListener>& listener,
                        const SingleSearchConfiguration& configuration);


    /*!
     * @brief Constructor for weather detail card search.
     *
     * @param context
     * @param box
     *        see weather pin layer search.
     * @param searchFilter
     *        search filter from weather pin layer reply.
     * @param listener
     * @param configuration
     */
    SingleSearchRequest(NB_Context* context,
                        const SearchFilter* searchFilter,
                        const std::shared_ptr<SingleSearchListener>& listener,
                        const SingleSearchConfiguration& configuration);


    /**
     * @brief Constructor for address search to support app2app.
     *
     * @param context
     * @param location
     * @param listener
     @param configuration
     */
    SingleSearchRequest(NB_Context* context,
                        const nbsearch::Location* location,
                        const std::shared_ptr<SingleSearchListener>& listener,
                        const SingleSearchConfiguration& configuration);

    /*!
     * @brief Destructor.
     */
    ~SingleSearchRequest();

    /*!
     * @brief Before StartRequest, user must set invocation context.
     *
     * @param inputSource  Specifies the source of the input content.
     * @param invocationMethod Specifies how the user initiated the request.
     * @param screenId  Specifies the user's screen.
     */
    void SetInvocationContext(const char* inputSource, const char* invocationMethod, const char* screenId);

    /*!
     * @brief Start the request.
     */
    void StartRequest();

    /*!
     * @brief Cancel the request.
     */
    void CancelRequest();

    /*!
     * @brief Checks whether request is in progress.
     *
     * @return true this request is in progress.
     */
    bool IsRequestInProgress();

private:
    SingleSearchRequestPrivate* m_privateRequest;
    std::shared_ptr<SingleSearchParameters> m_param;
};

}
#endif //__SINGLESEARCHREQUEST_H__

/*! @} */

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

    @file     nbsinglesearchinformation.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBSINGLESEARCHINFORMATION_H
#define NBSINGLESEARCHINFORMATION_H

/*!
    @addtogroup nbsinglesearchinformation
    @{
*/

#include "nbexp.h"
#include "nbplace.h"
#include "nbsearchtypes.h"
#include "nbsuggestmatch.h"
#include "nbextappcontent.h"

// Constants ....................................................................................


// Types ........................................................................................

typedef struct NB_SingleSearchInformation NB_SingleSearchInformation;


// Functions ....................................................................................

// @todo: Move the function 'NB_SearchInformationGetFormattedContentText' to a common place. User could also call
//        this function to get formatted content text in single search. Because this function is not related with
//        the NB_SearchInformation object.


/*! Destroy an instance of a NB_SingleSearchInformation object

    @return None
*/
NB_DEC void
NB_SingleSearchInformationDestroy(
    NB_SingleSearchInformation* information                             /*!< Single search information instance */
    );

/*! This function gets the requested search result style of the current single search operation

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetResultStyleType(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    NB_SearchResultStyleType* resultStyleType                           /*!< The result style type of single search request */
    );

/*! This function gets the count of results of the current single search operation

    If requested search result style is 'NB_SRST_SingleSearch' or 'NB_SRST_Geocode' or 'NB_SRST_Airport' returns the count of the POIs (place or fuel details).
    If requested search result style is 'NB_SRST_Suggestion' returns the count of the suggest matchs.

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetResultCount(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    uint32* count                                                       /*!< On return the count of results that are present in the current slice (maybe less than requested count) */
    );

/*! This function returns whether or not more results are available for the current single search operation

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchInformationHasMoreResults(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    nb_boolean* hasMoreResults                                          /*!< Has more results? */
    );

/*! This function gets the result type of the specified result by index

    Call this function if requested search result style is 'NB_SRST_SingleSearch' or 'NB_SRST_Geocode'.
    Single search could return multiple results with different types. Call this function first and then
    call specified functions to get the result.

    @return NE_OK if success
    @see NB_SingleSearchInformationGetPlace
    @see NB_SingleSearchInformationGetFuelDetails
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetResultType(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    uint32 resultIndex,                                                 /*!< The index to get the result type */
    NB_SearchResultType* resultType                                     /*!< On return the result type */
    );

/*! This function gets the suggest match of the specified result by index

    Call this function if requested search result style is 'NB_SRST_Suggestion'. The returned 'NB_SuggestMatch' object has to
    freed by calling the function 'NB_SuggestMatchDestroy' when it is useless.

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetSuggestMatch(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    uint32 resultIndex,                                                 /*!< The index to get the suggest match */
    NB_SuggestMatch** suggestMatch                                      /*!< On return the 'NB_SuggestMatch' object. This object has to be freed by calling the function
                                                                             'NB_SuggestMatchDestroy' when it is useless. */
    );

/*! This function gets the place details of the specified result by index

    Call this function if NB_SingleSearchInformationGetResultType() returns the result type 'NB_SRT_Place'.

    The extended place structure (if requested) is allocated by this function and NB_SingleSearchInformationFreeExtendedPlaces()
    has to be called to free the data.

    @return NE_OK if success
    @see NB_SingleSearchInformationGetResultType
    @see NB_SingleSearchInformationFreeExtendedPlaces
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetPlace(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    uint32 resultIndex,                                                 /*!< The index to get the place details */
    NB_Place* place,                                                    /*!< On return the place information. Required. */
    double* distance,                                                   /*!< On return optional distance from the center of the search to the location of this POI.
                                                                             Pass in NULL if not needed. */
    NB_ExtendedPlace** extendedPlace                                    /*!< On return optional extended place. Call NB_SingleSearchInformationFreeExtendedPlaces() to free.
                                                                             Pass in NULL if not needed. */
    );

/*! This function frees the internal extended places

    This function frees the internal extended places allocated by the function NB_SingleSearchInformationGetPlace().
    The internal extended places could be freed automatically when the NB_SingleSearchInformation instance is destroyed.

    @return NE_OK if success
    @see NB_SingleSearchInformationGetPlace
*/
NB_DEC NB_Error
NB_SingleSearchInformationFreeExtendedPlaces(
    NB_SingleSearchInformation* information                             /*!< Single search information instance */
    );

/*! This function gets the fuel details of the specified result by index

    Call this function if NB_SingleSearchInformationGetResultType() returns the result type 'NB_SRT_FuelDetails'.

    @return NE_OK if success
    @see NB_SingleSearchInformationGetResultType
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetFuelDetails(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    uint32 resultIndex,                                                 /*!< The index to get the fuel details */
    NB_FuelDetails* fuelDetails                                         /*!< On return the fuel details */
    );

/*! This function gets the fuel summary of the current single search operation

    The fuel summary is always based on one downloaded slice.

    @return NE_OK if success, NE_NOENT if there is no fuel summary returned from server
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetFuelSummary(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    NB_FuelSummary* fuelSummary                                         /*!< On return the fuel summary */
    );

/*! This function gets the analytics event id of the current single search operation

    @return NE_OK if success, NE_NOENT if there is no analytics event id
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetAnalyticsEventId(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    uint32* analyticsEventId                                            /*!< On return the analytics event id */
    );

/*! Check if a single search result specified by index is mappable

    Call this function if requested search result style is 'NB_SRST_SingleSearch' or 'NB_SRST_Geocode'.

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchInformationIsMappable(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    uint32 resultIndex,                                                 /*!< The index to check if a single search result is mappable */
    nb_boolean* isMappable                                              /*!< Is the single search result mappable? */
    );

/*! Get the requested keyword

    This function returns the requested keyword. User could identify if this keyword is same with the current input.

    @todo: I think just comparing the requested keyword with the current input could not confirm if they are the
           same requests. Because there are other informations in 'NB_SingleSearchParameters' when requesting. (Such
           as 'NB_SearchRegion'.) User should add other logics to identify the request.

    @return NE_OK if success, NE_NOENT if there is no keyword, NE_RANGE if 'maxKeywordSize' is not enough.
 */
NB_DEC NB_Error
NB_SingleSearchInformationGetRequestedKeyword(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    uint32 maxKeywordSize,                                              /*!< Maximum size of requested keyword. This size contains the end '\0'. */
    char* keyword                                                       /*!< On success, returns requested keyword */
    );

/*! Get URL of POI image.

    This function returns URL of POI image based on inputed index and flag.

    @note   URL is not copied for efficiency, so caller should never free the returned string.
    @return URL of POI image if found, or NULL otherwise.
*/
NB_DEC const char*
NB_SingleSearchInformationGetPoiImageUrl(NB_SingleSearchInformation* information, /*!< Single search information instance  */
                                         uint32 index,                            /*!< Index of POI content */
                                         nb_boolean wantThumbnail                 /*!< Flag to indicate the type of interested image.*/
                                         );

/*! Get palce ext app content.

    This function gets the place ext app content of the specified result by index

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetPlaceExtAppContent(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    uint32 resultIndex,                                                 /*!< The index to get the place details */
    NB_ExtAppContent* extAppContent                                     /*!< The address of a NB_ExtAppContent object to be filled in with the result */
    );

/*! Get data availability information.

    This function returns NB_DataAvailability from the search reply. The function doesn't allocate memory.
    Client should pass pointer to valid NB_DataAvailability object and it is responsible for object releasing.

    @return Error code.
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetDataAvailability(NB_SingleSearchInformation* information, /*!< Single search information instance  */
                                         uint32 resultIndex,                           /*!< The index to get the place details */
                                         NB_DataAvailability* dataAvailability         /*!< On return the NB_DataAvailability information. Required. */
                                         );

/*! Get data box information.

    This function returns box from the search reply. The function doesn't allocate memory.
    Client should pass pointer to valid lat and lon pointer.

    @return Error code.
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetDataBox(NB_SingleSearchInformation* information,  /*!< Single search information instance  */
                                              uint32 resultIndex,              /*!< The index to get the place details */
                                              double* topLeftLatitude,
                                              double* topLeftLongitude,
                                              double* bottomRightLatitude,
                                              double* bottomRightLongitude);

/*! Retrieve the parent category of a category in search result

    @return \a NE_OK if successful <br>
            \a NE_NOENT if the parent category could not be found
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetParentCategory(
    NB_SingleSearchInformation* information,    /*!< Single search information instance */
    const char* categoryCode,                   /*!< Category code to get the parent category of */
    NB_Category* parentCategory                 /*!< Points to a category structure to populate with parent category info */
    );

/*! Retrieve the structured hours of operation

    This function retrieves the structured hours of operation from the indexed search
    result, it must be destroyed by calling the function NB_HoursOfOperationDestroy
    when it is useless.

    @return \a NE_OK if successful <br>
            \a NE_NOENT if hours of operation does not exist in the indexed search result
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetHoursOfOperation(
    NB_SingleSearchInformation* information,    /*!< Single search information instance */
    uint32 resultIndex,                         /*!< The index to get the hours of operation */
    NB_HoursOfOperation* hoursOfOperation       /*!< Points to a hours of operation structure to populate with the
                                                     information if successful, it must be destroyed by calling the
                                                     function NB_HoursOfOperationDestroy if it is useless.
                                                */
    );

/*! Destroy the structured hours of operation

    @return None
*/
NB_DEC void
NB_HoursOfOperationDestroy(
    NB_HoursOfOperation* hoursOfOperation       /*!< A hours of operation structure to destroy */
    );

/*! This function retrieves the traffic incident of the specified result by index

    @return NE_OK If successful
*/
NB_DEC NB_Error
NB_SingleSearchInformationGetTrafficIncident(
                        NB_SingleSearchInformation* information,            /*!< A pointer to a NB_SingleSearchInformation instance */
                        int resultIndex,                                    /*!< The index of the result for which details will be returned */
                        NB_TrafficIncident* incident,                       /*!< On return traffic incident information. Required. */
                        double* distance                                    /*!< On return optional distance from the center of the search to the location of this POI.
                                                                                 Pass in NULL if not needed. */
                        );

/*! @} */

#endif

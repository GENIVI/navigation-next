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

    @file     nbsinglesearchparameters.h
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

#ifndef NBSINGLESEARCHPARAMETERS_H
#define NBSINGLESEARCHPARAMETERS_H

/*!
    @addtogroup nbsinglesearchparameters
    @{
*/

#include "nbimage.h"
#include "nblocation.h"
#include "nbiterationcommand.h"
#include "nbsearchtypes.h"
#include "nbsinglesearchinformation.h"

// Constants ....................................................................................


// Types ........................................................................................

typedef struct NB_SingleSearchParameters NB_SingleSearchParameters;


// Functions ....................................................................................

/*! Create an instance of a NB_SingleSearchParameters object by result style type

    This function is used for suggestion search or single search directly. The argument of result style type
    could be set to single search, suggestion search or geocode search.

    @return NE_OK for success, NE_NOMEM if memory allocation failed
    @see NB_SingleSearchParametersDestroy
    @see NB_SearchResultStyleType
*/
NB_DEC NB_Error
NB_SingleSearchParametersCreateByResultStyleType(
    NB_Context* context,                                                /*!< Pointer to current context */
    NB_SearchRegion* region,                                            /*!< Search region used for the request */
    const char* name,                                                   /*!< The name of the POI to search for. Can be left blank */
    const char* searchScheme,                                           /*!< The search scheme used for the request. Can be left blank */
    uint32 sliceSize,                                                   /*!< Specifies how many results should be returned per request */
    NB_POIExtendedConfiguration extendedConfiguration,                  /*!< Specifies the extended configuration for the request */
    const char* language,                                               /*!< Optional. Specifies the language for the request. If not specified US English is default */
    NB_SearchResultStyleType resultStyleType,                           /*!< Result style type used for the request */
    NB_SingleSearchParameters** parameters                              /*!< On success, returns pointer to single search parameters */
    );

/*! Create an instance of a NB_SingleSearchParameters object by suggestion

    This function is used for single search by the result of the last suggestion search.

    @return NE_OK for success, NE_NOMEM if memory allocation failed
    @see NB_SingleSearchParametersDestroy
*/
NB_DEC NB_Error
NB_SingleSearchParametersCreateBySuggestion(
    NB_Context* context,                                                /*!< Pointer to current context */
    NB_SearchRegion* region,                                            /*!< Search region used for the request */
    const char* name,                                                   /*!< The name of the POI to search for. Can be left blank
                                                                             @todo: Should the name be set by string content of line 1 of specified suggest match? */
    const char* searchScheme,                                           /*!< The search scheme used for the request. Can be left blank */
    uint32 sliceSize,                                                   /*!< Specifies how many results should be returned per request */
    NB_POIExtendedConfiguration extendedConfiguration,                  /*!< Specifies the extended configuration for the request */
    const char* language,                                               /*!< Optional. Specifies the language for the request. If not specified US English is default */
    NB_SuggestMatch* suggestMatch,                                      /*!< A 'NB_SuggestMatch' object */
    NB_SingleSearchParameters** parameters                              /*!< On success, returns pointer to single search parameters */
    );

/*! Create an instance of a NB_SingleSearchParameters object by iteration

    This function is used to search the previous or next sliced result of single search.

    @return NE_OK for success, NE_NOMEM if memory allocation failed
    @see NB_SingleSearchParametersDestroy
*/
NB_DEC NB_Error
NB_SingleSearchParametersCreateByIteration(
    NB_Context* context,                                                /*!< Pointer to current context */
    NB_SingleSearchInformation* information,                            /*!< The last single search result */
    NB_IterationCommand iterationCommand,                               /*!< The iteration command to perform */
    NB_SingleSearchParameters** parameters                              /*!< On success, returns pointer to single search parameters */
    );

/*! Destroy an instance of a NB_SingleSearchParameters object

    @return None
*/
NB_DEC void
NB_SingleSearchParametersDestroy(
    NB_SingleSearchParameters* parameters                               /*!< Single search parameters instance */
    );

/*! Get the search source

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchParametersGetSource(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    NB_SearchSource* source                                             /*!< On return the search source */
    );

/*! Set the search source

    @todo: Single search only support 3 types of search source so far. They are 'NB_SS_Address', 'NB_SS_Carousel'
           and 'NB_SS_Place'. We could just use 'NB_SS_Carousel' for single search of map as Android.

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchParametersSetSource(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    NB_SearchSource source                                              /*!< The search source */
    );

/*! Set the desired result sort order

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchParametersSetSearchSortBy(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    NB_SearchSortBy sortBy                                              /*!< The desired result sort order */
    );

/*! Set the user location to calc distance

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchParametersSetOrigin(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    NB_Location* location                                              /*!< User location */
    );

/*! Get the search input method

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchParametersGetSearchInputMethod(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    NB_SearchInputMethod* inputMethod                                   /*!< On return the search input method */
    );

/*! Set the search input method

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchParametersSetSearchInputMethod(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    NB_SearchInputMethod inputMethod                                    /*!< The search input method */
    );

/*! Add an extended key/value pair

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchParametersAddExtendedKeyValue(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    NB_POIExtendedConfiguration extendedConfiguration,                  /*!< The extended configuration to add the key/value pair to */
    const char* key,                                                    /*!< The key name */
    const char* value                                                   /*!< The value */
    );

/*! Add an extended image specification

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchParametersAddExtendedImage(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    NB_POIExtendedConfiguration extendedConfiguration,                  /*!< The extended configuration to add the image to */
    NB_ImageFormat format,                                              /*!< The image format */
    NB_ImageSize* size,                                                 /*!< The image size */
    uint16 dpi                                                          /*!< The image DPI */
    );

/*! Add a search filter key/value pair

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchParametersAddSearchFilterKeyValue(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    const char* key,                                                    /*!< The key name */
    const char* value                                                   /*!< The value */
    );

/*! Set a flag if this search is used only to request POIs along the route to be displayed on navigation maps.

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchParametersSetSearchWithCorridor(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    nb_boolean withCorridor                                             /*!< Is this search used only to request POIs
                                                                             along the route to be displayed on
                                                                             navigation maps? */
    );

/*! Set a flag that country results shall include data-availability element in the proxmatch element.

    NOTE: In order to get 'data-availability' element the following requirements should be met:
    - search name should be NULL
    - search source should be NB_SS_DefaultLocation
    - result style type should be NB_SRST_DefaultLocation
    - search filter should have "countrycode" key with code of country

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchParametersSetWantDataAvailability(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    nb_boolean wantDataAvailability                                     /*!< Shall country results include data-availability
                                                                             element in the proxmatch element? */
    );

NB_DEC NB_Error
NB_SingleSearchParametersSetSliceSize(
        NB_SingleSearchParameters* parameters,                          /*!< Single search parameters instance */
        uint32 sliceSize                                                /*!< new slice size */
        );


/*! Add a want content type

 @return NE_OK if success
 */
NB_DEC NB_Error
NB_SingleSearchParametersAddWantContentType(NB_SingleSearchParameters* parameters,  /*!< Single search parameters instance */
                                            NB_SearchWantContentType contentType    /*!< Want search content type */
                                            );
/*! @} */

#endif

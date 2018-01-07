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

    @file     nbsearchparameters.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBSEARCHPARAMETERS_H
#define NBSEARCHPARAMETERS_H

/*!
    @addtogroup nbsearchparameters
    @{
*/

#include "nbexp.h"
#include "paltypes.h"
#include "nbcontext.h"
#include "nbsearchtypes.h"
#include "nbiterationcommand.h"
#include "nbsearchinformation.h"
#include "nbimage.h"

struct NB_SearchParameters;
typedef struct NB_SearchParameters NB_SearchParameters;

// Public Types ..................................................................................

typedef enum
{
	NB_ST_Default = 0,
	NB_ST_Fuel,
	NB_ST_Weather,
	NB_ST_TrafficIncident,
	NB_ST_Event,
	NB_ST_EventVenue,
	NB_ST_Movie,
	NB_ST_MovieTheater

} NB_SearchType;

/* Fuel Search Criteria */

typedef enum
{
    NB_FST_Regular = 0,
    NB_FST_Diesel,
    NB_FST_Ethanol85,
    NB_FST_All

} NB_FuelSearchType;

typedef enum
{
	NB_FRT_All = 0,
	NB_FRT_CarouselLite

} NB_FuelResultsType;

/* Weather Search Criteria */

typedef enum
{
	NB_WRT_All = 0,
	NB_WRT_Current,
	NB_WRT_Forecast,
	NB_WRT_CarouselLite

} NB_WeatherResultsType;

/* Traffic Incident Search Criteria */

typedef enum
{
	NB_TIRT_All = 0

} NB_TrafficIncidentResultsType;

typedef enum
{
    NB_ER_All           = 0x0000,
    NB_ER_KidFriendly   = 0x0001

} NB_EventRating;

typedef enum
{
	NB_ERT_All = 0,
	NB_ERT_CarouselLite

} NB_EventResultsType;

typedef enum
{
	NB_EVRT_All = 0

} NB_EventVenueResultsType;

/* Movie Search Criteria */

typedef enum
{
    NB_MSB_MostPopular,
    NB_MSB_MostRecent,
    NB_MSB_Alphabetical

} NB_MovieSortBy;

typedef enum
{
    NB_MS_NowInTheaters,
    NB_MS_TopBoxOffice,
    NB_MS_OpeningThisWeek,
    NB_MS_ComingSoon,
    NB_MS_SpecificTime

} NB_MovieShowing;

typedef enum
{
	NB_MRT_All = 0,
	NB_MRT_CarouselLite

} NB_MovieResultsType;

typedef enum
{
	NB_MTRT_All = 0

} NB_MovieTheaterResultsType;


// Public Functions ..............................................................................

/*! Create search parameters for a search

    Call this function to create a search parameters object for a search.
    It specifies what you want to search for.

    @todo (BUG 56058): we need to provide a list of possible categories. Currently they can be found in:
    "//depot/winmobile/wm4main/main/vnavres/main/products/atlasbook/filesets/poi_categories/catconfig-premium-en-us.sexp"

    After calling this function use a NB_SearchHandler to download the results.

    @return \a NE_OK If successful
    @ingroup searchparams
*/
NB_DEC NB_Error NB_SearchParametersCreatePOI(
                NB_Context* context,                            /*!< The NB_Context */
                NB_SearchRegion* region,                        /*!< Search region used for the search */
                const char* name,                               /*!< The name of the POI to search for. Can be left blank */
                const char* searchScheme,                       /*!< The name of the search scheme to use.  Can be left blank */
                const char* categoryCodes[],
                int categoryCodeCount,
                uint32 sliceSize,                               /*!< Specifies how many results should be downloaded per download request. */
                NB_POIExtendedConfiguration extendedConfiguration, /*!< Specify extended search configuration */
                const char* language,                           /*!< Optional: Specify the language for the returned results. Default is English */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );

NB_DEC NB_Error NB_SearchParametersCreatePublicTransit(         /*!< 1st level search of public transit */
                NB_Context* context,                            /*!< The NB_Context */
                NB_SearchRegion* region,                        /*!< Search region used for the search */
                const char* name,                               /*!< The name of the POI to search for. Can be left blank */
                const char* categoryCodes[],                    /*!< The array of public transit type code.*/
                int categoryCodeCount,                          /*!< The count of the array above*/
                uint32 sliceSize,                               /*!< Specifies how many results should be downloaded per download request. */
                const char* language,                           /*!< Optional: Specify the language for the returned results. Default is English */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );

NB_DEC NB_Error
NB_SearchParametersCreatePublicTransitOverlay(
                NB_Context* context,                            /*!< The NB_Context */
                NB_SearchRegion* region,                        /*!< Search region used for the search */
                const char* name,                               /*!< The name of the POI to search for. Can be left blank */
                const char* transitTypes[],                    /*!< The array of transit types: "All", "Tram", "Metro", "Rail", "Bus", "Ferry", "cable car"  */
                int transitTypeCount,                           /*!< The count of the array above */
                uint32 departureTime,                           /*!< The time leaving the station */
                uint32 sliceSize,                               /*!< Specifies how many results should be downloaded per download request. */
                const char* language,                           /*!< Optional: Specify the language for the returned results. Default is English */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );

NB_DEC NB_Error NB_SearchParametersCreateFuel(
                NB_Context* context,                            /*!< The NB_Context */
                NB_SearchRegion* region,                        /*!< Search region used for the search */
                const char* name,                               /*!< The name of the POI to search for. Can be left blank */
                const char* searchScheme,                       /*!< The name of the search scheme to use.  Can be left blank */
                const char* categoryCodes[],
                int categoryCodeCount,
                NB_FuelSearchType searchType,                   /*!< The fuel search type */
	            NB_FuelResultsType resultsType,                 /*!< The fuel search results type */
                uint32 sliceSize,                               /*!< Specifies how many results should be downloaded per download request. */
                NB_POIExtendedConfiguration extendedConfiguration, /*!< Specify extended search configuration */
                const char* language,                           /*!< Optional: Specify the language for the returned results. Default is English */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );

NB_DEC NB_Error NB_SearchParametersCreateWeather(
                NB_Context* context,                            /*!< The NB_Context */
                NB_LatitudeLongitude* center,                   /*!< Center of the search */
                int forecastDays,                               /*!< The number of days to forecast for */
                NB_WeatherResultsType resultsType,              /*!< The weather search results type */
                NB_POIExtendedConfiguration extendedConfiguration, /*!< Specify extended search configuration */
                const char* language,                           /*!< Optional: Specify the language for the returned results. Default is English */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );

NB_DEC NB_Error NB_SearchParametersCreateTrafficIncident(
                NB_Context* context,                            /*!< The NB_Context */
                NB_SearchRegion* region,                        /*!< Search region used for the search */
                int severity,                                   /*!< Minimum traffic incident severity to return */
                NB_TrafficIncidentResultsType resultsType,      /*!< The weather search results type */
                uint32 sliceSize,                               /*!< Specifies how many results should be downloaded per download request. */
                NB_POIExtendedConfiguration extendedConfiguration, /*!< Specify extended search configuration */
                const char* language,                           /*!< Optional: Specify the language for the returned results. Default is English */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );

NB_DEC NB_Error NB_SearchParametersCreateEvent(
                NB_Context* context,                            /*!< The NB_Context */
                NB_SearchRegion* region,                        /*!< Search region used for the search */
                const char* name,                               /*!< The name of the POI to search for. Can be left blank */
                const char* categoryCodes[],                    /*!< Array of category codes. Set to NULL to search for all categories. */
                int categoryCodeCount,                          /*!< Number in category code array */
                uint32 startTime,                               /*!< Start time in GPS time */
                uint32 endTime,                                 /*!< End time in GPS time */
                NB_EventRating rating,
                NB_EventResultsType resultsType,
                uint32 sliceSize,                               /*!< Specifies how many results should be downloaded per download request. */
                NB_POIExtendedConfiguration extendedConfiguration, /*!< Specify extended search configuration */
                const char* language,                           /*!< Optional: Specify the language for the returned results. Default is English */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );

NB_DEC NB_Error NB_SearchParametersCreateEventVenue(
                NB_Context* context,                            /*!< The NB_Context */
                NB_SearchRegion* region,                        /*!< Search region used for the search */
                const char* name,                               /*!< The name of the POI to search for. Can be left blank */
                const char* categoryCodes[],                    /*!< Currently not supported. Pass in NULL. */
                int categoryCodeCount,                          /*!< Currently not supported. Pass in 0. */
                uint32 startTime,                               /*!< Start time in GPS time */
                uint32 endTime,                                 /*!< End time in GPS time */
                NB_EventRating rating,                          /*!< Currently not supported. Value is ignored. */
                NB_EventVenueResultsType resultsType,           /*!< Currently not supported. Value is ignored. */
                uint32 sliceSize,                               /*!< Specifies how many results should be downloaded per download request. */
                NB_POIExtendedConfiguration extendedConfiguration, /*!< Specify extended search configuration */
                const char* language,                           /*!< Optional: Specify the language for the returned results. Default is English */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );

NB_DEC NB_Error NB_SearchParametersCreateMovie(
                NB_Context* context,                            /*!< The NB_Context */
                NB_SearchRegion* region,                        /*!< Search region used for the search */
                const char* name,                               /*!< The name of the POI to search for. Can be left blank */
                uint32 startTime,                               /*!< Start time for the movies to search for in GPS time. Only used if 'showing' is set to 'NB_MS_SpecificTime'. */
                uint32 endTime,                                 /*!< End time for the movies to search for in GPS time. Only used if 'showing' is set to 'NB_MS_SpecificTime'. */
                NB_MovieShowing showing,
                const char* genreCodes[],                       /*!< Array of genre codes to search for. Set to NULL to search for all movies. */
                int genreCodeCount,                             /*!< Number in genre code array */
                NB_MovieSortBy sortBy,
                NB_MovieResultsType resultsType,
                uint32 sliceSize,                               /*!< Specifies how many results should be downloaded per download request. */
                NB_POIExtendedConfiguration extendedConfiguration, /*!< Specify extended search configuration */
                const char* language,                           /*!< Optional: Specify the language for the returned results. Default is English */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );

NB_DEC NB_Error NB_SearchParametersCreateMovieTheater(
                NB_Context* context,                            /*!< The NB_Context */
                NB_SearchRegion* region,                        /*!< Search region used for the search */
                const char* name,                               /*!< The name of the POI to search for. Can be left blank */
                uint32 startTime,                               /*!< Start time in GPS time */
                uint32 endTime,                                 /*!< End time in GPS time */
                NB_MovieTheaterResultsType resultsType,
                uint32 sliceSize,                               /*!< Specifies how many results should be downloaded per download request. */
                NB_POIExtendedConfiguration extendedConfiguration, /*!< Specify extended search configuration */
                const char* language,                           /*!< Optional: Specify the language for the returned results. Default is English */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );

NB_DEC NB_Error NB_SearchParametersCreateIteration(
                NB_Context* context,                            /*!< The NB_Context */
                NB_SearchInformation* information,              /*!< The Search Information returned from the previous search */
                NB_IterationCommand iterationCommand,           /*!< The iteration command to perform */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );

NB_DEC NB_Error NB_SearchParametersCreateSubSearch(
                NB_Context* context,                            /*!< The NB_Context */
                NB_SearchInformation* information,              /*!< The Search Information returned from the previous search */
                int resultIndex,                                /*!< The index for which the Sub-Search will be performed */
                NB_SearchParameters** parameters                /*!< On success, a newly created NB_SearchParameters object; NULL otherwise.  A valid object must be destroyed using NB_SearchParametersDestroy() */
                );


/*! Destroy a previously created SearchParameters object

@ingroup searchparameters
@returns NB_Error
*/
NB_DEC NB_Error NB_SearchParametersDestroy(
                NB_SearchParameters* parameters                 /*!< A NB_SearchParameters object created with NB_SearchParametersCreate() or NB_SearchParametersCreateSubSearch() */
                );


/*! Add an extended key/value pair

@param parameters The search parameters to add the key/value pair to
@param configuration The configuration type to add the pair to
@param key The key name
@param value The value
*/
NB_DEC NB_Error NB_SearchParametersAddExtendedKeyValue(NB_SearchParameters* parameters, NB_POIExtendedConfiguration configuration, const char* key, const char* value);

/*! Add an extended image specification

@param parameters The search parameters to add the image to
@param configuration The configuration type to add the image to
@param format The image format
@param size The image size
@param dpi The image DPI
*/
NB_DEC NB_Error NB_SearchParametersAddExtendedImage(NB_SearchParameters* parameters, NB_POIExtendedConfiguration configuration, NB_ImageFormat format, NB_ImageSize* size, uint16 dpi);

/*! Set the search source

@param parameters The search parameters to set the source for
@param source The source of the search
@return NB_Error
*/
NB_DEC NB_Error NB_SearchParametersSetSource(NB_SearchParameters* parameters, NB_SearchSource source);

/*! Get the search source

@param parameters The search parameters to get the source from
@param source The source of the search
@return NB_Error
 */
NB_DEC NB_Error NB_SearchParametersGetSource(NB_SearchParameters* parameters,  NB_SearchSource* source);

/*! Set the search sort order

@param parameters The search parameters to get the source from
@param sortBy Desired result sort order
@return NB_Error
 */
NB_DEC NB_Error NB_SearchParametersSetSearchSortBy(NB_SearchParameters* parameters, NB_SearchSortBy sortBy);

/*! Set the search input method

@param parameters The search parameters to set the search input method for
@param inputMethode Desired search input method
@return NB_Error
 */
NB_DEC NB_Error NB_SearchParametersSetSearchInputMethod(NB_SearchParameters* parameters, NB_SearchInputMethod inputMethod);

/*! Get the search input method

@param parameters The search parameters to get the search input method for
@param inputMethode Desired search input method
@return NB_Error
 */
NB_DEC NB_SearchInputMethod NB_SearchParametersGetSearchInputMethod(NB_SearchParameters* parameters);

/*! Enforce the usage of slice size.

@param parameters The search parameters to adjust
@return NB_Error
 */
NB_DEC NB_Error NB_SearchParametersEnforceSliceSize(NB_SearchParameters* parameters);

/*! Add filter key value pair to search parameters.

@param parameters The search parameters, wrapper over AddSearchFilterKeyValue
@param key - key name
@param value - value for this key
@return NB_Error
 */

NB_DEC NB_Error NB_SearchParametersAddSearchFilterKeyValue(NB_SearchParameters* parameters, const char* key, const char*  value);

/*! Set the search region

@param parameters The search parameters to set the search region for
@param region Search region used for the search
@return NB_Error
*/
NB_DEC NB_Error NB_SearchParametersSetSearchRegion(NB_SearchParameters* parameters, NB_SearchRegion* region);

/*! @} */

#endif // NBSEARCHHANDLER_H

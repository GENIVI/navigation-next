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

    @file     nbsinglesearchparametersprivate.h
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

#ifndef NBSINGLESEARCHPARAMETERSPRIVATE_H
#define NBSINGLESEARCHPARAMETERSPRIVATE_H

/*!
    @addtogroup nbsinglesearchparameters
    @{
*/

#include "nbsinglesearchparameters.h"
#include "data_search_query.h"

// Constants ....................................................................................

// Result style strings
#define RESULT_STYLE_SINGLE_SEARCH      "single-search"
#define RESULT_STYLE_SUGGEST            "suggest"
#define RESULT_STYLE_GEOCODE            "geocode"
#define RESULT_STYLE_DEFAULT_LOCATION   "default-location"

// Types ........................................................................................


// Functions ....................................................................................

/*! Create a TPS element object from the single search parameters

    @return NE_OK if success
*/
tpselt
NB_SingleSearchParametersToTPSQuery(
    NB_SingleSearchParameters* parameters                               /*!< Single search parameters instance */
    );

/*! Copy a search query of the single search parameters

     This function gets the search query of the single search parameters and returns a copy of it.

    @return NE_OK if success
*/
NB_Error
NB_SingleSearchParametersCopyQuery(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    data_search_query* query                                            /*!< On return the copied search query */
    );

/*! This function gets the analytics event id of the single search parameters

    @return NE_OK if success, NE_NOENT if there is no analytics event id
*/
NB_Error
NB_SingleSearchParametersGetAnalyticsEventId(
    NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
    uint32* analyticsEventId                                            /*!< On return the analytics event id */
    );

/*! @} */

#endif

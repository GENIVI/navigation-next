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

    @file     nbsinglesearchinformationprivate.h
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

#ifndef NBSINGLESEARCHINFORMATIONPRIVATE_H
#define NBSINGLESEARCHINFORMATIONPRIVATE_H

/*!
    @addtogroup nbsinglesearchinformation
    @{
*/

#include "nbsinglesearchinformation.h"
#include "data_search_query.h"
#include "data_place.h"

// Constants ....................................................................................


// Types ........................................................................................


// Functions ....................................................................................

/*! Create an instance of a NB_SingleSearchInformation object from a TPS reply

    @return NE_OK if success
*/
NB_Error
NB_SingleSearchInformationCreateFromTPSReply(
    tpselt reply,                                                       /*!< A TPS element containing a search reply from the server */
    data_search_query* query,                                           /*!< Search query */
    NB_Context* context,                                                /*!< Pointer to current context */
    NB_SearchSource searchSource,                                       /*!< search source */
    NB_SingleSearchInformation** information                            /*!< On success, returns pointer to single search information */
    );

/*! Get the iteration state of the single search information

    @return Iteration state
*/
data_blob*
NB_SingleSearchInformationGetIterationState(
    NB_SingleSearchInformation* information                             /*!< Single search information instance */
    );

/*! Get the search query of the single search information

    @return Search query
*/
data_search_query*
NB_SingleSearchInformationGetQuery(
    NB_SingleSearchInformation* information                             /*!< Single search information instance */
    );

/*! This function sets the analytics event id

    @return NE_OK if success
*/
NB_Error
NB_SingleSearchInformationSetAnalyticsEventId(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    uint32 analyticsEventId                                             /*!< The analytics event id */
    );

/*! Get the search source of single search information
    return search source.
 */
NB_SearchSource
NB_SingleSearchInformationGetSource(NB_SingleSearchInformation* information);

/*! Copy the search filter of the single search information of suggestion search

    This function gets the search filter of the search reply and returns a copy of it.

    @return NE_OK if success
*/
NB_Error
NB_SingleSearchInformationCopySearchFilterOfSuggestion(
    NB_SingleSearchInformation* information,                            /*!< Single search information instance */
    uint32 resultIndex,                                                 /*!< The index of the place to copy */
    data_search_filter* searchFilter                                    /*!< On return the copied search filter */
    );

/*! @} */

#endif

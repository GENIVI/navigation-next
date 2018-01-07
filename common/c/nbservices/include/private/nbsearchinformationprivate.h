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

    @file     nbsearchinfomation.h
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

#ifndef NBSEARCHINFORMATIONPRIVATE_H
#define NBSEARCHINFORMATIONPRIVATE_H

/*!
    @addtogroup nbsearchinformation
    @{
*/

#include "nbsearchinformation.h"
#include "tpselt.h"
#include "data_blob.h"
#include "data_proxpoi_query.h"
#include "data_proxpoi_reply.h"
#include "nbcontext.h"


/*! Create a search information object from a TPS.

    @return NE_OK on success
*/
NB_Error 
NB_SearchInformationCreateFromTPSReply(
                   tpselt reply,                        /*!< A TPS element containing a reply from the server */
                   data_proxpoi_query* query,           /*!< proxpoi query; if null, assumes TPS reply could from any query, and extracts proxmatch element only */
                   NB_Context* context,                 
                   NB_SearchInformation** information   /*!< On return a pointer to a new NB_SearchInformation instance */
                   );


/*! Get the iteration state from the search information.

    @return Iteration state
*/
data_blob* 
NB_SearchInformationGetIterationState(NB_SearchInformation* information);


/*! Get the server query of the search.

    @return server query
*/
data_proxpoi_query* 
NB_SearchInformationGetQuery(NB_SearchInformation* information);


/*! Clone the search filter from the reply.

    This function gets the search filter from the reply and returns a copy of it.

    @return NE_OK on success
*/
NB_Error
NB_SearchInformationCloneSearchFilter(NB_SearchInformation* information,        /*!< Search information containing the server reply */
                                      int resultIndex,                          /*!< Result index for which to retrieve the search filter */
                                      data_search_filter* searchFilter          /*!< On return the new cloned search filter */
                                      );


/*! Get a place associated with a camera

@param information The search information containing the place
@param index The index of the palce to retrieve
@param distance The camera position from the start of the route, in meters
@return The requested camera place
*/
data_place* NB_SearchInformationGetCameraPlace(NB_SearchInformation* information, int index, double* distance);

/*! Sets the event id associated with information object parameter

@param information The search information containing the place
@param eventId The value to set for the analytics event id
*/
void NB_SearchInformationSetAnalyticsEventId(NB_SearchInformation* information, uint32 eventId);


/*! Returns a pointer to the search event cookie returned for the current search, if any

Returns a pointer to the search event cookie returned from the current search, if one was returned.
The cookie is owned by the search information object and will be destroyed with it.  A copy should
be made if the cookie is needed beyond the life of the search information object.  NULL will be
returned if no search event cookie was returned for the search.

@param information Pointer to a search information object
@param searchEventCookie Pointer to a search event cookie pointer that on return will be populated with pointer to a search event cookie, or NULL
@return NB_Error
*/
NB_Error
NB_SearchInformationGetSearchEventCookie(NB_SearchInformation* information, NB_SearchEventCookie* searchEventCookie);


/*! @} */

#endif // NBSEARCHINFORMATIONPRIVATE_H


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
    @file     palrecents.h
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef PALRECENTS_H
#define PALRECENTS_H

#include "pal.h"
#include "paltypes.h"
#include "palstdlib.h"
#include "palmyplaces.h"

typedef struct PAL_Recents PAL_Recents;

/*! Create and initialize a new PAL_Recents object.
    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_RecentsCreate(PAL_Instance* pal,               /*!<The pal instance*/
           PAL_Recents** recents                /*!<new PAL_Recents object is created; NULL otherwise */
           );

/*! Destroy a previously created PAL_Recents object.
    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_RecentsDestroy(PAL_Recents* recents       /*!<The PAL_Recents instance*/
           );

/*! Get recent places by filter keyword and place them on myPlacesArray pointer.
    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_RecentsGet(PAL_Recents* recents,                /*!<The PAL_Recents instance*/
           char* filter,                            /*!<text filter*/
           PAL_MyPlace** myPlacesArray,             /*!<array for found places*/
           int* myPlaceCount,                       /*!<output limitation, default value = 100, function changes it if number of found places is smaller*/
           PAL_SortType sortType,                   /*!<sort type for found places, default value = PAL_ST_ByNone*/
           double latitude,                         /*!<latitude*/
           double longitude                         /*!<longitude*/
           );

/*! Add new place to recent.
    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_RecentsAdd(PAL_Recents* recents,           /*!<The PAL_Recents instance*/
           PAL_MyPlace* myPlace                /*!<place to add*/
           );

/*! Get recent place by RIM MyPlacesDB id
*/
PAL_DEC PAL_Error
PAL_RecentsGetPlaceById(PAL_Recents* recents,  /*!<The PAL_Recents instance*/
           uint32 id,                          /*!<id of the place to find*/
           PAL_MyPlace* myPlace                /*!<pointer for found place*/
           );

/*! Remove recent place by RIM MyPlacesDB id
*/
PAL_DEC PAL_Error
PAL_RecentsRemove(PAL_Recents* recents,  /*!<The PAL_Recents instance*/
           uint32 id                           /*!<id of the place to find*/
           );

/*! Clear all recent places.
    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_RecentsClear(PAL_Recents* recents);        /*!<The PAL_Recents instance*/

PAL_DEC PAL_Error
PAL_RecentsGetAllPlaces(PAL_Recents* recents, /*!<The PAL_Recents instance*/
           char* pFilter,                     /*!<text filter*/
           PAL_MyPlace** ppMyPlacesArray,     /*!<array for found places*/
           int* pMyPlaceCount                 /*!<output limitation, default value = 100, function changes it if number of found places is smaller*/
           );

PAL_DEC PAL_Error
PAL_HomeWorkGet(PAL_Recents* recents,                /*!<The PAL_Recents instance*/
           char* filter,                            /*!<text filter*/
           PAL_MyPlace** myPlacesArray,             /*!<array for found places*/
           int* myPlaceCount,                       /*!<output limitation, default value = 100, function changes it if number of found places is smaller*/
           PAL_SortType sortType,                   /*!<sort type for found places, default value = PAL_ST_ByNone*/
           double latitude,                         /*!<latitude*/
           double longitude                         /*!<longitude*/
           );

#endif
/*! @{ */

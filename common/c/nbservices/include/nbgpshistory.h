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

    @file     nbgpshistory.h
    
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

#ifndef NBGPSHISTORY_H
#define NBGPSHISTORY_H

#include "paltypes.h"
#include "nbcontext.h"
#include "nbgpstypes.h"
#include "nbexp.h"

/*!
    @addtogroup nbgpshistory
    @{
*/

/*! @struct NB_GpsHistory
GPS History structure
*/
typedef struct NB_GpsHistory NB_GpsHistory;

typedef void (*NB_GpsHistoryCallbackFunction)(void *arg, NB_GpsLocation *fix);

/*! Creates a NB_GpsHistory object

@param history On success, the newly created NB_GpsHistory; NULL otherwise
@returns NB_Error
*/
NB_DEC NB_Error	NB_GpsHistoryCreate(NB_Context* context, int max, double filterSpeed, NB_GpsHistory** history);

/*! Destroys the gpshistory and frees any allocated memory in NB_GpsHistory

@param history A NB_GpsHistory object created with NB_GpsHistoryCreate()
@returns NB_Error
*/
NB_DEC NB_Error	NB_GpsHistoryDestroy(NB_GpsHistory* history);

/*! Adds a gps fix to the gps history list, if set NB_GpsHistorySetLocationThreshhold with usedForAdding as TRUE,
    then this method will only add gps fix, else return NE_GPS error.

@param history NB_GpsHistory object
@param fix pointer NB_GpsLocation to be added to the history list
@returns NB_Error
*/
NB_DEC NB_Error	NB_GpsHistoryAdd(NB_GpsHistory* history, NB_GpsLocation* fix);

/*! Empties the Gps history list

@param history NB_GpsHistory object
@returns NB_Error
*/
NB_DEC NB_Error	NB_GpsHistoryClear(NB_GpsHistory* history);

/*! Sets minimum speed for filter

@param history NB_GpsHistory object
@param filterSpeed desired minimum fix speed
@returns NB_GpsLocation* pointer to NB_GpsLocation
*/
NB_DEC NB_Error   NB_GpsHistorySetMinimumFixSpeed(NB_GpsHistory* history, double filterSpeed);

/*! Gets the last NB_GpsLocation entry

@param history NB_GpsHistory object
@returns NB_GpsLocation* pointer to NB_GpsLocation
*/
NB_DEC NB_Error	NB_GpsHistoryGetLatest(NB_GpsHistory* history, NB_GpsLocation* fix);

/*! Gets the last NB_GpsLocation entry

@param history NB_GpsHistory object
@returns NB_GpsLocation* pointer to NB_GpsLocation
*/
NB_DEC NB_Error	NB_GpsHistoryGetLatestHeadingFix(NB_GpsHistory* history, NB_GpsLocation* fix);

/*! Gets the NB_GpsLocation at the specified index

@param history NB_GpsHistory object
@param index index of the desired GpsLocation 
@returns NB_GpsLocation* pointer to NB_GpsLocation
*/
NB_DEC NB_Error   NB_GpsHistoryGet(NB_GpsHistory* history, int index, NB_GpsLocation* fix);

/*! Gets amount of fixes currently in the GPS history

@param history NB_GpsHistory object
@returns int
*/
NB_DEC int   	NB_GpsHistoryGetCount(NB_GpsHistory* history);

/*! Calls a user-specified function with for every gps fix in the gps history

@param history NB_GpsHistory object
@param callback user-specified function to be called
@param callbackData callback parameter for user-specified function
@returns NB_Error
*/
NB_DEC NB_Error	NB_GpsHistoryForEach(NB_GpsHistory* history, NB_GpsHistoryCallbackFunction callback, void* callbackData);

/*! returns true if gps is poor

 @param history NB_GpsHistory object
 @returns NB_Error
 */

NB_DEC nb_boolean NB_GpsHistoryIsGpsPoor(NB_GpsHistory* history);

/*! Sets uncertain threshhold for location check.

@param threshhold uncertain threshhold
@param usedForAdding the threshhold will be used for NB_GpsHistoryAdd, and only gps fix can be added into history
@returns NB_Error
*/
NB_DEC NB_Error NB_GpsHistorySetLocationThreshhold(NB_GpsHistory* history, double threshhold, nb_boolean usedForAdding);

/*! Check if the last fix's heading is valid or not.

@param history NB_GpsHistory object
@param isValid output TRUE for valid, else FALSE
@returns NE_OK if success
*/
NB_DEC NB_Error NB_GpsHistoryIfLastHeadingValid(NB_GpsHistory* history, nb_boolean* isValid);

/*! @} */

#endif

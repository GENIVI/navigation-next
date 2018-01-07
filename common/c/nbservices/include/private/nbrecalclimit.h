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

    @file     nbrecalclimit.h
    
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

#ifndef NBRECALCLIMIT_H
#define NBRECALCLIMIT_H

#include "paltypes.h"
#include "nbcontext.h"

/*!
    @addtogroup nbrecalclimit
    @{
*/

/*! @struct NB_RecalcLimit
Navigation recalc limit structure
*/
typedef struct NB_RecalcLimit NB_RecalcLimit;

/*! Creates a NB_RecalcLimit object

@param context NB_Context
@param maxCount The maximum number of recalcs to allow
@param timePeriod The time period the maximum is applied within
@param limit On success, the newly created NB_RecalcLimit; NULL otherwise
@returns NB_Error
*/
NB_Error NB_RecalcLimitCreate(NB_Context* context, uint32 maxCount, uint32 timePeriod, NB_RecalcLimit** limit);

/*! Destroys the recalc limit object and frees any allocated memory in NB_RecalcLimitCreate

@param limit A NB_RecalcLimit object created with NB_RecalcLimitCreate()
@returns NB_Error
*/
NB_Error NB_RecalcLimitDestroy(NB_RecalcLimit* limit);

/*! Adds a recalc time to the limit list

@param limit NB_RecalcLimit object
@param recalcTime Reclac time to be added to the limit list
@returns NB_Error
*/
NB_Error NB_RecalcLimitAdd(NB_RecalcLimit* limit, nb_gpsTime recalcTime);

/*! Resets the limit list

@param limit NB_RecalcLimit object
@returns NB_Error
*/
NB_Error NB_RecalcLimitReset(NB_RecalcLimit* limit);

/*! Tests to see if the recalc limit has been exceeded

@param limit NB_RecalcLimit object
@param checkTime The time at which to check for limits exceeded
@returns Non-zero if the recalc limit has been exceeded; zero if the limit is ignored or has not been exceeded
*/
nb_boolean NB_RecalcLimitExceeded(NB_RecalcLimit* limit, nb_gpsTime checkTime);


/*! @} */

#endif

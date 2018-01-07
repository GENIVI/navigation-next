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

    @file     nbcouponrequestinformationprivate.h
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems, Inc. is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef NBCOUPONREQUESTINFORMATIONPRIVATE_H
#define NBCOUPONREQUESTINFORMATIONPRIVATE_H

#include "nbcouponrequestinformation.h"
#include "tpselt.h"
#include "nbcontext.h"
#include "data_blob.h"
#include "data_coupon_query.h"

/*!
    @addtogroup nbcouponrequestinformation
    @{
*/


/*! Create an information object from a tps network reply

@param context NAVBuilder Context Instance
@param reply The TPS reply from the server
@param query The original server query
@param information On success, a newly created NB_CouponRequestInformation object; NULL otherwise.  A valid object must be destroyed using NB_CouponRequestInformationDestroy()
@return NB_Error
*/
NB_Error NB_CouponRequestInformationCreate(NB_Context* context, tpselt reply, data_coupon_query* query, NB_CouponRequestInformation** information);


/*! Get the iteration state from the information.

@param information Information structure
@return Iteration state
*/
data_blob* NB_CouponRequestInformationGetIterationState(NB_CouponRequestInformation* information);


/*! Get the server query of the search.

@param information Information structure
@return server query
*/
data_coupon_query* NB_CouponRequestInformationGetQuery(NB_CouponRequestInformation* information);

/*! @} */

#endif

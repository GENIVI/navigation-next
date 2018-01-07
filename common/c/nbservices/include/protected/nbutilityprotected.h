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

    @file     nbutilprotected.h
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

#ifndef NBUTILITYPROTECTED_H
#define NBUTILITYPROTECTED_H

#include "data_locmatch.h"
#include "data_place.h"
#include "cslnetwork.h"
#include "nbnetwork.h"
#include "nbextappcontent.h"
#include "nbexp.h"

/*! @{ */

NB_DEC uint32 GetReplyErrorCode(NB_NetworkResponse* resp);
NB_DEC NB_NetworkRequestStatus TranslateNetStatus(NB_NetworkResponseStatus status);

NB_DEC NB_Error SetNIMPlaceFromPlace(NB_Place* pPlace, data_util_state* pDUS, data_place* pDataPlace);
NB_DEC NB_Error SetNIMLocationFromLocation(NB_Location* pLocation, data_util_state* pDUS, data_location* pLoc); 
NB_DEC NB_Error SetNIMLocationFromLocMatch(NB_Location* pLocation, data_util_state* pDUS, data_locmatch* pLocMatch); 
NB_DEC NB_Error SetNIMPhoneFromPhone(NB_Phone* pPhone, data_util_state* pDUS, data_phone* pPh);
NB_DEC NB_Error SetNIMCategoryFromCategory(NB_Category* pCategory, data_util_state* pDUS, data_category* pCat);
NB_DEC NB_Error SetNIMExtAppContentFromExtAppContentVector(NB_ExtAppContent* pExtAppContent, data_util_state* pDUS, CSL_Vector* pVecExtAppContent);
NB_DEC NB_Error SetExtAppContentVectorFromNIMExtAppContent(CSL_Vector* pVecExtAppContent, data_util_state* pDUS, NB_ExtAppContent* pExtAppContent);

NB_DEC void ClearNIMPlace(NB_Place* pPlace);

/*! @} */

#endif

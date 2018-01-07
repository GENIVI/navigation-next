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

    @file     nbcouponloadimageinformation.c
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

/*! @{ */

#include "nbcontextprotected.h"
#include "nbcouponloadimageinformationprivate.h"
#include "data_coupon_reply.h"

struct NB_CouponLoadImageInformation
{
    NB_Context*        context;
    data_coupon_reply  reply;
};

NB_DEF NB_Error
NB_CouponLoadImageInformationCreate(NB_Context* context, tpselt reply, NB_CouponLoadImageInformation** information)
{
    NB_CouponLoadImageInformation* pThis = NULL;
    NB_Error err = NE_OK;

    if (context == NULL || reply == NULL || information == NULL)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(NB_CouponLoadImageInformation));
    if (pThis == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(NB_CouponLoadImageInformation));

    err = data_coupon_reply_init(NB_ContextGetDataState(context), &pThis->reply);
    err = err ? err : data_coupon_reply_from_tps(NB_ContextGetDataState(context),
                                            &pThis->reply, reply);
    if (err == NE_OK)
    {
        pThis->context = context;
        *information = pThis;
    }
    else
    {
        NB_CouponLoadImageInformationDestroy(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_CouponLoadImageInformationGetImage(NB_CouponLoadImageInformation* information,
                                                byte** imageData, uint32* imageSize)
{
    if ( !information || !imageData || !imageSize ||
         !information->reply.load_image_response)
    {
        return NE_INVAL;
    }

    *imageSize = information->reply.load_image_response->image_bytes.size;
    *imageData = information->reply.load_image_response->image_bytes.data;

    return NE_OK;
}

NB_DEF NB_Error
NB_CouponLoadImageInformationDestroy(NB_CouponLoadImageInformation* information)
{
    if (information == NULL)
    {
        return NE_INVAL;
    }

    data_coupon_reply_free(NB_ContextGetDataState(information->context), &information->reply);
    nsl_free(information);

    return NE_OK;
}

/*! @} */

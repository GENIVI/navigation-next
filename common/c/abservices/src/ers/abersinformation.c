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

@file     abersinformation.c
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "abersinformation.h"
#include "abersparametersprivate.h"
#include "data_ers_contact.h"
#include "data_ers_reply.h"
#include "nbcontextprotected.h"
#include "nbutilityprotected.h"
#include "vec.h"


/*!
    @addtogroup abersinformation
    @{
*/


struct AB_ErsInformation
{
    NB_Context*         context;
    data_ers_reply      reply;    
};


AB_DEF NB_Error
AB_ErsInformationCreate(NB_Context* context, tpselt reply, AB_ErsInformation** information)
{
    AB_ErsInformation* pThis = 0;
    NB_Error err = NE_OK;
    
    if (!context || !reply || !information)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    err = data_ers_reply_from_tps(NB_ContextGetDataState(context), &pThis->reply, reply);    

    if (!err)
    {
        pThis->context = context;
        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}

AB_DEF NB_Error
AB_ErsInformationDestroy(AB_ErsInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_ers_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);
    nsl_free(pThis);        

    return NE_OK;
}

AB_DEF NB_Error
AB_ErsInformationGetContactCount(AB_ErsInformation* pThis, uint32* count)
{
    if (!pThis || !count)
    {
        return NE_INVAL;
    }

    *count = CSL_VectorGetLength(pThis->reply.vec_contacts);
    return NE_OK;
}

AB_DEF NB_Error
AB_ErsInformationGetContact(AB_ErsInformation* pThis, uint32 index, AB_ErsContact** contact)
{
    AB_ErsContact* newContact = 0;
    data_util_state* dataState = 0;
    data_ers_contact* dataContact = 0;

    if (!pThis || !contact || index > (uint32)CSL_VectorGetLength(pThis->reply.vec_contacts))
    {
        return NE_INVAL;
    }

    newContact = nsl_malloc(sizeof(*newContact));
    if (!newContact)
    {
        return NE_NOMEM;
    }

    dataState = NB_ContextGetDataState(pThis->context);
    dataContact = (data_ers_contact*)CSL_VectorGetPointer(pThis->reply.vec_contacts, index);
    SetNIMPhoneFromPhone(&newContact->phone,  dataState, &dataContact->phone);
    newContact->name = nsl_strdup(data_string_get(dataState, &dataContact->name));

    *contact = newContact;

    return NE_OK;
}

AB_DEF NB_Error
AB_ErsContactDestroy(AB_ErsContact* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    nsl_free((char*)pThis->name);
    nsl_free(pThis);

    return NE_OK;
}


/*! @} */

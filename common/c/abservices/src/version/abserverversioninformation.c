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

    @file abserverversioninformation.c
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


/*! @{ */


#include "nbcontextprotected.h"
#include "abserverversioninformation.h"
#include "abserverversioninformationprivate.h"


struct AB_ServerVersionInformation
{
    NB_Context*     context;
    char*           version;
};


NB_Error
AB_ServerVersionInformationCreate(NB_Context* context, tpselt reply, AB_ServerVersionInformation** information)
{
    AB_ServerVersionInformation* pThis = 0;
    NB_Error err = NE_OK;
    const char* version = 0;

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

    pThis->context = context;

    version = te_getattrc(reply, "formatted-version");
    if (version)
    {
        pThis->version = nsl_strdup(version);
    }

    if (pThis->version)
    {
        *information = pThis;
    }
    else
    {
        AB_ServerVersionInformationDestroy(pThis);
        err = NE_NOMEM;
    }

    return err;
}


AB_DEF NB_Error
AB_ServerVersionInformationDestroy(AB_ServerVersionInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    nsl_free(pThis->version);        
    nsl_free(pThis);        

    return NE_OK;
}


AB_DEF NB_Error
AB_ServerVersionInformationGetVersion(AB_ServerVersionInformation* pThis, const char** version)
{
    if (!pThis || !version)
    {
        return NE_INVAL;
    }

    *version = pThis->version;
    return NE_OK;
}


/*! @} */

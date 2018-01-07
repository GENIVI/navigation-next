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

    @file abserverversionparameters.c
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
#include "abserverversionparameters.h"
#include "abserverversionparametersprivate.h"


struct AB_ServerVersionParameters
{
    NB_Context*                 context;
    AB_ServerVersionQueryType   queryType;
    char*                       target;
};


AB_DEF NB_Error
AB_ServerVersionParametersCreate(NB_Context* context, AB_ServerVersionQueryType queryType, const char* target, AB_ServerVersionParameters** parameters)
{
    AB_ServerVersionParameters* pThis = 0;
    NB_Error err = NE_OK;

    if (!context || !target || !parameters)
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
    pThis->queryType = queryType;
    pThis->target = nsl_strdup(target);

    if (pThis->target)
    {
        *parameters = pThis;
    }
    else
    {
        AB_ServerVersionParametersDestroy(pThis);
        err = NE_NOMEM;
    }

    return err;
}


AB_DEF NB_Error
AB_ServerVersionParametersDestroy(AB_ServerVersionParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->target)
    {
        nsl_free(pThis->target);
    }

    nsl_free(pThis);

    return NE_OK;
}


const char*
AB_ServerVersionParametersGetTarget(AB_ServerVersionParameters* pThis)
{
    if (!pThis)
    {
        return 0;
    }

    return pThis->target;
}

tpselt
AB_ServerVersionParametersToTPSQuery(AB_ServerVersionParameters* pThis)
{
    if (!pThis)
    {
        return 0;
    }

    return te_new("version-query");
}


/*! @} */

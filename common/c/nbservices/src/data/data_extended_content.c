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

    @file     data_extended_content.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_extended_content.h"
#include "data_extended_content_region.h"

NB_Error
data_extended_content_init(data_util_state* state, data_extended_content* extendedContent)
{
    extendedContent->vec_extended_content_region = CSL_VectorAlloc(sizeof(data_extended_content_region));

    if (!extendedContent->vec_extended_content_region)
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

void
data_extended_content_free(data_util_state* state, data_extended_content* extendedContent)
{
    if (extendedContent->vec_extended_content_region)
    {
        int i = 0;
        int length = CSL_VectorGetLength(extendedContent->vec_extended_content_region);

        for (i =0; i < length; i++)
        {
            data_extended_content_region_free(state, (data_extended_content_region*)CSL_VectorGetPointer(extendedContent->vec_extended_content_region, i));
        }
        CSL_VectorDealloc(extendedContent->vec_extended_content_region);
        extendedContent->vec_extended_content_region = NULL;
    }
}

NB_Error
data_extended_content_from_tps(data_util_state* state, data_extended_content* extendedContent, tpselt tpsElement)
{
    NB_Error err = NE_OK;

    tpselt childElement = 0;
    int iterator = 0;
    data_extended_content_region extendedContentRegion;

    if (!tpsElement)
    {
        return NE_INVAL;
    }

    err = data_extended_content_region_init(state, &extendedContentRegion);
    if (err != NE_OK)
    {
        return err;
    }

    data_extended_content_free(state, extendedContent);

    err = data_extended_content_init(state, extendedContent);

    if (err == NE_OK)
    {
        iterator = 0;

        while ((childElement = te_nextchild(tpsElement, &iterator)) != NULL) 
        {
            if (nsl_strcmp(te_getname(childElement), "extended-content-region") == 0)
            {
                err = data_extended_content_region_from_tps(state, &extendedContentRegion, childElement);
                if (err == NE_OK)
                {
                    if (!CSL_VectorAppend(extendedContent->vec_extended_content_region, &extendedContentRegion))
                    {
                        err = NE_NOMEM;
                    }
                    nsl_memset(&extendedContentRegion, 0, sizeof(data_extended_content_region));
                }

                if (err != NE_OK)
                {
                    break;
                }
            }
        }
    }

    data_extended_content_region_free(state, &extendedContentRegion);

    if (err != NE_OK)
    {
        data_extended_content_free(state, extendedContent);
    }
    return err;
}

NB_Error
data_extended_content_copy(data_util_state* state, data_extended_content* destinationExtendedContent, data_extended_content* sourceExtendedContent)
{
    NB_Error err = NE_OK;

    DATA_VEC_COPY(state, err, destinationExtendedContent->vec_extended_content_region,
                              sourceExtendedContent->vec_extended_content_region,  data_extended_content_region);

    return err;
}

/*! @} */

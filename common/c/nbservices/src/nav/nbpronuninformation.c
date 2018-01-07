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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbcontextaccess.h"
#include "nbqalog.h"
#include "nbpronuninformation.h"
#include "nbpronuninformationprivate.h"
#include "nbpronunparameters.h"
#include "data_pronun_reply.h"

struct NB_PronunInformation
{
    NB_Context*                 context;
    data_pronun_reply              dataReply;      
};


static NB_PronunInformation* AllocatePronunInformation(NB_Context* context);

NB_DEF NB_Error 
NB_PronunInformationDestroy(NB_PronunInformation* pronun)
{
    data_util_state* dataState = NULL;
    if (pronun)
    {
        data_pronun_reply_free(dataState, &pronun->dataReply);
        nsl_free(pronun);
        return NE_OK;
    }

    return NE_INVAL;
}

NB_Error 
NB_PronunInformationCreateFromTPSReply(NB_Context* context, tpselt reply, NB_PronunParameters* params, NB_PronunInformation** information)
{
    NB_Error err = NE_OK;    
    data_util_state* dataState = NULL;    
    CSL_Cache* voiceCache = NULL; 
    NB_PronunInformation* pronun = NULL;
    int length;
    int i;

    dataState = NB_ContextGetDataState(context);

    pronun = AllocatePronunInformation(context);   
    
    if ( !pronun )
    {
        return NE_NOMEM;
    }
    
    voiceCache = NB_ContextGetVoiceCache(context);

    if ( !voiceCache )
    {
        NB_PronunInformationDestroy(pronun);
        return NE_INVAL;
    }

    err = data_pronun_reply_from_tps(dataState, &pronun->dataReply, reply);
    
    if (err == NE_OK)
    {
        length = CSL_VectorGetLength(pronun->dataReply.vec_file);        
        for (i=0; i<length; i++)
        {
            data_file* file = CSL_VectorGetPointer(pronun->dataReply.vec_file, i);
            if (file)
            {
                CSL_CacheAdd(voiceCache, file->name.data, file->name.size, file->data.data, file->data.size, TRUE, TRUE, 0);

                if (CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(context)))
                {
                    NB_QaLogPronunFileDownload(context, (const char*) file->name.data, (nb_size)file->name.size);
                }
            }
        }
        
    }
          
    if (err == NE_OK)
    {
        *information = pronun;
    }
    else 
    {
        NB_PronunInformationDestroy(pronun);
        *information = NULL;
    }

    return err;
}

static NB_PronunInformation* 
AllocatePronunInformation(NB_Context* context)
{
    NB_PronunInformation* pThis = NULL;

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NULL;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;  

    data_pronun_reply_init( NB_ContextGetDataState(context), &pThis->dataReply);
    
    return pThis;
}


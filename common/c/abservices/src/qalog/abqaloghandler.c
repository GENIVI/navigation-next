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

    @file     abqaloghandler.c
    @defgroup abqalog QA Logging
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

#include "abqaloghandler.h"
#include "abqaloginformationprivate.h"
#include "abqalogparametersprivate.h"
#include "nbcontextprotected.h"
#include "nbutilityprotected.h"
#include "nbnetworkprotected.h"
#include "cslnetwork.h"
#include "palfile.h"


/*! @{ */

#define UPLOAD_CHUNK_SIZE	        (1024 * 100)


enum QaHandler_UploadState { QHUS_Idle, QHUS_Upload, QHUS_Finished, QHUS_Cancel };


struct QaLogHandler_Query
{
    NB_NetworkQuery                 networkQuery;
    AB_QaLogHandler*                pThis;
};


struct AB_QaLogHandler
{
    NB_Context*                     context;
    NB_RequestHandlerCallback       callback;
    NB_Network*                     network;
    nb_boolean                      inProgress;
    struct QaLogHandler_Query       query;
    enum QaHandler_UploadState      uploadState;

    PAL_File*                       file;

    byte                            chunkData[UPLOAD_CHUNK_SIZE];
    uint32                          chunkDataLength;

    uint32                          fileSize;
    uint32                          totalChunks;
    uint32                          currentChunk;
    char                            logId[256];
};


static void QaLogHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
static void QaLogHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);
static NB_Error QaLogHandler_UploadChunk(AB_QaLogHandler* pThis);
static void QaLogHandler_Cleanup(AB_QaLogHandler* pThis);
static tpselt QaLogHandler_CreateChunkQuery(AB_QaLogHandler* pThis);


AB_DEF NB_Error AB_QaLogHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, AB_QaLogHandler** handler)
{
    AB_QaLogHandler* pThis = 0;
    
    if (!context || !handler)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (pThis == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;
    if (callback)
    {
        pThis->callback.callback = callback->callback;
        pThis->callback.callbackData = callback->callbackData;
    }

    pThis->query.pThis = pThis;
    pThis->query.networkQuery.action = QaLogHandler_NetworkCallback;
    pThis->query.networkQuery.progress = QaLogHandler_NetworkProgressCallback;
    pThis->query.networkQuery.qflags = 0;       // don't compress the reply since it is too small to matter
    pThis->query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(pThis->query.networkQuery.target, NB_ContextGetTargetMapping(pThis->context, "qalog-upload"), sizeof(pThis->query.networkQuery.target));
    pThis->query.networkQuery.targetlen = -1;

    pThis->uploadState = QHUS_Idle;

    *handler = pThis;

    return NE_OK;
}


AB_DEF NB_Error AB_QaLogHandlerDestroy(AB_QaLogHandler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        AB_QaLogHandlerCancelRequest(pThis);
    }

    if (pThis->network)
    {
        NB_NetworkDestroy(pThis->network);
    }

    QaLogHandler_Cleanup(pThis);

    nsl_free(pThis);

    return NE_OK;
}


AB_DEF NB_Error AB_QaLogHandlerStartRequest(AB_QaLogHandler* pThis, AB_QaLogParameters* parameters)
{
    NB_Error err = NE_OK;
    PAL_Error palErr = PAL_Ok;
    PAL_Instance* pal = 0;
    const char* filename = 0;

    if (!pThis || !parameters)
    {
        return NE_INVAL;
    }

    if (pThis->uploadState == QHUS_Upload)
    {
        return NE_BUSY;
    }

    pal = NB_ContextGetPal(pThis->context);
    filename = AB_QaLogParametersGetFilename(parameters);

    err = NB_NetworkCreate(pal, AB_QaLogParametersGetNetworkConfiguration(parameters), &pThis->network);
    if (err)
    {
        return err;
    }

    CSL_QaLogFlush(NB_ContextGetQaLog(pThis->context));
    palErr = PAL_FileGetSize(pal, filename, &pThis->fileSize);
    if (palErr)
    {
        return NE_FSYS;
    }

    palErr = PAL_FileOpen(pal, filename, PFM_Read, &pThis->file);
    if (palErr)
    {
        return NE_FSYS;
    }

    pThis->totalChunks = pThis->fileSize / UPLOAD_CHUNK_SIZE;
    if (pThis->fileSize % UPLOAD_CHUNK_SIZE)
    {
        pThis->totalChunks++;
    }

    pThis->currentChunk = 0;
    pThis->logId[0] = '\0';

    err = QaLogHandler_UploadChunk(pThis);

    return err;
}


AB_DEF NB_Error AB_QaLogHandlerCancelRequest(AB_QaLogHandler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->uploadState != QHUS_Upload)
    {
        return NE_UNEXPECTED;
    }

    pThis->uploadState = QHUS_Cancel;
    if (pThis->inProgress)
    {
        CSL_NetworkCancel(pThis->network, &pThis->query.networkQuery);
    }

    return NE_OK;
}


AB_DEF nb_boolean AB_QaLogHandlerIsRequestInProgress(AB_QaLogHandler* pThis)
{
    if (pThis)
    {
        return (nb_boolean)(pThis->uploadState == QHUS_Upload);
    }

    return FALSE;
}


AB_DEF NB_Error AB_QaLogHandlerGetInformation(AB_QaLogHandler* pThis, AB_QaLogInformation** information)
{
    NB_Error err = NE_OK;

    if (!pThis || !information)
    {
        return NE_INVAL;
    }
    
    if (nsl_strlen(pThis->logId) == 0)
    {
        return NE_UNEXPECTED;
    }

    err = AB_QaLogInformationCreate(pThis->logId, information);
    
    return err;
}

void QaLogHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
    NB_Error err = NE_OK;
    AB_QaLogHandler* pThis = ((struct QaLogHandler_Query*)query)->pThis;
    NB_NetworkRequestStatus status;
    uint32 replyerr = NE_OK;
    nb_boolean done = FALSE;

    pThis->inProgress = FALSE;

    replyerr = GetReplyErrorCode(resp);
    status  = TranslateNetStatus(resp->status);

    if (status == NB_NetworkRequestStatus_Success && resp->reply && pThis->logId[0] == '\0')
    {
        const char* logId = te_getattrc(resp->reply, "log-id");
        if (logId)
        {
            nsl_strlcpy(pThis->logId, logId, sizeof(pThis->logId));
        }
    }

    if (!err && status == NB_NetworkRequestStatus_Success)
    {
        if (pThis->uploadState == QHUS_Cancel)
        {
            status = NB_NetworkRequestStatus_Canceled;
            done = TRUE;
        }
        else if ((pThis->currentChunk + 1) < pThis->totalChunks)
        {
            // There were no errors and there are chunks left to upload
            if (pThis->callback.callback)
            {
                int percent = ((((pThis->currentChunk + 1) * UPLOAD_CHUNK_SIZE)) * 100) / pThis->fileSize;
                (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, TRUE, percent, pThis->callback.callbackData);
            }

            pThis->currentChunk++;
            err = QaLogHandler_UploadChunk(pThis);
        }
        else
        {
            done = TRUE;
        }
    }

    // Check to see it previous request failed, or next chunk upload failed
    if (done || err || status != NB_NetworkRequestStatus_Success)
    {
        pThis->uploadState = QHUS_Finished;
        if (pThis->callback.callback)
        {
            (pThis->callback.callback)(pThis, status, replyerr ? replyerr : err, FALSE, 100, pThis->callback.callbackData);
        }

        QaLogHandler_Cleanup(pThis);
    }
}


void QaLogHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    AB_QaLogHandler* pThis = ((struct QaLogHandler_Query*)query)->pThis;
    if (pThis->callback.callback)
    {
        int percent = 0;
        if (up)
        {
            percent = (((pThis->currentChunk * UPLOAD_CHUNK_SIZE) + cur) * 100) / pThis->fileSize;
        }
        else
        {
            percent = (cur * 100) / total;
        }

        (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, up, percent, pThis->callback.callbackData);
    }
}


NB_Error QaLogHandler_UploadChunk(AB_QaLogHandler* pThis)
{
    NB_Error err = NE_OK;

    if (pThis->query.networkQuery.query)
    {
        te_dealloc(pThis->query.networkQuery.query);
        pThis->query.networkQuery.query = NULL;
    }

    pThis->query.networkQuery.query = QaLogHandler_CreateChunkQuery(pThis);
    if (!pThis->query.networkQuery.query)
    {
        return NE_NOMEM;
    }
    pThis->inProgress = TRUE;
    pThis->uploadState = QHUS_Upload;
    
    CSL_NetworkRequest(pThis->network, &pThis->query.networkQuery);

    return err;
}


tpselt QaLogHandler_CreateChunkQuery(AB_QaLogHandler* pThis)
{
    tpselt te = NULL;
    uint32 bytesRead = 0;
    PAL_Error err = PAL_Ok;

    err = PAL_FileSetPosition(pThis->file, PFSO_Start, pThis->currentChunk * UPLOAD_CHUNK_SIZE);
    if (!err)
    {
        err = PAL_FileRead(pThis->file, pThis->chunkData, UPLOAD_CHUNK_SIZE, &bytesRead);
        if (!err && bytesRead != 0)
        {
            te = te_new("qalog-upload-query");
            if (te)
            {
                pThis->chunkDataLength = bytesRead;

                if (!te_setattr(te, "log-id", (const char*)pThis->logId, (size_t)nsl_strlen(pThis->logId)) ||
                    !te_setattr(te, "data", (const char*)pThis->chunkData, (size_t)bytesRead) ||
                    !te_setattru(te, "current", pThis->currentChunk) ||
                    !te_setattru(te, "total", pThis->totalChunks))
                {
                    te_dealloc(te);
                    te = NULL;
                }
            }
        }
    }

    return te;
}


void QaLogHandler_Cleanup(AB_QaLogHandler* pThis)
{
    if (pThis->query.networkQuery.query)
    {
        te_dealloc(pThis->query.networkQuery.query);
        pThis->query.networkQuery.query = 0;
    }

    if (pThis->file)
    {
        PAL_FileClose(pThis->file);
        pThis->file = 0;
    }

    pThis->logId[0] = '\0';
}


/*! @} */

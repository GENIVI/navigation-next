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

    @file abservermessageinformation.c
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
#include "abservermessageinformation.h"
#include "abservermessageinformationprivate.h"
#include "data_message_reply.h"
#include "data_message.h"
#include "data_string.h"
#include "vec.h"


#define MESSAGE_TYPE_EULA		"eula"
#define MESSAGE_TYPE_UPGRADE	"upgrade"
#define MESSAGE_TYPE_PROBE_EULA "probes_eula"
#define MESSAGE_TYPE_MOTD       "motd"

static void FreeServerMessages(AB_ServerMessageInformation* pThis);
static void FreeServerMessageVector(struct CSL_Vector* v);
static void FreeServerMessageDetail(AB_ServerMessageDetail* serverMessage);

static int CompareServerMessage(const void *a, const void *b);
static int CompareServerMessageNewestFirst(const void *a, const void *b);

static NB_Error GetDataMessage(AB_ServerMessageInformation* pThis, const char* pId, data_message* pm);
static int GetServerMessagePriority(const AB_ServerMessageDetail* pm);

static NB_Error SaveServerMessages(AB_ServerMessageInformation* pThis);
static NB_Error ConvertDataMessageToServerMessageDetail(data_util_state* pds, data_message* pdm, AB_ServerMessageDetail* pMsg);
static AB_ServerMessageNagUnit ConvertNagUnits(data_string s);


struct AB_ServerMessageInformation
{
    NB_Context*                 context;
    data_message_reply          reply;
    
    AB_ServerMessageQueryType   queryType;
    nb_boolean                  messagePending;
    
    struct CSL_Vector*          vecMessages;        /* vector of server messages, from request */
    struct CSL_Vector*          vecMessagesEnum;    /* temp vector of server messages for enum init */
    int                         currentIndex;       /* current enum index */
};


NB_Error
AB_ServerMessageInformationCreate(NB_Context* context, AB_ServerMessageQueryType queryType, tpselt reply, AB_ServerMessageInformation** information)
{
    AB_ServerMessageInformation* pThis = 0;
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

    pThis->context = context;
    pThis->queryType = queryType;
    
    switch (queryType)
    {
        case ASMQT_MessageQuery:
        case ASMQT_MessageConfirm:
            err = err ? err : data_message_reply_from_tps(NB_ContextGetDataState(pThis->context), &pThis->reply, reply);
            err = err ? err : SaveServerMessages(pThis);
            break;
        
        case ASMQT_MessageStatus:
            pThis->messagePending = (nb_boolean)(te_getchild(reply, "pending") != FALSE);
            break;
            
        default:
            err = NE_BADDATA;
            break;
    }

    if (!err)
    {
        *information = pThis;
    }
    else
    {
        AB_ServerMessageInformationDestroy(pThis);
    }

    return err;
}


AB_DEF NB_Error
AB_ServerMessageInformationDestroy(AB_ServerMessageInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }
    
    data_message_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    FreeServerMessages(pThis);

    nsl_free(pThis);        

    return NE_OK;
}


AB_DEF NB_Error
AB_ServerMessageInformationGetMessagesTimeStamp(AB_ServerMessageInformation* pThis, uint64* timeStamp)
{
    if (!pThis || !timeStamp)
    {
        return NE_INVAL;
    }

    *timeStamp = pThis->reply.ts;
    return NE_OK;
}


AB_DEF NB_Error
AB_ServerMessageInformationEnumerateInitialize(AB_ServerMessageInformation* pThis, const char* messageType, AB_ServerMessageEnumerationOrder messageOrder)
{
    NB_Error err = NE_OK;

    AB_ServerMessageDetail* serverMessage = 0;
    
    int i = 0;
    int len = 0;
    
    if (!pThis)
    {
        return NE_INVAL;
    }
    
    FreeServerMessageVector(pThis->vecMessagesEnum);
    pThis->vecMessagesEnum = NULL;

    pThis->vecMessagesEnum = CSL_VectorAlloc(sizeof(AB_ServerMessageDetail));
    if (!pThis->vecMessagesEnum)
    {
        return NE_NOMEM;
    }
    
    // copy messages from main to temporary enumlist, pruning as needed based on flags
    pThis->currentIndex = 0;

    if (pThis->vecMessages != NULL)
    {
        len = CSL_VectorGetLength(pThis->vecMessages);

        for (i = 0; i < len; i++)
        {
            serverMessage = (AB_ServerMessageDetail*)CSL_VectorGetPointer(pThis->vecMessages, i);

            if (!messageType || !messageType[0] || !nsl_stricmp(messageType, serverMessage->type))
            {
                if (!CSL_VectorAppend(pThis->vecMessagesEnum, serverMessage))
                {
                    err = NE_NOMEM;
                    break;
                }
            }
        }
    }

    if (messageOrder != ASMEO_NoSorting)
    {
        if (!CSL_VectorSort(pThis->vecMessagesEnum, (messageOrder == ASMEO_NewestFirst) ? CompareServerMessageNewestFirst : CompareServerMessage))
        {
            err = NE_NOMEM;
        }
    }

    return err;
}


AB_DEF nb_boolean
AB_ServerMessageInformationEnumerateNext(AB_ServerMessageInformation* pThis, AB_ServerMessageDetail** serverMessageInfo)
{
    nb_boolean has_next = FALSE;
    int len = 0;

    if (!pThis || !serverMessageInfo)
    {
        return FALSE;
    }

    *serverMessageInfo = NULL;

    if (pThis->vecMessagesEnum != NULL)
    {
        len = CSL_VectorGetLength(pThis->vecMessagesEnum);

        if (len > 0 && pThis->currentIndex < len)
        {
            *serverMessageInfo = (AB_ServerMessageDetail*)CSL_VectorGetPointer(pThis->vecMessagesEnum, pThis->currentIndex++);
            has_next = TRUE;
        }
    }

    return has_next;
}


AB_DEF NB_Error
AB_ServerMessageInformationEnumerateMessageText(AB_ServerMessageInformation* pThis, const char* id, AB_ServerMessageTextCallback serverMessageTextCallback, void* userData)
{
    NB_Error err = NE_OK;

    data_message dm = {{0}};
    data_format_element* pfe = 0;

    data_util_state* dataState = 0;
    
    NB_Font font = NB_Font_Normal;
    nb_color color  = MAKE_NB_COLOR(0,0,0);
    
    char empty[2] = { ' ', 0 };
    char* txt = 0;
    int i = 0;
    int len = 0;

    if (!pThis || !id || !serverMessageTextCallback)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(pThis->context);
    
    data_message_init(dataState, &dm);

    err = err ? err : GetDataMessage(pThis, id, &dm);
    if (err != NE_OK)
    {
        goto errexit;
    }

    len = CSL_VectorGetLength(dm.formatted_text.vec_format_elements);

    for (i=0; i<len; i++)
    {
        pfe = (data_format_element*) CSL_VectorGetPointer(dm.formatted_text.vec_format_elements, i);

        if (pfe != NULL)
        {
            switch (pfe->type)
            {
                case NB_FormatElement_Font:
                    err = data_format_element_get_font( pfe, &font );
                    break;
                case NB_FormatElement_Color:
                    color = pfe->color;
                    break;
                case NB_FormatElement_Text:
                    if (pfe->text != NULL)
                    {
                        txt = nsl_strdup(pfe->text);
                        if (txt != NULL)
                        {
                            serverMessageTextCallback(font, color, txt, FALSE, userData);
                        }
                        nsl_free(txt);
                    }
                    break;
                case NB_FormatElement_NewLine:
                    serverMessageTextCallback(font, color, empty, TRUE, userData);
                    break;
                default:
                    break;
            }
        }
    }

    data_message_free(dataState, &dm);

errexit:
    return err;
}


AB_DEF nb_boolean
AB_ServerMessageInformationGetMessagePending(AB_ServerMessageInformation* pThis)
{
    if (!pThis)
    {
        return FALSE;
    }

    return pThis->messagePending;
}


void
FreeServerMessages(AB_ServerMessageInformation* pThis) 
{
    FreeServerMessageVector(pThis->vecMessages);
    pThis->vecMessages = NULL;
    
    if (pThis->vecMessagesEnum)
    {
        CSL_VectorDealloc(pThis->vecMessagesEnum);
    }
    pThis->vecMessagesEnum = NULL;
}


void
FreeServerMessageVector(struct CSL_Vector* v)
{
    if (v)
    {
        int len = CSL_VectorGetLength(v);
        int i = 0;

        for (i = 0; i < len; i++)
        {
            FreeServerMessageDetail((AB_ServerMessageDetail*)CSL_VectorGetPointer(v, i));
        }
        CSL_VectorDealloc(v);
    }
}


void
FreeServerMessageDetail(AB_ServerMessageDetail* serverMessage)
{
    nsl_free(serverMessage->id);
    nsl_free(serverMessage->title);
    nsl_free(serverMessage->text);
    nsl_free(serverMessage->type);
    nsl_free(serverMessage->language);
    nsl_free(serverMessage->accept_text);
    nsl_free(serverMessage->center_text);
    nsl_free(serverMessage->decline_text);
    nsl_free(serverMessage->url);
}


int
CompareServerMessage(const void *a, const void *b)
{
    const AB_ServerMessageDetail* pA = a;
    const AB_ServerMessageDetail* pB = b;

    if (pA == NULL && pB == NULL)
        return 0;
    if (pA != NULL && pB == NULL)
        return 1;
    if (pA == NULL && pB != NULL)
        return -1;
    if (GetServerMessagePriority(pA) < GetServerMessagePriority(pB))
        return -1;
    if (GetServerMessagePriority(pA) > GetServerMessagePriority(pB))
        return 1;
    if (pA->time < pB->time)
        return -1;
    if (pA->time > pB->time)
        return 1;
    
    return 0;
}


int
CompareServerMessageNewestFirst(const void *a, const void *b)
{
    const AB_ServerMessageDetail* pA = a;
    const AB_ServerMessageDetail* pB = b;

    if (pA == NULL && pB == NULL)
        return 0;
    if (pA != NULL && pB == NULL)
        return 1;
    if (pA == NULL && pB != NULL)
        return -1;
    if (GetServerMessagePriority(pA) < GetServerMessagePriority(pB))
        return -1;
    if (GetServerMessagePriority(pA) > GetServerMessagePriority(pB))
        return 1;
    if (pA->time > pB->time)
        return -1;
    if (pA->time < pB->time)
        return 1;
    
    return 0;
}


int
GetServerMessagePriority(const AB_ServerMessageDetail* pm)
{
    // priorities:
    // 1: type == upgrade and exit_on_decline
    // 2: type == eula
    // 3: type == upgrade and !exit_on_decline
    // 4: type == traffic probe eula
    // 5: type == motd
    // 6: any other message type
    if (nsl_strcmp(pm->type, MESSAGE_TYPE_UPGRADE) == 0)
    {
        if (pm->exit_on_decline)
        {
            return 1;
        }
        else
        {
            return 3;
        }
    }
    else if (nsl_strcmp(pm->type, MESSAGE_TYPE_EULA) == 0)
    {
        return 2;
    }
    else if (nsl_strcmp(pm->type, MESSAGE_TYPE_PROBE_EULA) == 0)
    {
        return 4;
    }
    else if (nsl_strcmp(pm->type, MESSAGE_TYPE_MOTD) == 0)
    {
        return 5;
    }

    return 6;
}


NB_Error
GetDataMessage(AB_ServerMessageInformation* pThis, const char* pId, data_message* pm)
{
    NB_Error err = NE_OK;
    data_message* temp = NULL;

    int n = 0;
    int len = 0;

    if (!pThis || !pId || !pm)
    {
        return NE_INVAL;
    }
    
    len = CSL_VectorGetLength(pThis->vecMessages);
    err = NE_INVAL;

    for (n = 0; n < len; n++)
    {
        temp = (data_message*) CSL_VectorGetPointer(pThis->reply.vec_messages, n);

        if (nsl_strcmp(pId, temp->id) == 0)
        {
            err = data_message_copy(NB_ContextGetDataState(pThis->context), pm, temp);
            break;
        }
    }
    return err;
}


NB_Error
SaveServerMessages(AB_ServerMessageInformation* pThis)
{
    NB_Error err = NE_OK;
    AB_ServerMessageDetail msg = { 0 };
    data_message* pm = NULL;
    int i = 0;
    int len = 0;

    FreeServerMessages(pThis);

    pThis->vecMessages = CSL_VectorAlloc(sizeof(AB_ServerMessageDetail));
    if (pThis->vecMessages == NULL)
    {
        err = NE_NOMEM;
        goto exit;
    }

    pm = NULL;

    len = CSL_VectorGetLength(pThis->reply.vec_messages);

    for (i = 0; i < len; i++)
    {
        pm = (data_message*)CSL_VectorGetPointer(pThis->reply.vec_messages, i);

        err = err ? err : ConvertDataMessageToServerMessageDetail(NB_ContextGetDataState(pThis->context), pm, &msg);

        if (err == NE_OK && !CSL_VectorAppend(pThis->vecMessages, &msg))
        {
            err = NE_NOMEM;
            break;
        }
    }

exit:
    return err;
}

/// @todo Converting formatted text to plain text should not be done here.  The formatted text should be made available to the UI
NB_Error
ConvertFormattedTextToPlainText(data_util_state* pds, data_formatted_text* formattedText, char** plainText)
{
    int i = 0;
    int elementCount = CSL_VectorGetLength(formattedText->vec_format_elements);
    int textLength = 0;
    char* text = 0;
	data_format_element* element = 0;

    for (i = 0; i < elementCount; i++)
    {
        element = (data_format_element*)CSL_VectorGetPointer(formattedText->vec_format_elements, i);
        if (element->type == NB_FormatElement_Text)
        {
            textLength += nsl_strlen(element->text);
        }
    }

    text = nsl_malloc(textLength + 1);
    if (!text)
    {
        return NE_NOMEM;
    }
    nsl_memset(text, 0, textLength + 1);
    
    for (i = 0; i < elementCount; i++)
    {
        element = (data_format_element*)CSL_VectorGetPointer(formattedText->vec_format_elements, i);
        if (element->type == NB_FormatElement_Text)
        {
            nsl_strcat(text, element->text);
        }
    }
    *plainText = text;

    return NE_OK;
}


NB_Error
ConvertDataMessageToServerMessageDetail(data_util_state* pds, data_message* pdm, AB_ServerMessageDetail* pMsg)
{
    NB_Error err = NE_OK;

    nsl_memset(pMsg, 0, sizeof(*pMsg));

    pMsg->id = nsl_strdup(pdm->id); 
    pMsg->title = nsl_strdup(pdm->title);
    pMsg->type = nsl_strdup(pdm->type);
    pMsg->language = nsl_strdup(pdm->language);
    pMsg->accept_text = nsl_strdup(pdm->accept_text);
    pMsg->center_text = nsl_strdup(pdm->center_text);
    pMsg->decline_text = nsl_strdup(pdm->decline_text);

    if (pdm->url.value)     /* prevent passing NULL to nsl_strdup */
    {
        pMsg->url = nsl_strdup(pdm->url.value);
    }

    if (pMsg->id == NULL || pMsg->title == NULL ||pMsg->type == NULL || pMsg->language == NULL ||
            pMsg->accept_text == NULL || pMsg->center_text == NULL || pMsg->decline_text == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    err = ConvertFormattedTextToPlainText(pds, &pdm->formatted_text, &pMsg->text);
    if (err)
    {
        goto errexit;
    }

    pMsg->confirm = pdm->confirm;
    pMsg->exit_on_decline = pdm->exit_on_decline;
    
    pMsg->message_nag.freq_unit = ConvertNagUnits(pdm->message_nag.freq_unit);
    pMsg->message_nag.freq_count = pdm->message_nag.freq_count;
    pMsg->message_nag.expire_unit = ConvertNagUnits(pdm->message_nag.expire_unit);
    pMsg->message_nag.expire_count = pdm->message_nag.expire_count;
    pMsg->message_nag.expire_date = pdm->message_nag.expire_date;

    pMsg->time = pdm->time;

errexit:
    if (err != NE_OK)
    {
        FreeServerMessageDetail(pMsg);
        nsl_memset(pMsg, 0, sizeof(*pMsg));
    }

    return err;
}


AB_ServerMessageNagUnit
ConvertNagUnits(data_string s)
{
    if (nsl_strcmp(s, "uses") == 0)
    {
        return ASNU_Uses;
    }	
    else if (nsl_strcmp(s, "days") == 0)
    {
        return ASNU_Days;
    }	
    else if (nsl_strcmp(s, "seconds") == 0)
    {
        return ASNU_Seconds;
    }	
    else
    {
        return ASNU_Uses;
    }
}


/*! @} */

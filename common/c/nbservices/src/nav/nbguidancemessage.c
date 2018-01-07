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

    @file     nbguidanceinfo.c
    @defgroup nbguidanceinfo

    Information about guidance
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbguidancemessage.h"
#include "nbguidancemessageprivate.h"
#include "nbnavigationstateprivate.h"
#include "instructset.h"



struct NB_GuidanceMessage
{
    InstructionCodes codes;

    NB_NavigateAnnouncementSource       source;
    NB_GuidanceMessageType              type;
    NB_NavigateManeuverPos              pos;

    NB_GuidanceMessageCallbackFunction  playedCallback;
    void*                               playedCallbackUserData;

    NB_GuidanceMessageCallbackFunction  qaLogPlayedCallback;
    void*                               qaLogPlayedCallbackUserData;

    double                              playbackTime;
};

NB_DEF NB_Error 
NB_GuidanceMessageCreate(NB_Context* context, NB_NavigateAnnouncementSource source, NB_GuidanceMessageType type, NB_GuidanceMessage** message)
{
    NB_GuidanceMessage* pThis = NULL;
    
    if (!context)
    {
        return NE_INVAL;
    }

    if ((pThis = nsl_malloc(sizeof(NB_GuidanceMessage))) == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(NB_GuidanceMessage));

    // initialize the codes array
    (void)instructset_get_instruction_array(&pThis->codes);

    pThis->source = source;
    pThis->type = type;
    
    pThis->playedCallback = NULL;
    pThis->playedCallbackUserData = NULL;

    pThis->qaLogPlayedCallback = NULL;
    pThis->qaLogPlayedCallbackUserData = NULL;

    (*message)  = pThis;

    return NE_OK;
}

NB_DEF InstructionCodes*
NB_GuidanceMessageGetInstructionCodes(NB_GuidanceMessage* message)
{
    if (!message)
    {
        return NULL;
    }

    return &message->codes;
}

NB_DEF NB_NavigateAnnouncementSource
NB_GuidanceMessageGetSource(NB_GuidanceMessage* message)
{
    return message ? message->source : NB_NAS_Automatic;
}

NB_GuidanceMessage*
NB_GuidanceMessageClone(NB_GuidanceMessage* message)
{
    NB_GuidanceMessage* pThis = NULL;

    if ((pThis = nsl_malloc(sizeof(NB_GuidanceMessage))) == NULL)
    {
        return NULL;
    }

    nsl_memcpy(pThis, message, sizeof(NB_GuidanceMessage));
    if(message->qaLogPlayedCallbackUserData)
    {
        pThis->qaLogPlayedCallbackUserData = nsl_malloc(sizeof(message->qaLogPlayedCallbackUserData));
        nsl_memcpy(pThis->qaLogPlayedCallbackUserData,message->qaLogPlayedCallbackUserData,sizeof(message->qaLogPlayedCallbackUserData));
    }
    // initialize the codes array
    (void)instructset_get_instruction_array(&pThis->codes);

    return pThis;
}

NB_DEF NB_Error 
NB_GuidanceMessageDestroy(NB_GuidanceMessage* message )
{
    if (!message)
    {
        return NE_INVAL;
    }
    if(message->qaLogPlayedCallbackUserData)
    {
        nsl_free(message->qaLogPlayedCallbackUserData);
    }
    nsl_free(message);

    return NE_OK;
}

NB_DEF int 
NB_GuidanceMessageGetCodeCount(NB_GuidanceMessage* message)
{
    if (!message)
    {
        return 0;
    }

    return message->codes.count;
}

NB_DEF const char*
NB_GuidanceMessageGetCode(NB_GuidanceMessage* message, int index)
{
    if (!message)
    {
        return NULL;
    }

    if (index < 0 || index >= message->codes.count)
    {
        return NULL;
    }

    return message->codes.pointerArray[index];
}

NB_Error
NB_GuidanceMessageSetQaLogPlayedCallback(NB_GuidanceMessage* message, NB_GuidanceMessageCallbackFunction playedCallback, void* playedCallbackUserData)
{
    if (!message)
    {
        return NE_INVAL;
    }

    message->qaLogPlayedCallback = playedCallback;
    message->qaLogPlayedCallbackUserData = playedCallbackUserData;

    return NE_OK;
}

NB_Error
NB_GuidanceMessageSetPlayedCallback(NB_GuidanceMessage* message, NB_GuidanceMessageCallbackFunction playedCallback, void* playedCallbackUserData)
{
    if (!message)
    {
        return NE_INVAL;
    }

    message->playedCallback = playedCallback;
    message->playedCallbackUserData = playedCallbackUserData;
    
    return NE_OK;
}

NB_DEF void 
NB_GuidanceMessagePlayed(NB_GuidanceMessage* message)
{
    if (!message)
    {
        return;
    }

    if (message->qaLogPlayedCallback != NULL)
    {
        (message->qaLogPlayedCallback)(message, message->qaLogPlayedCallbackUserData);
        message->qaLogPlayedCallbackUserData = NULL;
    }
    if (message->playedCallback != NULL)
    {
        (message->playedCallback)(message, message->playedCallbackUserData);
    }
}

NB_GuidanceMessageType
NB_GuidanceMessageGetType(NB_GuidanceMessage* message)
{
    return message ? message->type : NB_GMT_None;
}

InstructionCodes*
NB_GuidanceMessageClearInstructionCodes(NB_GuidanceMessage* message)
{
    if (!message)
    {
        return NULL;
    }

    nsl_memset(&message->codes.codes, 0, sizeof(message->codes.codes));
    message->codes.count = 0;
    message->playbackTime = 0.0;
    return &message->codes;
}

double
NB_GuidanceMessageGetPlaybackTime(NB_GuidanceMessage* message)
{
    if (message)
    {
        return message->playbackTime;
    }
    return 0.0;
}

NB_Error
NB_GuidanceMessageSetPlaybackTime(NB_GuidanceMessage* message, double playbackTime)
{
    if (!message)
    {
        return NE_INVAL;
    }
    message->playbackTime = playbackTime;

    return NE_OK;
}

NB_Error
NB_GuidanceMessageSetManeuverPos(NB_GuidanceMessage* message, NB_NavigateManeuverPos pos)
{
    if (!message)
    {
        return NE_INVAL;
    }
    message->pos = pos;

    return NE_OK;
}

NB_NavigateManeuverPos
NB_GuidanceMessageGetManeuverPos(NB_GuidanceMessage* message)
{
    if (!message)
    {
        return NE_INVAL;
    }
    return message->pos;
}

/*! @} */

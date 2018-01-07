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

    @file     palimpl.c
    @date     02/03/2012
    @defgroup PALIMPL_H PAL Pal Functions

    Provides a platform-dependent Pal implementation.

    Qt QNX implementation for file.
*/
/*
    See file description in header file.

    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "paltypes.h"
#include "palstdlib.h"
#include "palimpl.h"
#include "taskmanagermultithreaded.h"

PAL_DEF PAL_Instance*
PAL_Create(PAL_Config *conf)
{
    PAL_Instance *pal = NULL;

    if (conf == NULL)
    {
        return (NULL);
    }

    pal = (PAL_Instance*) nsl_malloc(sizeof(*pal));
    if (pal == NULL)
    {
        return (NULL);
    }

    nsl_memset(pal, 0, sizeof(*pal));

    pal->config = *conf;

    // @todo This should be changed in the future.
    //       Currently we just use this method to get work path.
    extern char* g_workpath;
    g_workpath = strdup(conf->workpath);

    TaskManagerMultiThreaded::CreateTaskManager(pal, &(pal->taskManager));

    if (pal->taskManager == NULL)
    {
        PAL_Destroy(pal);
        return NULL;
    }

    pal->functionManager = new FinishFunctionManager(pal);
    if (pal->functionManager == NULL)
    {
        PAL_Destroy(pal);
        return NULL;
    }

    pal->timerManager = TimerManagerCreate(pal);
    if (pal->timerManager == NULL)
    {
        PAL_Destroy(pal);
        return (NULL);
    }

    if (PAL_EventCreate(pal, &pal->destroyEvent) != PAL_Ok)
    {
        PAL_Destroy(pal);
        return NULL;
    }

    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "PAL instance is created");

    return (pal);
}

PAL_DEF void
PAL_Destroy(PAL_Instance *pal)
{
    if (pal == NULL)
    {
        return;
    }

    if (pal->taskManager)
    {
        if (!pal->taskManager->Destroy())
        {
            PAL_EventWaitForEvent(pal->destroyEvent);
        }
    }

    if (pal->timerManager)
    {
        TimerManagerDestroy(pal->timerManager);
        pal->timerManager = NULL;
    }

    if (pal->functionManager)
    {
        delete pal->functionManager;
    }

    if (pal->taskManager)
    {
        if (!pal->taskManager->Destroy())
        {
            PAL_EventWaitForEvent(pal->destroyEvent);
        }
        delete pal->taskManager;
        pal->taskManager = NULL;
    }

    if (pal->destroyEvent)
    {
        PAL_EventDestroy(pal->destroyEvent);
        pal->destroyEvent = NULL;
    }

    nsl_free(pal);

    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "PAL instance is destroyed");
}

PAL_DEF nb_boolean
PAL_IsPalThread(PAL_Instance* pal)
{
    return (nb_boolean)(pal && pal->taskManager->GetEventThreadId() == (nb_threadId)pthread_self());
}

PAL_DEF PAL_Error
PAL_ConfigGet(PAL_Instance* pal, PAL_Config* config)
{
    if ((pal == NULL) || (config == NULL))
    {
        return PAL_ErrBadParam;
    }

    *config = pal->config;
    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_ConfigSet(PAL_Instance* pal, PAL_Config* config)
{
    if ((pal == NULL) || (config == NULL))
    {
        return PAL_ErrBadParam;
    }

    pal->config = *config;
    return PAL_Ok;
}

PAL_DEF nb_boolean
PAL_IsSimulator(PAL_Instance* pal)
{
    return FALSE;
}

PAL_DEF nb_boolean
PAL_IsNetworkBlocked(PAL_Instance* pal)
{
    return FALSE;
}

PAL_DEF PAL_Error
PAL_GetLastError(PAL_Instance* pal)
{
    if (pal)
    {
        return pal->lastError;
    }

    return PAL_Ok;
}

PAL_DEF void
PAL_SetLastError(PAL_Instance* pal, PAL_Error err)
{
    if (pal)
    {
        pal->lastError = err;
    }
}

void PAL_ScheduleDestroy(PAL_Instance* pal)
{
    if (!pal)
    {
        return;
    }
    PAL_EventSet(pal->destroyEvent);
}

void PAL_SetIsBackground(PAL_Instance* pal, nb_boolean isBackground)
{
    if (pal)
    {
        pal->isBackground = isBackground;
    }
}


nb_boolean PAL_IsBackground(PAL_Instance* pal)
{
    if (pal)
    {
        return pal->isBackground;
    }
    return FALSE;
}

PAL_DEF const char* PAL_GetLocale(PAL_Instance* pal)
{
    if (pal)
    {
        if ( strlen(pal->language) == 0)
            return "en-US";
        else
            return pal->language;
    }
    return "";
}
/* See header file for description. */
PAL_DEF const char* PAL_GetISO3CountryCode(PAL_Instance* pal, const char* locale)
{
    if (!locale)
    {
        return "";
    }
    else if (nsl_strcmp(locale, "en-US") == 0)
    {
        return "USA";
    }
    else if (nsl_strcmp(locale, "es-MX") == 0)
    {
        return "MEX";
    }
    else if (nsl_strcmp(locale, "es-ES") == 0)
    {
        return "ESP";
    }
    else if (nsl_strcmp(locale, "fr-CA") == 0)
    {
        return "CAN";
    }
    else if (nsl_strcmp(locale, "fr-FR") == 0)
    {
        return "FRA";
    }
    else if (nsl_strcmp(locale, "it-IT") == 0)
    {
        return "ITA";
    }
    else if (nsl_strcmp(locale, "de-DE") == 0)
    {
        return "DEU";
    }
    else if (nsl_strcmp(locale, "nl-NL") == 0)
    {
        return "NLD";
    }

    return "";
}

PAL_DEF PAL_Error
PAL_GenerateUUID(PAL_Instance* pal, uint8* buffer, uint32 bufferSize, uint32* bytesRead)
{
    return PAL_ErrUnsupported;
}

/*! @} */

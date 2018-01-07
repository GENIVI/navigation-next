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
* Copyright (c) 2008 Networks In Motion, Inc.
* All rights reserved. This file and associated materials are the
* trade secrets, confidential information, and copyrighted works of
* Networks In Motion, Inc.
*
* This intellectual property is for the internal use only by Networks
* In Motion, Inc. This source code contains proprietary information
* of Networks In Motion, Inc., and shall not be used, copied by, or
* disclosed to, anyone other than a Networks In Motion, Inc.,
* certified employee that has written authorization to view or modify
* said source code.
*/

#include "paltypes.h"
#include "palstdlib.h"
#include "palimpl.h"
#include "timerimpl.h"
#include "wm_global.h"
#include "palfile.h"
#include "pallock.h"
#include "taskmanagersinglethreaded.h"
#include "taskmanagermultithreaded.h"
#include "paldisplay.h"

#define SPI_GETPLATFORMNAME 260
#define NB_WM_DEVICE_PLATFORM_NAME_LEN 256

using namespace nimpal;

PAL_Instance*
PAL_Create(PAL_Config* conf)
{
    PAL_Instance* pal = NULL;
    if (conf == NULL)
    {
        return NULL;
    }

    pal = static_cast<PAL_Instance*>( nsl_malloc(sizeof(PAL_Instance)) );
    if (pal == NULL)
    {
        return NULL;
    }
    nsl_memset(pal, 0, sizeof(PAL_Instance));
    pal->config = *conf;
    pal->multiThreaded = conf->multiThreaded;
    if (conf->multiThreaded == FALSE)
    {
        TaskManagerSingleThreaded::CreateTaskManager(pal,&(pal->taskManager));
    }
    else
    {
        TaskManagerMultiThreaded::CreateTaskManager(pal, &(pal->taskManager));
    }

    if ((pal->taskManager == NULL))
    {
        PAL_Destroy(pal);
        return NULL;
    }

    pal->functionManager = new FinishFunctionManager(pal);
    if (pal->functionManager == NULL)
    {
        goto errexit;
    }

    PAL_DisplayInitialize(pal);

    timer_ctor(pal);

    PAL_EventCreate(pal, &pal->destroyEvent);

    Set_PAL_NBI_InstPointer(pal);

    pal->lastError = PAL_Ok;

    return (pal);

errexit:
    PAL_Destroy(pal);
    return NULL;
}

void
PAL_Destroy(PAL_Instance* pal)
{
    if (!pal)
    {
        return;
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

    if (pal->timer)
    {
        timer_dtor(pal);
    }

    Set_PAL_NBI_InstPointer(NULL);

    if (pal->functionManager)
    {
        delete pal->functionManager;
    }

    if (pal->destroyEvent)
    {
        PAL_EventDestroy(pal->destroyEvent);
    }

    nsl_free(pal);
}

nb_boolean PAL_IsPalThread(PAL_Instance* pal)
{
    if (pal && pal->multiThreaded)
    {
        return (nb_boolean)(pal->taskManager->GetEventThreadId() == GetCurrentThreadId());
    }
    else
    {
        return TRUE;
    }
}

PAL_Error
PAL_ConfigGet(PAL_Instance* pal, PAL_Config* config)
{
    if ((pal == NULL) || (config == NULL))
    {
        return PAL_ErrBadParam;
    }

    *config = pal->config;
    return PAL_Ok;
}

PAL_Error
PAL_ConfigSet(PAL_Instance* pal, PAL_Config* config)
{
    return PAL_ErrUnsupported;
}

nb_boolean
PAL_AssertPalThread(PAL_Instance* pal)
{
    nb_boolean isPalThread = FALSE;
    if (pal)
    {
        isPalThread = PAL_IsPalThread(pal);
        nsl_assert(isPalThread);
    }

    return isPalThread;
}

nb_boolean PAL_IsSimulator(PAL_Instance* pal)
{
#ifdef WINCE
    wchar_t* name = new wchar_t[NB_WM_DEVICE_PLATFORM_NAME_LEN];
    memset(name, 0, sizeof(wchar_t) * NB_WM_DEVICE_PLATFORM_NAME_LEN);

    BOOL rc = SystemParametersInfo(SPI_GETPLATFORMNAME, NB_WM_DEVICE_PLATFORM_NAME_LEN, name, 0);

    if (rc)
    {
        rc = (uint8)(wcscmp(name, L"DeviceEmulator") == 0);
    }

    delete[] name;

    return (nb_boolean)(rc);
#else
    return (nb_boolean)(TRUE);
#endif
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

/* See header file for description. */
PAL_DEF nb_boolean PAL_IsNetworkBlocked(PAL_Instance* pal)
{
    return FALSE;
}

void PAL_ScheduleDestroy(PAL_Instance* pal)
{
    if (!pal)
    {
        return;
    }
    PAL_EventSet(pal->destroyEvent);
}


/* See header file for description. */
PAL_DEF const char* PAL_GetLocale(PAL_Instance* pal)
{
    LANGID langId = GetSystemDefaultUILanguage();
    WORD primaryLang = (WORD)(PRIMARYLANGID(langId));
    WORD subLang = (WORD)(SUBLANGID(langId));

    if (primaryLang == LANG_SPANISH && subLang == SUBLANG_SPANISH_MEXICAN)
    {
        return "es-MX";
    }
    // use for all rest sublanguages
    else if (primaryLang == LANG_SPANISH)
    {
        return "es-ES";
    }
    else if (primaryLang == LANG_FRENCH && subLang == SUBLANG_FRENCH_CANADIAN)
    {
        return "fr-CA";
    }
    // use for all rest sublanguages
    else if (primaryLang == LANG_FRENCH)
    {
        return "fr-FR";
    }
    else if (primaryLang == LANG_ITALIAN)
    {
        return "it-IT";
    }
    else if (primaryLang == LANG_GERMAN)
    {
        return "de-DE";
    }
    else if (primaryLang == LANG_DUTCH)
    {
        return "nl-NL";
    }
    else
    {
        // return by default
        return "en-US";
    }
}
/* See header file for description. */
PAL_DEF const char* PAL_GetISO3CountryCode(PAL_Instance* pal, const char* locale)
{
    // Appropriate API is not found from WinAPI
    // Codes are obtained from http://en.wikipedia.org/wiki/ISO_3166-1_alpha-3
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

void PAL_SetIsBackground(PAL_Instance* pal, nb_boolean isBackground)
{
    // return PAL_ErrUnsupported;
}

nb_boolean PAL_IsBackground(PAL_Instance* pal)
{
    return FALSE;
}

PAL_DEF nb_boolean
PAL_IsRenderThread(PAL_Instance* pal)
{
    if (pal && (pal->taskManager->GetWorkerThreadIdByName("nbgmrendering") == GetCurrentThreadId()))
    {
        return TRUE;
    }

    return FALSE;
}

PAL_DEF PAL_Error
PAL_GenerateUUID(PAL_Instance* pal, uint8* buffer, uint32 bufferSize, uint32* bytesRead)
{
    return PAL_ErrUnsupported;
}

PAL_DEF
nb_boolean PAL_IsSingleThreaded(PAL_Instance* pal)
{
    if (pal)
    {
        return pal->multiThreaded ? FALSE : TRUE;
    }
    return FALSE;
}

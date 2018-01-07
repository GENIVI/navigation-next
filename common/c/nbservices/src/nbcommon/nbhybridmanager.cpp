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

    @file     nbhybridmanager.cpp
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbhybridmanager.h"
#include "commoncomponentconfiguration.h"
#include "ncdbobjectsmanager.h"
#include "HybridManager.h"

static Ncdb::SupportedLanguages ConvertStr2Lang(const char* lang);

using namespace nbcommon;

NB_DEF NB_Error
NB_HybridManagerSetStrategy(NB_Context* context, HybridStrategy* strategy)
{
    if(!context)
    {
        return NE_INVAL;
    }

    shared_ptr<HybridStrategy> sharedStrategy(strategy);
    HybridManager::GetInstance(context)->SetHybridStrategy(sharedStrategy);
    return NE_OK;
}

NB_DEF NB_Error
NB_HybridManagerSetLanguage(NB_Context* context, const char* lang)
{
    if (!context || !lang)
    {
        return NE_INVAL;
    }
    NcdbObjectsManager* ncdbObjManager = ContextBasedSingleton<nbcommon::NcdbObjectsManager>::getInstance(context).get();
    Ncdb::Session* ncdbSession = ncdbObjManager->GetNcdbSession().get();
    ncdbSession->SetLanguage();

    return NE_INVAL;
}

Ncdb::SupportedLanguages ConvertStr2Lang(const char* lang)
{
    // TODO: add other languages
    if (nsl_stricmp(lang, "en-us") == 0)
    {
        return Ncdb::US_English;
    }
    else if (nsl_strnicmp(lang, "es-", 3) == 0)
    {
        return Ncdb::Spanish;
    }
    else if (nsl_strnicmp(lang, "en-", 3) == 0)
    {
        return Ncdb::US_English;
    }
    else if (nsl_stricmp(lang, "zh-cn") == 0)
    {
        return Ncdb::Chinese;
    }

    return Ncdb::Invalid_Language;
}

NB_DEF NB_Error
NB_HybridManagerAddMapDataPath(NB_Context* context, const char* mapDataPath)
{
    if (!context || !mapDataPath)
    {
        return NE_INVAL;
    }

    NcdbObjectsManager* ncdbObjManager = ContextBasedSingleton<nbcommon::NcdbObjectsManager>::getInstance(context).get();
    ncdbObjManager->AddMapdataPath(mapDataPath);

    return NE_INVAL;
}

NB_DEC NB_Error
NB_HybridManagerAddWorldMapDataPath(NB_Context* context, const char* mapDataPath){
    if (!context || !mapDataPath)
    {
        return NE_INVAL;
    }

    NcdbObjectsManager* ncdbObjManager = ContextBasedSingleton<nbcommon::NcdbObjectsManager>::getInstance(context).get();
    ncdbObjManager->AddWorldMapdataPath(mapDataPath);

    return NE_INVAL;

}

NB_DEF NB_Error
NB_HybridManagerBeginLoadMapData(NB_Context* context)
{
    if (!context)
    {
        return NE_INVAL;
    }
    NcdbObjectsManager* ncdbObjManager = ContextBasedSingleton<nbcommon::NcdbObjectsManager>::getInstance(context).get();
    ncdbObjManager->BeginLoadMapdata();
    return NE_OK;
}

NB_DEF NB_Error
NB_HybridManagerEndLoadMapData(NB_Context* context)
{
    if (!context)
    {
        return NE_INVAL;
    }
    NcdbObjectsManager* ncdbObjManager = ContextBasedSingleton<nbcommon::NcdbObjectsManager>::getInstance(context).get();
    ncdbObjManager->EndLoadMapdata();
    return NE_OK;
}

NB_DEC const char *
NB_HybridManagerGetMapDataVersion(NB_Context* context)
{
    if (!context)
    {
        return "";
    }
    NcdbObjectsManager* ncdbObjManager = ContextBasedSingleton<nbcommon::NcdbObjectsManager>::getInstance(context).get();
    return ncdbObjManager->GetMapVersionString();
}

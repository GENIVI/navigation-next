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

@file     nbhybridmanager.h

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

#ifndef NBHYRBIDMANAGER_H
#define NBHYRBIDMANAGER_H

#include "nbcontext.h"
#include "HybridStrategy.h"
/*!
    @addtogroup nbhybridmanager
    @{
*/

/*! Set strategy for HybridManager.

    Onboard mode cannot be enabled if PAL is single-threaded. Onboard features require worker threads
    for correct processing.

@param context - A pointer to current context instance.
@param strategy - Callback function to choose which mode we select.
                  TRUE - enable onboard mode,
                  FALSE - enable offboard mode.
@returns NB_Error
*/
NB_DEC NB_Error
NB_HybridManagerSetStrategy(NB_Context* context, nbcommon::HybridStrategy* strategy);

/*! Set NCDB data language
 
 @param context A pointer to current context instance.
 @param lang  A string contained language code, for example 'en-us'(US English), 'es-ar'(Argentina Spanish).
              Country codes - ISO 3166, language codes - ISO 639-1.
 @returns NB_Error
 */
NB_DEC NB_Error
NB_HybridManagerSetLanguage(NB_Context* context, const char* lang);

/*! @} */

/*! Add onboard map data path

@param context - A pointer to current context instance.
@param mapConfig - full path to map config file.

@returns NB_Error
*/
NB_DEC NB_Error
NB_HybridManagerAddMapDataPath(NB_Context* context, const char* mapDataPath);

NB_DEC NB_Error
NB_HybridManagerAddWorldMapDataPath(NB_Context* context, const char* mapDataPath);

NB_DEC NB_Error
NB_HybridManagerBeginLoadMapData(NB_Context* context);
NB_DEC NB_Error
NB_HybridManagerEndLoadMapData(NB_Context* context);

NB_DEC  const char*
NB_HybridManagerGetMapDataVersion(NB_Context* context);

#endif // NBHYRBIDMANAGER_H

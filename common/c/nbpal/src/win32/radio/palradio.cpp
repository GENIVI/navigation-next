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

/*--------------------------------------------------------------------------

(C) Copyright 2011 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif 

#include "palradio.h"
#include "palstdlib.h"

typedef struct PAL_RadioContext
{
    PAL_Instance* pal;
} PAL_RadioContext;

PAL_DEF PAL_Error PAL_RadioContextCreate(PAL_Instance* pal, PAL_RadioServiceType type, PAL_RadioContext** radioContext)
{
    PAL_RadioContext* context = NULL;

    if (pal == NULL || radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }

    context = (PAL_RadioContext*)nsl_malloc(sizeof(PAL_RadioContext));

    if (context == NULL)
    {
        return PAL_ErrNoMem;
    }

    *radioContext = context;

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_RadioContextDestroy(PAL_RadioContext* radioContext)
{
    if (radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }

    nsl_free(radioContext);

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_RadioGetSystemMode(PAL_RadioContext* radioContext, PAL_RadioSystemModeCallback callback, void* userData)
{
    if (radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }
    return PAL_ErrUnsupported;
}

PAL_DEF PAL_Error PAL_RadioGetGSMNetworkInfo(PAL_RadioContext* radioContext, PAL_RadioGSMNetworkInfoCallback callback, void* userData)
{
    if (radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }
    return PAL_ErrUnsupported;
}

PAL_DEF PAL_Error PAL_RadioGetCDMANetworkInfo(PAL_RadioContext* radioContext, PAL_RadioCDMANetworkInfoCallback callback, void* userData)
{
    return PAL_ErrUnsupported;
}

PAL_DEF PAL_Error PAL_RadioGetWifiNetworkInfo(PAL_RadioContext* radioContext, PAL_RadioWifiNetworkInfoCallback callback, void* userData)
{
    if (radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }
    return PAL_ErrUnsupported;
}

PAL_DEF PAL_Error PAL_RadioGetAllWifiNetworksInfo(PAL_RadioContext* radioContext, PAL_RadioAllWifiNetworksInfoCallback callback, void* userData)
{
    if (radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }
    return PAL_ErrUnsupported;
}

PAL_DEF PAL_Error PAL_RadioCancelWifiNetworkInfo(PAL_RadioContext* radioContext)
{
    if (radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }
    return PAL_ErrUnsupported;
}

PAL_DEF PAL_Error PAL_RadioCancelAllWifiNetworksInfo(PAL_RadioContext* radioContext,
                                                     PAL_RadioAllWifiNetworksInfoCallback callback)
{
    if (radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }
    return PAL_ErrUnsupported;
}

#ifdef __cplusplus
 }
#endif 

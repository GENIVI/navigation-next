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

    @file     riltypes.h
    @date     4/27/2010
    @defgroup PAL_RADIO PAL Radio Functions

    Platform-independent radio API.

    The RIL types definition. "ril.h" is not included to WM SDK.
*/
/*
    (C) Copyright 2010 by TeleCommunication Systems, Inc.
                                                                  
    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef RILTYPES_H
#define RILTYPES_H

#include <windef.h>
#include "pal.h"

#define RIL_RESULT_OK 0x00000001 // RIL API call succeded
typedef void* HRIL;

// http://msdn.microsoft.com/en-us/library/aa922465.aspx
typedef void (CALLBACK *RILNOTIFYCALLBACK)(
    DWORD dwCode,
    const void* lpData,
    DWORD cbData,
    DWORD dwParam
    );

// http://msdn.microsoft.com/en-us/library/aa920069.aspx
typedef void (CALLBACK *RILRESULTCALLBACK)(
    DWORD dwCode,
    HRESULT hrCmdID,
    const void* lpData,
    DWORD cbData,
    DWORD dwParam
    );

// http://msdn.microsoft.com/en-us/library/aa921533.aspx
#define MAXLENGTH_BCCH  48
#define MAXLENGTH_NMR   16
typedef struct RILCELLTOWERINFO {
    DWORD cbSize;                      /* Structure size, in bytes. */
    DWORD dwParams;                    /* Specifies valid parameters. Must be one or a combination of the RILCELLTOWERINFO parameter constants. */
    DWORD dwMobileCountryCode;         /* Specifies the country/region code. */
    DWORD dwMobileNetworkCode;         /* Specifies the code of the mobile network. */
    DWORD dwLocationAreaCode;          /* Specifies the area code of the current location. */
    DWORD dwCellID;                    /* Specifies the ID of the cellular tower. */
    DWORD dwBaseStationID;             /* Specifies the ID of the base station. */
    DWORD dwBroadcastControlChannel;   /* Specifies the Broadcast Control Channel (BCCH). */
    DWORD dwRxLevel;                   /* Specifies the received signal level. */
    DWORD dwRxLevelFull;               /* Specifies the received signal level in the full network. */
    DWORD dwRxLevelSub;                /* Specifies the received signal level in the subsystem. */
    DWORD dwRxQuality;                 /* Specifies the received signal quality. */
    DWORD dwRxQualityFull;             /* Specifies the received signal quality in the full network. */
    DWORD dwRxQualitySub;              /* Specifies the received signal quality in the subsystem. */
    DWORD dwIdleTimeSlot;              /* Specifies the idle timeslot. */
    DWORD dwTimingAdvance;             /* Specifies the timing advance. */
    DWORD dwGPRSCellID;                /* Specifies the ID of the GPRS cellular tower. */
    DWORD dwGPRSBaseStationID;         /* Specifies the ID of the GPRS base station. */
    DWORD dwNumBCCH;                   /* Specifies the number of the BCCH. */
    BYTE rgbBCCH[MAXLENGTH_BCCH];      /* Range of the BCCH, in bytes. */
    BYTE rgbNMR[MAXLENGTH_NMR];        /* Length of the Non-Membership Report (NMR), in bytes. */
} RILCELLTOWERINFO;

// http://msdn.microsoft.com/en-us/library/aa921978.aspx
enum RILSYSTEMTYPES {
    RIL_SYSTEMTYPE_NONE         = 0,      /* No Networks in Coverage. */
    RIL_SYSTEMTYPE_IS95A        = 0x001,  /* IS-95A network support (Low Packet, or Circuit Switched Service). */
    RIL_SYSTEMTYPE_IS95B        = 0x002,  /* IS-95B network support. */
    RIL_SYSTEMTYPE_1XRTTPACKET  = 0x004,  /* CDMA-2000 Rev A (1xRTT) network support. */
    RIL_SYSTEMTYPE_GSM          = 0x008,  /* GSM network support. */
    RIL_SYSTEMTYPE_GPRS         = 0x010,  /* GPRS support. */
    RIL_SYSTEMTYPE_EDGE         = 0x020,  /* GSM EDGE network support. */
    RIL_SYSTEMTYPE_1XEVDOPACKET	= 0x040,  /* CDMA (1xEVDO) network support. */
    RIL_SYSTEMTYPE_1XEVDVPACKET	= 0x080,  /* CDMA (1xEVDV) network support. */
    RIL_SYSTEMTYPE_UMTS         = 0x100,  /* UMTS network support. */
    RIL_SYSTEMTYPE_HSDPA        = 0x200   /* HSDPA support. */
};
 
// ril.dll routines
typedef HRESULT (*PFRIL_Initialize)(DWORD dwIndex, RILRESULTCALLBACK pfnResult, RILNOTIFYCALLBACK pfnNotify,
                    DWORD dwNotificationClasses, DWORD dwParam, HRIL *phRil);
typedef HRESULT (*PFRIL_Deinitialize)(HRIL hRil);
typedef HRESULT (*PFRIL_GetCellTowerInfo)(HRIL hRil);
typedef HRESULT	(*PFRIL_GetCurrentSystemType)(HRIL hRil);

#endif

/*! @} */

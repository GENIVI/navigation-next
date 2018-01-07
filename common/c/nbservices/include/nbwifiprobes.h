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

    @file     nbwifiprobes.h
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

#ifndef NBWIFIPROBES_H_
#define NBWIFIPROBES_H_

#include "nbcontext.h"
#include "nbgpstypes.h"
#include "navpublictypes.h"
#include "nbplace.h"
#include "palradio.h"

/*!
    @addtogroup nbwifiprobes
    @{
*/

/*! @struct NB_WifiProbes
WifiProbes object
*/
typedef struct NB_WifiProbes NB_WifiProbes;


/*! Create an WifiProbes object

@param context NB_Context
@param radioContext PAL_RadioContext Radio context is necessary
       to recevie wi-fi networks info.
@param wifiProbes On success, pointer to NB_WifiProbes object returned.
@return NB_Error
*/
NB_DEC NB_Error NB_WifiProbesCreate(NB_Context* context, PAL_RadioContext* radioContext, NB_WifiProbes** wifiProbes);

/*! Destroy a previously created WifiProbes object

@param context NB_Context
@param wifiProbes A WifiProbes object created with NB_WifiProbesCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_WifiProbesDestroy(NB_WifiProbes* wifiProbes);

/*! Add Wi-Fi probe event

Add wi-fi probes to Device Analytics

@param context NB_Context
@param location NB_GpsLocation Current gps location
@returns NB_Error
*/
NB_DEC NB_Error NB_WifiProbesAddWifiProbe(NB_Context* context,
        NB_GpsLocation* location);

/* @} */
#endif
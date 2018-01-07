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

    @file     nbnetwork.h
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

#include "nbexp.h"
#include "nbnetwork.h"
#include "cslnetwork.h"

/*!
    @addtogroup nbnetwork
    @{
*/

NB_DEF NB_Error
NB_NetworkCreate(PAL_Instance* pal, NB_NetworkConfiguration* configuration, NB_Network** network)
{
    return CSL_NetworkCreate(pal, configuration, network);
}

/* See header file for description */
NB_DEF NB_Error
NB_NetworkCreateByCopy(NB_Network* sourceNetwork, NB_NetworkConfiguration* configuration, NB_Network** network)
{
    if (!sourceNetwork || !network)
    {
        return NE_INVAL;
    }

    return CSL_NetworkCreateByCopy(sourceNetwork, configuration, network);
}

NB_DEF NB_Error
NB_NetworkDestroy(NB_Network* network)
{
    if (!network)
    {
        return NE_INVAL;
    }

    CSL_NetworkDestroy(network);

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_NetworkSetIden(NB_Network* network, NB_NetworkConfiguration* configuration)
{
    if (!network || !configuration)
    {
        return NE_INVAL;
    }

    return CSL_NetworkSetIden(network, configuration);
}

/*! @} */

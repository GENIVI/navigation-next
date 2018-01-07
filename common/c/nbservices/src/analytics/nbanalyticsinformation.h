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

    @file     nbanalyticsinformation.h
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

#ifndef NBANALYTICSINFORMATION_H
#define NBANALYTICSINFORMATION_H

#include "paltypes.h"
#include "nbanalyticseventsconfig.h"

/*!
    @addtogroup nbanalyticsinformation
    @{
*/


typedef struct NB_AnalyticsInformation NB_AnalyticsInformation;

/*! Create an information object from a tps network reply

@param context NAVBuilder Context Instance
@param reply The TPS reply from the server
@param information On success, a newly created NB_AnalyticsInformation object;
       NULL otherwise.  A valid object must be destroyed using
       NB_AnalyticsInformationDestroy()
@returns NB_Error
*/
NB_Error NB_AnalyticsInformationCreate(NB_Context* context, tpselt reply,
        NB_AnalyticsInformation** information);

/*! Check if analytics config has been received.

@param information A NB_AnalyticsInformation object
@returns boolean: TRUE if analytics config have been received, FALSE otherwise.
*/
boolean NB_AnalyticsInformationIsConfigReceived(NB_AnalyticsInformation* pThis);

/*! Get a analytics configuration received

@param information A NB_AnalyticsInformation object
@param config The address of a NB_AnalyticsEventsConfig object to be filled in
        with the result
@returns NB_Error: NE_OK if config has been received and filled in 'config',
        error otherwise
*/
NB_Error NB_AnalyticsInformationGetConfig(NB_AnalyticsInformation* information,
        NB_AnalyticsEventsConfig* config);


/*! Destroy a previously created NB_AnalyticsInformation object

@param information A NB_AnalyticsInformation object
@returns NB_Error
*/
NB_Error NB_AnalyticsInformationDestroy(NB_AnalyticsInformation* information);

/*! @} */
#endif


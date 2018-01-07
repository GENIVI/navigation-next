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

    @file     cslerrorutil.c
*/
/*
    (C) Copyright 2008 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "cslerrorutil.h"

CSL_DEF NB_Error
CSL_TranslatePalError(PAL_Error palerror)
{
	NB_Error err = NE_OK;

	switch (palerror) {

		// General
		case PAL_Ok:
			err = NE_OK;
			break;

		// GPS
		case PAL_ErrGpsGeneralFailure:
			err = NEGPS_GENERAL;
			break;
		case PAL_ErrGpsTimeout:
			err = NEGPS_TIMEOUT;
			break;
		case PAL_ErrGpsInformationUnavailable:
			err = NEGPS_INFO_UNAVAIL;
			break;
		case PAL_ErrGpsAccuracyUnavailable:
			err = NEGPS_ACCURACY_UNAVAIL;
			break;
		case PAL_ErrGpsPrivacyRefused:
			err = NEGPS_PRIVACY_REFUSED;
			break;
		case PAL_ErrGpsServerError:
			err = NEGPS_GENERAL;  // there is no specific error for this one
			break;
		case PAL_ErrGpsRejected:
			err = NEGPS_PRIVACY_REFUSED;
			break;
		case PAL_ErrGpsNoResources:
			err = NEGPS_GENERAL; // there is no specific error for this one
			break;
		case PAL_ErrGpsBusy:
			err = NEGPS_RECEIVER_BUSY;
			break;
		case PAL_ErrGpsLocationDisabled:
			err = NE_GPS_LOCATION_OFF;
			break;

		// Network
		case PAL_ErrNetGeneralFailure:
			err = NE_NET;
			break;
		default:
			err = NE_UNEXPECTED;
	}

	return err;
}

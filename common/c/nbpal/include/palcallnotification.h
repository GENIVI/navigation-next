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

@file palcallnotification.h
@defgroup palcallnotification PAL API Functions to check phone state and subsribe to change notification

@brief Provides a platform-dependent call notification interface

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

/*! @{ */

#ifndef PALCALLNOTIFICATION_H
#define	PALCALLNOTIFICATION_H

#include "palexp.h"
#include "palerror.h"
#include "pal.h"


typedef enum
{
   CALL_STATE_NONE        = 0x00000000,
   CALL_STATE_IDLE        = 0x00000001,
   CALL_STATE_ORIG        = 0x00000002,
   CALL_STATE_INCOM       = 0x00000004,
   CALL_STATE_CONV        = 0x00000008,
   CALL_STATE_ENDED       = 0x00000010,
   CALL_STATE_ONHOLD      = 0x00000020,
   CALL_STATE_DORMANT     = 0x00000040,
   CALL_STATE_MULTIPARTY  = 0x00000080,
} PAL_CallState;


/*! Callback for PAL call notifier.

This typedef defines the function signature required for the call notifier callback.
*/

typedef void PAL_CallNotificationCallback(
    PAL_Instance *pal,                /*!< caller-supplied reference to PAL structure */
    void *userData,                   /*!< caller-supplied data reference for call notifier */
    PAL_CallState reason        /*!< caller-supplied reason for callback being called */
);

/*! Sets a call notification.

This function sets a call notification

@param pal The pal instance
@param subscribeTo This parameter specifies notification about what states need
@param callback The callback that will be called
@param userData The callback data

@return PAL error code
*/
PAL_DEC PAL_Error PAL_CallNotificationAddListener( PAL_Instance *pal, PAL_CallState subscribeTo, PAL_CallNotificationCallback *callback, void *userData );

/*! Cancels a call notification.

This function cancels a notification specified by the user-supplied callback and callback data

@param pal The pal instance
@param callback The callback
@param userData The callback data

@return PAL error code
*/
PAL_DEC PAL_Error PAL_CallNotificationCancelListener( PAL_Instance *pal, PAL_CallNotificationCallback *callback, void *userData );


/*! Check current phone state.

This function checks is phone in in-call or idle now

@param pal The pal instance
@param callback The callback
@param userData The callback data

@return PAL error code
*/
PAL_DEC PAL_Error PAL_CallNotificationGetCurrentState( PAL_Instance *pal,  PAL_CallState *currentState );

#endif

/*! @} */

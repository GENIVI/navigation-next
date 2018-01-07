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

@file paltimer.h
@defgroup PALTIMER_H PAL Timer Functions

@brief Provides a platform-dependent timer interface

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

/*! @{ */

#ifndef PALTIMER_H
#define	PALTIMER_H

#include "palerror.h"
#include "pal.h"
#include "paltaskqueue.h"

/*! File access modes */
typedef enum {	
	PTCBR_TimerFired=0,
	PTCBR_TimerCanceled,
} PAL_TimerCBReason;

/*! Callback for PAL timer.

This typedef defines the function signature required for the timer callback.
*/
typedef void PAL_TimerCallback(
    PAL_Instance *pal,          /*!< caller-supplied reference to PAL structure */
    void *userData,               /*!< caller-supplied data reference for timer */
    PAL_TimerCBReason reason    /*!< caller-supplied reason for callback being called */
);

/*! Sets a timer.

This function sets a timer that will call the user-supplied callback in the given milliseconds

@param pal The pal instance
@param milliseconds The time in milliseconds in which the timer will fire
@param callback The callback that will be called in Event thread when the timer fires
@param userData The callback data

@return PAL error code
*/
PAL_DEC PAL_Error PAL_TimerSet( PAL_Instance *pal, int32 milliseconds, PAL_TimerCallback *callback, void *userData );

/*! Sets a timer.

This function sets a timer that will call the user-supplied callback in the given milliseconds

@param pal The pal instance
@param handle The worker queue handle in where the callback will be called.
              DO NOT PASS EVENT OR UI QUEUE HANDLE
@param milliseconds The time in milliseconds in which the timer will fire
@param callback The callback that will be called when the timer fires
@param userData The callback data

@return PAL error code
*/

PAL_DEC PAL_Error PAL_TimerSetWithQueueHandle(PAL_Instance *pal,
                                              PAL_QueueHandle handle,
                                              int32 milliseconds,
                                              PAL_TimerCallback *callback,
                                              void *userData);

/*! Cancels a timer.

This function cancels a specified timer uses the user-supplied callback and callback data as the timer signature

@param pal The pal instance
@param callback The callback that will be called when the timer is canceled
@param userData The callback data

@return PAL error code. PAL_Ok if timer is cancelled successfully. PAL_ErrNotFound if timer is not found
*/
PAL_DEC PAL_Error PAL_TimerCancel( PAL_Instance *pal, PAL_TimerCallback *callback, void *userData );

/*! Get Expiration time.

This function gets expiration time that will call the user-supplied callback in the given milliseconds

@param pal The pal instance
@param callback The callback that will be called when the timer fires
@param userData The callback data

@return time in milliseconds
*/

uint32 PAL_TimerExpirationTime(PAL_Instance* pal, PAL_TimerCallback* callback, void* userData);

#endif

/*! @} */

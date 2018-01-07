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

    @file     timerimpl.h
    @date     03/09/2009
    @defgroup Timer Implemenation Header File

    Timer Manager for iPhone/iPad

    A more detailed description of this file/group and its purpose.
    HTML and other text formatting goodies can be used here.
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
#ifndef TIMERIMPL_H
#define TIMERIMPL_H

#include "palerror.h"
#include "pal.h"
#include "paltimer.h"

typedef struct TimerManager TimerManager;

/*! Create a timer manager.
 
 @param pal The pal instance
 
 @return Timer manager instance
*/
TimerManager* TimerManagerCreate(PAL_Instance* pal);

/*!
 Destroy a timer manager.
 
 @param timerManager The timer manager instance
*/
void TimerManagerDestroy(TimerManager *timerManager);

/*! Create and add a timer in timer manager.
 
 @param timerManager The timer manager instance
 @param milliseconds Timer interval (milliseconds)
 @param callback Timer callback
 @param userData Timer user data
 
 @return 'PAL_Fail' if creating or adding timer fails. 'PAL_Ok' if successful.
*/
PAL_Error TimerManagerCreateTimer(TimerManager *timerManager, PAL_QueueHandle handle,
        int32 milliseconds, PAL_TimerCallback *callback, void *userData);

/*! Destroy and remove a timer in timer manager.
 
 @param timerManager The timer manager instance
 @param callback Timer callback
 @param userData Timer user data
 
 @return 'PAL_Fail' if destroying or removing timer fails. 'PAL_Ok' if successful.
*/
PAL_Error TimerManagerDestroyTimer(TimerManager *timerManager,
        PAL_TimerCallback *callback, void *userData);

#endif  /* TIMERIMPL_H */

/*! @} */

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

    @file     callnotifierimpl.h
    @date     11/11/2011
    @defgroup Call notification manager implemenation header file

    Call notification manager for iPhone

    A more detailed description of this file/group and its purpose.
    HTML and other text formatting goodies can be used here.
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
#ifndef CALLNOTIFIERIMPL_H
#define CALLNOTIFIERIMPL_H

#include "palerror.h"
#include "pal.h"
#include "palcallnotification.h"

typedef struct CallNotifier CallNotifier;

/*! Create a call notification manager.
*/
CallNotifier* CallNotifierCreate(PAL_Instance* pal);

/*!
 Destroy a call notification manager.
*/
void CallNotifierDestroy(CallNotifier *CallNotifier);


/*! Add a call notification in call notification manager.
*/
PAL_Error CallNotifierAddListener(CallNotifier *notifier,
        PAL_CallState subscribeTo, PAL_CallNotificationCallback *callback, void *userData);

/*! Remove a call notification in call notification manager.
*/
PAL_Error CallNotifierCancelListener(CallNotifier *notifier,
        PAL_CallNotificationCallback *callback, void *userData);

/*! Return a current phone state.
*/
PAL_CallState CallNotifierGetCurrentState(CallNotifier *notifier);

#endif  /* CALLNOTIFIERIMPL_H */

/*! @} */

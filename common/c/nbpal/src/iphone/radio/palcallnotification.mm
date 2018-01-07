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

@file palcallnotification.c
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

#include "palcallnotification.h"
#include "palimpl.h"


PAL_DEF PAL_Error
PAL_CallNotificationAddListener( PAL_Instance *pal,
                                 PAL_CallState subscribeTo,
                                 PAL_CallNotificationCallback *callback,
                                 void *userData )
{
//    if (!pal || !callback || !userData)
//    {
//        return PAL_ErrBadParam;
//    }
//
//    if (PAL_IsSimulator(pal))
//    {
//        return PAL_ErrUnsupported;
//    }
//
//    return CallNotifierAddListener(pal->notificationMgr, subscribeTo, callback, userData);
    return PAL_ErrUnsupported;
}


PAL_DEF PAL_Error
PAL_CallNotificationCancelListener( PAL_Instance *pal,
                                    PAL_CallNotificationCallback *callback,
                                    void *userData )
{
//    if (!pal || !callback || !userData)
//    {
//        return PAL_ErrBadParam;
//    }
//
//    if (PAL_IsSimulator(pal))
//    {
//        return PAL_ErrUnsupported;
//    }
//
//    return CallNotifierCancelListener(pal->notificationMgr, callback, userData);
    return PAL_ErrUnsupported;
}


PAL_DEF PAL_Error
PAL_CallNotificationGetCurrentState( PAL_Instance *pal,
                                     PAL_CallState *currentState )
{
//    if (!pal || !currentState)
//    {
//        return PAL_ErrBadParam;
//    }
//
//    if (PAL_IsSimulator(pal))
//    {
//        return PAL_ErrUnsupported;
//    }
//
//    *currentState = CallNotifierGetCurrentState(pal->notificationMgr);
//    return PAL_Ok;
    return PAL_ErrUnsupported;
}

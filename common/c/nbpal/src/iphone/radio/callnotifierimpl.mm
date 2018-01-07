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

    @file     callnotifierimpl.mm
    @date     11/11/2011
    @defgroup Call notification manager implemenation source file
 */
/*
    See file description in header file.

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

#include "palstdlib.h"
#import <Foundation/Foundation.h>
#import <CoreTelephony/CTCallCenter.h>
#import <CoreTelephony/CTCall.h>
#include "callnotifierimpl.h"

@class CallNotification;

struct CallNotifier
{
    PAL_Instance*   pal;
    NSMutableArray* list;
    CTCallCenter*   callCenter;
    PAL_CallState   currentState;
};

@interface CallNotification : NSObject
{
    PAL_CallState                  _state;
    PAL_CallNotificationCallback*  _callback;
    void*                          _userData;
}

@property (nonatomic, assign) PAL_CallState _state;
@property (nonatomic, assign) PAL_CallNotificationCallback* _callback;
@property (nonatomic, assign) void* _userData;

@end

@implementation CallNotification

@synthesize _state;
@synthesize _callback;
@synthesize _userData;
//@synthesize _notifier;

- (id) init
{
    self = [super init];
    if(self != nil)
    {
        self._state = CALL_STATE_NONE;
        self._callback = NULL;
        self._userData = NULL;
    }
    return self;
}

- (void) dealloc
{
    [super dealloc];
}
@end

static CallNotification* CallNotificationCreate(CallNotifier* notifier, PAL_CallState state,
                                                PAL_CallNotificationCallback* callback, void* userData)
{

    if ((notifier == NULL) || (callback == NULL) || (userData == NULL))
    {
        return NULL;
    }

    CallNotification* notification = [[CallNotification alloc] init];
    if (notification)
    {
        notification._state = state;
        notification._callback = callback;
        notification._userData = userData;
    }

    return notification;
}

PAL_Error CallNotifierAddListener(CallNotifier* notifier,
                                  PAL_CallState state, PAL_CallNotificationCallback* callback, void* userData)
{
    /* create a timer and add to the list */
    if ((notifier == NULL) || (callback == NULL) || (userData == NULL))
    {
        return PAL_ErrBadParam;
    }

    CallNotification* notification = CallNotificationCreate(notifier, state, callback, userData);
    if(nil == notification)
    {
        return PAL_Failed;
    }

    /*  add to list */
    if (notifier->list == NULL)
    {
        [notification release];
        return PAL_Failed;
    }

    [notifier->list addObject:notification];
    [notification release];

    return PAL_Ok;
}

PAL_Error CallNotifierCancelListener(CallNotifier* notifier,
                                     PAL_CallNotificationCallback* callback, void* userData)
{
    if ((notifier == NULL) || (callback == NULL) || (userData == NULL))
    {
        return PAL_ErrBadParam;
    }

    for (CallNotification* notification in notifier->list)
    {
        if (callback == notification._callback && notification == notification._userData)
        {
            [notifier->list removeObject:notification];
        }
    }
    return PAL_Ok;
}

CallNotifier* CallNotifierCreate(PAL_Instance* pal)
{
    if (pal == NULL)
    {
        return NULL;
    }

    CallNotifier* notifier = (CallNotifier*)nsl_malloc(sizeof(CallNotifier));
    if (notifier)
    {
        CTCallCenter* call_center = [[CTCallCenter alloc] init];
        NSMutableArray* array = [[NSMutableArray alloc] init];

        notifier->currentState = CALL_STATE_NONE;
        notifier->pal = pal;
        notifier->list = array;
        notifier->callCenter = call_center;

        if (!call_center || !array)
        {
            CallNotifierDestroy(notifier);
            notifier = NULL;
            return notifier;
        }

        notifier->callCenter.callEventHandler = ^(CTCall* call)
        {
            if (call.callState == CTCallStateDisconnected)
            {
                notifier->currentState = CALL_STATE_ENDED;
            }
            else if (call.callState == CTCallStateConnected)
            {
                notifier->currentState = CALL_STATE_CONV;
            }
            else if (call.callState == CTCallStateIncoming)
            {
                notifier->currentState = CALL_STATE_INCOM;
            }
            else if (call.callState == CTCallStateDialing)
            {
                notifier->currentState = CALL_STATE_ORIG;
            }

            for (CallNotification* notification in notifier->list)
            {
                if (notifier->currentState&notification._state && notification._callback)
                {
                    notification._callback(notifier->pal, notification._userData, notifier->currentState);
                }
            }
        };
    }

    return notifier;
}

PAL_CallState CallNotifierGetCurrentState(CallNotifier *notifier)
{
    return notifier->currentState;
}


void CallNotifierDestroy(CallNotifier *notifier)
{
    if (notifier)
    {
        if (notifier->callCenter)
        {
            [notifier->callCenter release];
        }

        if (notifier->list)
        {
            /* free all timers */
            [notifier->list removeAllObjects];

            /* free the list */
            [notifier->list release];
        }

        nsl_free(notifier);
    }
}

/*! @} */

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

    @file pallock.c
    @date 10-9-28
    @defgroup pallock.c in nbpal
*/
/*
    See file description in header file.

    (C) Copyright 2010 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

/* all the file’s code goes here */

#import <Foundation/Foundation.h>
#import "pallock.h"
#import "palstdlib.h"

struct PAL_Lock
{
    NSRecursiveLock* lock;
};

/* See header file for description. */
PAL_DEF PAL_Error
PAL_LockCreate(PAL_Instance* pal, PAL_Lock** palLock)
{
    PAL_Error result = PAL_Ok;
    if (pal == NULL || palLock == NULL)
    {
        result = PAL_ErrBadParam;
    }
    else
    {
        PAL_Lock* lock = (PAL_Lock*)nsl_malloc(sizeof(PAL_Lock));
        if (lock != NULL)
        {
            NSRecursiveLock* nsLock = [[NSRecursiveLock alloc] init];
            lock->lock = [nsLock retain];
            [nsLock release];
        }

        if (lock == NULL || lock->lock == nil)
        {
            if (lock != NULL)
            {
                nsl_free(lock);
                lock = NULL;
            }
            *palLock = NULL;
            result = PAL_ErrNoMem;
        }
        else
        {
            *palLock = lock;
        }

    }
    return result;
}

/* See header file for description. */
PAL_DEC PAL_Error
PAL_LockLock(PAL_Lock* palLock)
{
    PAL_Error result = PAL_Ok;
    if (palLock == NULL || palLock->lock == nil)
    {
        result = PAL_ErrBadParam;
    }
    else
    {
        [palLock->lock lock];
    }
    return result;
}

/* See header file for description. */
PAL_DEC PAL_Error
PAL_LockUnlock(PAL_Lock* palLock)
{
    PAL_Error result = PAL_Ok;
    if (palLock == NULL || palLock->lock == nil)
    {
        result = PAL_ErrBadParam;
    }
    else
    {
        [palLock->lock unlock];
    }
    return result;
}

/* See header file for description. */
PAL_DEC PAL_Error
PAL_LockDestroy(PAL_Lock* palLock)
{
    PAL_Error result = PAL_Ok;
    if (palLock == NULL || palLock->lock == nil)
    {
        result = PAL_ErrBadParam;
    }
    else
    {
        [palLock->lock release];
        nsl_free(palLock);
    }
    return result;
}

/*! @} */

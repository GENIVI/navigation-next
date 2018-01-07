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

    @file pallock.h
    @date 10-9-28
    @defgroup pallock.h in nbpal

    The PAL_Lock object is used to coordinate the operation of the multiple threads of execution
    within the same application. It can be used to mediate access to an applications's global data
    or to protect a critical section of code, allowing it to run atomically.
*/
/*
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

#ifndef PALLOCK_H
#define PALLOCK_H

/* all the file’s code goes here */

#include "pal.h"
#include "palexp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PAL_Lock PAL_Lock;

/*! Create and initialize a new PAL_Lock object.

    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_LockCreate(PAL_Instance* pal,		/*!< The pal instance. */
               PAL_Lock** palLock		/*!< The PAL_Lock object created upon success. This object must be destroied by PAL_LockDestroy. */
               );

/*! Attempts to acquire a lock, blocking a thread's executing until the lock can be acquired.

	The lock can be locked recursively on the same thread without blocking. Only after all recursive
	locks are unlocked will the lock be released.

    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_LockLock(PAL_Lock* palLock			/*!< A PAL_Lock object created by PAL_LockCreate. */
             );

/*! Relinquishes a previously acquired lock.

    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_LockUnlock(PAL_Lock* palLock		/*!< A PAL_Lock object created by PAL_LockCreate. */
               );

/*! Destroy a previously created PAL_Lock object.
 
    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_LockDestroy(PAL_Lock* palLock		/*!< A PAL_Lock object created by PAL_LockCreate. */
                );

/*! Attempts to acquire a lock, don't block a thread's executing.

    @return nb_boolean.
*/
PAL_DEC nb_boolean
PAL_LockTryLock(PAL_Lock* palLock/*!< A PAL_Lock object created by PAL_LockCreate. */);

#ifdef __cplusplus
}
#endif

#endif //PALLOCK_H

/*! @} */

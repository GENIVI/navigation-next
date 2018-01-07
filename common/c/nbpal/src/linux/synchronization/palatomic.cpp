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

    @file     palatomic.cpp
    @date     02/10/2012
    @defgroup palatomic.h in nbpal

    This file contains implementation of atomic operations for QT QNX.
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "palatomic.h"
//#include <atomic>
/*Use gcc(>4.4) built_in atomic functions to do atomic actions*/
  typedef struct {
      volatile int atomic;
  } atomic_t;
   
#define atomic_read(x) ((x)->atomic)

#define atomic_set(x, val) (((x)->atomic) = (val))

#define atomic_inc(x) __sync_add_and_fetch((x),1)

#define atomic_dec(x) __sync_sub_and_fetch((x),1)

#define atomic_add(x,y) __sync_add_and_fetch((x),(y))

#define atomic_sub(x,y) __sync_sub_and_fetch((x),(y))

#define atomic_add_value(x,y) __sync_fetch_and_add((x),(y))

#define atomic_sub_value(x,y) __sync_fetch_and_sub((x),(y))



int32
PAL_AtomicIncrement(int32 volatile* addend)
{
    return (int32)atomic_add_value((volatile unsigned*)(addend), 1);
}

int32
PAL_AtomicDecrement(int32 volatile* addend)
{
    return (int32)atomic_sub_value((volatile unsigned*)(addend), 1);
}

void
PAL_AtomicExchange(int32 volatile* target, int32 value)
{
    atomic_set((atomic_t*)(target), value);
  //  atomic_set(target, value);
}

void
PAL_AtomicExchangeAdd(int32 volatile* addend, int32 value)
{
    atomic_add((volatile unsigned*)(addend), value);
}

void
PAL_AtomicCompareExchange(int32 volatile* destination, int32 exchange, int32 comparand)
{
    if (*destination == comparand)
    {
        *destination = exchange;
    }
}

void
PAL_AtomicCompareExchangePointer(void* volatile *destination, void* exchange, void* comparand)
{
    if (destination == comparand)
    {
        *destination = exchange;
    }
}

/*! @} */

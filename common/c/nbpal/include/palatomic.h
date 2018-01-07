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

    @file palatomic.h
    @date 08/29/2011
    @defgroup palatomic.h in nbpal

    This file contains definitions of atomic operations.

    Applications must synchronize access to variables that are shared by multiple threads. Applications must
    also ensure that operations on these variables are performed atomically (performed in their entirety or
    not at all).
    The interlocked functions provide a simple mechanism for synchronizing access to a variable that is shared
    by multiple threads. They also perform operations on variables in an atomic manner.

*/
/*
    (C) Copyright 2011 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PALATOMIC_H
#define PALATOMIC_H

#include "pal.h"
#include "palexp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! Increments (increases by one) the value of the specified 32-bit variable as an atomic operation.

    @return The resulting incremented value.
*/
PAL_DEC int32
PAL_AtomicIncrement(int32 volatile *addend /*!< A pointer to the variable to be incremented. */
                    );

/*! Decrements (decreases by one) the value of the specified 32-bit variable as an atomic operation.

    @return The resulting decremented value.
*/
PAL_DEC int32
PAL_AtomicDecrement(int32 volatile *addend /*!< A pointer to the variable to be decremented. */
                    );

/*! Sets a 32-bit variable to the specified value as an atomic operation.
*/
PAL_DEC void
PAL_AtomicExchange(int32 volatile *target, /*!< A pointer to the value to be exchanged. The function sets this variable to "value". */
                   int32 value             /*!< The value to be exchanged with the value pointed to by "target". */
                   );

/*! Performs an atomic addition of two 32-bit values.
*/
PAL_DEC void
PAL_AtomicExchangeAdd(int32 volatile *addend, /*!< A pointer to a variable. The value of this variable will be replaced with the result of the operation. */
                      int32 value             /*!< The value to be added to the variable pointed to by the "addend" parameter. */
                      );

/*! Performs an atomic compare-and-exchange operation on the specified values.
    The function compares two specified 32-bit values and exchanges with another 32-bit value based on the outcome
    of the comparison.

    If you are exchanging pointer values, this function has been superseded by the PAL_AtomicCompareExchangePointer function.
*/
PAL_DEC void
PAL_AtomicCompareExchange(int32 volatile *destination, /*!< A pointer to the destination value. */
                          int32 exchange,              /*!< The exchange value. */
                          int32 comparand              /*!< The value to compare to "destination". */
                          );

/*! Performs an atomic compare-and-exchange operation on the specified values.
    The function compares two specified pointer values and exchanges with another pointer value based on the outcome
    of the comparison.

    To operate on non-pointer values, use the PAL_AtomicCompareExchange function.
*/
PAL_DEC void
PAL_AtomicCompareExchangePointer(void* volatile *destination, /*!< A pointer to pointer to the destination value. */
                                 void* exchange,              /*!< The exchange value. */
                                 void* comparand              /*!< The value to compare to "destination". */
                                 );

#ifdef __cplusplus
}
#endif

#endif

/*! @} */

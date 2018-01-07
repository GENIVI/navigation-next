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

    @file     globals.h
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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GLOBALS_H
#define GLOBALS_H

/*!
Inflate Constants
*/
static uint32 bll[] =                         // bit length code lengths
    {
            16, 17, 18, 0, 8, 7, 9, 6, 10, 5,
            11, 4, 12, 3, 13, 2, 14, 1, 15
    };

static uint32 cll[] =                         // copy lengths for literal ..
    {                                       // ..codes 257 thru 285
            3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
            15, 17, 19, 23, 27, 31, 35, 43,
            51, 59, 67, 83, 99, 115, 131, 163,
            195, 227, 258, 0, 0
    };

static uint32 cle[] =                         // extra bits for literal
    {                                       // ..codes 257 thru 285
            0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
            1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4,
            4, 4, 5, 5, 5, 5, 0, 99, 99
    };

static uint32 cdo[] =                         // copy distance offsets
    {                                       // ..for codes 0 thru 29
            1, 2, 3, 4, 5, 7, 9, 13, 17, 25,
            33, 49, 65, 97, 129, 193, 257, 385,
            513, 769, 1025, 1537, 2049, 3073,
            4097, 6145, 8193, 12289, 16385, 24577
    };

static uint32 cde[] =                         // copy extra bits for distance
    {                                       // ..codes 0 thru 29
            0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4,
            5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
            11, 11, 12, 12, 13, 13
    };


/*!
malloc_chk() -- allocates memory with error processing
*/
void* malloc_chk(int n)   /*!< Size of block */
{
    void* s = NULL;
    s = nsl_malloc(n);
    if (!s)
    {
        return NULL;
    }

    nsl_memset(s, 0, (long) n);		  // else .. clear to nulls
    return s;                        // ..and return w/address
}

#endif

#ifdef __cplusplus
}
#endif


/*! @} */
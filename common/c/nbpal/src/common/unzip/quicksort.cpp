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

    @file     quicksort.cpp
    @defgroup QUICKSORT_H
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

#include "palfile.h"
#include <quicksort.h>

static void shortsort(char *lo, char *hi, unsigned int width, int (*comp)(const void *, const void *));
static void swap(char *p, char *q, unsigned int width);

void quicksort(void *base, unsigned int num, unsigned int width, int (*comp)(const void *, const void *))
{
    char *lo = NULL, *hi = NULL;
    char *mid = NULL;
    char *loguy = NULL, *higuy = NULL;
    unsigned int size = 0;
    char *lostk[30] = {NULL}, *histk[30] = {NULL};
    int stkptr = 0;

    if (num < 2 || width == 0)
    {
        return;
    }
    stkptr = 0;

    lo = (char*)base;
    hi = (char *) base + width * (num - 1);

    recurse:
    size = (hi - lo) / width + 1;

    if (size <= QUICK_CUTOFF)
    {
        shortsort(lo, hi, width, comp);
    }
    else
    {
        mid = lo + (size / 2) * width;
        swap(mid, lo, width);

        loguy = lo;
        higuy = hi + width;

        for (;;)
        {
            do { loguy += width; } while (loguy <= hi && comp(loguy, lo) <= 0);
            do { higuy -= width; } while (higuy > lo && comp(higuy, lo) >= 0);
            if (higuy < loguy)
            {
                break;
            }
            swap(loguy, higuy, width);
        }

        swap(lo, higuy, width);

        if (higuy - 1 - lo >= hi - loguy)
        {
            if (lo + width < higuy)
            {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy - width;
                ++stkptr;
            }

            if (loguy < hi)
            {
                lo = loguy;
                goto recurse;
            }
        }
        else
        {
            if (loguy < hi)
            {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;
            }

            if (lo + width < higuy)
            {
                hi = higuy - width;
                goto recurse;
            }
        }
    }

    --stkptr;
    if (stkptr >= 0)
    {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;
    }
    else
    {
        return;
    }
}

static void shortsort(char *lo, char *hi, unsigned int width, int (*comp)(const void *, const void *))
{
    char *p = NULL, *max = NULL;

    while (hi > lo)
    {
        max = lo;
        for (p = lo+width; p <= hi; p += width)
        {
            if (comp(p, max) > 0)
            {
                swap(max, hi, width);
            }
        }
        hi -= width;
    }
}

static void swap(char *a, char *b, unsigned int width)
{
    char tmp = 0;

    if (a != b)
    {
        while (width--)
        {
            tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
    }
}

int exp_cmp(const void *a, const void *b)
{
	return(nsl_strncmp((char *) a, (char *) b, 2));
}

#ifdef __cplusplus
}
#endif

/*! @} */

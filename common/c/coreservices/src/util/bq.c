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

/*
 * bq.c: created 2004/11/15 by Dima Dorfman.
 *
 * Copyright (C) 2004 Networks In Motion, Inc. All rights reserved.
 *
 * The information contained herein is confidential and proprietary to
 * Networks In Motion, Inc., and is considered a trade secret as
 * defined in section 499C of the California Penal Code. Use of this
 * information by anyone other than authorized employees of Networks
 * In Motion is granted only under a written non-disclosure agreement
 * expressly prescribing the scope and manner of such use.
 */

#include "paltypes.h"
#include "palstdlib.h"
#include "bq.h"
#include "cslutil.h"

void
csl_bqinit(struct bq *b)
{

	b->p = b->r = b->w = b->x = NULL;
}

void
csl_bqfree(struct bq *b)
{
	if (b->p)
	{
		nsl_free(b->p);
	}
	csl_bqinit(b);
}

static int
normalized(struct bq *b)
{

	return (b->r == b->p);
}

static void
normalize(struct bq *b)
{

	if (!normalized(b)) {
		nsl_memmove(b->p, b->r, b->w - b->r);
		b->w -= b->r - b->p;
		b->r = b->p;
	}
	nsl_assert(normalized(b));
}

static int
resize(struct bq *b, size_t size)
{
	size_t offw;
	char *p;

	nsl_assert(normalized(b));
	nsl_assert(size > 0);
	if (size != (size_t)(b->x - b->p)) {
		offw = b->w - b->p;
		p = (char *) nsl_realloc(b->p, size);
		if (p == NULL)
			return (0);
		b->p = b->r = p;
		b->w = b->p + offw;
		b->x = b->p + size;
	}
	return (1);
}

int
csl_bqprep(struct bq *b, size_t n)
{
	size_t offw, size;

#define	XMINUSW	((size_t)(b->x - b->w))
	if (b->p == NULL) {
		b->p = (char *)nsl_malloc(n);
		if (b->p == NULL)
			return (0);
		b->r = b->w = b->p;
		b->x = b->r + n;
	} else if (XMINUSW < n) {
		normalize(b);
		if (XMINUSW < n) {
			offw = b->w - b->p;
			size = b->x - b->p;
			while (size - offw < n && size > 0)
				size <<= 1;
			if (size == 0)
				return (0);
			if (!resize(b, size))
				return (0);
		}
	}
	nsl_assert(XMINUSW >= n);
	return (1);
#undef XMINUSW
}

static void
recons(struct bq *b)
{

	csl_bqfree(b);
	csl_bqinit(b);
}

/*
 * If possible, shrink the queue. csl_bqprep always extends the queue and
 * never reduces the amount of allocated memory. This function can be
 * called periodically to ensure that the queue does not grow
 * indefinitely after a spike.
 *
 * Note that this function shrinks aggressively. The queue will be
 * reduced to a minimal size even if it's already at its minimal
 * allocation. It is not a good idea to call this after every update
 * to the reader pointer, since that would negate the benefit of
 * overallocation.
 */
void
csl_bqshrink(struct bq *b)
{
	size_t size, need;

	if (b->r == b->w)
		recons(b);
	else {
		normalize(b);
		need = b->w - b->r;
		size = b->x - b->p;
		while (size > need)
			size >>= 1;
		if (size < need)
			size <<= 1;
		nsl_assert(size >= need);
		(void)resize(b, size);
	}
}

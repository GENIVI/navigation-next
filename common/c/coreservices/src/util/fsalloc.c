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

/*-
 * fsalloc.c: created 2003/09/11 by Mark Goddard.
 *
 * Copyright (c) 2003 Networks In Motion, Inc.
 * All rights reserved.  This file and associated materials are the
 * trade secrets, confidential information, and copyrighted works of
 * Networks In Motion, Inc.
 *
 * This intellectual property is for the internal use only by Networks
 * In Motion, Inc.  This source code contains proprietary information
 * of Networks In Motion, Inc., and shall not be used, copied by, or
 * disclosed to, anyone other than a Networks In Motion, Inc.,
 * certified employee that has written authorization to view or modify
 * said source code.
 *
 */

#include "fsalloc.h"
#include "palstdlib.h"
#include "cslutil.h"
#include "palmath.h"
#include "paldebuglog.h"

#define FSH_DEBUGF DEBUGF

#define FSA_ALL_USED	0xFFFFFFFF
#define FSA_NONE		0xFFFFFFFF

#define FSA_ISFREE(fh, n) \
	((fh->f[(n)>>5] & (1 << ((n) % 32))) == 0)

#define FSA_SETUSED(fh, n) \
	fh->f[(n)>>5] = (fh->f[(n)>>5] | (1 << ((n) % 32)))

#define FSA_SETFREE(fh, n) \
	fh->f[(n)>>5] = (fh->f[(n)>>5] & (~(1 << ((n) % 32))))

static int32 fsaheap_getslot(fs_heap fh, void* p);

CSL_DEF boolean fsaheap_init(fs_heap fh, uint32 os, uint32 hs)
{
	fh->os	= os;

	nsl_assert(hs % 32 == 0); // fsaheap_init: hs must be a multiple of 32

	if (hs % 32 != 0)
		return FALSE;

	if ((fh->os % 4) != 0)
		fh->os += (4 - (fh->os % 4));

	fh->hs	= hs;
	fh->m	= (byte*) nsl_malloc(hs * fh->os);

	if (fh->m == NULL)
		return FALSE;

	fh->f	= (uint32*) nsl_malloc((hs / 32) * sizeof (*fh->f));

	if (fh->f == NULL) {
		nsl_free(fh->m);
		return FALSE;
	}

	fh->n	= 0;
	fh->l = 0;
	fh->num = 0;
	fh->over = 0;
	fh->max = 0;

	nsl_memset(fh->f, 0, (hs / 32) * sizeof (*fh->f));

#ifdef AEE_SIMULATOR
	if (fh->m != NULL) {

		nsl_memset(fh->m, 0xFE, hs * fh->os);
	}
#endif

	return TRUE;
}

CSL_DEF void fsaheap_destroy(fs_heap fh)
{
#ifndef NDEBUG
	uint32 n;
#endif

	if (fh == NULL)
		return;

#ifndef NDEBUG

	if (fh->num != 0)
		DEBUGLOG(LOG_SS_MEMORY, LOG_SEV_MAJOR, ("fsheap not empty os=%u n=%u", fh->os, fh->num));

	if (fh->over != 0)
		DEBUGLOG(LOG_SS_MEMORY, LOG_SEV_MAJOR, ("fsheap overflow not freed os=%u n=%u", fh->os, fh->over));

	for (n=0;n<fh->hs;n++) {

		if (!FSA_ISFREE(fh, n))
			DEBUGLOG(LOG_SS_MEMORY, LOG_SEV_MAJOR, ("leaked fs block: size=%d data=%.*s", fh->os, fh->os, (fh->m + n * fh->os)));
	}
#endif

	nsl_free(fh->m);
	nsl_free(fh->f);
}

CSL_DEF void* fsaheap_alloc(fs_heap fh)
{
	void* ret = NULL;
	boolean bfs = FALSE;

	nsl_assert(fh->num <= fh->hs); // Busted num

	if (fh->num == fh->hs) {

		/*
		 * the heap is full, so allocate a block using the
		 * normal function
		 */

		DEBUGLOG(LOG_SS_MEMORY, LOG_SEV_MINOR, ("heap is full, allocating block with NIM_MALLOC"));
		ret = nsl_malloc(fh->os);

		if (ret != NULL)
			fh->over++;
	}
	else if (fh->n != FSA_NONE) {

		ret = fh->m + fh->n * fh->os;
		nsl_assert(FSA_ISFREE(fh, fh->n)); // Next available block not free
		FSA_SETUSED(fh, fh->n);
		fh->n = FSA_NONE;
		fh->num++;
		bfs = TRUE;
	}
	else if (((fh->l + 1) < fh->hs) && FSA_ISFREE(fh, fh->l + 1)) {

		fh->l++;
		fh->num++;
		FSA_SETUSED(fh, fh->l);
		ret = (fh->m + (fh->l) * fh->os);
		bfs = TRUE;
	}
	else {
		uint32 p1,p2;

		for (p1=0; p1<(fh->hs/32); p1++) {

			if (fh->f[p1] != FSA_ALL_USED) {

				for (p2 = 0; p2<32; p2++) {

					if (FSA_ISFREE(fh, p1*32+p2)) {

						fh->num++;
						FSA_SETUSED(fh, (p1*32+p2));
						ret = (fh->m + (p1*32+p2) * fh->os);
						bfs = TRUE;
						goto exit;
					}
				}
			}
		}

		nsl_assert(FALSE); // Misaccounting of memory blocks
	}

exit:

#ifdef AEE_SIMULATOR
	if (ret != NULL && bfs) {

		uint32 p;

		for (p = 0; p < fh->os; p++) {
			nsl_assert(*(((byte*)ret)+p) == 0xFE); // Not Free
		}

		MEMSET(ret, 0xCD, fh->os);
	}
#endif

	fh->max = MAX(fh->max, fh->num);

	return ret;
}

CSL_DEF void	fsaheap_free(fs_heap fh, void* p)
{
	int32 n;

#ifdef AEE_SIMULATOR
	if (p != NULL) {

		MEMSET(p, 0xFE, fh->os);
	}
#endif

	n = fsaheap_getslot(fh, p);

	if (n != -1) {

		nsl_assert(!FSA_ISFREE(fh, n));  // double free

		FSA_SETFREE(fh, n);

		fh->n = (uint32)n;
		nsl_assert(fh->num>0); // bad counting
		fh->num--;
	}
	else {

		/*
		 * this block must be an overflow block
		 * free using the normal allocation function
		 */

		nsl_assert(fh->over>=0);  // double free of overflow

		if (p != NULL)
			fh->over--;

        nsl_free(p);
	}
}

/*
 * Return the slot number of the pointer.  If the pointer is not in this heap, return -1.
 */
static int32	fsaheap_getslot(fs_heap fh, void* p)
{
	uint32 n;

	if ((byte*)p < fh->m)
		return -1;
	else {
		n = (uint32)((((byte*) p) - fh->m) / fh->os);
		return n < fh->hs ? n : (uint32)-1;
	}
}

/*
 * Return whether the pointer is on this heap.
 */
CSL_DEF boolean	fsaheap_owned(fs_heap fh, void* p)
{
	return (boolean) (fsaheap_getslot(fh, p) != -1);
}

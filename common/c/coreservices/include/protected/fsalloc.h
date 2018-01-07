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
 * fsalloc.h: created 2003/09/11 by Mark Goddard.
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

#ifndef _FS_HEAP
#define _FS_HEAP

#include "paltypes.h"
#include "nberror.h"
#include "cslexp.h"
#include "cslutil.h"

struct fsaheap {

	byte*	m;	/* the memory actually used */
	uint32*	f;	/* free list bitmap */
	uint32	os;	/* the size of each object */
	uint32	hs;	/* the size of the heap (must be a multiple of 32) */
	uint32	n;	/* the index of a free block we happen to know about */
	uint32	l;	/* index of the last block we allocated */
	uint32	num;
	
	uint32	over;
	uint32	max;
};

typedef struct fsaheap *fs_heap;


CSL_DEC boolean fsaheap_init(fs_heap, uint32 os, uint32 hs);

CSL_DEC void fsaheap_destroy(fs_heap);

CSL_DEC void* fsaheap_alloc(fs_heap);

CSL_DEC void	fsaheap_free(fs_heap, void* p);

CSL_DEC boolean fsaheap_owned(fs_heap, void* p);

#endif

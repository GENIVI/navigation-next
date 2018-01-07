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
 * dbuf.h: created 2003/11/10 by Dima Dorfman.
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
 */

#ifndef DBUF_H
#define	DBUF_H

#include "nberror.h"
#include "paltypes.h"
#include "palstdlib.h"
#include "cslexp.h"

/*
 * Dynamic buffer structure contents.  This is exposed in the header
 * file to allow users to allocate these on the stack, but its
 * contents should be considered implementation details.
 */
struct dynbuf {
	byte *p;		/* Beginning of the buffer. */
	byte *x;		/* Write pointer. */
	byte *l;		/* End (limit) of the buffer. */
	size_t absize;	/* Allocation block size. */
	NB_Error error; /* Error during buffer operations, if any. */
};

/* Creation and destruction. */
CSL_DEC NB_Error dbufnew(struct dynbuf *dbp, size_t absize);
CSL_DEC void dbufdel(struct dynbuf *dbp);

/* Manipulation. */
CSL_DEC void dbufadd(struct dynbuf *dbp, byte b);
CSL_DEC void dbufcat(struct dynbuf *dbp, const byte *data, size_t datalen);
CSL_DEC void dbufcats(struct dynbuf *dbp, const char *cstr);
CSL_DEC void dbuffmtd(struct dynbuf *dbp, double d);
CSL_DEC void dbufnuls(struct dynbuf *dbp);
CSL_DEC void dbufrst(struct dynbuf *dbp);

/* Retrieval. */
CSL_DEC NB_Error dbuferr(struct dynbuf *dbp);
CSL_DEC const byte *dbufget(struct dynbuf *dbp);
CSL_DEC const char *dbufgets(struct dynbuf *dbp);
CSL_DEC char dbufgetc(struct dynbuf *dbp, size_t idx);
CSL_DEC size_t dbuflen(struct dynbuf *dbp);

/* Wide character support. */
#if 0 
/* BREW SPECIFIC */
void dbufaddw(struct dynbuf *dbp, AECHAR wch);
void dbufcatw(struct dynbuf *dbp, const AECHAR *wstr);
AECHAR *dbufcpyw(struct dynbuf *dbp);
size_t	dbufcpyw_static(struct dynbuf *dbp, AECHAR* buf, size_t size);

AECHAR *dbufcpyw_chunk(struct dynbuf *dbp, size_t start, size_t len);
size_t	dbufcpyw_chunk_static(struct dynbuf *dbp, size_t start, size_t len, AECHAR* buf, size_t size);
/* BREW SPECIFIC */
#endif

#endif

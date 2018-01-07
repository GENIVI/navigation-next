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
 * dbuf.c: created 2003/11/10 by Dima Dorfman.
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

/*
 * This is a "dynamic buffer" library.  It's less memory-efficient
 * than the old "dynstring" (dstr) and less functional than std::string
 * (if we were able to use that), but it's intended to be very time-
 * efficient, especially for accumulating large strings (e.g., XML).
 *
 * Note about relation to C-strings: The buffer implementation supports
 * non-NUL-terminated data, but since most users will be working with
 * C-strings, convenience functions are provided.
 */

#include "dynbuf.h"
#include "cslutil.h"

static void db_extend(struct dynbuf *dbp);
static void db_realloc(struct dynbuf *dbp, size_t newsize);

// disabled to eliminate compiler's warning..
#if 0
static void db_extend_to(struct dynbuf *dbp, size_t newsize);
#endif

/*
 * Return if this dynbuf is an error state.
 */
#define	DB_RETIFERROR(dbp) if (dbp->error) return

/*
 * Append one character to this dynbuf.  The write pointer is *not*
 * maintained here!
 */
#define	DB_APPENDONE(dbp, ch) do {		\
	if (dbp->x == dbp->l) {				\
		db_extend(dbp);					\
		DB_RETIFERROR(dbp);				\
	}									\
	*dbp->x = ch;						\
} while (0)


/*
 * Initialize a new dynamic buffer.  If an error was returned, dstrdel()
 * *must not* be called.
 */
CSL_DEF NB_Error
dbufnew(struct dynbuf *dbp, size_t absize)
{

	dbp->absize = absize;
	dbp->p = dbp->x = dbp->l = NULL;
	dbp->error = 0;
	db_extend(dbp);
	nsl_assert(!dbp->error || dbp->p == NULL); // first db_extend failed but p non-NULL
	return (dbp->error);
}

/*
 * Free dynamic buffer contents.
 */
CSL_DEF void
dbufdel(struct dynbuf *dbp)
{

	nsl_free(dbp->p);
	dbp->p = NULL;
	dbp->error = NE_INVAL;
}

/*
 * Append a byte to the buffer.  dbufcat()'s comments apply here.
 */
CSL_DEF void
dbufadd(struct dynbuf *dbp, byte b)
{

	DB_RETIFERROR(dbp);
	DB_APPENDONE(dbp, b);
	++dbp->x;
}

/*
 * Append data to the buffer, extending as appropriate.  Allocation errors
 * can be checked for by calling dbuferr().  If the buffer is in an error
 * state, the result of this operation is unspecified.
 *
 * XXX: This and the other cat variants can be improved by using the standard
 * memory or string copy routines which might take advantage of some processor-
 * specific optimizations.
 */
CSL_DEF void
dbufcat(struct dynbuf *dbp, const byte *data, size_t datalen)
{
	const byte *dx;

	DB_RETIFERROR(dbp);
	for (dx = data; dx < data + datalen; ++dx) {
		DB_APPENDONE(dbp, *dx);
		++dbp->x;
	}
}

/*
 * Append a C-string to the buffer.  Comments for dbufcat() apply here
 * as well.  dstrnuls() must still be called before dstrget() will
 * return a C-string.
 */
CSL_DEF void
dbufcats(struct dynbuf *dbp, const char *cstr)
{
	const char *cx;
	
	DB_RETIFERROR(dbp);
	for (cx = cstr; *cx != '\0'; ++cx) {
		DB_APPENDONE(dbp, *cx);
		++dbp->x;
	}
}

/*
 * Format a double into the buffer
 */
CSL_DEF void
dbuffmtd(struct dynbuf *dbp, double d)
{
    char buf[20] = { 0 };
    nsl_sprintf(buf, "%f", d);
    dbufcats(dbp, buf);
}

/*
 * NUL-terminate the buffer contents.  This routine guarantees that
 * a subsequent call to dstrget() (without any other operations in
 * between!) will return a C-string, but it does not guarantee that
 * there are no other NULs in the buffer.  It is safe to call this
 * more than once, and other manipulations are allowed after this
 * call, provided that the user no longer assumes the buffer value
 * is NUL-terminated.
 */
CSL_DEF void
dbufnuls(struct dynbuf *dbp)
{

	DB_RETIFERROR(dbp);
	/*
	 * In order to make this idempotent and safe to do other manipulations
	 * after, we want to:
	 *  (a) do nothing if the last character is already a NUL, and
	 *  (b) leave the write pointer on the terminating NUL.
	 */
	if (dbp->x == dbp->l || *dbp->x != '\0')
		DB_APPENDONE(dbp, '\0');
}

/*
 * Return the error status of the buffer.  Operations that modify
 * the buffer could cause the buffer's error status to change if
 * they don't succeed (e.g., they run out of memory).  The error
 * is stored inside the buffer and available using this interface
 * to save the user the trouble of checking for an error after every
 * buffer manipulation.
 */
CSL_DEF NB_Error
dbuferr(struct dynbuf *dbp)
{

	return (dbp->error);
}

/*
 * Get a reference to the contents of the buffer.  NULL is returned
 * if the buffer is an error state.
 */
CSL_DEF const byte *
dbufget(struct dynbuf *dbp)
{

	return (dbp->error ? NULL : dbp->p);
}

/*
 * NUL-terminate the buffer and return its contents (i.e., return a
 * C-string of the buffer contents).  NULL is returned if the buffer
 * is in an error state.
 */
CSL_DEF const char *
dbufgets(struct dynbuf *dbp)
{

	dbufnuls(dbp);
	return ((const char *)dbufget(dbp));
}

CSL_DEF char 
dbufgetc(struct dynbuf *dbp, size_t idx)
{
	if (dbp->error)
		return 0;

	if (dbp->p + idx > dbp->x)
		return 0;

	return (char) dbp->p[idx];
}

/*
 * Return the length of the buffer contents.  The return value is
 * unspecified if the buffer is an error state.
 */
CSL_DEF size_t
dbuflen(struct dynbuf *dbp)
{

	return (dbp->x - dbp->p);
}

/*
 * Reset the buffer to a clean state.  This function resets the contents
 * and the error state (it can't fail).
 */
CSL_DEF void
dbufrst(struct dynbuf *dbp)
{
	byte *newp;

	newp = nsl_realloc(dbp->p, dbp->absize);
	/*
	 * Shrinking the buffer should always succeed, but if it doesn't, it's
	 * not a big deal (except as an indicator that the system's broken).
	 */
	if (newp != NULL) {
		dbp->p = newp;
		dbp->l = dbp->p + dbp->absize;
	}
	dbp->x = dbp->p;
	dbp->error = 0;
}

/*
 * Extend this buffer by another block.  We currently extend linearly,
 * but we might allow the user the option to extend exponentially in
 * the future.
 */
static void
db_extend(struct dynbuf *dbp)
{

	db_realloc(dbp, dbp->l - dbp->p + dbp->absize);
}

# if 0 //@note: disabled to eliminate compile warnings..
/*
 * Extend this buffer to at least the specified size.
 */
static void
db_extend_to(struct dynbuf *dbp, size_t newsize)
{

	if (newsize > (unsigned)(dbp->l - dbp->p))
		db_realloc(dbp, newsize + newsize % dbp->absize);
}
#endif

/*
 * Reallocate memory for this buffer, adjusting our internal pointers and
 * error state as necessary.  It is an error to call this routine if we
 * are already in an error state.
 */
static void
db_realloc(struct dynbuf *dbp, size_t newsize)
{
	size_t xoff;
	byte *newp;

	nsl_assert(!dbp->error); // db_realloc called in error state
	xoff = dbp->x - dbp->p;
	newp = nsl_realloc(dbp->p, newsize);
	if (newp == NULL)
		dbp->error = NE_NOMEM;
	else {
		dbp->p = newp;
		dbp->l = dbp->p + newsize;
		dbp->x = dbp->p + xoff;
	}
}

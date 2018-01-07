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

@file     audbuf.h
@date     01/16/2009
@defgroup PAL Audio API

@brief    Dynamic buffer for Audio API

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

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

/*! @{ */

#include "audbuf.h"

static void ExtendDB(PalAudioBuffer* dbp);
static void ExtendToDB(PalAudioBuffer* dbp, size_t newsize);
static void ReallocDB(PalAudioBuffer* dbp, size_t newsize);

/*
 * Return if this audbuf is an error state.
 */
#define    DB_RETIFERROR(dbp) if (!dbp || dbp->error) return

/*
 * Append one character to this audbuf.  The write pointer is *not*
 * maintained here!
 */
#define    DB_APPENDONE(dbp, ch) do {        \
    if (dbp->x == dbp->l) {                \
        ExtendDB(dbp);                    \
        DB_RETIFERROR(dbp);                \
    }                                    \
    *dbp->x = ch;                        \
} while (0)

/*
 * Initialize a new dynamic buffer.  If an error was returned, dstrdel()
 * *must not* be called.
 */
ABPAL_DEF PAL_Error
AudioBufferNew(PalAudioBuffer* dbp, size_t absize)
{
    if(!dbp)
        return PAL_ErrBadParam;

    dbp->absize = absize;
    dbp->p = dbp->x = dbp->l = NULL;
    dbp->error = PAL_Ok;
    ExtendDB(dbp);
    return dbp->error;
}

/*
 * Free dynamic buffer contents.
 */
ABPAL_DEF void
AudioBufferDelete(PalAudioBuffer* dbp)
{
    if(!dbp)
        return;

    nsl_free(dbp->p);
    dbp->p = NULL;
    dbp->error = PAL_ErrNoInit;
}

/*
 * Append a byte to the buffer.  dbufcat()'s comments apply here.
 */
ABPAL_DEF void
AudioBufferAdd(PalAudioBuffer* dbp, byte b)
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
ABPAL_DEF void
AudioBufferCat(PalAudioBuffer* dbp, const byte* data, size_t datalen)
{
    const byte* dx;

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
ABPAL_DEF void
AudioBufferCats(PalAudioBuffer* dbp, const char* cstr)
{
    const char* cx;

    DB_RETIFERROR(dbp);
    for (cx = cstr; *cx != '\0'; ++cx) {
        DB_APPENDONE(dbp, *cx);
        ++dbp->x;
    }
}

/*
 * Append a formatted string.  dstrcats()'s comments apply here.
 */
ABPAL_DEF void
AudioBufferFmt(PalAudioBuffer* dbp, const char* fmt, ...)
{
    va_list ap;
    int len;

    /*
     * The Standard vsnprintf is specified to return the number of characters
     * that would have been printed if the size were unlimited. This allows
     * the user to try the operation with existing memory and, if necessary,
     * redo it after allocating more memory. Unforunately, BREW decided that
     * they had a better idea about what the semantics should be, and their
     * docs say that VSNPRINTF only behaves like that if the buffer is NULL.
     * While they were there, they decided to make the interface "safer"
     * (read: more confusing to use) by making the return include the
     * trailing NUL (which is misspelled as NULL in the docs--who are these
     * guys?).
     */
    va_start(ap, fmt);
    len = nsl_vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    ExtendToDB(dbp, dbp->x - dbp->p + len);
    DB_RETIFERROR(dbp);
    va_start(ap, fmt);
    nsl_vsnprintf((char *)dbp->x, len, fmt, ap);
    va_end(ap);
    dbp->x += len - 1;
}

/*
 * Return the error status of the buffer.  Operations that modify
 * the buffer could cause the buffer's error status to change if
 * they don't succeed (e.g., they run out of memory).  The error
 * is stored inside the buffer and available using this interface
 * to save the user the trouble of checking for an error after every
 * buffer manipulation.
 */
ABPAL_DEF PAL_Error
AudioBufferError(PalAudioBuffer* dbp)
{
    if(!dbp)
        return PAL_ErrBadParam;

    return dbp->error;
}

/*
 * Get a reference to the contents of the buffer.  NULL is returned
 * if the buffer is an error state.
 */
ABPAL_DEF const byte *
AudioBufferGet(PalAudioBuffer* dbp)
{
    if(!dbp)
        return NULL;

    return (dbp->error ? NULL : dbp->p);
}

ABPAL_DEF char
AudioBufferGetc(PalAudioBuffer* dbp, size_t idx)
{
    if(!dbp)
        return 0;

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
ABPAL_DEF size_t
AudioBufferLen(PalAudioBuffer* dbp)
{
    if(!dbp)
        return 0;

    return (dbp->x - dbp->p);
}


/*
 * Extend this buffer by another block.  We currently extend linearly,
 * but we might allow the user the option to extend exponentially in
 * the future.
 */

static void
ExtendDB(PalAudioBuffer* dbp)
{
    if(!dbp)
        return;

    ReallocDB(dbp, dbp->l - dbp->p + dbp->absize);
}


/*
 * Extend this buffer to at least the specified size.
 */
static void
ExtendToDB(PalAudioBuffer* dbp, size_t newsize)
{
    if(!dbp)
        return;

    if (newsize > (unsigned)(dbp->l - dbp->p))
        ReallocDB(dbp, newsize + newsize % dbp->absize);
}

/*
 * Reallocate memory for this buffer, adjusting our internal pointers and
 * error state as necessary.  It is an error to call this routine if we
 * are already in an error state.
 */

static void
ReallocDB(PalAudioBuffer* dbp, size_t newsize)
{
    size_t xoff;
    byte *newp;

    if(!dbp)
        return;

    xoff = dbp->x - dbp->p;
    newp = (byte *)nsl_realloc(dbp->p, newsize);
    if (newp == NULL)
        dbp->error = PAL_ErrNoMem;
    else {
        dbp->p = newp;
        dbp->l = dbp->p + newsize;
        dbp->x = dbp->p + xoff;
    }
}


/*! @} */

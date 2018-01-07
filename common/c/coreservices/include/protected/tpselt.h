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
 * tpselt.h: created 2004/12/04 by Dima Dorfman.
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

/*
 * Interface to elements in a templated packet stream.
 *
 * This interface defines mutations. It can be used to build up new
 * element trees for output. Elements that are created in other ways
 * don't have to support mutation.
 */

#ifndef TPSELT_H
#define	TPSELT_H

#include "paltypes.h"
#include "palstdlib.h"
#include "cslexp.h"

struct CSL_Vector;
struct tpseltvt;

struct tpselts {
	struct tpseltvt *vt;
};

typedef struct tpselts *tpselt;

typedef int tevt_attach(tpselt, tpselt);
typedef void tevt_dealloc(tpselt);
typedef void tevt_delattr(tpselt, const char *);
typedef int tevt_getattr(tpselt, const char *, char **, size_t *);
typedef struct CSL_Vector *tevt_getattrs(tpselt);
typedef const char *tevt_getname(tpselt);
typedef tpselt tevt_getparent(tpselt);
typedef tpselt tevt_nextchild(tpselt, int *);
typedef int tevt_setattr(tpselt, const char *, const char *, size_t);
typedef void tevt_unlink(tpselt);

struct tpseltvt {
	tevt_attach *attach;
	tevt_dealloc *dealloc;
	tevt_delattr *delattr;
	tevt_getattr *getattr;
	tevt_getattrs *getattrs;
	tevt_getname *getname;
	tevt_getparent *getparent;
	tevt_nextchild *nextchild;
	tevt_setattr *setattr;
	tevt_unlink *unlink;
};

/*
 * Attach che to te (make it a child of te). che must not have an
 * existing parent. Returns whether the operation succeeded.
 */
CSL_DEC int te_attach(tpselt te, tpselt che);

/*
 * Deallocate the element tree. The supplied pointer and any pointers
 * to values in this element or any of its children are invalid after
 * this operation.
 */
CSL_DEC void te_dealloc(tpselt te);

/*
 * Delete attribute.
 */
CSL_DEC void te_delattr(tpselt te, const char *name);

/*
 * Get attribute value. The value of *out is a pointer into an
 * internal buffer, so the caller must make a copy if it is to access
 * that data beyond the lifetime of the element. Returns whether the
 * attribute exists.
 */
CSL_DEC int te_getattr(tpselt te, const char *name, char **out, size_t *outsize);

/*
 * Get attribute value as a C string. As with te_getattr, the pointer
 * returned is not a new allocation. Returns NULL if the attribute
 * does not exist or if it is not completely representable as a C
 * string.
 */
CSL_DEC const char *te_getattrc(tpselt te, const char *name);

/*
 * Get an attribute value as an unsigned integer value.  
 */

CSL_DEC uint32 te_getattru(tpselt te, const char *name);

/*
* Get an attribute value as a string and convert it to a signed integer value.  
*/
CSL_DEC int te_getattratoi(tpselt te, const char *name);

/*
 * Get an attribute value as an unsigned integer value.  
 */

CSL_DEC int te_getattru64(tpselt te, const char *name, uint64 *value);

/*
 * Get an attribute value as a float value.
 */
CSL_DEC int te_getattrf(tpselt te, const char *name, double *value);

/*
 * Get an attribute value as a double value.
 */
CSL_DEC int te_getattrd(tpselt te, const char *name, double *value);

/*
 * Get a list of pointers to attribute names. Template libraries need this
 * to determine which template to use for this element. The pointers
 * inside the list are valid until the next mutation.
 */
CSL_DEC struct CSL_Vector *te_getattrs(tpselt te);

/*
 * Return a child element with the specified name.
 */
CSL_DEC tpselt te_getchild(tpselt te, const char *name);

/*
 * Return the element's name.
 */
CSL_DEC const char *te_getname(tpselt te);

/*
 * Return this element's parent or NULL if it doesn't have one.
 */
CSL_DEC tpselt te_getparent(tpselt te);

/*
 * Create a new mutable element.
 */
CSL_DEC tpselt te_new(const char *name);

/*
 * Return the child after the one specified by the iterator or NULL if
 * that was the last one. The first time this is called, *iter should
 * be initialized to 0. On subsequent calls, the previous value should
 * be retained (it might not be sequential!).
 */
CSL_DEC tpselt te_nextchild(tpselt te, int *iter);

/*
 * Returns the nth child.  n is zero-indexed.  If n >= number of children
 * NULL is returned.
 */
CSL_DEC tpselt te_nthchild(tpselt te, int n);

/*
 * Set attribute value. The supplied value is copied into an internal
 * structure. The old value is thrown away. Returns whether operation
 * succeeded.
 */
CSL_DEC int te_setattr(tpselt te, const char *name, const char *in, size_t insize);

/*
 * Set attribute value to a C string. Comments as for te_setattr.
 */
CSL_DEC int te_setattrc(tpselt te, const char *name, const char *value);

/*
 * Set attribute value to a unsigned integer value.  This function will
 * convert the value to nework byte order and remove unnecessary leading
 * zero bytes. Comments as for te_setattr.
 */

CSL_DEC int te_setattru(tpselt te, const char *name, uint32 value);

/*
 * Set attribute value to a 64-bit unsigned integer value.  This function 
 * will convert the value to nework byte order and remove unnecessary 
 * leading zero bytes. Comments as for te_setattr.
 */

CSL_DEC int te_setattru64(tpselt te, const char *name, uint64 value);

/*
 * Set attribute value to a float value.  This function will
 * convert the value to nework byte order. Comments as for te_setattr.
 */

CSL_DEC int te_setattrf(tpselt te, const char *name, double value);


/*
 * Set attribute value to a double value.  This function will
 * convert the value to nework byte order. Comments as for te_setattr.
 */

CSL_DEC int te_setattrd(tpselt te, const char *name, double value);

/*
 * Unlink this element from its parent. It must have one.
 */
CSL_DEC void te_unlink(tpselt te);

/*
 * Clone an element.  Useful for converting a immutable element to a 
 * mutable one.
 */
CSL_DEC tpselt te_clone(tpselt te);

#endif

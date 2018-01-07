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
 * tpseltgen.c: created 2004/12/05 by Dima Dorfman.
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
 * Generic TPS element interface defined in tpselt.h. These functions
 * are either indirections through a vtable or are based on other
 * primitives and should work with any implementation.
 */

/* dmcpherson
 * de-brew
 * #include "nimbrew.h"
 */
#include "paltypes.h"
#include "tpselt.h"
#include "vec.h"
#include "cslutil.h"

CSL_DEF int
te_attach(tpselt te, tpselt che)
{
	return (te->vt->attach(te, che));
}

CSL_DEF void
te_dealloc(tpselt te)
{
    if (te && te->vt)
    {
        te->vt->dealloc(te);
    }
}

CSL_DEF void
te_delattr(tpselt te, const char *name)
{
    if (te && te->vt && name)
    {
        te->vt->delattr(te, name);
    }
}

CSL_DEF int
te_getattr(tpselt te, const char *name, char **out, size_t *outsize)
{
	return (te->vt->getattr(te, name, out, outsize));
}

CSL_DEF const char *
te_getattrc(tpselt te, const char *name)
{
	char *value = NULL;
	size_t vlen = 0;
	char *cp = NULL;
	int i = 0;

	/*
	 * We require the implementation to supply a nul terminator
	 * for every attribute. It only costs one byte.
	 *
	 * Subtle: We expect to find this byte *after* the specified size!
	 */
	i = te_getattr(te, name, &value, &vlen);
	if (!i)
		return (NULL);
	for (cp = value; cp < value + vlen; ++cp)
		if (*cp == '\0')
			return (NULL);
	nsl_assert(*cp == '\0');
	return (value);
}

CSL_DEF uint32 
te_getattru(tpselt te, const char *name)
{
	char *value = NULL;
	size_t vlen = 0;
	int i = -1 ,j = -1;
	byte val[4] = {0};

	/*
	 * We require the implementation to supply a nul terminator
	 * for every attribute. It only costs one byte.
	 *
	 * Subtle: We expect to find this byte *after* the specified size!
	 */
	i = te_getattr(te, name, &value, &vlen);
	if (!i || vlen > 4 || vlen < 1)
		return 0;

	*((uint32*) val) = 0;

	for (j=3,i=(int)vlen-1; i >= 0 ; --j,--i)
		val[j] = value[i];

	return nsl_ntohl(*((uint32*) val));
}

#define DIGIT_LIMIT 8

CSL_DEF int 
te_getattratoi(tpselt te, const char *name)
{
	char *value;
	size_t vlen;
	int i = -1;
	int v=0;

	/*
	* We require the implementation to supply a nul terminator
	* for every attribute. It only costs one byte.
	*
	* Subtle: We expect to find this byte *after* the specified size!
	*/
	i = te_getattr(te, name, &value, &vlen);
	if (!i || vlen > DIGIT_LIMIT || vlen < 1) // limited to 4 digits
		return 0;

	v = nsl_atoi(value);

	return v;
}

CSL_DEF int
te_getattru64(tpselt te, const char *name, uint64* pval)
{
	char *value;
	size_t vlen;
	int i,j;
	byte val[8];

	/*
	 * We require the implementation to supply a nul terminator
	 * for every attribute. It only costs one byte.
	 *
	 * Subtle: We expect to find this byte *after* the specified size!
	 */
	i = te_getattr(te, name, &value, &vlen);
	if (!i || vlen > 8 || vlen < 1)
		return 0;

	*((uint64*) val) = 0;

	for (j=7,i=(int)vlen-1; i >= 0 ; --j,--i)
		val[j] = value[i];

	str_rev_into(pval, val, 8);

	return (1);
}

CSL_DEF int
te_getattrf(tpselt te, const char *name, double *value)
{
	float f;
	char *v;
	size_t vlen;

	if (!te_getattr(te, name, &v, &vlen) || vlen != sizeof(f))
		return (0);
	str_rev_into(&f, v, sizeof(f));
	*value = f;
	return (1);
}

CSL_DEF int
te_getattrd(tpselt te, const char *name, double *value)
{
	char *v;
	size_t vlen;

	if (!te_getattr(te, name, &v, &vlen) || vlen != sizeof(*value))
		return (0);
	str_rev_into(value, v, sizeof(*value));
	return (1);
}

CSL_DEF struct CSL_Vector *
te_getattrs(tpselt te)
{

	return (te->vt->getattrs(te));
}

CSL_DEF tpselt
te_getchild(tpselt te, const char *name)
{
	tpselt xe;
	int i;

	for (i = 0; (xe = te_nextchild(te, &i)) != NULL; )
		if (nsl_strcmp(name, te_getname(xe)) == 0)
			return (xe);
	return (NULL);
}

CSL_DEF const char *
te_getname(tpselt te)
{

	return (te->vt->getname(te));
}

CSL_DEF tpselt
te_getparent(tpselt te)
{

	return (te->vt->getparent(te));
}

CSL_DEF tpselt
te_nextchild(tpselt te, int *iter)
{
	int i, *ip;

	if (iter == NULL) {
		i = 0;
		ip = &i;
	} else
		ip = iter;
	return (te->vt->nextchild(te, ip));
}

CSL_DEF tpselt
te_nthchild(tpselt te, int n)
{
	int i = -1;
	int iter = 0;
	tpselt elt;

	do {
		elt = te_nextchild(te, &iter);
		i++;
	} while (elt && i < n);
	return elt;
}

CSL_DEF int
te_setattr(tpselt te, const char *name, const char *in, size_t insize)
{

	return (te->vt->setattr(te, name, in, insize));
}

CSL_DEF int
te_setattrc(tpselt te, const char *name, const char *value)
{

	return (te_setattr(te, name, value, nsl_strlen(value)));
}

CSL_DEF int 
te_setattru(tpselt te, const char *name, uint32 value)
{
	uint32 nbo = nsl_htonl(value);
	const byte*  pb = (const byte*) &nbo;
	size_t nb = sizeof(nbo);

	while (*pb == 0 && nb > 1) {

		pb++;
		nb--;
	}

	return te_setattr(te, name, (const char*) pb, nb);
}

CSL_DEF int 
te_setattru64(tpselt te, const char *name, uint64 value)
{
	uint64 nbo = 0;
	const byte*  pb = (const byte*) &nbo;
	size_t nb = sizeof(nbo);

	str_rev_into(&nbo, &value, sizeof(value));

	while (*pb == 0 && nb > 1) {

		pb++;
		nb--;
	}

	return te_setattr(te, name, (const char*) pb, nb);
}

CSL_DEF int 
te_setattrf(tpselt te, const char *name, double value)
{
	float f = (float)value;
	char buf[sizeof(f)];

	str_rev_into(buf, &f, sizeof(f));
	return te_setattr(te, name, buf, sizeof(buf));
}

CSL_DEF int 
te_setattrd(tpselt te, const char *name, double value)
{
	char buf[sizeof(value)];

	str_rev_into(buf, &value, sizeof(value));
	return te_setattr(te, name, buf, sizeof(buf));
}

CSL_DEF void
te_unlink(tpselt te)
{

	te->vt->unlink(te);
}

CSL_DEF tpselt 
te_clone(tpselt te)
{
	tpselt ce, copy, child_copy;
	const char* name;
	struct CSL_Vector* attrs;
	int l, n;
	char* data;
	size_t size;

    if (te == NULL)
        return NULL;

	name = te_getname(te);

	if (name == NULL)
		return NULL;

	copy = te_new(name);

	if (copy == NULL)
		return NULL;

	attrs = te_getattrs(te);

	if (attrs != NULL) {
	
		l = CSL_VectorGetLength(attrs);
	
		for (n = 0; n < l; n++) {
	
			name = *((const char**)CSL_VectorGetPointer(attrs, n));

			if (!te_getattr(te, name, &data, &size))
				goto errexit;

			if (!te_setattr(copy, name, data, size))
				goto errexit;
		}
	
		CSL_VectorDealloc(attrs);
	}

	n = 0;
	while ((ce = te_nextchild(te, &n)) != NULL) {

		child_copy = te_clone(ce);

		if (child_copy == NULL)
			goto errexit;

		if (!te_attach(copy, child_copy)) {

			te_dealloc(child_copy);
			goto errexit;
		}
	}

	return copy;

errexit:
	te_dealloc(copy);
	return NULL;
}

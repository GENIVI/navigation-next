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
 * tpslib_preload.c: created 2004/12/05 by Dima Dorfman.
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
 * Preloaded template library support.
 */

#include "paltypes.h"
#include "palstdlib.h"

#include "csdict.h"
#include "ht.h"
#include "intpack.h"
#include "tpslib.h"
#include "vec.h"

struct tpslib {
	char id[TPSLIB_IDLEN];
	struct CSL_Dictionary *caps;
	struct CSL_Vector *templates;
	struct ht *byfmt;
};

struct cl_byte {
	tpselt te;
	struct CSL_Vector *attrs;
};

static int
bytecmp(void *abtc, void *atpl)
{
	struct cl_byte *btc = abtc;
	struct tpstpl *tpl = atpl;
	const char **attr;
	char **x;
	int i;
	const char* cname = te_getname(btc->te);

	if (nsl_strcmp(cname, tpl->name) != 0)
		return (0);
	x = tpl->attrs;
	for (x = tpl->attrs, i = 0; i < CSL_VectorGetLength(btc->attrs) && *x != NULL; ) {
		attr = CSL_VectorGetPointer(btc->attrs, i++);
		if (nsl_strcmp(*x++, *attr) != 0)
			return (0);
	}
	return (i == CSL_VectorGetLength(btc->attrs) && *x == NULL);
}

static int
attrcmp(const void *a, const void *b)
{
	const char * const *a_ = a, * const *b_ = b;

	return (nsl_strcmp(*a_, *b_));
}

CSL_DEF struct tpstpl *
tpslib_byte(struct tpslib *tl, tpselt te)
{
	struct cl_byte btc;
	struct tpstpl *tpl;
	const char **attr;
	const char* name;
	long hash;
	int i;

	btc.te = te;
	btc.attrs = te_getattrs(te);
	if (btc.attrs == NULL)
		return (NULL);
	CSL_VectorCoreSort(btc.attrs, attrcmp);
	name = te_getname(te);
	hash = bytehashs(name);
	for (i = 0; i < CSL_VectorGetLength(btc.attrs); ++i) {
		attr = CSL_VectorGetPointer(btc.attrs, i);
		hash ^= bytehashs(*attr);
	}
	tpl = *htlook(tl->byfmt, hash, bytecmp, &btc);
	CSL_VectorDealloc(btc.attrs);
	return (tpl);
}

CSL_DEF struct tpstpl *
tpslib_byid(struct tpslib *tl, tpstid id)
{

	if (id >= CSL_VectorGetLength(tl->templates))
		return (NULL);
	return (CSL_VectorGetPointer(tl->templates, id));
}

static int
#ifndef UNUSED_NDEFINED
tplcmp(void *a __unused, void *b __unused)
#else
tplcmp(void *a, void *b)
#endif
{

	/*
	 * XXX: Do real comparison? There can't be a match unless the
	 * template is duplicated, and for a preloaded library, we
	 * don't care if we can be made to waste our memory.
	 */
	return (0);
}

static int
fillbyfmt(struct tpslib *tl)
{
	struct tpstpl *tpl;
	void **slot;
	long hash;
	char **x;
	int i;

	tl->byfmt = htalloc(16);
	if (tl->byfmt == NULL)
		return (0);
	for (i = 0; i < CSL_VectorGetLength(tl->templates); ++i) {
		tpl = CSL_VectorGetPointer(tl->templates, i);
		hash = bytehashs(tpl->name);
		for (x = tpl->attrs; *x != NULL; ++x)
			hash ^= bytehashs(*x);
		slot = htlook(tl->byfmt, hash, tplcmp, tpl);
		if (htins(tl->byfmt, slot, hash, tpl) == NULL)
			return (0);
	}
	return (1);
}

static int
aptpl(struct tpslib *tl, const char *s, size_t len)
{
	const char *p, *x;
	struct tpstpl tpl;
	char *d, *q;
	char **b;
	int n;

	for (n = 0, p = s, x = s + len; p < x; ++p)
		if (*p == '\0')
			++n;
	if (p[-1] != '\0')
		return (0);
	b = nsl_malloc(n * sizeof(char *));
	if (b == NULL)
		return (0);
	d = nsl_malloc(len);
	if (d == NULL) {
		nsl_free(b);
		return (0);
	}
	nsl_memcpy(d, s, len);
	tpl.id = (tpstid)CSL_VectorGetLength(tl->templates);
	tpl.name = d;
	tpl.attrs = b;
	for (q = d + nsl_strlen(d) + 1, x = d + len; q < x; q += nsl_strlen(q) + 1)
		*b++ = q;
	*b = NULL;
	if (!CSL_VectorAppend(tl->templates, &tpl)) {
		nsl_free(tpl.name);
		nsl_free(tpl.attrs);
		return (0);
	}
	return (1);
}

static int
nextword(const char *text, size_t textlen, size_t *x, const char **word)
{
	const char *s, *p;

	s = text + *x;
	for (p = s; p < text + textlen; ++p)
		if (*p == '\0')
			break;
	if (*p != '\0')
		return (0);
	*word = s;
	*x += p - s + 1;
	return (1);
}

/*
 * XXX: The only reason for having the entire text available at once
 * is implementation simplicity.
 *
 * XXX: Check shasum
 */
CSL_DEF struct tpslib *
tpslib_preload(const char *text, size_t textlen)
{
	unsigned long i, ncaps;
	/* Initialize to prevent ARM warnings */
	const char *w1 = NULL, *w2 = NULL; 
	struct tpslib *tl;
	size_t x;
	
	tl = nsl_malloc(sizeof(*tl));
	if (tl == NULL)
		return (NULL);
	tl->byfmt = NULL;
	tl->caps = CSL_DictionaryAlloc(4);
	if (tl->caps == NULL) {
		nsl_free(tl);
		return (NULL);
	}
	tl->templates = CSL_VectorAlloc(sizeof(struct tpstpl));
	if (tl->templates == NULL) {
		CSL_DictionaryDealloc(tl->caps);
		nsl_free(tl);
		return (NULL);
	}
	if (textlen < TPSLIB_IDLEN) {
bad:		tpslib_dealloc(tl);
		return (NULL);
	}
	nsl_memcpy(tl->id, text, TPSLIB_IDLEN);
	x = TPSLIB_IDLEN;
	if (textlen - x < 4)
		goto bad;
	ncaps = beunpackl(text + x, 4);
	x += 4;
	while (ncaps-- > 0) {
		if (!nextword(text, textlen, &x, &w1))
			goto bad;
		if (!nextword(text, textlen, &x, &w2))
			goto bad;
		if (!CSL_DictionarySet(tl->caps, w1, w2, nsl_strlen(w2) + 1))
			goto bad;
	}
	while (x < textlen) {
		if (textlen - x < 4)
			goto bad;
		i = beunpackl(text + x, 4);
		x += 4;
		if (textlen - x < i)
			goto bad;
		if (!aptpl(tl, text + x, i))
			goto bad;
		x += i;
	}
	if (!fillbyfmt(tl))
		goto bad;
	return (tl);
}

CSL_DEF void
tpslib_dealloc(struct tpslib *tl)
{
	struct tpstpl *tpl;
	int i;

	for (i = 0; i < CSL_VectorGetLength(tl->templates); ++i) {
		tpl = CSL_VectorGetPointer(tl->templates, i);
		nsl_free(tpl->attrs);
		nsl_free(tpl->name);
	}
	CSL_VectorDealloc(tl->templates);
	CSL_DictionaryDealloc(tl->caps);
	if (tl->byfmt != NULL)
		htdealloc(tl->byfmt);
	nsl_free(tl);
}

CSL_DEF struct CSL_Dictionary *
tpslib_getcaps(struct tpslib *tl)
{

	return (tl->caps);
}

CSL_DEF void
tpslib_getid(struct tpslib *tl, char *out)
{

	nsl_memcpy(out, tl->id, sizeof(tl->id));
}

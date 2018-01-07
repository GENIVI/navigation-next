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
 * tpseltmut.c: created 2004/12/09 by Dima Dorfman.
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

//#include "nimbrew.h"
#include "paltypes.h"
#include "palstdlib.h"
#include "bsdqueue.h"
#include "csdict.h"
#include "tpselt.h"
#include "vec.h"
#include "paldebuglog.h"
#include "cslutil.h"

struct elt {
	struct tpseltvt *vt;
	char *name;
	struct CSL_Dictionary *attrs;
	struct elt *parent;
	STAILQ_ENTRY(elt) list;
	STAILQ_HEAD(eltshead, elt) children;
};

#define	te2e(te)	((struct elt *)te)

int
attach(tpselt te, tpselt che)
{
	struct elt *e = te2e(te);
	struct elt *ce = te2e(che);

	/*
	 * XXX: If we want to link to arbitrary elements, we have to
	 * expose a link API through the vtable.
	 */
	nsl_assert(ce->parent == NULL);
	ce->parent = e;
	STAILQ_INSERT_TAIL(&e->children, ce, list);
	return (1);
}

void
dealloc(tpselt te)
{
	struct elt *e = te2e(te);

	while (!STAILQ_EMPTY(&e->children))
		dealloc((tpselt)STAILQ_FIRST(&e->children));
	if (te_getparent(te) != NULL)
		te_unlink(te);
	nsl_free(e->vt);
	nsl_free(e->name);
	CSL_DictionaryDealloc(e->attrs);
	nsl_free(e);
}

void
delattr(tpselt te, const char *name)
{

	CSL_DictionaryDelete(te2e(te)->attrs, name);
}

int
getattr(tpselt te, const char *name, char **out, size_t *outsize)
{
	struct elt *e = te2e(te);

	*out = CSL_DictionaryGet(e->attrs, name, outsize);
	if (*out != NULL && outsize != NULL)
		--*outsize;	/* Strip the sentinel what we added. */
	return (*out != NULL);
}

struct CSL_Vector *
getattrs(tpselt te)
{
	struct elt *e = te2e(te);
	const char *key;
	struct CSL_Vector *v;
	int i;

	v = CSL_VectorAlloc(sizeof(char *));
	if (v == NULL)
		return (NULL);
	for (i = 0; CSL_DictionaryNext(e->attrs, &i, &key, NULL, NULL); )
		if (!CSL_VectorAppend(v, &key)) {
			CSL_VectorDealloc(v);
			return (NULL);
		}
	return (v);
}

const char *
getname(tpselt te)
{

	return (te2e(te)->name);
}

tpselt
getparent(tpselt te)
{

	return ((tpselt)te2e(te)->parent);
}

tpselt
nextchild(tpselt te, int *iter)
{
	struct elt *e = te2e(te);
	struct elt *xe;
	int i;

	i = 0;
	STAILQ_FOREACH(xe, &e->children, list)
		if (i == *iter) {
			*iter = i + 1;
			return ((tpselt)xe);
		} else
			++i;
	return (NULL);
}

int
setattr(tpselt te, const char *name, const char *in, size_t insize)
{
	struct elt *e = te2e(te);
	char *v = NULL;

	/* Subtle: getattrc expects to find a nul byte one after the size. */
	if (!CSL_DictionarySetEx(e->attrs, name, in, insize, 1))
		return (0);
	v = CSL_DictionaryGet(e->attrs, name, NULL);
	nsl_assert(v != NULL);
	v[insize] = '\0';
	return (1);
}

void
teunlink(tpselt te)
{
	struct elt *e = te2e(te);

	nsl_assert(e->parent != NULL);
	STAILQ_REMOVE(&e->parent->children, e, elt, list);
	e->parent = NULL;
}

static void
setvt(struct tpseltvt *vt)
{

	vt->attach = attach;
	vt->dealloc = dealloc;
	vt->delattr = delattr;
	vt->getattr = getattr;
	vt->getattrs = getattrs;
	vt->getname = getname;
	vt->getparent = getparent;
	vt->nextchild = nextchild;
	vt->setattr = setattr;
	vt->unlink = teunlink;
}

CSL_DEF tpselt
te_new(const char *name)
{
	struct elt *e;

	e = nsl_malloc(sizeof(*e));
	if (e == NULL)
		return (NULL);
	e->vt = nsl_malloc(sizeof(*e->vt));
	if (e->vt == NULL) {
out1:		nsl_free(e);
		return (NULL);
	}
	setvt(e->vt);
	e->name = nsl_strdup(name);
	if (e->name == NULL) {
out2:		nsl_free(e->vt);
		goto out1;
	}
	e->attrs = CSL_DictionaryAlloc(2);
	if (e->attrs == NULL) {
		nsl_free(e->name);
		goto out2;
	}
	e->parent = NULL;
	STAILQ_INIT(&e->children);
	return ((tpselt)e);
}

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
 * tpspack.c: created 2004/11/30 by Dima Dorfman.
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
 * Packing TPS elements.
 *
 *
 * TPS packet structure:
 * 
 *   +--------------------+----------------+--------------+------+
 *   | Number of elements | Tree structure | Template IDs | Data |
 *   +--------------------+----------------+--------------+------+
 *
 * Unsigned integers:
 *
 *   Several of the sections use "uints", which are variable-length
 *   unsigned integers. The value is packed, big end first, into the
 *   lower seven bits of a sequence of bytes, and the last byte is marked
 *   by setting its highest bit.
 *
 * Number of elements:
 *
 *   A uint indicating how many elements there are in a tree. This is
 *   sufficient to determine the length of the tree structure and to
 *   determine the end of the template IDs.
 *
 * Tree structure:
 *
 *   Two bits for every element, in preorder; the first bit indicates
 *   whether the element is the last sibling and the second bit indicates
 *   whether the element has no children. This format is based on TAOCP
 *   Theorem 2.3.1A. The unused lower bits of the last byte are filled
 *   with zeroes.
 *
 * Template IDs:
 *
 *   One uint, representing the template ID, for every element in the
 *   packet, in the same order as the tree structure. This section ends
 *   when the right number of template IDs have been unpacked. Since
 *   uints have a variable length, it's not possible to tell where this
 *   section ends begins without unpacking its contents.
 *
 * Data:
 *
 *   A sequence of (uint, value) pairs where the uint indicates the
 *   length of the value. One such pair for every attribute for every
 *   element in the packet, in the same order as the template IDs. The
 *   correct templates are required to correlate the attributes to the
 *   elements and to figure out where this section ends.
 */

#include "paltypes.h"
#include "palstdlib.h"
#include "paldebuglog.h"
#include "bq.h"
#include "csdict.h"
#include "intpack.h"
#include "tpselt.h"
#include "tpsio.h"
#include "tpslib.h"
#include "cslutil.h"

struct cl_tpspack {
	struct tpslib *tl;
	long nelts;
	struct bq tids;
	struct bq data;

	unsigned char *tree;
	int tree_len;
	int tree_asize;
	int tree_j;

	int obsecurity;
};

struct cl_packrec {
	struct cl_tpspack *tpc;
	tpselt te;
	int iter;
};

static int
aptree(struct cl_tpspack *tpc, int lsib, int trmn)
{
	unsigned char *p;

	if (tpc->tree == NULL) {
		tpc->tree_asize = 4;
		tpc->tree = nsl_malloc(tpc->tree_asize * sizeof(*tpc->tree));
		if (tpc->tree == NULL)
			return (0);
		tpc->tree_len = 1;
		tpc->tree_j = 0;
		tpc->tree[0] = 0;
	}
	if (tpc->tree_j == 4) {
		++tpc->tree_len;
		tpc->tree_j = 0;
		if (tpc->tree_len == tpc->tree_asize) {
			tpc->tree_asize <<= 1;
			if (tpc->tree_asize == 0)
				return (0);
			p = nsl_realloc(tpc->tree,
			    tpc->tree_asize * sizeof(*tpc->tree));
			if (p == NULL)
				return (0);
			tpc->tree = p;
		}
		tpc->tree[tpc->tree_len - 1] = 0;
	}
	nsl_assert(tpc->tree_j < 4);
	tpc->tree[tpc->tree_len - 1] |=
	    (lsib << 1 | trmn) << (6 - 2 * tpc->tree_j++);
	return (1);
}

static enum tpsio_status packrec(struct cl_tpspack *, tpselt, int);

static enum tpsio_status
packchildren(struct cl_packrec *pc, tpselt che, tpselt nche)
{
	enum tpsio_status stat;

	if (che == NULL && nche == NULL) {
		che = te_nextchild(pc->te, &pc->iter);
		if (che == NULL)
			return (TIO_READY);
		nche = te_nextchild(pc->te, &pc->iter);
		return (packchildren(pc, che, nche));
	}
	nsl_assert(che != NULL);
	stat = packrec(pc->tpc, che, nche == NULL);
	if (stat != TIO_READY)
		return (stat);
	if (nche == NULL)
		return (TIO_READY);
	return (packchildren(pc, nche, te_nextchild(pc->te, &pc->iter)));
}

static enum tpsio_status
packattrs(struct cl_tpspack *tpc, tpselt ce, char **attrs)
{
	char uintb[MAXLONGPACKLEN];
	size_t attrlen;
	char *attr;
	int i;

	for (; *attrs != NULL; ++attrs) {
		if (!te_getattr(ce, *attrs, &attr, &attrlen))
			return (TIO_MALTPL);
		i = uintpackl(attrlen, uintb);
		if (!csl_bqprep(&tpc->data, i + attrlen))
			return (TIO_NOMEM);
		nsl_memcpy(tpc->data.w, uintb, i);
		nsl_memcpy(tpc->data.w + i, attr, attrlen);
		if (tpc->obsecurity)
			obsecure(tpc->data.w + i, attrlen);
		tpc->data.w += i + attrlen;
	}
	return (TIO_READY);
}

static enum tpsio_status
packrec(struct cl_tpspack *tpc, tpselt ce, int lsib)
{
	char uintb[MAXLONGPACKLEN];
	enum tpsio_status stat;
	struct cl_packrec pc;
	struct tpstpl *tpl;
	int i;

	tpl = tpslib_byte(tpc->tl, ce);
	if (tpl == NULL) {
//		DEBUGLOG(LOG_SS_TPS, LOG_SEV_MAJOR, ("can't find template for %s", te_getname(ce)));
		return (TIO_NOTPL);
	}
	stat = packattrs(tpc, ce, tpl->attrs);
	if (stat != TIO_READY)
		return (stat);
	i = uintpackl(tpl->id, uintb);
	if (!csl_bqprep(&tpc->tids, i))
		return (TIO_NOMEM);
	nsl_memcpy(tpc->tids.w, uintb, i);
	tpc->tids.w += i;
	if (!aptree(tpc, lsib, te_nextchild(ce, NULL) == NULL))
		return (TIO_NOMEM);
	++tpc->nelts;
	pc.tpc = tpc;
	pc.te = ce;
	pc.iter = 0;
	return (packchildren(&pc, NULL, NULL));
}

CSL_DEF enum tpsio_status
tps_pack(struct tpslib *tl, tpselt te, tpspackreader *reader, void *arg)
{
	char uintb[MAXLONGPACKLEN];
	enum tpsio_status stat;
	struct cl_tpspack tpc;
	struct CSL_Dictionary *caps;
	int i;

	tpc.tl = tl;
	tpc.nelts = 0;
	tpc.tree = NULL;
	caps = tpslib_getcaps(tl);
	tpc.obsecurity = caps!=NULL && CSL_DictionaryGet(caps, "obsecurity", NULL)!=NULL;
	csl_bqinit(&tpc.tids);
	csl_bqinit(&tpc.data);
	stat = packrec(&tpc, te, 1);
	if (stat != TIO_READY)
		goto out;
	nsl_assert(tpc.nelts > 0 && tpc.tree != NULL);
	nsl_assert(tpc.tids.r < tpc.tids.w  && tpc.data.r <= tpc.data.w);
	i = uintpackl(tpc.nelts, uintb);
#define	FEED(b, n)	if ((stat = reader(arg, b, n)) != TIO_READY) goto out;
	FEED(uintb, i);
	FEED((char *)tpc.tree, tpc.tree_len);
	FEED(tpc.tids.r, tpc.tids.w - tpc.tids.r);
	FEED(tpc.data.r, tpc.data.w - tpc.data.r);
#undef FEED
	stat = TIO_READY;
out:	nsl_free(tpc.tree);
	csl_bqfree(&tpc.tids);
	csl_bqfree(&tpc.data);
	return (stat);
}

void
obsecure(char *s, size_t len)
{
	char *x;

	for (x = s + len; s < x; ++s)
		*s ^= 0xff;
}

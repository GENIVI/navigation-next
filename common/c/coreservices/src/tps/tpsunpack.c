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
 * tpsunpack.c: created 2004/12/04 by Dima Dorfman.
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
 * Unpacking TPS documents.
 *
 * Since we don't have to support mutation, we implement the tpselt
 * interface in terms of fixed, sequential allocations.
 */

/*
 * #include "nimbrew.h"
 */
#include "paltypes.h"
#include "csdict.h"
#include "intpack.h"
#include "tpselt.h"
#include "tpsio.h"
#include "tpslib.h"
#include "vec.h"
#include "cslutil.h"

/*
 * Type of absolute element addresses. This limits the number of
 * supported elements per document. If this needs to be longer than
 * long, the uintunpackl calls have to be changed appropriately.
 */
typedef short eltaddr;

struct doc;

struct uvt {
    struct tpseltvt vt;
    struct doc *doc;
};

struct elt {
    struct uvt *vt;
    struct tpstpl *tpl;
    eltaddr sib;
    eltaddr parent;
    size_t attrs;
};

struct attr {
    size_t len;
    size_t data;
};

struct doc {
    struct uvt vt;
    long nelts;
    struct elt *elts;
    struct attr *attrs;
    char *data;
};

struct telt {
    struct tpstpl *tpl;
    unsigned int lsib : 1;
    unsigned int trmn : 1;
};

struct tps_unpackstate {
    struct tpslib *tl;
    size_t packetlen;
    nb_size (*section)(struct tps_unpackstate *, const char *, size_t);
    enum tpsio_status status;
    size_t consumed;
    long nelts, nattrs;
    struct telt *telts;
    eltaddr x;
    unsigned long y;
    struct doc *doc;
    char *dx;
    int obsecurity;
};


static void
dealloc(tpselt te)
{
    struct elt *s = (struct elt *)te;
    struct doc *doc = s->vt->doc;

    nsl_assert(s == doc->elts);
    nsl_free(doc);
}

static int
getattr(tpselt te, const char *name, char **value, size_t *valsize)
{
    struct elt *s = (struct elt *)te;
    struct doc *doc = s->vt->doc;
    char **x = NULL;
    long i = -1;

    for (x = s->tpl->attrs; *x != NULL; ++x)
        if (nsl_strcmp(*x, name) == 0) {
            i = s->attrs + (x - s->tpl->attrs);
            *valsize = doc->attrs[i].len;
            *value = doc->data + doc->attrs[i].data;
            return (1);
        }
    return (0);
}

static struct CSL_Vector *
getattrs(tpselt te)
{
    struct elt *s = (struct elt *)te;
    struct CSL_Vector *v;
    char **x;

    v = CSL_VectorAlloc(sizeof(char *));
    if (v == NULL)
        return (NULL);
    for (x = s->tpl->attrs; *x != NULL; ++x)
        if (!CSL_VectorAppend(v, x)) {
            CSL_VectorDealloc(v);
            return (NULL);
        }
    return (v);
}

static const char *
getname(tpselt te)
{
    struct elt *s = (struct elt *)te;

    return (s->tpl->name);
}

static tpselt
getparent(tpselt te)
{
    struct elt *s = (struct elt *)te;
    struct doc *doc = s->vt->doc;

    return ((tpselt)(doc->elts + s->parent));
}

static tpselt
nextchild(tpselt te, int *iter)
{
    struct elt *s = (struct elt *)te;
    struct doc *doc = s->vt->doc;
    struct elt *cur;
    int next = -1 , self = -1;

    self = (int)(s - doc->elts);
    if (*iter == 0) {
        next = self + 1;
        if (next == doc->nelts)
            return (NULL);
        if (doc->elts[next].parent != self)
            return (NULL);
    } else {
        cur = doc->elts + *iter;
        if (cur->sib == -1)
            return (NULL);
        next = cur->sib;
        nsl_assert(doc->elts[next].parent == cur->parent);
    }
    *iter = next;
    return ((tpselt)(doc->elts + next));
}

static void
setvt(struct tpseltvt *t)
{

    t->attach = NULL;
    t->dealloc = dealloc;
    t->delattr = NULL;
    t->getattr = getattr;
    t->getattrs = getattrs;
    t->getname = getname;
    t->getparent = getparent;
    t->nextchild = nextchild;
    t->setattr = NULL;
    t->unlink = NULL;
}

static nb_size unpack_attr1(struct tps_unpackstate *, const char *, size_t);

static nb_size
unpack_attr2(struct tps_unpackstate *us, const char *data, size_t datalen)
{
    size_t i;

    i = us->y;
    if (i > datalen)
        i = datalen;
    nsl_memcpy(us->dx, data, i);
    if (us->obsecurity)
        obsecure(us->dx, i);
    us->dx += i;
    us->y -= i;
    if (us->y == 0) {
        *us->dx++ = '\0';
        us->section = unpack_attr1;
    }
    return (i);
}

static nb_size
unpack_attr1(struct tps_unpackstate *us, const char *data, size_t datalen)
{
    unsigned long len;
    int i;

    if (us->consumed + datalen > us->packetlen)
        datalen = us->packetlen - us->consumed;
    if (us->x == us->nattrs) {
        if (us->consumed != us->packetlen) {
            us->status = TIO_MALDOC;
            return (-1);
        }
        return (0);
    }
    i = uintunpackl(data, datalen, &len);
    if (i == -1)
        return (0);

    /* Don't use signed short as index here. If number of attrs exceeds maximum value of
       signed short, it will cause buffer underflow. */
    us->doc->attrs[(uint16)(us->x)].len = len;
    us->doc->attrs[(uint16)(us->x)].data = us->dx - us->doc->data;
    ++us->x;
    us->y = len;
    us->section = unpack_attr2;
    return (i);
}

static int
countattrstpl(struct tpstpl *tpl)
{
    char **x;
    int n;

    for (n = 0, x = tpl->attrs; *x != NULL; ++x)
        ++n;
    return (n);
}

static int
countattrstelt(struct tps_unpackstate *us)
{
    int i;
    int n;

    for (n = 0, i = 0; i < us->nelts; ++i)
        n += countattrstpl(us->telts[i].tpl);
    return (n);
}

static enum tpsio_status
makedoc(struct tps_unpackstate *us)
{
    enum tpsio_status stat;
    struct CSL_Vector *sibs;
    struct elt *e, *s;
    int attrs, upnext;
    struct doc *doc;
    struct telt *t;
    int i;
    uint32 result = 0;
    uint32 size1 = 0;
    uint32 size2 = 0;

    /*
     * The allocation for the document has four sections:
     *
     *   struct doc | array of elts | array of attrs | attribute data
     *
     * The attribute data section is sized based on the size of
     * the data section of the TPS document, which we know by now
     * because we've unpacked everything before it and we required
     * the user to tell us the packet length. The TPS doc stores
     * the lengths of the values near them, but we store them in
     * the attrs array for faster access. We only need one extra
     * byte besides the value (for the nul terminator), so the
     * rest of those bytes are wasted. This waste is very small
     * compared to the length of the values.
     */
    us->nattrs = countattrstelt(us);
    nsl_assert(us->consumed <= us->packetlen);
    size1 = (uint32)(sizeof(*doc) + us->nelts * sizeof(*doc->elts) + us->nattrs * sizeof(*doc->attrs));
    size2 = (uint32)(us->packetlen - us->consumed);
    result = size1 + size2;
    if( result < size1 )
    {
        return (TIO_NOMEM);
    }

    doc = nsl_malloc(result);
    if (doc == NULL)
        return (TIO_NOMEM);
    setvt(&doc->vt.vt);
    doc->vt.doc = doc;
    doc->nelts = us->nelts;
    /* XXX: elts and attrs may require alignment. */
    doc->elts = (struct elt *)((char *)doc + sizeof(*doc));
    doc->attrs = (struct attr *)((char *)doc->elts +
        doc->nelts * sizeof(*doc->elts));
    doc->data = (char *)doc->attrs + us->nattrs * sizeof(*doc->attrs);
    sibs = CSL_VectorAlloc(sizeof(e));
    if (sibs == NULL) {
        stat = TIO_NOMEM;
bad1:        nsl_free(doc);
        return (stat);
    }
    for (attrs = upnext = i = 0; i < doc->nelts; ++i) {
        e = doc->elts + i;
        t = us->telts + i;
        e->vt = &doc->vt;
        e->tpl = t->tpl;
        if (upnext) {
            if (CSL_VectorGetLength(sibs) == 0) {
maltree:            stat = TIO_MALTREE;
bad2:                CSL_VectorDealloc(sibs);
                goto bad1;
            }
            CSL_VectorPop(sibs, &s);
            s->sib = (eltaddr)i;
            e->parent = s->parent;
            upnext = 0;
        } else if (i == 0)
            e->parent = -1;
        else
            e->parent = (eltaddr)i - 1;
        if (t->trmn)
            upnext = 1;
        if (t->lsib)
            e->sib = -1;
        else if (!CSL_VectorAppend(sibs, &e)) {
            stat = TIO_NOMEM;
            goto bad2;
        }
        e->attrs = attrs;
        attrs += countattrstpl(e->tpl);
        nsl_assert(attrs <= us->nattrs);
    }
    if (CSL_VectorGetLength(sibs) != 0)
        goto maltree;
    CSL_VectorDealloc(sibs);
    nsl_free(us->telts);
    us->doc = doc;
    return (TIO_READY);
}

static nb_size
unpack_makedoc(struct tps_unpackstate *us, const char *data, size_t datalen)
{

    us->status = makedoc(us);
    if (us->status != TIO_READY)
        return (-1);
    us->x = 0;
    us->dx = us->doc->data;
    us->section = unpack_attr1;
    return (0);
}

static nb_size
unpack_tids(struct tps_unpackstate *us, const char *data, size_t datalen)
{
    unsigned long tid;
    size_t i;
    int j;

    for (i = 0; i < datalen && us->x < us->nelts; ++us->x) {
        j =  uintunpackl(data + i, datalen - i, &tid);
        if (j == -1)
            return (i);
        us->telts[us->x].tpl = tpslib_byid(us->tl, (tpstid)tid);
        if (us->telts[us->x].tpl == NULL) {
            us->status = TIO_NOTPL;
            return (-1);
        }
        i += j;
    }
    if (us->x == us->nelts)
        us->section = unpack_makedoc;
    return (nb_size)(i);
}

static nb_size
unpack_tree(struct tps_unpackstate *us, const char *data, size_t datalen)
{
    unsigned char ch;
    const char *cp;
    long i, j;

    for (cp = data; cp < data + datalen && us->x < us->nelts; ++cp) {
        ch = (unsigned char)*cp;
        i = us->nelts - us->x;
        if (i > 4)
            i = 4;
        for (j = 3; j >= 4 - i; --j, ++us->x) {
            nsl_assert(us->x < us->nelts);
            us->telts[us->x].lsib = ch >> (j * 2 + 1) & 1;
            us->telts[us->x].trmn = ch >> (j * 2) & 1;
        }
    }
    if (us->x == us->nelts) {
        us->x = 0;
        us->section = unpack_tids;
    }
    return (nb_size)(cp - data);
}

static nb_size
unpack_nelts(struct tps_unpackstate *us, const char *data, size_t datalen)
{
    unsigned long l;
    nb_size i;

    i = uintunpackl(data, datalen, &l);
    if (i == -1)
        return (0);
    us->nelts = l;
    if (us->nelts >= 1 << (sizeof(eltaddr) * 8 - 1)) {
        us->status = TIO_NOMEM;
        return (-1);
    }
    if (us->nelts < 1) {
        us->status = TIO_MALDOC;
        return (-1);
    }
    us->telts = nsl_malloc(us->nelts * sizeof(*us->telts));
    if (us->telts == NULL)
        return (-1);
    us->x = 0;
    us->section = unpack_tree;
    return (i);
}

CSL_DEF struct tps_unpackstate *
tps_unpack_start(struct tpslib *tl, size_t packetlen)
{
    struct tps_unpackstate *us;
    struct CSL_Dictionary *caps;

    /*
     * This "optimization" is required. Not having it would
     * require being able to extend the data section, which we
     * don't want to support yet.
     */
    nsl_assert(packetlen > 0);

    us = nsl_malloc(sizeof(*us));
    if (us == NULL)
        return (NULL);
    nsl_memset(us, 0, sizeof(*us));
    us->tl = tl;
    us->consumed = 0;
    us->packetlen = packetlen;
    us->status = TIO_READY;
    us->section = unpack_nelts;
    caps = tpslib_getcaps(tl);
    us->obsecurity = caps!=NULL && CSL_DictionaryGet(caps, "obsecurity", NULL)!=NULL;
    us->telts = NULL;
    return (us);
}

CSL_DEF nb_size
tps_unpack_feed(struct tps_unpackstate *us, const char *data, size_t datalen)
{
    nb_size (*f)(struct tps_unpackstate *, const char *, size_t);
    nb_size i;
    nb_size j;

    j = 0;
    do {
        f = us->section;
        i = f(us, data + j, datalen - j);
        if (i == -1)
            return (-1);
        j += i;
        us->consumed += i;
        nsl_assert(us->consumed <= us->packetlen);
    } while (us->section != f);
    return (j);
}

CSL_DEF enum tpsio_status
tps_unpack_result(struct tps_unpackstate *us, tpselt *tep)
{
    enum tpsio_status stat;

    if (us->section == unpack_attr1 && (uint16)us->x == us->nattrs &&
        us->status == TIO_READY) {
        *tep = (tpselt)us->doc->elts;
        nsl_free(us);
        return (TIO_READY);
    } else {
        if (us->section == unpack_attr1 || us->section == unpack_attr2)
            nsl_free(us->doc);
        else if (us->section != unpack_nelts)
            nsl_free(us->telts);
        stat = us->status;
        nsl_free(us);
        return (stat == TIO_READY ? TIO_MALDOC : stat);
    }
}

size_t tps_packet_len(struct tps_unpackstate *us)
{
    if (!us)
    {
        return (size_t)-1;
    }
    return us->packetlen;
}

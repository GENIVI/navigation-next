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
 * ht.c: created 2004/11/12 by Dima Dorfman.
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
 * Hash table.
 *
 * The basic algorithm is due to Knuth and the collision resolution
 * strategy is due to the Python dictobject code (probably Tim Peters'
 * work). A lot of the finer details were also borrowed from
 * dictobject. Many thanks!
 */

/*
 * dmcpherson de-brew
 * #include "nimbrew.h"
 */
#include "paltypes.h" 
#include "palstdlib.h"
#include "ht.h"
#include "cslutil.h"

/*
 * Slot entry. The table is an array of these structures. hash is the
 * hash value or -1 if this is a dummy entry. entry is the user's
 * entry pointer if this slot is active and NULL otherwise.
 *
 * A slot is
 *  - unused if entry is NULL and hash is not -1;
 *  - active if entry is non-NULL; and
 *  - dummy if entry is NULL and hash is -1.
 */
struct htslot {
	long hash;
	void *entry;
};

/* Collision resolution strategy. See Python's dictobject.c. */
#define	PERTURB_SHIFT	5

/*
 * Find an entry in the table. This function returns a slot pointer;
 * it may be dereferenced to get the entry (which may be NULL if there
 * was no matching entry) or passed to another ht* function. It is
 * valid until the next resize (an htins into an inactive slot).
 */
void **
htlook(struct ht *hp, long hash, cmpfun *cfun, void *cfarg)
{
	struct htslot *sp, *fsp;
	unsigned perturb; // don't modify, this has to be unsinged value!
	unsigned i;

	nsl_assert(hp->fill <= hp->mask);
	i = hash & hp->mask;
	sp = hp->table + i;
	if (sp->entry != NULL) {
		if (sp->hash == hash && cfun(cfarg, sp->entry))
			return (&sp->entry);
		fsp = NULL;
	} else {
		if (sp->hash != -1)
			return (&sp->entry);
		fsp = sp;
	}
	for (perturb = hash; ; perturb >>= PERTURB_SHIFT) {
		i = (i << 2) + i + perturb + 1;
		sp = hp->table + (i & hp->mask);
		if (sp->entry != NULL) {
			if (sp->hash == hash && cfun(cfarg, sp->entry))
				return (&sp->entry);
		} else {
			if (sp->hash != -1)
				return (&(fsp != NULL ? fsp : sp)->entry);
			else if (fsp != NULL)
				fsp = sp;
		}
	}
	/* Must have one unused slot for the loop to terminate. */
}

/*
 * Comparison function that doesn't match anything. Used to find free slots.
 */
static int
#ifndef UNUSED_NDEFINED
nullcmp(void *a __unused, void *b __unused)
#else
nullcmp(void *a, void *b)
#endif
{

	return (0);
}

/*
 * Convert the user slot pointer into a pointer to its htslot.
 */
static struct htslot *
userslot(struct ht *hp, void **slot)
{
	char *cs = (char *)slot;
	char *ct = (char *)hp->table;

	nsl_assert(cs >= ct && cs < (char *)(hp->table + hp->mask + 1));
	return (hp->table + (cs - ct) / sizeof(*hp->table));
}

static int
resize(struct ht *hp, int minused)
{
	struct htslot *newtable, *oldtable;
	int i, newsize, oldused;
	struct htslot *sp;
	void **slot;

	newsize = hp->minsize;
	while (newsize <= minused && newsize > 0)
		newsize <<= 1;
	if (newsize <= 0)
		return (-1);
	newtable = nsl_malloc(sizeof(*newtable) * newsize);
	if (newtable == NULL)
		return (-1);
	nsl_memset(newtable, 0, sizeof(*newtable) * newsize);
	oldtable = hp->table;
	oldused = hp->used;
	i = hp->fill;
	hp->table = newtable;
	hp->fill = hp->used = 0;
	hp->mask = newsize - 1;
	for (sp = oldtable; i > 0; ++sp)
		if (sp->entry != NULL) {
			slot = htlook(hp, sp->hash, nullcmp, NULL);
			(void)htins(hp, slot, sp->hash, sp->entry);
			--i;
		} else if (sp->hash == -1)
			--i;
	nsl_free(oldtable);
	nsl_assert(hp->used == oldused);
	return (0);
}

/*
 * Insert an entry into the table. slot is a slot pointer returned by
 * htlook and hash and entry are the entry parameters. hash must be
 * the same as what was passed to htlook. The slot must not be already
 * active; if it is, the user should just write the new entry into the
 * slot pointer.
 *
 * Because it may be necessary to resize the table, this function can
 * fail if memory allocation fails, in which case it returns NULL;
 * otherwise, it returns a pointer to the slot it filled, which may be
 * different if the table was resized.
 */
void **
htins(struct ht *hp, void **slot, long hash, void *entry)
{
	struct htslot *sp;
	int new;

	sp = userslot(hp, slot);
	nsl_assert(sp->entry == NULL);
	new = sp->hash != -1;
	if ((hp->fill + new) * 3 >= (hp->mask + 1) * 2) {
		if (resize(hp, hp->used * 2) == -1)
			return (NULL);
		sp = userslot(hp, htlook(hp, hash, nullcmp, NULL));
	}
	if (new)
		++hp->fill;
	sp->hash = hash;
	sp->entry = entry;
	++hp->used;
	return (&sp->entry);
}

/*
 * Clear the entry at the given slot.
 */
void
htclr(struct ht *hp, void **slot)
{
	struct htslot *sp;

	sp = userslot(hp, slot);
	nsl_assert(sp->entry != NULL);
	sp->hash = -1;
	sp->entry = NULL;
	--hp->used;
}

/*
 * Iterate over active slots. iter should be set to 0 initially and
 * not modified by the user after the first call. The table must not
 * be modified during the iteration.
 */
void **
htnext(struct ht *hp, int *iter)
{
	int i;

	for (i = *iter; i <= hp->mask; ++i)
		if (hp->table[i].entry != NULL) {
			*iter = i + 1;
			return (&hp->table[i].entry);
		}
	*iter = hp->mask + 1;
	return (NULL);
}

int
htlen(struct ht *hp)
{

	return (hp->used);
}

/*
 * Allocate a new hash table.
 */
struct ht *
htalloc_extra(int minsize, size_t extra)
{
	struct ht *hp;
	int newsize;

	hp = nsl_malloc(sizeof(*hp)+extra);
	if (hp == NULL)
		return (NULL);
	newsize = minsize;
	for (newsize = 4; newsize < minsize && newsize > 0; newsize <<= 1)
		;
	nsl_assert((newsize & (newsize - 1)) == 0);
	hp->minsize = newsize;
	hp->fill = hp->used = 0;
	hp->mask = hp->minsize - 1;
	hp->table = nsl_malloc(sizeof(*hp->table) * hp->minsize);
	if (hp->table == NULL) {
		nsl_free(hp);
		return (NULL);
	}
	nsl_memset(hp->table, 0, sizeof(*hp->table) * hp->minsize);
	return (hp);
}

struct ht *
htalloc(int minsize)
{
	return htalloc_extra(minsize, 0);
}


/*
 * Deallocate the hash table.
 */
void
htdealloc(struct ht *hp)
{

	nsl_free(hp->table);
	nsl_free(hp);
}

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
 * CSL_Dictionary.c: created 2004/12/05 by Dima Dorfman.
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
 * C string dictionary. Keys must be C strings, but values may be
 * arbitrary byte sequences.
 */

/*
 * dmcpherson de-brew
 * #include "nimbrew.h"
 */

#include "paltypes.h"
#include "palstdlib.h"
#include "csdict.h"
#include "ht.h"
#include "cslutil.h"

struct CSL_Dictionary 
{
	struct ht	ht;
	CSL_DictionaryDeleteFunction*	delfunc;
	void*		deluser;
};

/* A dict is just a hash table with extra data appended, saving an extra allocation. */
#define	d2hp(dictionary)		((struct ht *)dictionary)
#define	hp2d(hp)	((struct CSL_Dictionary *)hp)

/* The entry allocation is: key length | value length | key | value. */
#define	EKLN(e)		(*(size_t *)e)
#define	EVLN(e)		(*((size_t *)e + 1))
#define	EKEY(e)		((char *)e + 2 * sizeof(size_t))
#define	EVAL(e)		(EKEY(e) + EKLN(e))

struct entry 
{
	long hash;
	char *key;
	char *value;
};


// Local functions ...............................................................................

static int
cmpkey(void *e, void *ks)
{
	return (nsl_strcmp(e, EKEY(ks)) == 0);
}

static void **
look(struct CSL_Dictionary *dictionary, const char *key)
{
	return (htlook(d2hp(dictionary), bytehash(key, nsl_strlen(key)), cmpkey, (char *)key));
}


// Public functions ..............................................................................

/* See header file for description */
CSL_DEF struct CSL_Dictionary *
CSL_DictionaryAlloc(int minsize)
{
	return CSL_DictionaryAllocEx(minsize, NULL, NULL);
}

/* See header file for description */
CSL_DEF struct CSL_Dictionary *
CSL_DictionaryAllocEx(int minsize, CSL_DictionaryDeleteFunction delfunc, void* deluserdata)
{
	struct CSL_Dictionary *csd;
	csd = (hp2d(htalloc_extra(minsize, sizeof(struct CSL_Dictionary)-sizeof(struct ht))));
	
	csd->delfunc = delfunc;
	csd->deluser = deluserdata;

	return csd;
}

/* See header file for description */
CSL_DEF void
CSL_DictionaryDelete(struct CSL_Dictionary *dictionary, const char *key)
{
	CSL_DictionaryDeleteEx(dictionary, key, 1);
}

/* See header file for description */
CSL_DEF void
CSL_DictionaryDeleteEx(struct CSL_Dictionary *dictionary, const char *key, int assert_present)
{
	void **slot;

	slot = look(dictionary, key);
	if (assert_present) 
    {
		nsl_assert(*slot != NULL);
    }

	if (*slot != NULL) 
    {
		if (dictionary->delfunc != NULL) 
        {
			dictionary->delfunc(dictionary->deluser, EKEY(*slot), EVAL(*slot), EVLN(*slot));
		}
		nsl_free(*slot);
		htclr(d2hp(dictionary), slot);
	}
}


/* See header file for description */
CSL_DEF void
CSL_DictionaryDealloc(struct CSL_Dictionary *dictionary)
{
	void **slot;
	int i = 0;

    if(!dictionary)
    {
        return;
    }
	for (i = 0; (slot = htnext(d2hp(dictionary), &i)) != NULL; ) 
    {
		if (dictionary->delfunc != NULL) 
        {
			dictionary->delfunc(dictionary->deluser, EKEY(*slot), EVAL(*slot), EVLN(*slot));
		}
		nsl_free(*slot);
	}
	htdealloc(d2hp(dictionary));
}

/* See header file for description */
CSL_DEF char *
CSL_DictionaryGet(struct CSL_Dictionary *dictionary, const char *key, size_t *valueLength)
{
	void **slot;

	slot = look(dictionary, key);
	if (*slot == NULL)
    {
		return (NULL);
    }
	if (valueLength != NULL)
    {
		*valueLength = EVLN(*slot);
    }
	return (EVAL(*slot));
}

/* See header file for description */
CSL_DEF int
CSL_DictionaryLength(struct CSL_Dictionary *dictionary)
{
	return (htlen(d2hp(dictionary)));
}

/* See header file for description */
CSL_DEF int
CSL_DictionaryNext(struct CSL_Dictionary *dictionary, int *iter, const char **key, char **value,size_t *valueLength)
{
	void **slot;

	slot = htnext(d2hp(dictionary), iter);
	if (slot == NULL)
    {
		return (0);
    }
	if (key != NULL)
    {
		*key = EKEY(*slot);
    }
	if (value != NULL)
    {
		*value = EVAL(*slot);
    }
	if (valueLength != NULL)
    {
		*valueLength = EVLN(*slot);
    }
	return (1);
}

/* See header file for description */
CSL_DEF int
CSL_DictionarySet(struct CSL_Dictionary *dictionary, const char *key, const char *value, size_t valueLength)
{
	return (CSL_DictionarySetEx(dictionary, key, value, valueLength, 0));
}

/* See header file for description */
CSL_DEF int
CSL_DictionaryIteration(struct CSL_Dictionary *dictionary, CSL_DictionaryIterationFunction func, void* userdata)
{
	int i = 0;
	void **slot;

	if (func == NULL || dictionary == NULL)
    {
		return 0;
    }

	for (i = 0; (slot = htnext(d2hp(dictionary), &i)) != NULL; ) 
    {
		if (!func(userdata, EKEY(*slot), EVAL(*slot), EVLN(*slot))) 
        {
			/* delete the item*/
			if (dictionary->delfunc != NULL) 
            {
				dictionary->delfunc(dictionary->deluser, EKEY(*slot), EVAL(*slot), EVLN(*slot));
			}
			nsl_free(*slot);
			htclr(d2hp(dictionary), slot);
		}
	}

	return 1;
}

/* See header file for description */
CSL_DEF int
CSL_DictionarySetEx(struct CSL_Dictionary *dictionary, const char *key, const char *value, size_t valueLength,
    size_t extra)
{
	size_t klen = 0;
	size_t klen_pad = 0;
	void **slot = NULL;
	void *e = NULL;

	slot = look(dictionary, key);
	if (*slot != NULL) 
    {
		if (dictionary->delfunc != NULL) 
        {
			dictionary->delfunc(dictionary->deluser, EKEY(*slot), EVAL(*slot), EVLN(*slot));
		}
		nsl_free(*slot);
		htclr(d2hp(dictionary), slot);
		slot = look(dictionary, key);	/* Possibly find a better slot. */
	}
	klen = nsl_strlen(key);
	klen_pad = klen + 1;
	if (klen_pad%4)
    {
		klen_pad += (4 - (klen_pad%4)); // align the value in case it is a struct
    }
	e = nsl_malloc(2 * sizeof(size_t) + klen_pad + valueLength + extra);
	if (e == NULL)
    {
		return (0);
    }
	EKLN(e) = klen_pad;
	EVLN(e) = valueLength + extra;
	nsl_strcpy(EKEY(e), key);
	nsl_memcpy(EVAL(e), value, valueLength);
	return (htins(d2hp(dictionary), slot, bytehash(key, klen), e) != NULL);
}

/*
 * Byte sequence hashing. This algorithm is from Python's string_hash.
 */
CSL_DEF long
bytehash(const char *s, size_t size)
{
	long len = 0;
	long x = 0;

	len = size;
	x = *s << 7;
	while (--len >= 0)
    {
		x = (1000003 * x) ^ *s++;
    }
	x ^= size;
	return (x);
}

CSL_DEF long
bytehashs(const char *s)
{
	return (bytehash(s, nsl_strlen(s)));
}

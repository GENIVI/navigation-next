/*****************************************************************************
 *
 *  tsthaidi.h - Thai dictionary support.
 *
 *  Copyright (C) 2004-2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSTHAIDI_H
#define TSTHAIDI_H

#include "tstypes.h"
#include "tstxtinf.h"
#include "tsrsrc.h"
#include "tstag.h"

TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  The TsThaiDict object.
 *
 *  Description:
 *      This object provides a means by which WTLE or a WT Shaper client
 *      provides WT Shaper with a dictionary of Thai words for use in
 *      determining Thai word boundaries and potential line break points.
 *
 *  <GROUP thaidict>
 */
typedef struct TsThaiDict_ TsThaiDict;


/*****************************************************************************
 *
 *  Creates a new memory managed TsThaiDict object.
 *
 *  Parameters:
 *      memMgr      - [in] pointer to a memory manager object or NULL
 *      resource    - [in] a properly initialized TsResource object
 *
 *  Description:
 *      This function returns an opaque handle to an object that can be passed
 *      to TsShaper_markBoundaries (for clients of WT Shaper). The resource
 *      must be initialized with the compiled Thai dictionary file that is
 *      shipped with the SDK.
 *
 *      WTLE clients do not need to use this API.
 *
 *  Return value:
 *      Pointer to new TsThaiDict object or NULL upon error.
 *
 *  <GROUP thaidict>
 */
TS_EXPORT(TsThaiDict *)
TsThaiDict_new(TsMemMgr *memMgr, TsResource *resource);



/*****************************************************************************
 *
 *  Frees a TsThaiDict object.
 *
 *  Parameters:
 *      dict      - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP thaidict>
 */
TS_EXPORT(void)
TsThaiDict_delete(TsThaiDict *dict);



/* private section. Clients should not access the structures and functions below */


#define TsTag_Wtdd TsMakeTag('W','t','d','d') /* Thai dictionary tag (this is in the actual dictionary files) */

#define TS_THAI_DICT_VERSION 0x00000001 /* i.e. 0000.0001 */

struct TsThaiDict_
{
    TsMemMgr *memMgr;
    TsResource *resource;
    void  *dictFile;                /* could, under Linux, be memory mapped file */

    TsTag idTag;                    /* ID string 'Wtdd' (WTDD == Wtle Thai Dictionary Data)' */
    TsUInt32 version;               /* 2 major version bytes + 2 minor version bytes */
    TsUInt32 offsetToTrieRecords;   /* in 32 bit words from start of file */
    TsUInt32 numTrieRecords;        /* total number of edges in trie */
    TsUInt32 edgesInRootNode;       /* number of edges in root node of trie */
    TsUInt32 longestWordLength;     /* length of longest word in trie, in characters */
    TsUInt8 *trie;                  /* pointer to start of trie */

    TsDynamicArray best, all;       /* used by word breaking algorithm to keep track of boundaries it has tried */
};


TsResult
TsThaiDict_init(TsThaiDict *dict, TsResource *resource);

void
TsThaiDict_done(TsThaiDict *dict);

TsBool
TsThaiDict_build(TsThaiDict *dict);

TsLength
TsThaiDict_markWordBoundaries(TsThaiDict *dict, TsTextProcessInfo *info, TsLength start, TsLength length);

TS_END_HEADER

#endif /* TSTHAIDI_H */

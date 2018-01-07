/*****************************************************************************
 *
 *  tssubsdi.h - Substitution dictionary support.
 *
 *  Copyright (C) 2006, 2007 Monotype Imaging Inc. All rights reserved.
 *
 *  Monotype Imaging Confidential
 *
 ****************************************************************************/

#ifndef TSSUBSDI_H
#define TSSUBSDI_H

#include "tstypes.h"
#include "tsrsrc.h"
#include "tsstring.h"

TS_BEGIN_HEADER


struct TsSubstDict_;


/*****************************************************************************
 *
 *  Substitution dictionary object.
 *
 *  Description:
 *
 *      The substitution dictionary object contains a mapping between input
 *      strings and their replacement character and an option image ID.
 *
 *  <GROUP tssubsdict>
 */
typedef struct TsSubstDict_ TsSubstDict;


/******************************************************************************
 *
 *  Creates a TsSubstDict object.
 *
 *  Description:
 *      Returns a pointer to an initialized substitution dictionary object.
 *
 *  Parameters:
 *      resource   - [in] pointer to a properly initialized resource object
 *      result     - [out] pointer to receive a TsResult value.
 *
 *  Return value:
 *      A pointer to a TsSubstDict object. Will be NULL on error.
 *      *result will TS_OK on success.
 *
 *  Notes:
 *
 *  <GROUP tssubsdict>
 */
TS_EXPORT(TsSubstDict *)
TsSubstitutionDict_new(TsResource *resource, TsResult *result);


/******************************************************************************
 *
 *  Creates a memory managed TsSubstDict object.
 *
 *  Description:
 *      Returns a pointer to an initialized substitution dictionary object.
 *
 *  Parameters:
 *      memMgr     - [in] pointer to a memory manager object or NULL
 *      resource   - [in] pointer to a properly initialized resource object
 *      result     - [out] pointer to receive a TsResult value.
 *
 *  Return value:
 *      A pointer to a TsSubstDict object. Will be NULL on error.
 *      *result will TS_OK on success.
 *
 *  Notes:
 *
 *  <GROUP tssubsdict>
 */
TS_EXPORT(TsSubstDict *)
TsSubstitutionDict_newex(TsMemMgr *memMgr, TsResource *resource, TsResult *result);


/******************************************************************************
 *
 *  Deletes a TsSubstDict object.
 *
 *  Description:
 *      Releases memory associated with the substitution dictionary object.
 *
 *  Parameters:
 *      dict       - [in] pointer to a substitution dictionary object
 *
 *  Return value:
 *      None.
 *
 *  <GROUP tssubsdict>
 */
TS_EXPORT(void)
TsSubstitutionDict_delete(TsSubstDict *dict);


/******************************************************************************
 *
 *  Read an item from a substitution dictionary.
 *
 *  Description:
 *      Call this function to retrieve the next substitution entry after the one
 *      corresponding with string defined by str argument. To obtain the first
 *      entry, str should be empty (not NULL). Upon return the str
 *      argument contains the next substitution string. An empty return str
 *      indicates that there are no more entries.
 *
 *      The calling program is responsible for constructing the TsString and
 *      destroying it when finished.
 *
 *  Parameters:
 *      dict     - [in] this
 *      str      - [in/out] input string that indicates starting point, output
 *                          string corresponding to next item
 *      subChar  - [out] return substitution character
 *      imageID  - [out] return substitution imageID, 0 means no image
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tssubsdict>
 */
TS_EXPORT(TsResult)
TsSubstitutionDict_readEntry(TsSubstDict *dict, TsString *str, TsUInt32 *subChar, TsUInt32 *imageID);


TS_END_HEADER

#endif /* TSSUBSDI_H */

/*****************************************************************************
 *
 *  otlaymgr.h - OpenType layout manager.
 *
 *  Copyright (C) 2002, 2008 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef OTLAYMGR_H
#define OTLAYMGR_H

#include "otlayout.h"
#include "otfont.h"
#include "tstag.h"
#include "tsmem.h"

TS_BEGIN_HEADER

struct TsOtLayoutManager_;

/******************************************************************************
 *
 *  Summary:
 *      Manages the existing TsOtLayout objects.
 *
 *  Description:
 *      An object created and owned by a user of WT Shaper and
 *      used within the TsShaperFont "findOtLayout" callback.
 *
 *      This OT Layout Access class manages a "current" list of
 *      TsOtLayout objects, creating these objects when necessary, storing
 *      internal references to the objects, and returning a pointer to a
 *      TsOtLayout object, when requested, for shaping.
 *
 *      Assumption is that the methods would be utilized by the TsShaper object
 *      and would be used when a TsShaper determines that an OT font is being
 *      used and should be tried first for shaping.  The "findOtLayout" method
 *      will determine if the font contains layout tables, and if not, return
 *      a NULL pointer result indicating that an alternative shaping engine
 *      needs to be accessed.
 *
 *  <GROUP opentype>
 */
typedef struct TsOtLayoutManager_ TsOtLayoutManager;


/******************************************************************************
 *
 *  Creates a new TsOtLayoutManager object.
 *
 *  Parameters:
 *      memMgr  - [in] pointer to a memory manager object or NULL
 *
 *  Return value:
 *      TsOtLayoutManager * or NULL if error.
 *
 *  <GROUP opentype>
 */
TS_EXPORT(TsOtLayoutManager *)
TsOtLayoutManager_new(TsMemMgr *memMgr);


/******************************************************************************
 *
 *  Finds or creates a TsOtLayout object reference associated with parameters.
 *
 *  Parameters:
 *      lm          - [in] this
 *      font        - [in] font object / font handle
 *      script      - [in] OT tag with name of script used from font
 *      langSys     - [in] OT tag with name of language system used
 *
 *  Return value:
 *      TsOtLayout * or NULL if error or NULL if no valid layout tables
 *          exist for font/script/language system combination provided.
 *
 *  Notes:
 *      This method will attempt to read OT Layout tables from the OT
 *      font identified by the argument of type TsOtFont.  If the font
 *      contains layout information for the given script name and language
 *      system designation, the return result will be a valid
 *      TsOtLayout reference.  If the font does not contain layout
 *      information for the script and language system combination,
 *      then the return is NULL.
 *
 *      If "script" does not explicitly exist in the OT layout tables,
 *      then a script with name "DFLT" is searched for.  If this "DFLT"
 *      script is also non-existent, then the return will be NULL.
 *
 *      If "langSys" does not explicitly exist as an option for the selected
 *      script, then the OT script table-defined "default language system"
 *      is used.  If the script does not define a "default language system",
 *      then the return will be NULL.
 *
 *  <GROUP opentype>
 */
TS_EXPORT(TsOtLayout *)
TsOtLayoutManager_findOtLayout(TsOtLayoutManager *lm, TsOtFont *font,
                               TsTag script, TsTag langSys);


/******************************************************************************
 *
 *  Delete a TsOtLayout object from management.
 *
 *  Parameters:
 *      lm          - [in] this
 *      font        - [in] font identification object / font handle
 *      script      - [in] OT tag with name of script used from font
 *      langSys     - [in] OT tag with name of language system used
 *
 *  Return value:
 *      void
 *
 *  Notes:
 *      Delete the internal management data associated with the TsOtLayout
 *      object identified by the three arguments, and in turn, the
 *      TsOtLayout object itself.
 *
 *  <GROUP opentype>
 */
TS_EXPORT(void)
TsOtLayoutManager_deleteOtLayout(TsOtLayoutManager *lm, TsOtFont *font,
                                 TsTag script, TsTag langSys);


/******************************************************************************
 *
 *  Delete all TsOtLayout objects associated with a particular font.
 *
 *  Parameters:
 *      lm          - [in] this
 *      font        - [in] font identification object / font handle
 *
 *  Return value:
 *      void
 *
 *  Notes:
 *      Delete the internal management data associated with the TsOtLayout
 *      objects that are associated with the font identified by the argument,
 *      and in turn, the TsOtLayout object itself.
 *
 *  <GROUP opentype>
 */
TS_EXPORT(void)
TsOtLayoutManager_deleteAllForFont(TsOtLayoutManager *lm, TsOtFont *font);


/******************************************************************************
 *
 *  Delete all managed TsOtLayout objects.
 *
 *  Parameters:
 *      lm      - [in] this
 *
 *  Return value:
 *      void
 *
 *  Notes:
 *      Delete all of the internal management data associated with all of the
 *      currently managed TsOtLayout objects, and in turn, these objects
 *      themselves.
 *
 *  <GROUP opentype>
 */
TS_EXPORT(void)
TsOtLayoutManager_deleteAll(TsOtLayoutManager *lm);


/******************************************************************************
 *
 *  TsOtLayoutManager destructor.
 *
 *  Parameters:
 *      lm      - [in] this
 *
 *  Return value:
 *      void
 *
 *  Notes:
 *      The object destructor deletes/frees all internal data and the
 *      object referenced by the parameter.
 *
 *  <GROUP opentype>
 */
TS_EXPORT(void)
TsOtLayoutManager_delete(TsOtLayoutManager *lm);


TS_END_HEADER

#endif /* OTLAYMGR_H */

/*****************************************************************************
 *
 *  tsfntmgr.h - Interface to font manager class.
 *
 *  Copyright (C) 2003-2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSFNTMGR_H
#define TSFNTMGR_H

#include "tsport.h"
#include "tsfe_mgr.h"
#include "tseudata.h"


TS_BEGIN_HEADER

struct TsFontMgr_;

/******************************************************************************
 *
 *  The font manager object.
 *
 *  Description:
 *      This class manages a list of TsFont objects.  The class holds a
 *      pointer to a TsFontEngineMgr and can access its managed engines.  This
 *      allows the TsFontMgr to provide a method to create a TsFont from a
 *      TsFontSpec, polling each known TsFontEngine until one successfully
 *      creates and provides a new TsFont object.  This TsFont will be added
 *      to the managed list of TsFont objects within the TsFontMgr object.
 *      Users can also add TsFont objects of their own.  Each managed TsFont
 *      can be retrieved from the manager.  This class allows efficient
 *      sharing of TsFont objects.
 *
 *  <GROUP fontmgr>
 */
typedef struct TsFontMgr_ TsFontMgr;


/******************************************************************************
 *
 *  Creates and initializes a new TsFontMgr object.
 *
 *  Parameters:
 *      mgr     - [in] pointer to TsFontEngineMgr object
 *
 *  Return value:
 *      Pointer to new font manager object or NULL if error.
 *
 *  <GROUP fontmgr>
 */
TS_EXPORT(TsFontMgr *)
TsFontMgr_new(TsFontEngineMgr *mgr);


/******************************************************************************
 *
 *  Creates and initializes a new memory managed TsFontMgr object.
 *
 *  Parameters:
 *      memMgr  - [in] pointer to a memory manager object or NULL
 *      mgr     - [in] pointer to TsFontEngineMgr object
 *
 *  Return value:
 *      Pointer to new font manager object or NULL if error.
 *
 *  <GROUP fontmgr>
 */
TS_EXPORT(TsFontMgr *)
TsFontMgr_newex(TsMemMgr *memMgr, TsFontEngineMgr *mgr);


/******************************************************************************
 *
 *  Deletes a TsFontMgr object.
 *
 *  Parameters:
 *      fm - [in] TsFontMgr pointer.
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontmgr>
 */
TS_EXPORT(void)
TsFontMgr_delete(TsFontMgr *fm);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsFontMgr object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsFontMgr_releaseHandle.
 *
 *  Parameters:
 *      fm      - [in] pointer to TsFontMgr object
 *
 *  Return value:
 *      Pointer to font manager object.
 *
 *  <GROUP fontmgr>
 */
TS_EXPORT(TsFontMgr *)
TsFontMgr_copyHandle(TsFontMgr *fm);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsFontMgr object.
 *
 *  Description:
 *      This function reverses the action of TsFontMgr_copyHandle.
 *
 *  Parameters:
 *      fm      - [in] pointer to TsFontMgr object
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontmgr>
 */
TS_EXPORT(void)
TsFontMgr_releaseHandle(TsFontMgr *fm);


/*****************************************************************************
 *
 *  Appends a TsFontSpec/TsFont pair to the list of managed fonts.
 *
 *  Parameters:
 *      fm      - [in] pointer to TsFontMgr object
 *      spec    - [in] TsFontSpec font specification object
 *      font    - [in] pointer to TsFont object to be appended to list
 *
 *  Return value:
 *      TsResult value.
 *
 *  <GROUP fontmgr>
 */
TS_EXPORT(TsResult)
TsFontMgr_addFont(TsFontMgr *fm, TsFontSpec spec, TsFont *font);


/*****************************************************************************
 *
 *  Get a pointer to a TsFont object from the list of managed fonts.
 *
 *  Description:
 *      This function searches the list of managed fonts and returns
 *      the TsFont object pointer associated with the TsFontSpec if it
 *      is found.  When the TsFontSpec/TsFont pair is not found, an attempt
 *      is made to create the TsFont from each of the TsFontEngine objects
 *      that are managed by the TsFontEngineMgr object.  If a TsFont object
 *      is successfully created by one of the TsFontEngine objects, then
 *      that TsFont and its associated TsFontSpec is added to the list of
 *      managed fonts.  The new TsFont object pointer is returned.  When
 *      all attempts to create the TsFont fail, a NULL pointer is returned.
 *
 *  Parameters:
 *      fm      - [in] pointer to TsFontMgr object
 *      spec    - [in] TsFontSpec font specification object
 *
 *  Return value:
 *      Pointer to TsFont font object or NULL if error.
 *
 *  <GROUP fontmgr>
 */
TS_EXPORT(TsFont *)
TsFontMgr_getFont(TsFontMgr *fm, TsFontSpec spec);


/*****************************************************************************
 *
 *  Get a pointer to a TsFont object from the list of managed fonts
 *  using the provided string name to identify the font.  If the font
 *  is not found, NULL is returned.  It is assumed that the font has
 *  already been constructed and is managed.  No attempt is made to
 *  construct a font that is not found.
 *
 *  Parameters:
 *      fm      - [in] pointer to TsFontMgr object
 *      name    - [in] TsChar name string
 *
 *  Return value:
 *      Pointer to TsFont font object or NULL if not found.
 *
 *  <GROUP fontmgr>
 */
TS_EXPORT(TsFont *)
TsFontMgr_getFontByName(TsFontMgr *fm, TsChar *name);


/******************************************************************************
 *
 *  Remove a TsFont object from management.
 *
 *  Description:
 *      Remove the internal management data associated with the TsFont
 *      object identified by the argument and call the TsFont object's
 *      TsFont_releaseHandle() method.
 *
 *  Parameters:
 *      fm      - [in] pointer to TsFontMgr object
 *      font    - [in] pointer to TsFont object to be removed
 *
 *  Return value:
 *      TS_OK on success, else error.
 *
 *  <GROUP fontmgr>
 */
TS_EXPORT(TsResult)
TsFontMgr_removeFont(TsFontMgr *fm, TsFont *font);


/******************************************************************************
 *
 *  Remove all managed TsFont objects.
 *
 *  Description:
 *      Remove all of the internal management data associated with all of the
 *      currently managed TsFont objects and call each object's
 *      TsFont_releaseHandle() method.
 *
 *  Parameters:
 *      fm      - [in] pointer to TsFontMgr object
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontmgr>
 */
TS_EXPORT(void)
TsFontMgr_removeAll(TsFontMgr *fm);


/*****************************************************************************
 *
 *  Returns a pointer to an object which contains external data managed by
 *  the user.
 *
 *  Description:
 *      This function returns a TsExternalUserData pointer which allows
 *      the user to access external data that was set by a call to
 *      TsExternalUserData_init.
 *
 *      TS_USE_EXTERNAL_USERDATA must be defined in order to use the
 *      private data mechanism. If not defined this function returns NULL.
 *
 *      The TsExternalUserData object that is returned by this function
 *      becomes invalid when this TsFontMgr object is destroyed.
 *      Therefore, you must keep reference to the TsFontMgr object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      fm          - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP fontmgr>
 */
TS_EXPORT(TsExternalUserData *)
TsFontMgr_getExternalUserdata(TsFontMgr *fm);


TS_END_HEADER

#endif /* TSFNTMGR_H */

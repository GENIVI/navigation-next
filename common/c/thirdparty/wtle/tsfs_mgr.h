/*****************************************************************************
 *
 *  tsfs_mgr.h - Interface to font style manager class.
 *
 *  Copyright (C) 2003-2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSFS_MGR_H
#define TSFS_MGR_H

#include "tsport.h"
#include "tsfontst.h"
#include "tseudata.h"

TS_BEGIN_HEADER

struct TsFontStyleMgr_;

/******************************************************************************
 *
 *  The font style manager object.
 *
 *  Description:
 *      This class manages a list of TsFontStyle objects.  TsFontStyle objects
 *      are added to the managed list from the TsFontStyleMgr_getFontStyle()
 *      method, when necessary, or users can also add TsFontStyle objects of
 *      their own with TsFontStyleMgr_addFontStyle().  Each managed TsFontStyle
 *      can be retrieved from the manager.  This class allows efficient
 *      sharing of TsFontStyle objects.
 *
 *  <GROUP fontstylemgr>
 */
typedef struct TsFontStyleMgr_ TsFontStyleMgr;


/******************************************************************************
 *
 *  Creates and initializes a new TsFontStyleMgr object.
 *
 *  Parameters:
 *      none
 *
 *  Return value:
 *      Pointer to new font manager object or NULL if error.
 *
 *  <GROUP fontstylemgr>
 */
TS_EXPORT(TsFontStyleMgr *)
TsFontStyleMgr_new(void);


/******************************************************************************
 *
 *  Creates and initializes a new memory managed TsFontStyleMgr object.
 *
 *  Parameters:
 *      memMgr   - [in] pointer to a memory manager object or NULL
 *
 *  Return value:
 *      Pointer to new font manager object or NULL if error.
 *
 *  <GROUP fontstylemgr>
 */
TS_EXPORT(TsFontStyleMgr *)
TsFontStyleMgr_newex(TsMemMgr *memMgr);


/******************************************************************************
 *
 *  Deletes a TsFontStyleMgr object.
 *
 *  Parameters:
 *      fm          - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontstylemgr>
 */
TS_EXPORT(void)
TsFontStyleMgr_delete(TsFontStyleMgr *fm);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsFontStyleMgr object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsFontStyleMgr_releaseHandle.
 *
 *  Parameters:
 *      fm          - [in] this
 *
 *  Return value:
 *      Pointer to font manager object.
 *
 *  <GROUP fontstylemgr>
 */
TS_EXPORT(TsFontStyleMgr *)
TsFontStyleMgr_copyHandle(TsFontStyleMgr *fm);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsFontStyleMgr object.
 *
 *  Description:
 *      This function reverses the action of TsFontStyleMgr_copyHandle.
 *
 *  Parameters:
 *      fm          - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontstylemgr>
 */
TS_EXPORT(void)
TsFontStyleMgr_releaseHandle(TsFontStyleMgr *fm);


/*****************************************************************************
 *
 *  Appends a TsFontStyle to the list of managed fonts.
 *
 *  Parameters:
 *      fm          - [in] this
 *      font        - [in] pointer to TsFontStyle object to be appended to list
 *
 *  Return value:
 *      TsResult value.
 *
 *  <GROUP fontstylemgr>
 */
TS_EXPORT(TsResult)
TsFontStyleMgr_addFontStyle(TsFontStyleMgr *fm, TsFontStyle *font);


/*****************************************************************************
 *
 *  Get a pointer to a TsFontStyle object from the list of managed objects.
 *
 *  Description:
 *      This function searches the list of managed font styles and returns
 *      the TsFontStyle object pointer associated with the TsFontParam if it
 *      is found.  When the TsFontParam/TsFontStyle pair is not found, an attempt
 *      is made to create the TsFontStyle from the provided TsFont object.
 *      If a TsFontStyle object is successfully created, then that
 *      TsFontStyle and its associated TsFontParam is added to the list of
 *      managed font styles.  The new TsFontStyle object pointer is returned.
 *      When the attempt to create the TsFontStyle fails, a NULL pointer
 *      is returned.
 *
 *  Parameters:
 *      fm          - [in] this
 *      font        - [in] TsFont from which to create the TsFontStyle
 *      param       - [in] TsFontParam font style parameters
 *
 *  Return value:
 *      Pointer to TsFontStyle font object or NULL if error.
 *
 *  Notes:
 *      This function increments the reference count on the returned TsFontStyle
 *      object; so TsFontStyle_releaseHandle must be called once you are
 *      done using it.
 *
 *  <GROUP fontstylemgr>
 */
TS_EXPORT(TsFontStyle *)
TsFontStyleMgr_getFontStyle(TsFontStyleMgr *fm, TsFont *font, TsFontParam *param);


/******************************************************************************
 *
 *  Remove a TsFontStyle object from management.
 *
 *  Description:
 *      Remove the internal management data associated with the TsFontStyle
 *      object identified by the argument and call the TsFontStyle object's
 *      TsFontStyle_releaseHandle() method.
 *
 *  Parameters:
 *      fm          - [in] this
 *      font        - [in] pointer to TsFontStyle object to be removed
 *
 *  Return value:
 *      TS_OK on success, else error.
 *
 *  <GROUP fontstylemgr>
 */
TS_EXPORT(TsResult)
TsFontStyleMgr_removeFontStyle(TsFontStyleMgr *fm, TsFontStyle *font);


/******************************************************************************
 *
 *  Removes all font styles associated with 'font' from the font style manager.
 *
 *  Description:
 *      For any fontStyle in the manager's list that was built using 'font',
 *      remove the internal management data associated with the TsFontStyle
 *      object and call the TsFontStyle object's  TsFontStyle_releaseHandle()
 *      method.
 *
 *  Parameters:
 *      fm          - [in] this
 *      font        - [in] pointer to TsFont object
 *
 *  Return value:
 *      TS_OK on success, else error.
 *
 *  <GROUP fontstylemgr>
 */
TS_EXPORT(TsResult)
TsFontStyleMgr_removeFont(TsFontStyleMgr *fm, TsFont *font);


/******************************************************************************
 *
 *  Remove all managed TsFontStyle objects.
 *
 *  Description:
 *      Remove all of the internal management data associated with all of the
 *      currently managed TsFontStyle objects and call each object's
 *      TsFontStyle_releaseHandle() method.
 *
 *  Parameters:
 *      fm          - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontstylemgr>
 */
TS_EXPORT(void)
TsFontStyleMgr_removeAll(TsFontStyleMgr *fm);


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
 *      becomes invalid when this TsFontStyelMgr object is destroyed.
 *      Therefore, you must keep reference to the TsFontStyelMgr object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      fm          - [in] this
 *
 *  Return value:
 *      Pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP fontstylemgr>
 */
TS_EXPORT(TsExternalUserData *)
TsFontStyleMgr_getExternalUserdata(TsFontStyleMgr *fm);


TS_END_HEADER

#endif /* TSFS_MGR_H */

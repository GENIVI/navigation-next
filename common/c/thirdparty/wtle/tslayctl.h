/*****************************************************************************
 *
 *  tslayctl.h  - LayoutControl object interface.
 *
 *  Copyright (C) 2005-2006 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSLAYOUTCONTROL_H
#define TSLAYOUTCONTROL_H

#include "tstypes.h"
#include "tsevent.h"
#include "tstag.h"
#include "tsrsrc.h"


TS_BEGIN_HEADER

struct TsLayoutControl_;


/*****************************************************************************
 *
 *  TsLayout control object
 *
 *  Description:
 *      The TsLayoutControl object manages information for text layout.
 *      Each client should create one TsLayoutControl upon startup and delete
 *      it upon shutdown.
 *
 *      This object is used when creating a new TsLayout using TsLayout_new.
 *      TsLayout keeps a pointer to the TsLayoutControl object, so it should not
 *      be deleted until all TsLayout objects that reference it are deleted.
 *
 *      Clients in a single thread can share the same TsLayoutControl object,
 *      but clients in separate threads or processes must declare their own.
 *
 *  <GROUP layoutcontrol>
 */
typedef struct TsLayoutControl_ TsLayoutControl;


/*****************************************************************************
 *
 *  Creates a TsLayoutControl object.
 *
 *  Parameters:
 *      none
 *
 *  Return value:
 *      Pointer to new TsLayoutControl object
 *
 *  <GROUP layoutcontrol>
 */
TS_EXPORT(TsLayoutControl *)
TsLayoutControl_new(void);


/*****************************************************************************
 *
 *  Creates a memory managed TsLayoutControl object.
 *
 *  Parameters:
 *      memMgr  - [in] pointer to a memory manager object or NULL
 *
 *  Return value:
 *      Pointer to new TsLayoutControl object
 *
 *  <GROUP layoutcontrol>
 */
TS_EXPORT(TsLayoutControl *)
TsLayoutControl_newex(TsMemMgr *memMgr);


/*****************************************************************************
 *
 *  Frees a TsLayoutControl object.
 *
 *  Parameters:
 *      control  - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP layoutcontrol>
 */
TS_EXPORT(void)
TsLayoutControl_delete(TsLayoutControl *control);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsLayoutControl object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsLayoutControl_releaseHandle.
 *
 *  Parameters:
 *      control      - [in] pointer to TsLayoutControl object
 *
 *  Return value:
 *      Pointer to TsLayoutControl object.
 *
 *  <GROUP layoutcontrol>
 */
TS_EXPORT(TsLayoutControl *)
TsLayoutControl_copyHandle(TsLayoutControl *control);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsLayoutControl object.
 *
 *  Description:
 *      This function reverses the action of TsLayoutControl_copyHandle.
 *
 *  Parameters:
 *      control      - [in] pointer to TsLayoutControl object
 *
 *  Return value:
 *      void
 *
 *  <GROUP layoutcontrol>
 */
TS_EXPORT(void)
TsLayoutControl_releaseHandle(TsLayoutControl *control);


/*****************************************************************************
 *
 *  Register an observer with the layout control object.
 *
 *  Parameters:
 *      control         - [in] this
 *      observer        - [in] pointer to object that is observing the layout
 *                             control
 *      callback        - [in] callback function that is called when something
 *                             about the layout control changes
 *
 *  Return value:
 *      TsResult        - TS_OK if the function completes successfully;
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is passed in.
 *
 *  <GROUP layoutcontrol>
 */
TS_EXPORT(TsResult)
TsLayoutControl_registerObserver(TsLayoutControl *control, void *observer, TsEventCallback callback);


/*****************************************************************************
 *
 *  Unregister an observer with the layout control object.
 *
 *  Parameters:
 *      control         - [in] this
 *      observer        - [in] pointer to object that is observing the layout
 *                             control
 *
 *  Return value:
 *      TsResult        - TS_OK if the function completes successfully;
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is passed in.
 *
 *  <GROUP layoutcontrol>
 */
TS_EXPORT(TsResult)
TsLayoutControl_unregisterObserver(TsLayoutControl *control, void *observer);


/*****************************************************************************
 *
 *  Macro that makes a 'Wsd1' tag for use when adding a
 *  substitution dictionary.
 *
 *  <GROUP layoutcontrol>
 */
#define TsTag_Wsd1 TsMakeTag('W','s','d','1')


/*****************************************************************************
 *
 *  Macro that makes a 'Wsd2' tag for use when when adding a
 *  substitution dictionary.
 *
 *  <GROUP layoutcontrol>
 */
#define TsTag_Wsd2 TsMakeTag('W','s','d','2')


/*****************************************************************************
 *
 *  Macro that makes a 'Thai' tag for use when adding a Thai dictionary.
 *
 *  <GROUP layoutcontrol>
 */
#define TsTag_Thai TsMakeTag('T','h','a','i')


/*****************************************************************************
 *
 *  Adds external dictionary item to a TsLayoutControl object.
 *
 *  Description:
 *      This function provides a way for clients to control certain
 *      large dictionary items needed by WTLE.
 *
 *      Support data items:
 *
 *      <TABLE>
 *      <B>Tag          Description</B>
 *      Thai            Thai dictionary for Thai word breaking
 *      Wsd1            Substitution dictionary (base)
 *      Wsd2            Substitution dictionary (secondary)
 *      </TABLE>
 *
 *      If proper Thai line breaking is desired then call this function
 *      prior to building a TsLayout. The dictionary file can be in a file
 *      system or in a memory pointer. Certain systems support memory
 *      mapped files.
 *
 *  Parameters:
 *      control     - [in] this
 *      tag         - [in] data identifier
 *      resource    - [in] pointer to initialized resource
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layoutcontrol>
 */
TS_EXPORT(TsResult)
TsLayoutControl_addDictionary(TsLayoutControl *control, TsTag tag,
                        TsResource *resource);


/*****************************************************************************
 *
 *  Removes external dictionary item to a TsLayoutControl object.
 *
 *  Parameters:
 *      control         - [in] this
 *      tag             - [in] dictionary identifier
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layoutcontrol>
 */
TS_EXPORT(TsResult)
TsLayoutControl_removeDictionary(TsLayoutControl *control, TsTag tag);


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
 *      becomes invalid when this TsLayoutControl object is destroyed.
 *      Therefore, you must keep reference to the TsLayoutControl object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      control     - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP layoutcontrol>
 */
TS_EXPORT(TsExternalUserData *)
TsLayoutControl_getExternalUserdata(TsLayoutControl *control);


/*****************************************************************************
 *
 *  Backward compatibility macros
 *
 *  <GROUP layoutcontrol>
 */
#define TsLayoutControl_addData TsLayoutControl_addDictionary
#define TsLayoutControl_removeData TsLayoutControl_removeDictionary

TS_END_HEADER

#endif /* TSLAYOUTCONTROL_H */


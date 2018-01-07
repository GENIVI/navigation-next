/*****************************************************************************
 *
 *  tsfe_mgr.h - Interface to font engine manager class.
 *
 *  Copyright (C) 2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSFE_MGR_H
#define TSFE_MGR_H

#include "tsproj.h"
#include "tsfnteng.h"
#include "tseudata.h"


TS_BEGIN_HEADER

struct TsFontEngineMgr_;

/*****************************************************************************
 *
 *  The font engine manager object.
 *
 *  Description:
 *      This object maintains a list of TsFontEngine pointers.
 *
 *  <GROUP fontenginemgr>
 */
typedef struct TsFontEngineMgr_ TsFontEngineMgr;


/******************************************************************************
 *
 *  Creates and initializes a new TsFontEngineMgr object.
 *
 *  Parameters:
 *
 *  Return value:
 *      Pointer to new font engine manager object or NULL if error.
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(TsFontEngineMgr *)
TsFontEngineMgr_new(void);

/******************************************************************************
 *
 *  Creates and initializes a new memory managed TsFontEngineMgr object.
 *
 *  Parameters:
 *      memMgr      - [in] pointer to memory manager object or NULL
 *
 *  Return value:
 *      Pointer to new font engine manager object or NULL if error.
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(TsFontEngineMgr *)
TsFontEngineMgr_newex(TsMemMgr *memMgr);

/******************************************************************************
 *
 *  Deletes a TsFontEngineMgr object.
 *
 *  Parameters:
 *      fem         - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(void)
TsFontEngineMgr_delete(TsFontEngineMgr *fem);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsFontEngineMgr object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsFontEngineMgr_releaseHandle.
 *
 *  Parameters:
 *      fem         - [in] this
 *
 *  Return value:
 *      Pointer to font engine manager object.
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(TsFontEngineMgr *)
TsFontEngineMgr_copyHandle(TsFontEngineMgr *fem);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsFontEngineMgr object.
 *
 *  Description:
 *      This function reverses the action of TsFontEngineMgr_copyHandle.
 *
 *  Parameters:
 *      fem         - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(void)
TsFontEngineMgr_releaseHandle(TsFontEngineMgr *fem);


/*****************************************************************************
 *
 *  Appends a TsFontEngine to the list of managed engines.
 *
 *  Parameters:
 *      fem      - [in] this
 *      eng      - [in] pointer to TsFontEngine object to be appended to list
 *
 *  Return value:
 *      TsResult value.
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(TsResult)
TsFontEngineMgr_addEngine(TsFontEngineMgr *fem, TsFontEngine *eng);


/*****************************************************************************
 *
 *  Returns the number of TsFontEngine objects that are in the managed list.
 *
 *  Parameters:
 *      fem      - [in] this
 *
 *  Return value:
 *      TsLength number of engines.
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(TsLength)
TsFontEngineMgr_numEngines(TsFontEngineMgr *fem);


/*****************************************************************************
 *
 *  Returns the TsFontEngine pointer at the given index.
 *
 *  Description:
 *      This function returns a TsFontEngine pointer to the engine that is
 *      stored at the indexed position in the managed list.
 *
 *  Parameters:
 *      fem      - [in] this
 *      i        - [in] index of engine of interest
 *
 *  Return value:
 *      TsFontEngine pointer.
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(TsFontEngine *)
TsFontEngineMgr_engine(TsFontEngineMgr *fem, TsIndex i);


/*****************************************************************************
 *
 *  Convenience function that creates one or more TsFontEngine objects and
 *  adds them to the managed list of engines.
 *
 *  Description:
 *
 *      The TsFontEngine objects to be created and added is a compiled-in
 *      list found in the file fontsrv/tsfe_mgr.c. As of WTLE 2.0, this
 *      list is the iType font engine and/or the UFST font engine,
 *      if appropriate macros are defined at compile time.
 *
 *      When constructing an iType font engine, this function passes
 *      the ITYPE_HEAP_SIZE configuration option to specify heap size.
 *
 *      If you wish to control iType heap size at run time then call
 *      TsFontEngineMgr_addNew_iType instead.
 *
 *      If you wish to attach to an existing iType instance then
 *      call TsFontEngineMgr_addAttached_iType instead.
 *
 *  Parameters:
 *      fem         - [in] this
 *
 *  Return value:
 *      TsResult value of TS_OK if successful, otherwise an appropriate error
 *      condition value.
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(TsResult)
TsFontEngineMgr_addDefaultList(TsFontEngineMgr *fem);


/*****************************************************************************
 *
 *  Convenience function that creates a TsFontEngine that uses an existing iType
 *  and adds that font engine to the font engine manager.
 *
 *  Description:
 *
 *      This call requires a pointer to an existing FS_STATE instance of
 *      iType (casted to void).
 *
 *      This function wraps the functionality provided by the iType
 *      FS_new_client function. This instance of WTLE is the new client.
 *      The iType client is then used to construct a WTLE TsFontEngine.
 *      Then that engine is added to the list of engines being managed by
 *      the TsFontEngineMgr.
 *
 *      When using iType 3.1 or earlier, this function attaches directly
 *      to the state without creating a new client - except in the case
 *      of mulithreaded iType builds in which case a new client is 
 *      created. That is because the 3.1 and earlier code only supported
 *      multiple clients when there was a multithreaded build. 
 *      The impact of not generating a separate client is that
 *      the WTLE glue code has to setup the iType state each time it
 *      interacts with it to get glyphs, map characters, and so forth.
 *
 *  Parameters:
 *      fem              - [in] this
 *      state            - [in] void pointer to initialized iType FS_STATE structure
 *      heapSizeIncrease - [in] amount that the iType heap size may be increased for this client
 *
 *  Return value:
 *      TsResult value
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(TsResult)
TsFontEngineMgr_addAttached_iType(TsFontEngineMgr *fem, /* (FS_SATE*) */void *state, TsSize heapSizeIncrease);


/*****************************************************************************
 *
 *  Convenience function that creates a TsFontEngine based on a new iType
 *  and adds that font engine to the font engine manager.
 *
 *  Description:
 *      Use this function if you wish to create a new iType instance and
 *      want to specify the heap size.
 *
 *      This call requires a pointer to an existing FS_STATE instance of
 *      iType (casted to void). This should only be used if the iType
 *      code was built using the FS_MULTI_THREAD directive.
 *
 *      This function wraps the functionality provided by the iType
 *      FS_new_client function. This instance of WTLE is the new client.
 *      The iType client is then used to construct a WTLE TsFontEngine.
 *      Then that engine is added to the list of engines being managed by
 *      the TsFontEngineMgr.
 *
 *  Parameters:
 *      fem             - [in] this
 *      heapSize        - [in] iType heap size
 *
 *  Return value:
 *      TsResult value
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(TsResult)
TsFontEngineMgr_addNew_iType(TsFontEngineMgr *fem, TsSize heapSize);


/*****************************************************************************
 *
 *  Version of TsFontEngineMgr_addNew_iType that take a client allocated
 *  heap to be used by iType. Valid only if FS_INT_MEM is defined.
 *
 *  Description:
 *      Use this function if you wish to create a new iType instance and
 *      want to specify the heap size.
 *
 *      This call requires a pointer to an existing FS_STATE instance of
 *      iType (casted to void). This should only be used if the iType
 *      code was built using the FS_MULTI_THREAD directive.
 *
 *      This function wraps the functionality provided by the iType
 *      FS_new_client function. This instance of WTLE is the new client.
 *      The iType client is then used to construct a WTLE TsFontEngine.
 *      Then that engine is added to the list of engines being managed by
 *      the TsFontEngineMgr.
 *
 *  Parameters:
 *      fem             - [in] this
 *      heapSize        - [in] iType heap size
 *      heap            - [in] client allocated buffer that has size heapSize
 *
 *  Return value:
 *      TsResult value
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(TsResult)
TsFontEngineMgr_addNew_iTypeEx(TsFontEngineMgr *fontEngMgr, TsSize heapSize, TsByte *heap);


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
 *      becomes invalid when this TsFontEngineMgr object is destroyed.
 *      Therefore, you must keep reference to the TsFontEngineMgr object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      fem         - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP fontenginemgr>
 */
TS_EXPORT(TsExternalUserData *)
TsFontEngineMgr_getExternalUserdata(TsFontEngineMgr *fem);


TS_END_HEADER

#endif /* TSFE_MGR_H */

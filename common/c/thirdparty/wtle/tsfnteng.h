/*****************************************************************************
 *
 *  tsfnteng.h  - Defines the interface to the font engine class.
 *
 *  Copyright (C) 2003-2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSFNTENG_H
#define TSFNTENG_H

#include "tsport.h"
#include "tsfont.h"
#include "tseudata.h"


TS_BEGIN_HEADER

struct TsFontEngine_;

/*****************************************************************************
 *
 *  The font engine object.
 *
 *  Description:
 *      This object provides an interface to a font engine which primarily
 *      provides TsFont objects.  The TsFontEngine class is an abstract class
 *      using the C++ definition, and must be derived from for concrete
 *      font engine services.
 *
 *  <GROUP fontengine>
 */
typedef struct TsFontEngine_ TsFontEngine;


/*****************************************************************************
 *
 *  Macro indicating the maximum font engine name string length.
 *
 *  <GROUP fontengine>
 */

#define TS_FONT_ENGINE_NAME_MAX_LENGTH      30


/*****************************************************************************
 *
 *  Macro indicating the maximum font engine version string length.
 *
 *  <GROUP fontengine>
 */
#define TS_FONT_ENGINE_VERSION_MAX_LENGTH   20


/*****************************************************************************
 *
 *  Structure for returning font engine information.
 *
 *  Description:
 *      This structure is used with TsFontEngine_getEngineInfo.
 *
 *  <GROUP fontengine>
 */
typedef struct TsFontEngineInfo_
{
    TsChar name[TS_FONT_ENGINE_NAME_MAX_LENGTH + 1];    /* Font engine name */
    TsChar version[TS_FONT_ENGINE_VERSION_MAX_LENGTH + 1]; /* Font engine version */
} TsFontEngineInfo;



/*****************************************************************************
 *
 *  The font engine virtual function table.
 *
 *  Description:
 *      This structure provides the "virtual function" pointers that are
 *      necessary for the TsFontEngine abstract class to provide engine
 *      services.  This structure must be properly initialized and passed
 *      to the TsFontEngine constructor via TsFontEngine_new or TsFontEngine_newex.
 *
 *  <GROUP fontengine>
 */
typedef struct TsFontEngineFuncs_
{
    TsFont *(*createFont)(void *data, TsFontSpec);  /* construct a font object */
    void (*deleteIt)(void *data); /* delete the derived class */
    TsResult (*getEngineInfo)(void *data, TsFontEngineInfo *info); /* get info about the font engine */
} TsFontEngineFuncs;


/******************************************************************************
 *
 *  Creates and initializes a new TsFontEngine object.
 *
 *  Description:
 *      Users will not usually call this function directly. Instead, call
 *      the derived class constructor, which in  turn will call this function.
 *
 *      The WTLE SDK ships with two derived class implementations, one for
 *      iType and one for UFST. The corresponding constructors are
 *      iTypeFontEngine_new() and UFSTFontEngine_new().
 *
 *  Parameters:
 *      funcs    - [in] a properly initialized TsFontEngineFuncs structure.
 *      data     - [in] the derived class' private data.  This void pointer
 *                      will be passed as the first argument to each of the
 *                      functions defined in TsFontEngineFuncs.
 *
 *  Return value:
 *      Pointer to new font engine object or NULL if error.
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(TsFontEngine *)
TsFontEngine_new(const TsFontEngineFuncs *funcs, void *data);


/******************************************************************************
 *
 *  Creates and initializes a new a new memory managed TsFontEngine object.
 *
 *  Description:
 *      Users will not usually call this function directly. Instead, call
 *      the derived class constructor, which in turn will call this function.
 *
 *      The WTLE SDK ships with two derived class implementations, one for
 *      iType and one for UFST. The corresponding constructors are
 *      iTypeFontEngine_newex() and UFSTFontEngine_newex().
 *
 *  Parameters:
 *      memMgr   - [in] pointer to memory manager object or NULL
 *      funcs    - [in] a properly initialized TsFontEngineFuncs structure.
 *      data     - [in] the derived class' private data.  This void pointer
 *                      will be passed as the first argument to each of the
 *                      functions defined in TsFontEngineFuncs.
 *
 *  Return value:
 *      Pointer to new font engine object or NULL if error.
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(TsFontEngine *)
TsFontEngine_newex(TsMemMgr *memMgr, const TsFontEngineFuncs *funcs, void *data);


/******************************************************************************
 *
 *  Deletes a TsFontEngine object.
 *
 *  Parameters:
 *      fontEngine - [in] TsFontEngine pointer.
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(void)
TsFontEngine_delete(TsFontEngine *fontEngine);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsFontEngine object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsFontEngine_releaseHandle.
 *
 *  Parameters:
 *      fontEngine   - [in] pointer to TsFontEngineMgr object
 *
 *  Return value:
 *      Pointer to font engine object.
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(TsFontEngine *)
TsFontEngine_copyHandle(TsFontEngine *fontEngine);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsFontEngine object.
 *
 *  Description:
 *      This function reverses the action of TsFontEngine_copyHandle.
 *
 *  Parameters:
 *      fontEngine   - [in] pointer to TsFontEngine object
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(void)
TsFontEngine_releaseHandle(TsFontEngine *fontEngine);


/*****************************************************************************
 *
 *  Requests that the font engine create a TsFont object.
 *
 *  Description:
 *      This function requests a TsFont be created by the TsFontEngine.  The
 *      font is identified by the TsFontSpec argument.
 *
 *  Parameters:
 *      fontEngine   - [in] pointer to TsFontEngine object
 *      spec         - [in] TsFontSpec object
 *
 *  Return value:
 *      TsFont pointer or NULL if error.
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(TsFont *)
TsFontEngine_createFont(TsFontEngine *fontEngine, TsFontSpec spec);


/*****************************************************************************
 *
 *  Gets information about a font engine.
 *
 *  Description:
 *      This function fills in a TsFontEngineInfo structure with strings
 *      that represent the name and version of the passed in font engine.
 *
 *  Parameters:
 *      fontEngine   - [in] pointer to TsFontEngine object
 *      info         - [in] pointer to a TsFontEngineInfo object that will be
 *                          filled in
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(TsResult)
TsFontEngine_getEngineInfo(TsFontEngine *fontEngine, TsFontEngineInfo *info);


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
 *      becomes invalid when this TsFontEngine object is destroyed.
 *      Therefore, you must keep reference to the TsFontEngine object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      fontEngine  - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(TsExternalUserData *)
TsFontEngine_getExternalUserdata(TsFontEngine *fontEngine);


/*****************************************************************************
 *
 *  Constructs an iType-specific TsFontEngine.
 *
 *  Description:
 *      This funciton will construct a TsFontEngine that uses iType as
 *      its underlying font engine.
 *
 *      If the 'state' argument is NULL then this function constructs a
 *      new iType instance using FS_init() and gives it a heap size equal
 *      to 'heapSize'. The result of this constructor is a TsFontEngine
 *      that is the primary "owner" of the iType instance. When this
 *      TsFontEngine is eventually destroyed using TsFontEngine_delete()
 *      it will result in a call to FS_exit().
 *
 *      If the 'state' argument is non-NULL then it should be casted from
 *      a valid FS_STATE pointer that points to a valid iType client.
 *      This pointer will be passed to FS_new_client(), which constructs
 *      a new iType 'child' client. The client that is passed in is the
 *      'parent'. When this new TsFontEngine is eventually destroyed
 *      using TsFontEngine_delete() it will result in a call to
 *      FS_end_client(). Child clients should be deleted before the
 *      the parent clients.
 *
 *      This function is available only if ITYPE_FENG is defined in the
 *      project or makefile.
 *
 *  Parameters:
 *      state        - [in] pointer to existing iType instance or NULL
 *      heapSize     - [in] increase to iType heap size
 *
 *  Return value:
 *      Pointer to new TsFontEngine object.
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(TsFontEngine *)
iTypeFontEngine_new(/*FS_STATE */ void *state, TsSize heapSize);


/*****************************************************************************
 *
 *  Constructs an iType-specific TsFontEngine.
 *
 *  Description:
 *       This is a memory managed version of iTypeFontEngine_new. See the
 *       description of iTypeFontEngine_new for details.
 *
 *  Parameters:
 *      memMgr       - [in] pointer to memory manager object or NULL
 *      state        - [in] pointer to existing iType instance or NULL
 *      heapSize     - [in] increase to iType heap size
 *      heap         - [in] clinet allocated heap if using internal memory, or NULL
 *
 *  Return value:
 *      Pointer to new TsFontEngine object.
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(TsFontEngine *)
iTypeFontEngine_newex(TsMemMgr *memMgr, /*FS_STATE */ void *state, TsSize heapSize, TsByte *heap);


/*****************************************************************************
 *
 *  Returns a pointer to the FS_STATE that is "owned" by this TsFontEngine.
 *
 *  Description:
 *      The pointer is returned as a void pointer so that this module does
 *      not have to manage the iType include path. The returned value
 *      should be casted to (FS_STATE *) by the calling function.
 *
 *      The FS_STATE pointer can then be used to create a new iType client
 *      with a direct call to FS_new_client(). This new client shares
 *      the same underlying iType server, fonts and heap as the one "owned"
 *      by this TsFontEngine. This new iType client must be destroyed using
 *      end client prior to destroying this TsFontEngine.
 *
 *      Another use scenario is to use the FS_STATE pointer with
 *      the function iTypeFontEngine_new() to create a new TsFontEngine
 *      that shares the same underlying iType engine, fonts, and heap as the one
 *      "owned" by this TsFontEngine. The fonts will still have to be
 *      added using TsFontManager calls.
 *
 *      The calling program may use the FS_STATE pointer to access iType functionality
 *      directly, such as resizing the iType heap, but it should not access the
 *      pointer after releasing its TsFontEngine handle.
 *
 *      It is a checked run-time error to pass in a TsFontEngine that does not
 *      have an underlying iType font engine. The error is handled by
 *      printing a message (debug builds only) and by returning NULL.
 *
 *      This function is available only if ITYPE_FENG is defined in the
 *      project or makefile.
 *
 *  Parameters:
 *      fontEngine   - [in] pointer to this TsFontEngine
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(void *)
iTypeFontEngine_getFS_STATE(TsFontEngine *fontEngine);


/*****************************************************************************
 *
 *  Returns the number of bytes being used by the underlying iType font engine
 *  heap.
 *
 *  Description:
 *      This function probes the iType FS_STATE to determine the amount
 *      of heap memory in bytes currently being used by iType.
 *
 *      It is an UNCHECKED runtime error to pass a TsFontEngine that does
 *      not have an underlying iType font engine.
 *
 *      This function is available only if ITYPE_FENG is defined in the
 *      project or makefile.
 *
 *  Parameters:
 *      fontEngine   - [in] pointer to this TsFontEngine
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(TsUInt32)
iTypeFontEngine_getHeapBytesAllocated(TsFontEngine *fontEngine);



/*****************************************************************************
 *
 *  Constructs a UFST-specific TsFontEngine.
 *
 *  Description:
 *      This function will construct a TsFontEngine that uses UFST as
 *      its underlying font engine.
 *
 *      This function is only available if UFST_FENG is defined in the
 *      project or makefile.
 *
 *  Parameters:
 *
 *  Return value:
 *      Pointer to new TsFontEngine object.
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(TsFontEngine *)
UFSTFontEngine_new(void);


/*****************************************************************************
 *
 *  Constructs a UFST-specific memory managed TsFontEngine.
 *
 *  Description:
 *      This function will construct a TsFontEngine that uses UFST as
 *      its underlying font engine.
 *
 *      This function is only available if UFST_FENG is defined in the
 *      project or makefile.
 *
 *  Parameters:
 *      memMgr       - [in] pointer to memory manager object or NULL
 *
 *  Return value:
 *      Pointer to new TsFontEngine object.
 *
 *  <GROUP fontengine>
 */
TS_EXPORT(TsFontEngine *)
UFSTFontEngine_newex(TsMemMgr *memMgr);

TS_END_HEADER

#endif /* TSFNTENG_H */

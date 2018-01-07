/*****************************************************************************
 *
 *  tsmem.h - Interface to memory management functions
 *
 *  Copyright (C) 2008 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TS_MEMMGR_H
#define TS_MEMMGR_H

#include "tsproj.h"
#include "tsevent.h"
#include "tstag.h"
#include "tseudata.h"

TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  Memory manager object.
 *
 *  Description:
 *      The TsMemMgr object tracks memory usage and sends messages to observers
 *      when a user-defined limit is exceeded. Objects now have either a
 *      "newex" or "new" constructor that takes a TsMemMgr argument.
 *
 *      Usage of TsMemMgr is optional. The TsMemMgr argument may be NULL.
 *      Constructors without the TsMemMgr argument are provided for backward
 *      compatibility. These generally just call the newex constructor, so
 *      new users are recommended to call them directly.
 *
 *      You may choose to use a TsMemMgr only for some objects such as font services.
 *      When the memory limit is exceeded calls will be made to eliminate
 *      TsFontStyle and TsFont objects that are managed but otherwise unreferenced,
 *      thus freeing memory.
 *
 *      Two memory debugging build options are supported:
 *          TS_MEM_FILE_AND_LINE_INFO - when defined memory operations are mapped to
 *              TS_xALLOC_DEBUG macros defined in tsport.h - supports printing of file
 *              and line number initiating the memory operation
 *          TS_MEM_TRACING - when defined descriptive comments about the memory operation
 *              will be printed.
 *
 *      The TsMemMgr object will report any memory leaks (if TS_PRINTF_DEBUG is
 *      defined).
 *
 *      The TsMemMgr's heap size is specified in the call to TsMemMgr_new and
 *      can be dynamically changed using TsMemMgr_setHeapSize.
 *
 *      Clients can register as an observer of a TsMemMgr using
 *      TsMemMgr_registerObserver to get feedback when the heap size would be
 *      exceeded by an allocation request or when memory needed to be freed to
 *      satisfy a heap resize to a smaller size.
 *
 *      Internally the TsFontStyleManager and TsFontManager register as
 *      an observer of the TsMemMgr that they are passed when created.
 *
 *  <GROUP memmgr>
 */
typedef struct TsMemMgr_ TsMemMgr;


/*****************************************************************************
 *
 * Macro that gives the minimum allowed heap size for a TsMemMgr.
 *
 *  <GROUP memmgr>
 */
#define TS_MINIMUM_ALLOWED_HEAP_SIZE 5000


/*****************************************************************************
 *
 *  Creates a memory manager object.
 *
 *  Parameters:
 *      heapSize    - [in] maximum allowed heap space for this object
 *
 *  Return value:
 *      Pointer to new memory manager object or NULL if there is
 *      insufficient memory available.
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(TsMemMgr *)
TsMemMgr_new(TsSize heapSize);



/*****************************************************************************
 *
 *  Creates a memory manager object.
 *
 *  Description:
 *      If TS_INT_MEM is defined: This version allows the client to pass a 
 *      block of system memory that will be used by WTLE. If the passed in
 *      sysheap parameter is NULL, WTLE will allocate one large block of 
 *      size heapsize, from which requested block are doled out.
 *
 *      If TS_INT_MEM is not defined: it is an error to pass a non-NULL
 *      sysheap to this function , otherwise this function operates like
 *      TsMemMgr_new and returns a memory manager object that will limit the
 *      total memory used by WTLE.
 *
 *  Parameters:
 *      sysheap     - [in] pointer to system heap
 *      heapSize    - [in] maximum allowed heap space for this object
 *
 *  Return value:
 *      Pointer to new memory manager object if successful.
 *      NULL if there is insufficient memory available, or TS_INT_MEM is
 *      define and a system block is passed.
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(TsMemMgr *)
TsMemMgr_newex(void *sysheap, TsSize heapSize);



/*****************************************************************************
 *
 *  Frees a memory manager object.
 *
 *  Parameters:
 *      mgr    - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(void)
TsMemMgr_delete(TsMemMgr *mgr);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsMemMgr object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsMemMgr_releaseHandle.
 *
 *  Parameters:
 *      mgr   - [in] this
 *
 *  Return value:
 *      Pointer to memory manager object.
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(TsMemMgr *)
TsMemMgr_copyHandle(TsMemMgr *mgr);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsMemMgr object.
 *
 *  Description:
 *      This function reverses the action of TsMemMgr_copyHandle.
 *
 *  Parameters:
 *      mgr   - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(void)
TsMemMgr_releaseHandle(TsMemMgr *mgr);

/*****************************************************************************
 *
 *  Predefined priority values for observers
 *
 *  <GROUP memmgr>
 */
#define TS_OBSERVER_PRIORITY_FIRST          TS_INT32_MIN    /* indicates this observer should be called first. Only one observer of a TsMemMgr can have this value. */
#define TS_OBSERVER_PRIORITY_FONTSTYLEMGR   -2              /* private value for internal use only */
#define TS_OBSERVER_PRIORITY_FONTMGR        -1              /* private value for internal use only */
#define TS_OBSERVER_PRIORITY_DEFAULT        0               /* default priority */
#define TS_OBSERVER_PRIORITY_LAST           TS_INT32_MAX    /* indicates this observer should be called last. Only one observer of a TsMemMgr can have this value. */


/*****************************************************************************
 *
 *  Register an observer with the memory manager object.
 *
 *  Parameters:
 *      mgr         - [in] this
 *      observer    - [in] pointer to object that is observing the memory
 *                         manager
 *      priority    - [in] priority of this observer
 *      callback    - [in] callback function that is called when the memory
 *                         manager needs to send a message (i.e. to inform
 *                         clients to free some memory if possible)
 *
 *  Return value:
 *      TsResult    - TS_OK if the function completes successfully;
 *                  TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is
 *                  passed in.
 *
 *  Description:
 *          Register an observer with the memory manager object with a given
 *          priority. Operation: the priority can be any number. But see
 *          predefined values above. When memory recovery is needed, the
 *          observers are called in order of their priority number (lowest
 *          to highest). Clients can specify that their observer get the first
 *          or last chance to recover memory, but only one observer of a
 *          TsMemMgr can have each of these values. This will give the client
 *          the first (last) chance at freeing resources. Otherwise clients
 *          can set the priority as TS_OBSERVER_PRIORITY_DEFAULT. This will
 *          result in the default behavior which is that WTLE will attempt
 *          to free available memory from within its own objects;  by calling
 *          the font style manager observer callback and then the font manager
 *          observer callback; then client-set observers. (Client should use
 *          the same TsMemMgr for all the font related objects for the most
 *          effective memory recovery.)
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(TsResult)
TsMemMgr_registerObserver(TsMemMgr *mgr, void *observer, TsInt32 priority, TsEventCallback callback);


/*****************************************************************************
 *
 *  Unregister an observer with the memory manager object.
 *
 *  Parameters:
 *      mgr         - [in] this
 *      observer    - [in] pointer to object that is observing the memory
 *                         manager
 *
 *  Return value:
 *      TsResult    - TS_OK if the function completes successfully;
 *                  TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is
 *                  passed in.
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(TsResult)
TsMemMgr_unregisterObserver(TsMemMgr *mgr, void *observer);


#ifndef TS_MEM_FILE_AND_LINE_INFO


/*****************************************************************************
 *
 *  Memory manager 'malloc' macro.
 *
 *  Description:
 *      Macro maps to a TsMemMgr malloc function - the form of which
 *      depends on whether TS_MEM_FILE_AND_LINE_INFO is defined.
 *      That function will call TS_MALLOC or TS_MALLOC_DEBUG,
 *      which are declared in tsport.h.
 *
 *  Parameters:
 *      mgr         - [in] this - may be NULL
 *      size        - [in] the number of bytes to allocate
 *
 *  Return value:
 *      Pointer to allocated space, or NULL if there is insufficient system
 *      memory available or the size requested exceeds the space available
 *      in the memory manager's heap.
 *
 *  <GROUP memmgr>
 */
#define TsMemMgr_malloc(m,s)    TsMemMgr_mallocAux(m,s)


/*****************************************************************************
 *
 *  Memory manager 'calloc' macro.
 *
 *  Description:
 *      Macro maps to a TsMemMgr calloc function - the form of which
 *      depends on whether TS_MEM_FILE_AND_LINE_INFO is defined.
 *      That function will call TS_CALLOC or TS_CALLOC_DEBUG,
 *      which are declared in tsport.h.
 *
 *  Parameters:
 *      mgr         - [in] this - may be NULL
 *      nmemb       - [in] number of objects to allocate
 *      size        - [in] the size in bytes of each object
 *
 *  Return value:
 *      Pointer to allocated space, or NULL if there is insufficient system
 *      memory available or the size requested exceeds the space available
 *      in the memory manager's heap.
 *
 *  <GROUP memmgr>
 */
#define TsMemMgr_calloc(m,n,s)  TsMemMgr_callocAux(m,n,s)


/*****************************************************************************
 *
 *  Memory manager 'realloc' macro.
 *
 *  Description:
 *          Macro maps to a TsMemMgr realloc function - the form of which
 *          depends on whether TS_MEM_FILE_AND_LINE_INFO is defined.
 *          That function will call TS_REALLOC or TS_REALLOC_DEBUG,
 *          which are declared in tsport.h.
 *
 *  Parameters:
 *      mgr         - [in] this - may be NULL
 *      ptr         - [in] ptr to be reallocated
 *      size        - [in] the number of bytes to allocate
 *
 *  Return value:
 *      A pointer to the new location, or NULL if the new space
 *      could not be allocated.
*
 *  <GROUP memmgr>
 */
#define TsMemMgr_realloc(m,p,s) TsMemMgr_reallocAux(m,p,s)


/*****************************************************************************
 *
 *  Memory manager 'free' macro.
 *
 *  Description:
 *          Macro maps to a TsMemMgr free function - the form of which
 *          depends on whether TS_MEM_FILE_AND_LINE_INFO is defined.
 *          That function will call TS_FREE or TS_REALLOC_FREE,
 *          which are declared in tsport.h.
 *
 *          The mgr argument may be NULL but the argument must match
 *          that used when allocating the memory.
 *
 *          If ptr is NULL then returns without error.
 *
 *  Parameters:
 *      mgr         - [in] this
 *      ptr         - [in] pointer to the memory to be freed
 *
 *  Return value:
 *      none
 *
 *  <GROUP memmgr>
 */

#define TsMemMgr_free(m,p)      TsMemMgr_freeAux(m,p)



TS_EXPORT(void *)
TsMemMgr_mallocAux(TsMemMgr *mgr, TsSize size);



TS_EXPORT(void *)
TsMemMgr_callocAux(TsMemMgr *mgr, TsSize nmemb, TsSize size);


TS_EXPORT(void *)
TsMemMgr_reallocAux(TsMemMgr *mgr, void *ptr, TsSize size);


TS_EXPORT(void)
TsMemMgr_freeAux(TsMemMgr *mgr, void *ptr);


#else


#define TsMemMgr_malloc(m,s)    TsMemMgr_mallocAux(m,s,__FILE__,__LINE__)
#define TsMemMgr_calloc(m,n,s)  TsMemMgr_callocAux(m,n,s,__FILE__,__LINE__)
#define TsMemMgr_realloc(m,p,s) TsMemMgr_reallocAux(m,p,s,__FILE__,__LINE__)
#define TsMemMgr_free(m,p)      TsMemMgr_freeAux(m,p,__FILE__,__LINE__)


TS_EXPORT(void *)
TsMemMgr_mallocAux(TsMemMgr *mgr, TsSize size, const char * const file, int line);
TS_EXPORT(void *)
TsMemMgr_callocAux(TsMemMgr *mgr, TsSize nmemb, TsSize size, const char * const file, int line);
TS_EXPORT(void *)
TsMemMgr_reallocAux(TsMemMgr *mgr, void *ptr, TsSize size, const char * const file, int line);
TS_EXPORT(void)
TsMemMgr_freeAux(TsMemMgr *mgr, void *ptr, const char * const file, int line);

#endif


/*****************************************************************************
 *
 *  Sets the heap size of the memory manager.
 *
 *  Parameters:
 *      mgr          - [in] this
 *      newHeapSize  - [in] new requested heap size
 *
 *  Return value:
 *      TsResult        TS_OK if resize succeeds. TS_ERR_RESIZE_FAIL if
 *                      it is not possible to shrink the heap to the requested
 *                      size. Note that there is minimum heap size allowed.
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(TsResult)
TsMemMgr_setHeapSize(TsMemMgr *mgr, TsSize newHeapSize);


/*****************************************************************************
 *
 *  Gets the size of the memory manager's heap.
 *
 *
 *  Parameters:
 *      mgr         - [in] this
 *
 *  Return value:
 *      TsSize      The size of the heap.
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(TsSize)
TsMemMgr_getHeapSize(TsMemMgr *mgr);


/*****************************************************************************
 *
 *  Gets the amount of memory currently being used by the memory manager.
 *
 *
 *  Parameters:
 *      mgr         - [in] this
 *
 *  Return value:
 *      TsSize      The amount of memory currently being used by the
 *                  memory manager object.
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(TsSize)
TsMemMgr_getHeapUsed(TsMemMgr *mgr);


/*****************************************************************************
 *
 *  Structure for passing TsMemMgr message data.
 *
 *  <GROUP memmgr>
 */
typedef struct TsMemMgrMessageData_
{
    TsBool freeMaximumPossible;     /* if TRUE, free as much memory as possible, if FALSE stop after freeing bytesNeeded bytes */
    TsSize bytesNeeded;             /* amount of memory required, ignored if freeMaximumPossible is TRUE */
    TsTag senderTag;                /* tag indicating what type of object is originating the message */
    TsInt32 senderID;               /* id of object sending the message */
    void *sender;                   /* pointer to object that originated the message, or NULL */
} TsMemMgrMessageData;


/*****************************************************************************
 *
 * Tags that are set when a TsMemMgr sends a message
 *
 *  <GROUP memmgr>
 */
#define TsTag_memMgrMalloc TsMakeTag('m','m','a','l');  /* used when the WTLE heap needs to grow to fulfill an allocation request */
#define TsTag_memMgrReduce TsMakeTag('m','m','r','d');  /* used when a heap reduction was requested (with no message data supplied)*/
#define TsTag_memMgrResize TsMakeTag('m','m','s','z');  /* used when TsMemMgr_resize is called with a request to shrink the heap to a size below what is currently allocated */
#define TsTag_memMgrFntEng TsMakeTag('m','m','f','e');  /* set in message data passed to TsMemMgr_reduce when the font engine returns a malloc error */

/*****************************************************************************
 *
 *  Attempt to reduce amount of memory used by object through sending message
 *  to registered observers to free memory if possible.
 *
 *  Description:    Send a message to observers of the object indicating that
 *                  memory needs to freed.
 *
 *                  If the flag freeMaximumPossible is set, the function will
 *                  return TS_OK if any memory is freed.
 *                  If the flag is not set, the function will return TS_OK if
 *                  md->bytesNeeded or more bytes were freed
 *
 *                  if md is NULL, function will free as much as possible from the heap
 *
 *  Parameters:
 *      mgr     - [in] this
 *      md      - [in] a TsMemMgrMessageData, or NULL
 *
 *  Return value:
 *      TsResult   TS_OK if successful (see above);
 *                 TS_ERR_RESIZE_FAIL otherwise.
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(TsResult)
TsMemMgr_reduce(TsMemMgr *mgr, TsMemMgrMessageData *md);


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
 *      becomes invalid when this TsMemMgr object is destroyed.
 *      Therefore, you must keep reference to the TsMemMgr object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      mgr   - [in] this
 *
 *  Return value:
 *      Pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP memmgr>
 */
TS_EXPORT(TsExternalUserData *)
TsMemMgr_getExternalUserdata(TsMemMgr *mgr);


/*****************************************************************************
 *
 * Macros and functions that exist only when the TS_MEM_TRACING configuration
 * options is defined in tsconfig.h.
 *
 *  Description:
 *      When TS_MEM_TRACING is defined, information about the allocation/free
 *      will be piped through TS_PRINTF_DEBUG. This includes the pointers,
 *      sizes, and a string that is set with the TS_SET_MEMSTR macro. The
 *      string is a message that can give detail about the allocation (what
 *      function it is in for example).
 *
 *      When TS_MEM_TRACING is defined, The TsMemMgr object has a pointer to
 *      store the string; however if the allocation/free routine is called
 *      with a NULL TsMemMgr, the string is stored in a global pointer.
 *
 *      If the TS_MEM_FILE_AND_LINE_INFO is also defined, the information will
 *      also include the file and line number where the allocation/free takes
 *      place.
 *
 *      The data that printed is in tab-delimited format so that it can be
 *      viewed more easily in a spreadsheet program.
 *
 *      If TS_MEM_TRACING is not defined, the macros resolve to nothing.
 *
 *  <GROUP memmgr>
 */
#ifdef TS_MEM_TRACING
#define TS_SET_MEMSTR(mgr, string)  TsMemMgr_setString(mgr, string);
#define TS_CLEAR_MEMSTR(mgr)        TsMemMgr_clearString(mgr);
#define TS_STORE_MEMSTR(mgr)        TsMemMgr_storeString(mgr);
#define TS_RESTORE_MEMSTR(mgr)      TsMemMgr_restoreString(mgr);

TS_EXPORT(void)
TsMemMgr_setString(TsMemMgr *mgr, const char *str);

TS_EXPORT(void)
TsMemMgr_clearString(TsMemMgr *mgr);

TS_EXPORT(void)
TsMemMgr_storeString(TsMemMgr *mgr);

TS_EXPORT(void)
TsMemMgr_restoreString(TsMemMgr *mgr);

#else

#define TS_SET_MEMSTR(mgr, string)
#define TS_CLEAR_MEMSTR(mgr)
#define TS_STORE_MEMSTR(mgr)
#define TS_RESTORE_MEMSTR(mgr)

#endif


/* Deprecated - use TsMemMgr functions instead */

TS_EXPORT(void *)
TsMem_calloc(TsSize nmemb, TsSize size);

TS_EXPORT(void)
TsMem_free(void *ptr);

TS_EXPORT(void *)
TsMem_malloc(TsSize size);

TS_EXPORT(void *)
TsMem_realloc(void *ptr, TsSize size);


TS_END_HEADER


#endif /* TS_MEMMGR_H */



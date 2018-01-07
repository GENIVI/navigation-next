/*****************************************************************************
 *
 *  tsrsrc.h - Defines interface to TsResource structure.
 *
 *  Copyright (C) 2003-2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSRSRC_H
#define TSRSRC_H

#include "tsproj.h"
#include "tsmem.h"

TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  An enumeration of resource types.
 *
 *  Version:
 *      Version 3.1: eliminated TS_RESOURCETYPE_PATH_PLUS.
 *      Instead TS_RESOURCETYPE_PATH is used more generally.
 *
 *  <GROUP resource>
 */
typedef enum TsResourceType
{
    TS_RESOURCETYPE_MEMORY,         /* memory-mapped file */
    TS_RESOURCETYPE_PATH            /* file with a file path */
} TsResourceType;


/*****************************************************************************
 *
 *  A private structure that describes a memory unit (location and length in bytes)
 *
 */
struct TsMemLoc_
{
    void *address;              /* address */
    TsSize length;              /* length in bytes */
};


/*****************************************************************************
 *
 *  A private structure that specifies a file stored in a file system:
 *  path, subfile, and file manager info.
 *
 */
struct TsFileSpec_
{
    TsChar *path;               /* path of file */
    TsSize offset;              /* offset to beginning of subfile */
    TsSize length;              /* subfile length in bytes */
    void *fileMgr;              /* ptr to fileMgr */
};


/*****************************************************************************
 *
 *  A private structure that describes a resource stored in a file or memory.
 *
 */
struct TsResource_
{
    TsMemMgr *memMgr;
    TsResourceType type;
    union
    {
        struct TsMemLoc_ mem;       /* memory location and length */
        struct TsFileSpec_ spec;    /* extended file info */
    } file;
};



/*****************************************************************************
 *
 *  An object that describes a resource stored in a file or memory.
 *
 *  Description:
 *      This object should be declared locally and initialized using
 *      one of the TsResource_init functions.
 *
 *      The object may allocate memory to copy the file path.
 *      When finished, use TsResource_done to cleanup any allocated data.
 *
 *  <GROUP resource>
 */
typedef struct TsResource_ TsResource;


/*****************************************************************************
 *
 *  Initializes a TsResource structure as a file path.
 *
 *  Description:
 *      The path argument is a string indicating the full path to a file
 *      stored on a file system. This function will copy that string.
 *
 *      You must call TsResource_done() when finished in order to
 *      properly clean up this structure.
 *
 *      Do not call this function with any TsResource that has already
 *      been initialized unless you first call TsResource_done.
 *
 *  Parameters:
 *      resource    - [in/out] pointer to resource structure
 *      path        - [in] pointer to full file path
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP resource>
 */
TS_EXPORT(TsResult)
TsResource_initPath(TsResource *resource, TsChar *path);


/*****************************************************************************
 *
 *  Initializes a TsResource structure as a path plus extended information
 *  for supporting subfiles, an external file manager and/or use of a
 *  memory manager.
 *
 *  Description:
 *      The path argument is a string indicating the full path to a file
 *      stored on a file system. This function will copy that string.
 *
 *      You must call TsResource_done() when finished in order to
 *      properly clean up this structure.
 *
 *      Do not call this function with any TsResource that has already
 *      been initialized unless you first call TsResource_done.
 *
 *      When the file contains more than one resource, you may specify
 *      offset and length to indicate the byte location and length of the
 *      subfile of interest. Setting both of these to 0 indicates normal
 *      operation, i.e. the entire file is used.
 *
 *      The fileMgr argument is passed through to the file i/o macros defined
 *      in tsport.h. If you have file i/o functions that need an external file
 *      manager then modify those macros when doing the port. Set this
 *      argument to default if your file i/o functions do not need a file
 *      manager argument.
 *
 *  Parameters:
 *      resource    - [in/out] pointer to resource structure
 *      memMgr      - [in] pointer to a memory manager object or NULL
 *      path        - [in] pointer to full file path
 *      offset      - [in] offset in bytes within file to start of subfile
 *      length      - [in] length in bytes of subfile
 *      fileMgr     - [in] optional void pointer to file manager
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP resource>
 */
TS_EXPORT(TsResult)
TsResource_initPathPlus(TsResource *resource, TsMemMgr *memMgr, TsChar *path, TsSize offset,
                       TsSize length, void *fileMgr);


/*****************************************************************************
 *
 *  Intialize a TsResource structure as a memory location.
 *
 *  Parameters:
 *      resource    - [in/out] pointer to resource structure
 *      address     - [in] address of memory-mapped file
 *      length      - [in] length of memory-mapped file in bytes.
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP resource>
 */
TS_EXPORT(TsResult)
TsResource_initMem(TsResource *resource, void *address, TsSize length);


/*****************************************************************************
 *
 *  Deinitializes the TsResource.
 *
 *  Description:
 *      Frees any memory that has been allocated.
 *      This function should be called when finished with the TsResource.
 *
 *  Parameters:
 *      resource    - [in/out] pointer to resource structure
 *
 *  Return value:
 *      none
 *
 *  <GROUP resource>
 */
TS_EXPORT(void)
TsResource_done(TsResource *resource);


/*****************************************************************************
 *
 *  Compares two TsResource objects for equality.
 *
 *  Parameters:
 *      r1          - [in] pointer to first of two resource structures
 *      r2          - [in] pointer to second of two resource structures
 *
 *  Return value:
 *      TsBool     TRUE if two resources are considered equal, FALSE otherwise
 *
 *  <GROUP resource>
 */
TS_EXPORT(TsBool)
TsResource_isEqual(TsResource *r1, TsResource *r2);


/*****************************************************************************
 *
 *  Copy a TsResource structure
 *
 *  Description:
 *      This function copies the structure including underlying string data.
 *
 *      You must call TsResource_done() when finished with 'dest'
 *      in order to properly clean up this structure.
 *
 *      Do not call this function with any 'dest' resource that has already
 *      been initialized unless you first call TsResource_done with 'dest'.
 *
 *  Parameters:
 *      dest        - [out] pointer to destination resource structure
 *      source      - [in] pointer to source resource structure
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP resource>
 */
TS_EXPORT(TsResult)
TsResource_copy(TsResource *dest, TsResource *source);



TS_END_HEADER

#endif /* TSRSRC_H */

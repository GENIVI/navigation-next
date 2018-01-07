/******************************************************************************
 *
 *  fs_scratch.h - Declares public FsScratchSpace
 *
 *  Copyright (C) 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential information of Monotype Imaging Inc.
 *
 *****************************************************************************/


#ifndef FS_SCRATCH_H
#define FS_SCRATCH_H

#include "fs_err.h"
#include "fs_port.h"
#include "fs_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/* FsScratchSpace_new                                                        */
/* Allocates scratch space and initializes to internal block of specified    */
/* size. Returns pointer to new FsScratchSpace object.                       */
/* Calls FsScratchSpace_init(newlyAllocatedSS, FS_state_ptr, size) to        */
/* initialize scratch space memory.  Returns NULL and sets FS_STATE::error   */
/* appropriately upon error.                                                 */
FsScratchSpace *
FsScratchSpace_new(FS_STATE *FS_state_ptr, FS_ULONG size);

/* FsScratchSpace_init                                                       */
/* Initializes provided scratch space to internal block of specified size.   */
/* Memory is initialized to zero. Returns error or SUCCESS.                  */
/* Upon error, FS_STATE::error will also be set appropriately.               */
FS_LONG
FsScratchSpace_init(FsScratchSpace *ss, FS_STATE *FS_state_ptr, FS_ULONG size);
 
/* FsScratchSpace_done                                                       */
/* Releases all internal memory management & guts of provided scratch space. */
/* FS_STATE::error set appropriately upon return.                            */
void
FsScratchSpace_done(FsScratchSpace *ss, FS_STATE *FS_state_ptr);

/* FsScratchSpace_delete                                                     */
/* Calls FsScratchSpace_done(ss, FS_state_ptr) which releases all scratch    */
/* space memory and then frees the scratch space object “ss”.                */
/* Upon return, FS_STATE::error will be set appropriately, and “ss” will no  */
/* longer be a pointer to a valid object/memory location.                    */
void
FsScratchSpace_delete(FsScratchSpace *ss, FS_STATE *FS_state_ptr);

/* FsScratchSpace_reserve                                                    */
/* Reserves a block of nbytes of scratch space memory using provided “ss”    */
/* object. Returns a pointer which can be cast into other types.             */
/* If the return pointer is null, STATE.error should also be set.            */
void *
FsScratchSpace_reserve(FsScratchSpace *ss, FS_STATE *FS_state_ptr, FS_ULONG nbytes);

/* FsScratchSpace_release                                                    */
/* Releases the block of memory indicated by the addr pointer which is       */
/* currently being managed by the provided “ss”                              */
/* Return error or SUCCESS (and set FS_STATE::error appropriately).          */
FS_LONG
FsScratchSpace_release(FsScratchSpace *ss, FS_STATE *FS_state_ptr, void *addr);

/* FsScratchSpace_shrink                                                     */
/* Currently a stub for future use.                                          */
/* Generally to be used as a callback from memory recovery mechanisms.       */
/* Will free up as much internal memory and management overhead as possible  */
/* while retaining originally-allocated block and all outstanding pointers.  */
/* Returns error or SUCCESS.   Will set “gotSome” to 0 if nothing freed, or  */
/* positive size of memory freed otherwise.                                  */
FS_LONG
FsScratchSpace_shrink(FsScratchSpace *ss, FS_STATE *FS_state_ptr, int *gotSome);


#ifdef __cplusplus
}
#endif

#endif /* FS_SCRATCH_H */

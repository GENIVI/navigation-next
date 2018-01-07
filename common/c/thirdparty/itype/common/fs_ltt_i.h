/******************************************************************************
 *
 *  fs_ltt_i.h - Declares .ltt file input functions.
 *
 *  Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential information of Monotype Imaging Inc.
 *
 *****************************************************************************/


#ifndef FS_LTT_I_H
#define FS_LTT_I_H

#include <stdio.h>

#include "fs_ltt_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 *
 *  Reads a linked font from disk.
 *
 *  Description:
 *      This function reads a linked font from disk using a file pointer.
 *      Memory for the FsLtt must be created prior to calling this function.
 *      The file must be opened prior to calling this function. 
 *      The loadAdjustments argument controls whether component adjustments
 *      are loaded along with other component information.
 *      
 *  Parameters:
 *      ltt         - [in] pointer to linked font (FsLtt)
 *      fp          - [in] open file pointer
 *      loadAdjustments - [in] flag to control loading component adjustments
 *
 *  Return Value:
 *      SUCCESS, or error code if component could not be copied.
 *
 *      <TABLE>
 *      Return values               Reason
 *      ------------------          --------------------------------
 *      ERR_MALLOC_FAIL             System memory allocation failure
 *      </TABLE>
 *
 *  See Also:
 *      FsLtt_load()
 *
 *  <GROUP FsLttInputFunctions>
 */  
ITYPE_API FS_ERROR
FsLtt_read(FsLtt *ltt, FS_FILE *fp, FS_BOOLEAN loadAdjustments);

/*****************************************************************************
 *
 *  Loads a linked font from a memory address.
 *
 *  Description:
 *      This function loads a linked font using a memory address that points
 *      to the linked font data. Memory for the FsLtt must be created prior 
 *      to calling this function.
 *      The loadAdjustments argument controls whether component adjustments
 *      are loaded along with other component information.
 *      
 *  Parameters:
 *      ltt         - [in] pointer to linked font (FsLtt)
 *      mem         - [in] memory address of linked font data
 *      loadAdjustments - [in] flag to control loading component adjustments
 *
 *  Return Value:
 *      SUCCESS, or error code if component could not be copied.
 *
 *      <TABLE>
 *      Return values               Reason
 *      ------------------          --------------------------------
 *      ERR_MALLOC_FAIL             System memory allocation failure
 *      </TABLE>
 *
 *  See Also:
 *      FsLtt_read()
 *
 *  <GROUP FsLttInputFunctions>
 */  
ITYPE_API FS_ERROR
FsLtt_load(FsLtt *ltt, FS_BYTE *mem, FS_BOOLEAN loadAdjustments);


#ifdef __cplusplus
}
#endif

#endif /* FS_LTT_I_H */

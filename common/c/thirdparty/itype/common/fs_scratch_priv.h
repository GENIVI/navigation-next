/******************************************************************************
 *
 *  fs_scratch_priv.h - Defines private FsScratchSpace
 *
 *  Copyright (C) 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential information of Monotype Imaging Inc.
 *
 *****************************************************************************/


#ifndef FS_SCRATCH_PRIV_H
#define FS_SCRATCH_PRIV_H

#include "fs_port.h"

#ifdef __cplusplus
extern "C" {
#endif


struct FsScratchSpace_
{
    FS_BYTE *buffer;
    FS_BYTE *beyond;
};


typedef struct FsScratchSpace_ FsScratchSpace;


#ifdef __cplusplus
}
#endif

#endif /* FS_SCRATCH_PRIV_H */

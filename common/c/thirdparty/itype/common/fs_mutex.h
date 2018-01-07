
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_mutex.h */

#ifndef FS_MUTEX_H
#define FS_MUTEX_H

#if (defined(FS_MULTI_PROCESS) || defined(FS_MULTI_THREAD))

/* prototypes */
FS_LONG FS_create_mutex(FS_STATE *sp);
FS_LONG FS_obtain_mutex(FS_STATE *sp);
FS_LONG FS_release_mutex(FS_STATE *sp);
FS_LONG FS_delete_mutex(FS_STATE *sp);

#endif

#endif /* FS_MUTEX_H */

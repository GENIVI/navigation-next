/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!--------------------------------------------------------------------------
 
 @file     paldberror.h
 @date     09/01/2010
 @defgroup PAL DB API
 
 @brief    SQLITE error message 
 
 */
/*
 (C) Copyright 2010 by Networks In Motion, Inc.                
 
 The information contained herein is confidential, proprietary 
 to Networks In Motion, Inc., and considered a trade secret as 
 defined in section 499C of the penal code of the State of     
 California. Use of this information by anyone other than      
 authorized employees of Networks In Motion is granted only    
 under a written non-disclosure agreement, expressly           
 prescribing the scope and manner of such use.                 
 
 ---------------------------------------------------------------------------*/

#ifndef PALDBERROR_H
#define PALDBERROR_H

#include "palsqlerror.h"


#define PAL_DB_OK                   SQLITE_OK              /* Successful result */

/* beginning-of-error-codes */
#define PAL_DB_ERROR                SQLITE_ERROR           /* SQL error or missing database */
#define PAL_DB_INTERNAL             SQLITE_INTERNAL        /* Internal logic error in SQLite */
#define PAL_DB_PERM                 SQLITE_PERM            /* Access permission denied */
#define PAL_DB_ABORT                SQLITE_ABORT           /* Callback routine requested an abort */
#define PAL_DB_BUSY                 SQLITE_BUSY            /* The database file is locked */
#define PAL_DB_LOCKED               SQLITE_LOCKED          /* A table in the database is locked */
#define PAL_DB_ENOMEM               SQLITE_NOMEM           /* A malloc() failed */
#define PAL_DB_READONLY             SQLITE_READONLY        /* Attempt to write a readonly database */
#define PAL_DB_INTERRUT             SQLITE_INTERRUPT       /* Operation terminated by sqlite3_interrupt()*/
#define PAL_DB_IOERR                SQLITE_IOERR          /* Some kind of disk I/O error occurred */
#define PAL_DB_CORRUPT              SQLITE_CORRUPT        /* The database disk image is malformed */
#define PAL_DB_NOTFOUND             SQLITE_NOTFOUND       /* NOT USED. Table or record not found */
#define PAL_DB_FULL                 SQLITE_FULL           /* Insertion failed because database is full */
#define PAL_DB_CANTOPEN             SQLITE_CANTOPEN       /* Unable to open the database file */
#define PAL_DB_PROTOCOL             SQLITE_PROTOCOL       /* NOT USED. Database lock protocol error */
#define PAL_DB_EMPTY                SQLITE_EMPTY          /* Database is empty */
#define PAL_DB_SCHEMA               SQLITE_SCHEMA         /* The database schema changed */
#define PAL_DB_TOOBIG               SQLITE_TOOBIG         /* String or BLOB exceeds size limit */
#define PAL_DB_CONSTRAINT           SQLITE_CONSTRAINT     /* Abort due to constraint violation */
#define PAL_DB_MISMATCH             SQLITE_MISMATCH       /* Data type mismatch */
#define PAL_DB_MISUSE               SQLITE_MISUSE         /* Library used incorrectly */
#define PAL_DB_NOLFS                SQLITE_NOLFS          /* Uses OS features not supported on host */
#define PAL_DB_AUTH                 SQLITE_AUTH           /* Authorization denied */
#define PAL_DB_FORMAT               SQLITE_FORMAT         /* Auxiliary database format error */
#define PAL_DB_RANGE                SQLITE_RANGE          /* 2nd parameter to sqlite3_bind out of range */
#define PAL_DB_NOTADB               SQLITE_NOTADB         /* File opened that is not a database file */
#define PAL_DB_ROW                  SQLITE_ROW           /* sqlite3_step() has another row ready */
#define PAL_DB_DONE                 SQLITE_DONE          /* sqlite3_step() has finished executing */

#endif
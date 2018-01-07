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

@file paltypes.h
@defgroup PALTYPES PALTYPES

@brief Declarations for common types used throughout the SDK

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PALTYPES_H
#define PALTYPES_H

#include "palexp.h"
#define PATH_DELIMITER '\\'


#ifndef NB_DISABLE_TYPES
typedef long long           int64;      /* Signed 64 bit value */
typedef unsigned long long  uint64;     /* Unsigned 64 bit value */
typedef signed long         int32;      /* Signed 32 bit value */
typedef unsigned int        uint32;     /* Unsigned 32 bit value */
typedef unsigned int        uint;       /* Unsigned 32 bit value */
typedef unsigned short      uint16;     /* Unsigned 16 bit value */
typedef signed short        int16;      /* Signed 16 bit value */
typedef signed char         int8;       /* signed 8 bit value */
typedef unsigned char       uint8;      /* Unsigned 8 bit value */
typedef unsigned char       byte;       /* byte type */
#endif


typedef int32               nb_unixTime;
typedef int32               nb_size;
typedef uint32              nb_usize;
typedef unsigned char       nb_boolean;
typedef double              nb_floatTime;

typedef uint32              nb_gpsTime;
typedef unsigned long       nb_threadId;

typedef struct
{
    uint32                  major;
    uint32                  minor;
    uint32                  feature;
    uint32                  build;
} nb_version;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define NB_MINIMUM_VALID_TIME   946684800 /* 2000/01/01 00:00:00 UTC */

#endif

/*! @} */

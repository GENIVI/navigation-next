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

@file     audbuf.h
@date     11/10/2003
@defgroup NIM AB PAL

@brief    Dynamic Buffer Functions

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


/*! @( */

#ifndef AUDBUF_H
#define    AUDBUF_H

#include "palerror.h"
#include "paltypes.h"
#include "palstdlib.h"
#include "abpalexp.h"


 /*! Dynamic buffer structure for PAL Audio. */
typedef struct _AudioBuffer {
    byte* p;         /*!< Beginning of the buffer. */
    byte* x;            /*!< Write pointer. */
    byte* l;          /*!< End (limit) of the buffer. */
    size_t absize;     /*!< Allocation block size. */
    PAL_Error error; /*!< Error during buffer operations, if any. */
}PalAudioBuffer;

/* Creation and destruction. */
ABPAL_DEC PAL_Error       AudioBufferNew(PalAudioBuffer* dbp, size_t absize);
ABPAL_DEC void            AudioBufferDelete(PalAudioBuffer* dbp);

/* Manipulation. */
ABPAL_DEC void            AudioBufferAdd(PalAudioBuffer* dbp, byte b);
ABPAL_DEC void             AudioBufferCat(PalAudioBuffer* dbp, const byte* data, size_t datalen);
ABPAL_DEC void            AudioBufferCats(PalAudioBuffer* dbp, const char* cstr);
ABPAL_DEC void            AudioBufferFmt(PalAudioBuffer* dbp, const char* fmt, ...);
ABPAL_DEC void            AudioBufferFmtd(PalAudioBuffer* dbp, double d);

/* Retrieval. */
ABPAL_DEC PAL_Error       AudioBufferError(PalAudioBuffer* dbp);
ABPAL_DEC const byte*     AudioBufferGet(PalAudioBuffer* dbp);
ABPAL_DEC const char*     AudioBufferGets(PalAudioBuffer* dbp);
ABPAL_DEC char            AudioBufferGetc(PalAudioBuffer* dbp, size_t idx);
ABPAL_DEC size_t          AudioBufferLen(PalAudioBuffer* dbp);


#endif


/*! @} */

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

 @file     palaudioqnx.h
 @date     04/18/2012
 @defgroup PAL QNX Audio API

 @brief    QNX Audio API interface

 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PALAUDIOQNX_H_
#define PALAUDIOQNX_H_

#include "abpalaudio.h"
#include <palerror.h>
#include <pthread.h>
#include "paltestlog.h"

#ifndef NDEBUG
#define CONLOG(level,fmt,...) NB_TEST_LOG(PAL_LogBuffer_1,level,"[QNXP:%d] "fmt,pthread_self(),##__VA_ARGS__)
#else
#define CONLOG(...)
#endif
#define ERRLOG(error,fmt,...) NB_TEST_LOG(PAL_LogBuffer_1,PAL_LogLevelError,"[QNXP:%d]EEE %d: "fmt,pthread_self(),error,##__VA_ARGS__)

typedef struct QNXPlayer ABPAL_QNXPlayer;

PAL_Error
QNXPlayerCreate(ABPAL_QNXPlayer **p);

PAL_Error
QNXPlayerDestroy(ABPAL_QNXPlayer *p);

PAL_Error
QNXPlayerPlay(ABPAL_QNXPlayer *p, byte *data, uint32 nSize, ABPAL_AudioFormat format, ABPAL_AudioPlayerCallback *cb, void *cbdata);

PAL_Error
QNXPlayerStop(ABPAL_QNXPlayer *p);

PAL_Error
QNXPlayerSetVolume(ABPAL_QNXPlayer *p, int32 level);

PAL_Error
QNXPlayerGetVolume(ABPAL_QNXPlayer *p, int32 *level);

PAL_Error
QNXPlayerSetOutput(ABPAL_QNXPlayer *p, int32 *output);

#endif /* PALAUDIOQNX_H_ */

/*! @} */

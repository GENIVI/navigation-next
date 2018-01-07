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

@file paltestlog.h
@defgroup PALTESTLOG_H PAL

@brief Provides the PAL interface

*/
/*
(C) Copyright 2013 by Telecommunication Systems, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PALTESTLOG_H
#define	PALTESTLOG_H

#include "paltypes.h"
#include "palerror.h"
#include "palconfig.h"
#include "pal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum  {
    PAL_LogBuffer_0 = 0,
    PAL_LogBuffer_1,
    PAL_LogBuffer_2,
    PAL_LogBuffer_Invalid,
} PAL_LogBufferNumber;


/*! Logging Levels. */
typedef enum  {
    PAL_LogLevelNone = 0,                            /*!< No events logged */
    PAL_LogLevelCritical,                              /*!< Critical are logged */
    PAL_LogLevelError,                               /*!< Errors are logged */
    PAL_LogLevelWarning,                             /*!< Warnings are logged */
    PAL_LogLevelInfo,                                /*!< General Information is logged */
    PAL_LogLevelDebug,                               /*!< Debug Information is logged */
    PAL_LogLevelVerbose,                             /*!< Verbose Information is logged */
} PAL_LogLevel;


/*  This function logs the user input to a platform dependent implementation

    DON'T call this function directly! It does NOT get disabled in release mode!
    Call NB_TEST_LOG instead! 
*/
PAL_DEC void PAL_TestLog(PAL_LogBufferNumber bufferNumber, PAL_LogLevel level, const char *fmt, ...);

PAL_DEC void PAL_SetVersionString(char *version);

PAL_DEC void PAL_SetStartTime(char *time);

#ifdef ENABLE_LOGGING_TO_OUTPUT
#ifdef _WIN32
#define NB_TEST_LOG PAL_TestLog
#else
#define NB_TEST_LOG(bufferNumber, level, ...) PAL_TestLog((bufferNumber), (PAL_LogLevel)(level), ##__VA_ARGS__)
#endif
#else
#define NB_TEST_LOG(bufferNumber, level, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif

/*! @} */

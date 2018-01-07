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

@file cslqalog.h
@defgroup cslqalog QA Logging
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

#ifndef CSLQALOG_H
#define CSLQALOG_H

/*! @{ */


#include "cslqarecord.h"
#include "cslexp.h"
#include "pal.h"
#include "nberror.h"


typedef struct CSL_QaLog CSL_QaLog;


typedef struct
{
    const char* productName;
	nb_version  productVersion;
	uint32      platformId;
	uint64      mobileDirectoryNumber;
	uint64      mobileInformationNumber;
} CSL_QaLogHeaderData;


/*! Creates a new instance of a CSL_QaLog object

@param pal An instance of the PAL
@param headerData The product and handset header data for the QA log
@param filename The name of the QA log file
@param verbose Enable verbose logging
@param log On success, the newly created CSL_QaLog instance.  A valid object must be destroyed with CSL_QaLogDestroy
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaLogCreate(PAL_Instance* pal, CSL_QaLogHeaderData* headerData, const char* filename, nb_boolean verbose, CSL_QaLog** log);

/*! Destroy a previously created QA Log object

@param log A CSL_QaLog object previously created with CSL_QaLogCreate
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaLogDestroy(CSL_QaLog* log);

/*! Setup QaLog filter

@param log A QA Log instance
@param id ID of QaRecord
@param enable Enable or disable the log ID
@return NB_Error
*/
CSL_DEC NB_Error
CSL_QaLogFilterSet(CSL_QaLog* log, uint16 id, nb_boolean enable);

/*! Set QaLog filter for all IDs

@param log A QA Log instance
@param enable Enable or disable all log IDs
@return NB_Error
*/
CSL_DEC NB_Error
CSL_QaLogFilterSetAll(CSL_QaLog* log, nb_boolean enable);

/*! Setup KPI freeform event filter

@param log A QA Log instance
@param eventLabel Label of the KPI event
@param enable Enable or disable the label
@return NB_Error
*/
CSL_DEC NB_Error
CSL_QaLogKpiFreeformEventFilterSet(CSL_QaLog* log,
        const char* eventLabel,
        nb_boolean enable);

/*! Setup KPI freeform event filter for all labels

@param log A QA Log instance
@param enable Enable or disable all KPI freeform events
*/
CSL_DEC void
CSL_QaLogKpiFreeformEventFilterSetAll(CSL_QaLog* log,
        nb_boolean enable);

/*! Returns is KPI event with the label enabled or not

@param log A QA Log instance
@param eventLabel Label of the KPI event
@return TRUE if the event is disabled
*/
CSL_DEC nb_boolean
CSL_QaLogKpiFreeformEventFilterIsDisabled(CSL_QaLog* log,
        const char* eventLabel);

/*! Returns an unique IID (interval ID) for KPI feature
  Call CSL_QaLogKpiIidRelease() if the ID is no longer needed and can be reused

@param log A QA Log instance
@return New positive interval ID or 0 in error cases
*/
CSL_DEC uint32
CSL_QaLogKpiIidAllocate(CSL_QaLog* pThis);

/*! Releases the IID (interval ID) to make it available for CSL_QaLogKpiIidAllocate() function

@param log A QA Log instance
@param id The IID to release
@return New positive interval ID or 0 in error cases
*/
CSL_DEC void
CSL_QaLogKpiIidRelease(CSL_QaLog* pThis,
        uint32 id);


/*! Determine if verbose QA logging is enabled

@param log A QA Log instance
@return Non-zero if verbose QA logging is enabled; zero otherwise
*/
CSL_DEC nb_boolean CSL_QaLogIsVerboseLoggingEnabled(CSL_QaLog* log);


/*! Write to the QA log file

@param log A QA Log instance
@param record The QA Log Record to write
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaLogWrite(CSL_QaLog* log, CSL_QaRecord* record);

/*! Write to the QA log file

@param log A QA Log instance
@param data The QA Log data to write
@param dataSize The data size
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaLogWriteData(CSL_QaLog* log, const uint8* data, uint32 dataSize);

/*! Log an application state message to the QA Log

@param log A QA Log instance
@param state A NULL terminated string containing the state message
*/
CSL_DEC NB_Error CSL_QaLogAppState(CSL_QaLog* log, const char* state);

/*! Flush QA Log contents into persistent file.

  @param log A QA Log instance
*/
CSL_DEC void CSL_QaLogFlush(CSL_QaLog* log);

CSL_DEC void CSL_QaLogClear(CSL_QaLog* pThis, uint32 retainSessionCount);

/*! @} */

#endif

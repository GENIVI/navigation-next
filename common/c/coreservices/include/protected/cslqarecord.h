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

@file cslqarecord.h
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

/*! @{ */

#ifndef CSLQARECORD_H
#define CSLQARECORD_H

#include "cslexp.h"
#include "nberror.h"
#include "paltypes.h"


typedef struct CSL_QaRecord CSL_QaRecord;


/*! Creates a new instance of a CSL_QaRecord object

@param gpsTime The GPS time to be written to the record
@param id The record identifier
@param size Initial size of internal data buffer to allocate
@param record On success, the newly created CSL_QaRecord instance.  A valid object must be destroyed with CSL_QaRecordDestroy
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaRecordCreate(nb_gpsTime gpsTime, uint16 id, uint16 size, CSL_QaRecord** record);


/*! Destroy a previously created QA Record object

@param record A CSL_QaRecord object previously created with CSL_QaRecordCreate
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaRecordDestroy(CSL_QaRecord* record);


/*! Returns ID of the QA Record

@param record A CSL_QaRecord object previously created with CSL_QaRecordCreate
@return ID of the QA Record
*/
CSL_DEC uint16 CSL_QaRecordGetId(CSL_QaRecord* record);


/*! Write text to a QA Record

@param record A CSL_QaRecord object
@param text The text to be written
@param maximumLength The length of the text field
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaRecordWriteText(CSL_QaRecord* record, const char* text, uint32 fieldLength);


/*! Write a 32 bit Signed integer to a QA Record

@param record A CSL_QaRecord object
@param value The number to be written
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaRecordWriteInt32(CSL_QaRecord* record, int32 value);


/*! Write a 32 bit unsigned integer to a QA Record

@param record A CSL_QaRecord object
@param value The number to be written
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaRecordWriteUint32(CSL_QaRecord* record, uint32 value);


/*! Write a 16 bit unsigned integer to a QA Record

@param record A CSL_QaRecord object
@param value The number to be written
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaRecordWriteUint16(CSL_QaRecord* record, uint16 value);


/*! Write an 8 bit unsigned integer to a QA Record

@param record A CSL_QaRecord object
@param value The number to be written
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaRecordWriteUint8(CSL_QaRecord* record, uint8 value);


/*! Write a double to a QA Record

@param record A CSL_QaRecord object
@param value The number to be written
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaRecordWriteDouble(CSL_QaRecord* record, double value);


/*! Write binary data to a QA Record

@param record A CSL_QaRecord object
@param data The data to be written
@param dataSize The size of the data to be written
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaRecordWriteData(CSL_QaRecord* record, const byte* data, nb_size dataSize);


/*! Write the file location to a QA Record

@param record A CSL_QaRecord object
@param function The name of the function
@param file The name of the source file
@param line The line number
@return NB_Error
*/
CSL_DEC NB_Error CSL_QaRecordWriteFileLocation(CSL_QaRecord* record, const char* function, const char* file, uint32 line);

#endif

/*! @} */

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

@file nbqarecord.c
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

#include "nbqarecord.h"
#include "cslqarecord.h"

NB_DEF NB_Error
NB_QaRecordCreate(nb_gpsTime gpsTime, uint16 id, uint16 size, NB_QaRecord** record)
{
    return CSL_QaRecordCreate(gpsTime, id, size, record);
}

NB_DEF NB_Error
NB_QaRecordDestroy(NB_QaRecord* pThis)
{
    return CSL_QaRecordDestroy(pThis);
}

NB_DEF NB_Error
NB_QaRecordWriteText(NB_QaRecord* pThis, const char* text, uint32 fieldLength)
{

    return CSL_QaRecordWriteText(pThis, text, fieldLength);
}

NB_DEF NB_Error
NB_QaRecordWriteInt32(NB_QaRecord* pThis, int32 value)
{
    return CSL_QaRecordWriteInt32(pThis, value);
}

NB_DEF NB_Error
NB_QaRecordWriteUint32(NB_QaRecord* pThis, uint32 value)
{
    return CSL_QaRecordWriteUint32(pThis, value);
}

NB_DEF NB_Error
NB_QaRecordWriteUint16(NB_QaRecord* pThis, uint16 value)
{
    return CSL_QaRecordWriteUint16(pThis, value);
}

NB_DEF NB_Error
NB_QaRecordWriteUint8(NB_QaRecord* pThis, uint8 value)
{
    return CSL_QaRecordWriteUint8(pThis, value);
}

NB_DEF NB_Error
NB_QaRecordWriteDouble(NB_QaRecord* pThis, double value)
{
    return CSL_QaRecordWriteDouble(pThis, value);
}

NB_DEF NB_Error
NB_QaRecordWriteData(NB_QaRecord* pThis, const byte* data, nb_size dataSize)
{
    return CSL_QaRecordWriteData(pThis, data, dataSize);
}

NB_DEF NB_Error
NB_QaRecordWriteFileLocation(NB_QaRecord* pThis, const char* function, const char* file, uint32 line)
{
    return CSL_QaRecordWriteFileLocation(pThis, function, file, line);
}

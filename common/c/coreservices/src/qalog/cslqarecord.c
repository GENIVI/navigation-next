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

@file cslqarecord.c
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

#include "cslqarecord.h"
#include "cslqarecorddefinitions.h"
#include "dynbuf.h"
#include "cslutil.h"

static void WriteNetworkUint32(CSL_QaRecord* pThis, struct dynbuf* buffer, uint32 value);
static void WriteNetworkUint16(CSL_QaRecord* pThis, struct dynbuf* buffer, uint16 value);


struct CSL_QaRecord
{
    nb_gpsTime      gpsTime;
    uint16          id;
    struct dynbuf   record;
    struct dynbuf   data;
};


CSL_DEF NB_Error
CSL_QaRecordCreate(nb_gpsTime gpsTime, uint16 id, uint16 size, CSL_QaRecord** record)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* pThis = 0;

    if (!record)
    {
        return NE_INVAL;
    }
    *record = 0;

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    err = dbufnew(&pThis->record, 8);
    if (err)
    {
        nsl_free(pThis);
        return err;
    }

    err = dbufnew(&pThis->data, size);
    if (!err)
    {
        pThis->gpsTime = gpsTime;
        pThis->id = id;

        *record = pThis;
    }
    else
    {
        dbufdel(&pThis->record);
        nsl_free(pThis);
    }

    return err;
}

CSL_DEF NB_Error
CSL_QaRecordDestroy(CSL_QaRecord* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    dbufdel(&pThis->record);
    dbufdel(&pThis->data);
    nsl_free(pThis);

    return NE_OK;
}

CSL_DEF NB_Error
CSL_QaRecordWriteText(CSL_QaRecord* pThis, const char* text, uint32 fieldLength)
{
    char* buffer = 0;
    if (!pThis || !text)
    {
        return NE_INVAL;
    }

    buffer = nsl_malloc(fieldLength);
    if (!buffer)
    {
        return NE_NOMEM;
    }

    nsl_memset(buffer, 0, fieldLength);
    nim_strncpyt(buffer, text, fieldLength - 1);
    dbufcat(&pThis->data, (const byte*)buffer, fieldLength);
    nsl_free(buffer);

    return NE_OK;
}

CSL_DEF NB_Error
CSL_QaRecordWriteInt32(CSL_QaRecord* pThis, int32 value)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    WriteNetworkUint32(pThis, &pThis->data, (uint32) value);
    return NE_OK;
}
CSL_DEF NB_Error
CSL_QaRecordWriteUint32(CSL_QaRecord* pThis, uint32 value)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    WriteNetworkUint32(pThis, &pThis->data, value);
    return NE_OK;
}

CSL_DEF NB_Error
CSL_QaRecordWriteUint16(CSL_QaRecord* pThis, uint16 value)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    WriteNetworkUint16(pThis, &pThis->data, value);
    return NE_OK;
}

CSL_DEF NB_Error
CSL_QaRecordWriteUint8(CSL_QaRecord* pThis, uint8 value)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    dbufcat(&pThis->data, (const byte*)&value, sizeof(value));
    return NE_OK;
}

CSL_DEF NB_Error
CSL_QaRecordWriteDouble(CSL_QaRecord* pThis, double value)
{
    char buffer[sizeof(double)] = { 0 };
    char* src = (char*)&value;
    char* p = src + sizeof(double);
    char* dest = buffer;

    while (p > src)
    {
        *dest++ = *--p;
    }

    dbufcat(&pThis->data, (const byte*)buffer, sizeof(double));
    return NE_OK;
}


CSL_DEF NB_Error
CSL_QaRecordWriteData(CSL_QaRecord* pThis, const byte* data, nb_size dataSize)
{
    dbufcat(&pThis->data, data, dataSize);
    return NE_OK;
}


CSL_DEF NB_Error
CSL_QaRecordWriteFileLocation(CSL_QaRecord* pThis, const char* function, const char* file, uint32 line)
{
    NB_Error err = CSL_QaRecordWriteText(pThis, function, MAX_FILE_NAME_LEN);
    err = err ? err : CSL_QaRecordWriteText(pThis, file, MAX_FILE_NAME_LEN);
    err = err ? err : CSL_QaRecordWriteUint32(pThis, line);

    return err;
}


CSL_DEF uint16
CSL_QaRecordGetId(CSL_QaRecord* pThis)
{
    if (!pThis)
    {
        return RECORD_ID_NONE;
    }

    return pThis->id;
}


NB_Error
CSL_QaRecordGetData(CSL_QaRecord* pThis, const uint8** data, uint32* dataSize)
{
    if (!pThis || !data || !dataSize)
    {
        return NE_INVAL;
    }

    if (dbuflen(&pThis->record) == 0)
    {
        WriteNetworkUint32(pThis, &pThis->record, pThis->gpsTime);
        WriteNetworkUint16(pThis, &pThis->record, pThis->id);
        WriteNetworkUint16(pThis, &pThis->record, (uint16)dbuflen(&pThis->data));
        dbufcat(&pThis->record, dbufget(&pThis->data), dbuflen(&pThis->data));
    }

    *data = dbufget(&pThis->record);
    *dataSize = (uint32)dbuflen(&pThis->record);

    return NE_OK;
}

void
WriteNetworkUint32(CSL_QaRecord* pThis, struct dynbuf* buffer, uint32 value)
{
    uint32 network = nsl_htonl(value);
    dbufcat(buffer, (const byte*)&network, sizeof(network));
}

void
WriteNetworkUint16(CSL_QaRecord* pThis, struct dynbuf* buffer, uint16 value)
{
    uint16 network = nsl_htons(value);
    dbufcat(buffer, (const byte*)&network, sizeof(network));
}

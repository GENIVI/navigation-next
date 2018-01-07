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

@file cslqalog.c
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

#include "cslqalog.h"
#include "cslqarecorddefinitions.h"
#include "cslqarecordinternal.h"
#include "fileutil.h"
#include "palstdlib.h"
#include "cslutil.h"
#include "palclock.h"
#include "vec.h"
#include "paltimer.h"

static const byte qakey[] = {
    152, 123, 70, 221, 139, 152, 40, 6, 
    26, 154, 54, 227, 238, 207, 142, 52, 
    71, 227, 138, 31, 12, 160, 113, 214, 
    68, 215, 170, 210, 231, 210, 126, 193, 
    183, 189, 56, 157, 146, 121, 3, 122, 
    155, 60, 248, 220, 233, 106, 121, 141, 
    242, 248, 29, 205, 98, 51, 66, 209, 
    160, 175, 88, 83, 163, 64, 242, 121
};

#define QAKEY       qakey
#define QAKEYSIZE   sizeof(qakey)

/* Macroses for the CSL_QaLogExtension::filter
  FILTER - array of at least 8192 chars
  ID - 16-bit integer, with log ID
  Note:
    8192 bytes * 8 bits = 65536 bit flags to cover all possible QA log 16-bit IDs
    if the bit is 1 then the proper ID is enabled, otherwise it is disabled
*/
#define CSL_QALOG_FILTER_SIZE 8192
#define CSL_QA_LOG_FILTER_SET_ENABLED(FILTER, ID)  ((FILTER)[(ID) / 8]) |= (1 << ((ID) % 8))
#define CSL_QA_LOG_FILTER_SET_DISABLED(FILTER, ID) ((FILTER)[(ID) / 8]) &= ~(1 << ((ID) % 8))
#define CSL_QA_LOG_IS_ENABLED(FILTER, ID) (((FILTER)[(ID) / 8]) & (1 << ((ID) % 8)))

#define INITIAL_KPI_FILTER_DICTIONARY_SIZE 16
#define KPI_INITIAL_MAX_IID                8
#define IDLE_TIMER_INTERVAL                30000

/*
   Simple stack to store KPI interval ID values.
   Zero values are not allowed.
*/
typedef struct
{
    uint32*  data;    // buffer for IID values
    uint32   maxSize; // size of the 'data' buffer
    uint32   size;    // number of elements in the stack
} KpiIidStack;

/*
   Stores all necessary information for KPI IID (interval ID) feature

   All available IID are stored in the availableIidStack.
   CSL_QaLogKpiIidAllocate() function extracts available id from availableIidStack and returns it to user.
   If availableIidStack is empty, then it increases maxIid and returns it to user.
   When user calls CSL_QaLogKpiIidRelease(), the id returned to availableIidStack and can be reused.
*/
typedef struct
{
    KpiIidStack*  availableIidStack;
    uint32        maxIid;
} KpiIid;


/*
   This extension expands QaLog with filter and with KPI feature for NBI
*/
struct CSL_QaLogExtension
{
    /* if NULL - all logs are enabled (default)
        otherwise it contains an array with length of 8192 bytes (65536 bits)
        each bit holds the state of the corresponding log with ID (16-bit)
        use QA_LOG_IS_ENABLED macros to determine whether the log is enabled or not
    */
    unsigned char*  filter;

    /* contains disabled (if kpiFilterLogsAreEnabledByDefault is TRUE)
        or enabled (if kpiFilterLogsAreEnabledByDefault is FALSE) KPI labels
    */
    CSL_Vector*     kpiFilter;

    /* if TRUE then all KPI logs are enabled, except labels in kpiFilter,
       if FALSE then all KPI logs are disabled, except labels in kpiFilter
    */
    nb_boolean      kpiFilterLogsAreEnabledByDefault;

    /* used by NB_QaLogKpiIntervalBegin() for generating an unique IID
       'IID' means interval ID
    */
    KpiIid*         kpiIid;
};

struct SessionHeader
{
    uint32  qaFileVersion;
    char    productName[FILE_HEADER_PRODUCT_NAME_LEN];
    uint32  productVersion;
    uint32  platformId;
    char    mdn[FILE_HEADER_MDN_LEN];
    char    min[FILE_HEADER_MIN_LEN];
};

struct CSL_QaLog
{
    PAL_Instance*           pal;
    char*                   filename;
    boolean                 verbose;
    boolean                 sessionHeaderWritten;
    struct SessionHeader    sessionHeader;
    int                     ignoredRecordCount;
    struct CSL_QaLogExtension extension;
    PAL_File*               fileHandle; /*! PAL_File instance, created for CSL_QaLog.filename  */
};


static CSL_QaRecord* GetSessionHeader(CSL_QaLog* pThis);
static NB_Error PrepareLogFile(CSL_QaLog* pThis);
static void QaLogIdleTimerCallback(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason);
static NB_Error WriteRecord(CSL_QaLog* pThis, CSL_QaRecord* record);
static NB_Error WriteData(CSL_QaLog* pThis, const uint8* data, uint32 dataSize);
static void InitializeSessionHeader(struct SessionHeader* header, CSL_QaLogHeaderData* data);
static uint32 MakeQaLogVersion(nb_version* version);

static nb_boolean KpiFilterContains(CSL_Vector* kpiFilter, const char* key);
static NB_Error AddToKpiFilter(CSL_Vector* kpiFilter, const char* key);
static void RemoveFromKpiFilter(CSL_Vector* kpiFilter, const char* key);
static void RemoveAllFromKpiFilter(CSL_Vector* kpiFilter);
static KpiIid* KpiIidCreate(uint32 maxIid);
static void KpiIidDestroy(KpiIid* iid);
static KpiIidStack* KpiIidStackCreate(uint32 maxSize);
static void KpiIidStackDestroy(KpiIidStack* stack);
static NB_Error KpiIidStackPush(KpiIidStack* stack, uint32 value);
static uint32 KpiIidStackPop(KpiIidStack* stack);


CSL_DEF NB_Error
CSL_QaLogCreate(PAL_Instance* pal, CSL_QaLogHeaderData* headerData, const char* filename, nb_boolean verbose, CSL_QaLog** log)
{
    NB_Error err = NE_OK;
    CSL_QaLog* pThis = 0;

    if (!pal || !headerData || !filename || !log || (nsl_strlen(filename) == 0))
    {
        return NE_INVAL;
    }
    *log = 0;

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->filename = nsl_strdup(filename);
    if (!pThis->filename)
    {
        nsl_free(pThis);
        return NE_NOMEM;
    }
    pThis->pal = pal;
    pThis->verbose = verbose;
    pThis->sessionHeaderWritten = FALSE;

    InitializeSessionHeader(&pThis->sessionHeader, headerData);

    pThis->extension.kpiFilter = CSL_VectorAlloc(sizeof(char*));
    if (!pThis->extension.kpiFilter)
    {
        CSL_QaLogDestroy(pThis);
        return NE_NOMEM;
    }

    pThis->extension.kpiIid = KpiIidCreate(KPI_INITIAL_MAX_IID);
    if (!pThis->extension.kpiIid)
    {
        CSL_QaLogDestroy(pThis);
        return NE_NOMEM;
    }

    pThis->extension.kpiFilterLogsAreEnabledByDefault = TRUE;

    *log = pThis;

    return err;
}

CSL_DEF NB_Error
CSL_QaLogDestroy(CSL_QaLog* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    PAL_TimerCancel(pThis->pal, QaLogIdleTimerCallback, pThis);
    if (pThis->fileHandle)
    {
        PAL_FileClose(pThis->fileHandle);
        pThis->fileHandle = NULL;
    }

    nsl_free(pThis->extension.filter);

    if (pThis->extension.kpiFilter)
    {
        RemoveAllFromKpiFilter(pThis->extension.kpiFilter);
        CSL_VectorDealloc(pThis->extension.kpiFilter);
    }

    KpiIidDestroy(pThis->extension.kpiIid);

    nsl_free(pThis->filename);
    nsl_free(pThis);

    return NE_OK;
}

CSL_DEF NB_Error
CSL_QaLogFilterSet(CSL_QaLog* pThis, uint16 id, nb_boolean enable)
{
    NB_Error result = NE_OK;

    if (!pThis)
    {
        return NE_INVAL;
    }

    if (!pThis->extension.filter)
    {
        pThis->extension.filter = (unsigned char*)nsl_malloc(CSL_QALOG_FILTER_SIZE);
        if (!pThis->extension.filter)
        {
            return NE_NOMEM;
        }
        nsl_memset(pThis->extension.filter, 0, CSL_QALOG_FILTER_SIZE);
    }

    if (enable)
    {
        CSL_QA_LOG_FILTER_SET_ENABLED(pThis->extension.filter, id);
    }
    else
    {
        CSL_QA_LOG_FILTER_SET_DISABLED(pThis->extension.filter, id);
    }

    return result;
}

CSL_DEF NB_Error
CSL_QaLogFilterSetAll(CSL_QaLog* pThis, nb_boolean enable)
{
    NB_Error result = NE_OK;

    if (!pThis)
    {
        return NE_INVAL;
    }

    if (!pThis->extension.filter)
    {
        pThis->extension.filter = (unsigned char*)nsl_malloc(CSL_QALOG_FILTER_SIZE);
        if (!pThis->extension.filter)
        {
            return NE_NOMEM;
        }
    }

    nsl_memset(pThis->extension.filter, enable ? 0xFF : 0, CSL_QALOG_FILTER_SIZE);

    return result;
}

static nb_boolean KpiFilterContains(CSL_Vector* kpiFilter, const char* key)
{
    int i = 0, len = 0;
    char** item = NULL;

    len = CSL_VectorGetLength(kpiFilter);

    for (i = 0; i < len; i++)
    {
        item = (char**)CSL_VectorGetPointer(kpiFilter, i);
        if (nsl_strcmp(*item, key) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static NB_Error AddToKpiFilter(CSL_Vector* kpiFilter, const char* key)
{
    char* item = NULL;

    if (nsl_strlen(key) == 0 || KpiFilterContains(kpiFilter, key))
    {
        return NE_OK;
    }

    item = nsl_strdup(key);
    if (!item)
    {
        return NE_NOMEM;
    }

    if (!CSL_VectorAppend(kpiFilter, &item))
    {
        nsl_free(item);
        return NE_NOMEM;
    }

    return NE_OK;
}

static void RemoveFromKpiFilter(CSL_Vector* kpiFilter, const char* key)
{
    int i = 0, len = 0;
    char** item = NULL;

    if (nsl_strlen(key) == 0)
    {
        return;
    }

    len = CSL_VectorGetLength(kpiFilter);

    for (i = 0; i < len; i++)
    {
        item = (char**)CSL_VectorGetPointer(kpiFilter, i);
        if (nsl_strcmp(*item, key) == 0)
        {
            nsl_free(*item);
            CSL_VectorRemove(kpiFilter, i);
            break;
        }
    }
}

static void RemoveAllFromKpiFilter(CSL_Vector* kpiFilter)
{
    int i = 0, len = 0;
    char** item = NULL;

    len = CSL_VectorGetLength(kpiFilter);

    for (i = 0; i < len; i++)
    {
        item = (char**)CSL_VectorGetPointer(kpiFilter, i);
        nsl_free(*item);
    }

    CSL_VectorRemoveAll(kpiFilter);
}


CSL_DEF NB_Error
CSL_QaLogKpiFreeformEventFilterSet(CSL_QaLog* pThis,
        const char* eventLabel,
        nb_boolean enable)
{
    if (!pThis || !eventLabel || !pThis->extension.kpiFilter)
    {
        return NE_INVAL;
    }

    if (pThis->extension.kpiFilterLogsAreEnabledByDefault)
    {
        if (enable)
        {
            RemoveFromKpiFilter(pThis->extension.kpiFilter, eventLabel);
        }
        else
        {
            return AddToKpiFilter(pThis->extension.kpiFilter, eventLabel);
        }
    }
    else
    {
        if (enable)
        {
            return AddToKpiFilter(pThis->extension.kpiFilter, eventLabel);
        }
        else
        {
            RemoveFromKpiFilter(pThis->extension.kpiFilter, eventLabel);
        }
    }

    return NE_OK;
}


CSL_DEF void
CSL_QaLogKpiFreeformEventFilterSetAll(CSL_QaLog* pThis,
        nb_boolean enable)
{
    if (!pThis || !pThis->extension.kpiFilter)
    {
        return;
    }

    pThis->extension.kpiFilterLogsAreEnabledByDefault = enable;

    RemoveAllFromKpiFilter(pThis->extension.kpiFilter);
}


CSL_DEF nb_boolean
CSL_QaLogKpiFreeformEventFilterIsDisabled(CSL_QaLog* pThis,
        const char* eventLabel)
{
    nb_boolean filtered = FALSE;

    if (!pThis || !eventLabel || !pThis->extension.kpiFilter)
    {
        return FALSE;
    }

    filtered = KpiFilterContains(pThis->extension.kpiFilter, eventLabel);

    return pThis->extension.kpiFilterLogsAreEnabledByDefault ? filtered : !filtered;
}

static KpiIidStack*
KpiIidStackCreate(uint32 maxSize)
{
    KpiIidStack* stack = nsl_malloc(sizeof(KpiIidStack));

    if (!stack)
    {
        return NULL;
    }

    stack->data = (uint32*)nsl_malloc(maxSize * sizeof(uint32));
    if (!stack->data)
    {
        nsl_free(stack);
        return NULL;
    }
    stack->maxSize = maxSize;
    stack->size = 0;

    return stack;
}

static void
KpiIidStackDestroy(KpiIidStack* stack)
{
    if (stack)
    {
        nsl_free(stack->data);
        nsl_free(stack);
    }
}

static NB_Error
KpiIidStackPush(KpiIidStack* stack, uint32 value)
{
    uint32 i = 0;

    if (value == 0)
    {
        return NE_INVAL;
    }

    if (stack->size)
    {
        for (i = 0; i < stack->size; i++)
        {
            if (value == stack->data[i])
            {
                return NE_OK; // id is in the stack already, just exit
            }
        }
    }

    if (stack->size == stack->maxSize)
    {
        uint32* newData = NULL;
        stack->maxSize *= 2;
        newData = (uint32*)nsl_realloc(stack->data, stack->maxSize * sizeof(uint32));
        if (!newData)
        {
            return NE_NOMEM;
        }
        stack->data = newData;
    }

    stack->data[stack->size++] = value;

    return NE_OK;
}

static uint32
KpiIidStackPop(KpiIidStack* stack)
{
    if (stack->size == 0)
    {
        return 0;
    }

    stack->size--;

    return stack->data[stack->size];
}

static KpiIid*
KpiIidCreate(uint32 maxIid)
{
    uint32 i = 0;
    KpiIid* iid = (KpiIid*)nsl_malloc(sizeof(KpiIid));

    if (!iid)
    {
        return NULL;
    }

    iid->availableIidStack = KpiIidStackCreate(maxIid * 2);
    if (!iid->availableIidStack)
    {
        nsl_free(iid);
        return NULL;
    }

    iid->maxIid = maxIid;

    for (i = 0; i < maxIid; i++)
    {
        if (KpiIidStackPush(iid->availableIidStack, maxIid - i) != NE_OK)
        {
            KpiIidDestroy(iid);
            return NULL;
        }
    }

    return iid;
}

static void
KpiIidDestroy(KpiIid* iid)
{
    if (iid)
    {
        KpiIidStackDestroy(iid->availableIidStack);
        nsl_free(iid);
    }
}

CSL_DEF uint32
CSL_QaLogKpiIidAllocate(CSL_QaLog* pThis)
{
    uint32 id = 0;

    if (!pThis)
    {
        return 0;
    }

    id = KpiIidStackPop(pThis->extension.kpiIid->availableIidStack);
    if (id == 0)
    {
        pThis->extension.kpiIid->maxIid++;
        id = pThis->extension.kpiIid->maxIid;
    }

    return id;
}

CSL_DEF void
CSL_QaLogKpiIidRelease(CSL_QaLog* pThis, uint32 id)
{
    if (!pThis || id == 0 || id > pThis->extension.kpiIid->maxIid)
    {
        return;
    }

    KpiIidStackPush(pThis->extension.kpiIid->availableIidStack, id);
}


CSL_DEF nb_boolean
CSL_QaLogIsVerboseLoggingEnabled(CSL_QaLog* pThis)
{
    return (nb_boolean)(pThis && pThis->verbose);
}


CSL_DEF NB_Error
CSL_QaLogWrite(CSL_QaLog* pThis, CSL_QaRecord* record)
{
    NB_Error err = NE_OK;

    if (!pThis || !record)
    {
        return NE_INVAL;
    }

    if (pThis->extension.filter && !CSL_QA_LOG_IS_ENABLED(pThis->extension.filter, CSL_QaRecordGetId(record)))
    {
        return NE_OK;  // log is not enabled, just exit
    }

    if (!PAL_IsPalThread(pThis->pal))
    {
        nsl_assert(FALSE);
        pThis->ignoredRecordCount++;
        return NE_UNEXPECTED;
    }

    if (!pThis->sessionHeaderWritten)
    {
        CSL_QaRecord* header = GetSessionHeader(pThis);
        if (header)
        {
            err = WriteRecord(pThis, header);

            CSL_QaRecordDestroy(header);
        }
        else
        {
            err = NE_NOMEM;
        }

        pThis->sessionHeaderWritten = TRUE;
    }

    err = err ? err : WriteRecord(pThis, record);

    if (!err && pThis->ignoredRecordCount > 0)
    {
        char buf[50] = { 0 };
        nsl_sprintf(buf, "QALOG: ignored %d records", pThis->ignoredRecordCount);
        pThis->ignoredRecordCount = 0;
        CSL_QaLogAppState(pThis, buf);
    }

    return err;
}

CSL_DEF NB_Error
CSL_QaLogWriteData(CSL_QaLog* pThis, const uint8* data, uint32 dataSize)
{
    NB_Error err = NE_OK;

    if (!pThis || !data || dataSize == 0)
    {
        return NE_INVAL;
    }

    if (!PAL_IsPalThread(pThis->pal))
    {
        nsl_assert(FALSE);
        pThis->ignoredRecordCount++;
        return NE_UNEXPECTED;
    }

    err = err ? err : WriteData(pThis, data, dataSize);

    if (!err && pThis->ignoredRecordCount > 0)
    {
        char buf[50] = { 0 };
        nsl_sprintf(buf, "QALOG: ignored %d records", pThis->ignoredRecordCount);
        pThis->ignoredRecordCount = 0;
        CSL_QaLogAppState(pThis, buf);
    }

    return err;
}


CSL_QaRecord*
GetSessionHeader(CSL_QaLog* pThis)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    err = CSL_QaRecordCreate(gpstime(), RECORD_ID_FILE_HEADER, 100, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteUint32(record, pThis->sessionHeader.qaFileVersion);
        err = err ? err : CSL_QaRecordWriteText(record, pThis->sessionHeader.productName, FILE_HEADER_PRODUCT_NAME_LEN);
        err = err ? err : CSL_QaRecordWriteUint32(record, pThis->sessionHeader.productVersion);
        err = err ? err : CSL_QaRecordWriteUint32(record, pThis->sessionHeader.platformId);
        err = err ? err : CSL_QaRecordWriteText(record, pThis->sessionHeader.mdn, FILE_HEADER_MDN_LEN);
        err = err ? err : CSL_QaRecordWriteText(record, pThis->sessionHeader.min, FILE_HEADER_MIN_LEN);
    }

    if (err)
    {
        CSL_QaRecordDestroy(record);
        record = 0;
    }

    return record;
}

NB_Error PrepareLogFile(CSL_QaLog* pThis)
{
    NB_Error   error  = NE_OK;
    do
    {
        PAL_Error  result = PAL_FileOpen(pThis->pal, pThis->filename, PFM_Append, &pThis->fileHandle);
        if (result == PAL_ErrFileNotExist)
        {
            result = PAL_FileOpen(pThis->pal, pThis->filename, PFM_Create, &pThis->fileHandle);
        }

        if (result != PAL_Ok)
        {
            error = NE_FSYS_OPEN_ERROR;
            break;
        }

        result = PAL_TimerSet(pThis->pal, IDLE_TIMER_INTERVAL, QaLogIdleTimerCallback, pThis);
        if (result != PAL_Ok)
        {
            error = NE_UNEXPECTED;
        }

    } while (0);

    if (error != NE_OK && pThis->fileHandle)
    {
        PAL_FileClose(pThis->fileHandle);
        pThis->fileHandle = NULL;
    }
    return error;
}

/*! Callback to close PAL_File instance when qalog is idle. */
void QaLogIdleTimerCallback(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason)
{
    CSL_QaLog* pThis = (CSL_QaLog*)userData;
    if (!pal || !userData || reason != PTCBR_TimerFired || !pThis->fileHandle)
    {
        return;
    }

    PAL_FileClose(pThis->fileHandle);
    pThis->fileHandle = NULL;
}

NB_Error
WriteRecord(CSL_QaLog* pThis, CSL_QaRecord* record)
{
    NB_Error err = NE_OK;
    const uint8* data = 0;
    uint32 dataSize = 0;

    err = err ? err : CSL_QaRecordGetData(record, &data, &dataSize);
    if (!err)
    {
        if (!pThis->fileHandle)
        {
            err = PrepareLogFile(pThis);
        }

        if (err == NE_OK)
        {
            if (!appendpalfile_obfuscate(pThis->pal, pThis->fileHandle, (uint8*)data,
                                        dataSize, QAKEY, QAKEYSIZE))
            {
                err = NE_FSYS;
            }
        }
    }

    return err;
}

NB_Error
WriteData(CSL_QaLog* pThis, const uint8* data, uint32 dataSize)
{
    NB_Error err = NE_OK;

    if (!pThis->fileHandle)
    {
        err = PrepareLogFile(pThis);
    }

    if (err == NE_OK)
    {
        if (!appendpalfile_obfuscate(pThis->pal, pThis->fileHandle, (uint8*)data,
                                     dataSize, QAKEY, QAKEYSIZE))
        {
            err = NE_FSYS;
        }
    }

    return err;
}

void InitializeSessionHeader(struct SessionHeader* header, CSL_QaLogHeaderData* data)
{
    header->qaFileVersion = 0;        /// @todo What is the right QA file version number?
    nsl_strncpy(header->productName, data->productName, FILE_HEADER_PRODUCT_NAME_LEN);
    header->productName[FILE_HEADER_PRODUCT_NAME_LEN - 1] = '\0';
    header->productVersion = MakeQaLogVersion(&data->productVersion);
    header->platformId = data->platformId;
    nsl_uint64tostr(data->mobileDirectoryNumber, header->mdn, FILE_HEADER_MDN_LEN);
    nsl_uint64tostr(data->mobileInformationNumber, header->min, FILE_HEADER_MIN_LEN);
}


uint32 MakeQaLogVersion(nb_version* version)
{
    return (uint32)((1 << 31) |
                    ((version->major & 0x7F) << 24) |
                    ((version->minor & 0xFF) << 16) |
                    ((version->feature & 0x0F) << 12) |
                    ((version->build & 0xFFF)));
}

CSL_DEF NB_Error 
CSL_QaLogAppState(CSL_QaLog* pThis, const char* state)
{
    NB_Error err = NE_OK;

    CSL_QaRecord* record = 0;

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_APP_STATE, 250, &record);

    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, state, APP_STATE_LEN);

        err = err ? err : CSL_QaLogWrite(pThis, record);

        CSL_QaRecordDestroy(record);
    }

    return err;
}

CSL_DEF void
CSL_QaLogFlush(CSL_QaLog* pThis)
{
    if (pThis && pThis->fileHandle)
    {
        PAL_TimerCancel(pThis->pal, QaLogIdleTimerCallback, pThis);
        PAL_FileClose(pThis->fileHandle);
        pThis->fileHandle = NULL;
    }
}
struct RecordDataHeader
{
    uint32 gpsTime;
    uint16 id;
    uint16 length;
};

struct RecordDataInfo
{
    struct RecordDataHeader header;
    uint32 pos;
};
#define QALOG_BUFFER_SIZE 512

CSL_DEF void
CSL_QaLogClear(CSL_QaLog* pThis, uint32 retainSessionCount)
{
    NB_Error err = NE_OK;
    CSL_Vector* records;
    uint32 pos = 0;
    struct RecordDataInfo info;
    struct RecordDataInfo* pRecord = NULL;
    uint32 sessionCount;
    uint32 oldPos;
    uint32 newPos;
    uint8 buf[QALOG_BUFFER_SIZE];
    int32 readCount = 0;
    uint32 writeCount = 0;
    if(!pThis)
    {
        return;
    }
    if (!pThis->fileHandle)
    {
        err = PrepareLogFile(pThis);
    }
    records = CSL_VectorAlloc(sizeof(info));
    if(pThis->fileHandle)
    {
        if(err == NE_OK)
        {
            PAL_FileSetPosition(pThis->fileHandle, PFSO_Start, 0);
            //find totle session count
            while(read_obfuscate(pThis->fileHandle, &info.header, sizeof(info.header),QAKEY, QAKEYSIZE) != 0)
            {
                info.header.gpsTime = nsl_ntohl(info.header.gpsTime);
                info.header.id = nsl_ntohs(info.header.id);
                info.header.length = nsl_ntohs(info.header.length);
                PAL_FileGetPosition(pThis->fileHandle, &pos);
                info.pos = pos - sizeof(info.header);
                if(info.header.id == RECORD_ID_FILE_HEADER)
                {
                    CSL_VectorAppend(records, &info);
                }
                PAL_FileSetPosition(pThis->fileHandle, PFSO_Current, info.header.length);
            }
        }
        sessionCount = CSL_VectorGetLength(records);
        if(retainSessionCount < sessionCount)
        {
            PAL_FileClose(pThis->fileHandle);
            PAL_FileOpen(pThis->pal, pThis->filename, PFM_ReadWrite, &pThis->fileHandle);
            pRecord = (struct RecordDataInfo*)CSL_VectorGetPointer(records, sessionCount - retainSessionCount);
            oldPos = pRecord->pos;
            newPos = 0;
            PAL_FileSetPosition(pThis->fileHandle, PFSO_Start, oldPos);
            do
            {
                readCount = read_obfuscate(pThis->fileHandle, buf, QALOG_BUFFER_SIZE,QAKEY, QAKEYSIZE);
                PAL_FileSetPosition(pThis->fileHandle, PFSO_Start, newPos);
                write_obfuscate(pThis->fileHandle, buf, readCount, QAKEY, QAKEYSIZE, &writeCount);
                oldPos += readCount;
                newPos += readCount;
                PAL_FileSetPosition(pThis->fileHandle, PFSO_Start, oldPos);
            }
            while(readCount == QALOG_BUFFER_SIZE);
            PAL_FileTruncate(pThis->fileHandle, newPos);
            PAL_FileClose(pThis->fileHandle);
            pThis->fileHandle = NULL;
        }
    }
    CSL_VectorRemoveAll(records);
    CSL_VectorDealloc(records);
}

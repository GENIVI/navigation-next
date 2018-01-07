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

    @file     nbanalyticseventsdb.c
    @defgroup nbanalyticseventsdb
*/
/*
    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "palstdlib.h"
#include "fileutil.h"
#include "nbcontext.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "nbanalytics.h"
#include "nbanalyticsprotected.h"
#include "nbanalyticseventstailq.h"
#include "nbanalyticseventsdb.h"

/*! @{ */

#define EVENTS_DB_FILE_NAME "ae.db"
#define REMOVED_EVENTS_DB_FILE_NAME "ae.idx"
#define ANALYTICS_FILENAME_MAX 256

#define ANALYTICS_EVENT_DBUF_SIZE 1024

struct NB_AnalyticsEventsDb
{
    NB_Analytics* analytics;

    char* eventsDbFileName;
};

static const byte analytics_key[] = { /* same as qakey */
    152, 123, 70, 221, 139, 152, 40, 6,
    26, 154, 54, 227, 238, 207, 142, 52,
    71, 227, 138, 31, 12, 160, 113, 214,
    68, 215, 170, 210, 231, 210, 126, 193,
    183, 189, 56, 157, 146, 121, 3, 122,
    155, 60, 248, 220, 233, 106, 121, 141,
    242, 248, 29, 205, 98, 51, 66, 209,
    160, 175, 88, 83, 163, 64, 242, 121
};

#define ANALYTICS_KEY analytics_key
#define ANALYTICS_KEY_SIZE sizeof(analytics_key)

static NB_Error NB_AnalyticsAddEvent(NB_Analytics* pThis,
                                     data_analytics_event* event,
                                     NB_AnalyticsEventsCategory category)
{
    return NE_NOSUPPORT;
}

NB_AnalyticsEventsDb*
NB_AnalyticsEventsDbOpen(NB_Analytics* analytics, const char* dir)
{
    NB_AnalyticsEventsDb* db = (NB_AnalyticsEventsDb*) nsl_malloc(sizeof(*db));
    if (db == NULL)
    {
        return NULL;
    }

    nsl_memset(db, 0, sizeof(*db));

    db->analytics = analytics;

    if ((dir == NULL) || (*dir == '\0')) /* empty string */
    {
        db->eventsDbFileName = nsl_strdup(EVENTS_DB_FILE_NAME);
    }
    else
    {
        uint32 length = (uint32) (nsl_strlen(dir) + 1 + nsl_strlen(EVENTS_DB_FILE_NAME) + 1);
        db->eventsDbFileName = nsl_malloc(length);
        if (db->eventsDbFileName)
        {
            nsl_strlcpy(db->eventsDbFileName, dir, length);
            PAL_FileAppendPath(NB_ContextGetPal(NB_AnalyticsGetContext(analytics)), db->eventsDbFileName, length, EVENTS_DB_FILE_NAME);
        }
    }

    if (db->eventsDbFileName == NULL)
    {
        NB_AnalyticsEventsDbClose(db);
        return NULL;
    }

    return db;
}

void
NB_AnalyticsEventsDbClose(NB_AnalyticsEventsDb* db)
{
    if (db)
    {
        if (db->eventsDbFileName)
        {
            nsl_free(db->eventsDbFileName);
        }

        nsl_free(db);
    }
}

NB_Error
NB_AnalyticsEventsDbRestoreEvents(NB_AnalyticsEventsDb* pThis,
        NB_AnalyticsEventsTailQ*     eventsTailQs,
        uint32 tailQCount)
{
    NB_Error err = NE_OK;
    NB_Context* context = NB_AnalyticsGetContext(pThis->analytics);
    data_util_state* pds = NB_ContextGetDataState(context);
    PAL_Instance* pal = NB_ContextGetPal(context);
    byte* pData = NULL;
    byte* pDataTemp = NULL;
    nb_size size = 0;
    NB_AnalyticsEventsCategory eventCategory;

    size = readfile_obfuscate(pal, pThis->eventsDbFileName, (void**)&pData,
                ANALYTICS_KEY, ANALYTICS_KEY_SIZE);
    if (size <= 0)
    {
        return NE_OK;
    }

    pDataTemp = pData;

    while (size > 0)
    {
        data_analytics_event* pEvent;
        size_t tSize = (size_t)size;

        /*  create event */
        pEvent = (data_analytics_event*) nsl_malloc(sizeof(*pEvent));
        if (pEvent == NULL)
        {
            err = NE_NOMEM;
            break;
        }

        err = data_analytics_event_init(pds, pEvent);
        if (err)
        {
            nsl_free(pEvent);
            break;
        }

        err = data_analytics_event_from_binary(pds, pEvent, &pDataTemp, &tSize);
        if (err)
        {
            data_analytics_event_free(pds, pEvent);
            nsl_free(pEvent);
            break;
        }

        /*  add entry to taiq */
        eventCategory = NB_AnalyticsGetEventCategory(pThis->analytics, pEvent);
        err = NB_AnalyticsAddEvent(pThis->analytics, pEvent, eventCategory);

        if (err)
        {
            /*  add event failed */
            data_analytics_event_free(pds, pEvent);
            nsl_free(pEvent);
            break;
        }
        else
        {
            /* log event status */
            NB_AnalyticsLogEventStatus(pThis->analytics, pEvent,
                    NB_AES_Restored);
        }
    }

    nsl_free(pData);
    return err;
}

NB_Error
NB_AnalyticsEventsDbWriteEvent(NB_AnalyticsEventsDb* pThis,
        data_analytics_event* pEvent)
{
    NB_Error err = NE_OK;
    NB_Context* context = NB_AnalyticsGetContext(pThis->analytics);
    data_util_state* pds = NB_ContextGetDataState(context);
    PAL_Instance* pal = NB_ContextGetPal(context);
    struct dynbuf dbuf;

    err = dbufnew(&dbuf, ANALYTICS_EVENT_DBUF_SIZE);
    if (err)
    {
        return err;
    }

    data_analytics_event_to_buf(pds, pEvent, &dbuf);
    err = dbuferr(&dbuf);
    if (!err)
    {
        const byte* data = dbufget(&dbuf);
        uint32 dataSize = (uint32)dbuflen(&dbuf);
        if (!appendfile_obfuscate(pal, pThis->eventsDbFileName, (uint8*)data,
                    dataSize, ANALYTICS_KEY, ANALYTICS_KEY_SIZE))
        {
            err = NE_FSYS;
        }
        else
        {
            /* log event status */
            NB_AnalyticsLogEventStatus(pThis->analytics, pEvent,
                    NB_AES_Saved);
        }
    }

    dbufdel(&dbuf);
    return err;
}

NB_Error
NB_AnalyticsEventsDbRemoveSavedEvents(NB_AnalyticsEventsDb* pThis)
{
    NB_Context* context = NB_AnalyticsGetContext(pThis->analytics);
    PAL_Instance* pal = NB_ContextGetPal(context);
    NB_Error err = NE_OK;

    /* remove all events first */
    if (!removefile(pal, pThis->eventsDbFileName))
    {
        err = NE_FSYS;
        return err;
    }

    return err;
}

NB_Error
NB_AnalyticsEventsDbSaveEvents(NB_AnalyticsEventsDb* pThis,
        NB_AnalyticsEventsTailQ*     eventsTailQs,
        uint32 tailQCount)
{
    NB_Error err = NE_OK;
    uint32 i;

    for (i=0; i < tailQCount; i++)
    {
        NB_AnalyticsEventsTailQEntry* pEntry;

        /* write event in time order, older event first, newer event later */
        NB_AE_TAILQ_FOREACH_REVERSE(pEntry, &eventsTailQs[i])
        {
            data_analytics_event* pEvent = pEntry->event;
            err = NB_AnalyticsEventsDbWriteEvent(pThis, pEvent);
            if (err)
            {
                break;
            }
        }
    }

    return err;
}

/* @} */


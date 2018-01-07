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

  @file nimnet.c
*/
/*
  (C) Copyright 2004 - 2009 by Networks In Motion, Inc.

  The information contained herein is confidential, proprietary
  to Networks In Motion, Inc., and considered a trade secret as
  defined in section 499C of the penal code of the State of
  California. Use of this information by anyone other than
  authorized employees of Networks In Motion is granted only
  under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.

  ---------------------------------------------------------------------------*/

// Uncomment this define if you want verbose output
//#define CORE_VERBOSE_DEBUG_OUTPUT

// Uncomment this define to dump all query and reply tps elements
//#define DUMP_ALL_NETWORK_TRAFFIC


#ifdef CORE_VERBOSE_DEBUG_OUTPUT
#undef NDEBUG
#define DEBUG_LOG_ALL 1
#include "paldebuglog.h"
#else
#define	DEBUGLOG(x, y, z)	(void)0
#endif

#include "paltypes.h"
#include "palstdlib.h"
#include "nberror.h"
#include "palmath.h"
#include "pal.h"
#include "palclock.h"
#include "palnet.h"
#include "paltimer.h"
#include "cslqalog.h"
#include "cslqarecorddefinitions.h"
#include "nbnetworkconfiguration.h"

#include "bq.h"
#include "csdict.h"
#include "intpack.h"
#include "cslnetwork.h"
#include "cslutil.h"
#include "tpselt.h"
#include "tpsio.h"
#include "tpslib.h"
#include "palunzip.h"
#include <time.h>

#if !defined(NDEBUG) && defined(DUMP_ALL_NETWORK_TRAFFIC)
#include "tpsdebug.h"
#endif

#define IPV6_ADDRESS_LEN 40

#define	IDEN_TARGET				"iden!"
#define	IDENINFO_LEN			31
#define	TRIAGE_TIMER_PERIOD		5
#define TRIAGE_TIMEOUT_SEC		30

#define ERROR_PACKET_ID			255
#define IDEN_PACKET_ID			256
#define SOCKET_PACKET_ID        257

#define HTTP_VERB               "POST"
#define HTTP_ACCEPT_TYPES       "*/*"
#define HTTP_ADDITIONAL_HEADERS "Connection: close\r\nReplies: 2\r\n"

#define MAX_EVENT_CALLBACKS     6

static uint32 HandleIdenReceipt(CSL_Network* pThis, tpselt receipt);
static void FireNotifyEventCallback(CSL_Network* pThis, NB_NetworkNotifyEvent event, void* data);

static NB_Error NetWriteQaLogEvent(CSL_Network* pThis, const char* event, const char* function, const char* file, int line);

static NB_Error NetWriteQaLogDnsResult(CSL_Network* pThis, PAL_Error errorCode, const char* hostName, uint32 const* const* addressList, uint32 addressCount);

static NB_Error WriteQaLogDnsRecord(CSL_Network* pThis, uint16 recordID, PAL_Error errorCode, const char* hostName, uint32 const* const* addressList, uint32 addressCount);

static NB_Error NetWriteQaLogQueryReplyExtended(CSL_Network* pThis, uint16 id, const char* name, int queryId, int size, NB_NetworkResponseStatus status);
static NB_Error NetWriteQaLogErrorExtended(CSL_Network* pThis, const char* name, int queryId, NB_NetworkResponseStatus status, NB_Error error);
static NB_Error GetServerErrorCodeFromTps(tpselt te);
static tpselt GetDefaultInvocationContext(CSL_Network* pThis);

static NB_TransactionInfo* TransactionInfoCreate();
static void TransactionInfoDestroy(NB_TransactionInfo*);

#define NET_WRITE_QALOG_EVENT(net, event)           NetWriteQaLogEvent(net, event, DBG_FUNCTION, __FILE__, __LINE__);

struct conn
{
    PAL_NetConnection* palConn;
    struct bq rq;
    struct bq wq;
    int ready;
    int nextid;
    nb_unixTime lastwrite;
    nb_unixTime lastread;
    nb_unixTime emptysince;
    NB_NetworkQuery* pending;

    nb_unixTime connecttime; // for statistics

    /* mux packet processing */
    struct
    {
        size_t
        (*section)(CSL_Network*, const char *, size_t);
        size_t bodylenlen;
        size_t namelen;
        char name[64];
        size_t bodylen;
        size_t bodyleft;
        int reading_body;
        int rflags;
        int id;
        struct tps_unpackstate *ups;
        int complete;
        uint32 reply_unpacking_time;
        nb_boolean wantBinaryChunk;
        struct bq  chunkQueue;
    } pkt;
};

struct CSL_Network
{
    PAL_Instance* pal;
    NB_NetworkConfiguration config;                     /*!< Network configuration options */
    struct tpslib* tl;                                  /*!< TPS library for encoding/decoding data */
    struct conn* conn;                                  /*!< Current connection */
    struct paltimer* tt;                                /*!< Triage timer */
    STAILQ_HEAD(querieshead, NB_NetworkQuery) queries;  /*!< Pending queries */
    int failedconnect;
    int connectretry;
    CSL_QaLog* qalog;
    int eventCallbackCount;                             /*!< Number of registered callbacks */
    NB_NetworkNotifyEventCallback eventCallbacks[MAX_EVENT_CALLBACKS]; /*!< Network event callbacks */
    tpselt defaultInvocationContext;                    /*!< Default Invocation Context */
};

static void conn_fail(CSL_Network* pThis, NB_NetworkResponseStatus status, NB_Error error);
static void conn_fini(CSL_Network* pThis, boolean fast);
static void triage(CSL_Network* pThis);
static size_t input_lengths(CSL_Network* pThis, const char *, size_t);

/*
 * Return a unix-ish time suitable for network timing calculations. In
 * practice, code expects this to be monotonically increasing, but
 * most platforms don't make that guarantee.
 */
static nb_unixTime
nettime(void)
{
    return (PAL_ClockGetUnixTime());
}

static int
istcp(NB_NetworkProtocol protocol)
{
    return (protocol == NB_NetworkProtocol_TCP || protocol == NB_NetworkProtocol_TCPTLS);
}

static void
inputreset(CSL_Network* pThis)
{
    struct conn *nc = pThis->conn;

    if (nc == NULL)
    {
        return;
    }

    nc->pkt.reading_body = 0;
    nc->pkt.section      = input_lengths;
    nc->pkt.complete     = 0;

    nc->pkt.wantBinaryChunk = FALSE;
    csl_bqfree(&nc->pkt.chunkQueue);
}

static void
complete(CSL_Network* pThis, NB_NetworkQuery* nq, NB_NetworkResponse* nr)
{
    NB_TransactionInfo* transactionEvent = NULL;

    if ((!pThis) || (!nq) || (!nr))
    {
        return;
    }

    STAILQ_REMOVE(&pThis->queries, nq, NB_NetworkQuery, list);
    te_dealloc(nq->query);
    nq->invocation = NULL;
    nq->query = NULL; /* Don't let the cb access this since it has been deallocated */

#if !defined(NDEBUG) && defined(DUMP_ALL_NETWORK_TRAFFIC)
    DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("DUMP_ALL_NETWORK complete"));
    dumpelt(nr->reply, 0);
#endif

    /* Collect transaction information for later use. */
    if (nr->reply_headers && nr->reply_headers->vt)
    {
        tpselt tinfo = te_getchild(nr->reply_headers, "transaction-info");
        if (tinfo)
        {
            transactionEvent = TransactionInfoCreate();
            transactionEvent->transaction_id = te_getattrc(tinfo, "request-time");
            transactionEvent->reply_unpacking_time =
                    pThis->conn ? PAL_ClockGetTimeMs() -  pThis->conn->pkt.reply_unpacking_time : 0;
            transactionEvent->query_packing_time = nq->query_packing_time;
            transactionEvent->query_name  = nr->source ? nsl_strdup(nr->source) : NULL;
            transactionEvent->target_name = nq->target ? nsl_strdup(nq->target) : NULL;
        }
    }

    nq->action(nq, nr);
    /* nq invalid */
    FireNotifyEventCallback(pThis, NB_NetworkNotifyEvent_TransactionInfo, transactionEvent);
    TransactionInfoDestroy(transactionEvent);

    if (STAILQ_EMPTY(&pThis->queries))
    {
        nb_boolean value = FALSE;
        FireNotifyEventCallback(pThis, NB_NetworkNotifyEvent_DataPending, &value);
    }
}

static void
failwith(CSL_Network* pThis, NB_NetworkQuery* nq, NB_NetworkResponseStatus stat, NB_Error error)
{
    NB_NetworkResponse nr = {0};

    if (!error)
    {
        nr.status = stat;
        nr.reply = NULL;
        nr.source = NULL;
        nr.sourcelen = 0;
    }
    else
    {
        const char* source = "cslnetwork";

        nr.status = RESP_ERROR;
        nr.reply = te_new("error");
        if (nr.reply)
        {
            char errorString[15] = { 0 };
            nsl_sprintf(errorString, "%u", error);

            if (!te_setattr(nr.reply, "code", errorString, nsl_strlen(errorString)))
            {
                te_dealloc(nr.reply);
                nr.reply = 0;
            }
        }

        nr.source = source;
        nr.sourcelen = nsl_strlen(source);
    }

    NetWriteQaLogErrorExtended(pThis, nq->target, nq->sentid, stat, error);
    complete(pThis, nq, &nr);

    if (error && nr.reply)
    {
        te_dealloc(nr.reply);
    }
}

void get_reply_header_from_tps(const tpselt tpsdoc, tpselt *reply, tpselt *reply_header)
{
    if (tpsdoc && tpsdoc->vt)
    {
        tpselt ce;
        int n = 0;
        while ((ce = te_nextchild(tpsdoc, &n)) != NULL)
        {
            if (nsl_strcmp(te_getname(ce), "reply-headers") == 0)
            {
                *reply_header = te_clone(ce);
            }
            else
            {
                *reply = ce;
                break;
            }
        }
    }

    return;
}

static void
inputcomplete(CSL_Network* pThis)
{
    struct conn *nc = pThis->conn;
    enum tpsio_status stat;
    NB_NetworkResponse nr = {0};
    NB_NetworkQuery* nq = NULL;
    NB_NetworkQuery* tnq = NULL;
    size_t plen = 0;
    boolean closeConnection = FALSE;

    nsl_assert(nc->pkt.complete);

    nr.tpsdoc = NULL;

    nsl_memset(&nr, 0, sizeof(nr));
    nr.source = nc->pkt.name;
    nr.sourcelen = nc->pkt.namelen;

    DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("inputcomplete: id=%d name=%.*s", nc->pkt.id, nr.sourcelen, nr.source));

    if (nc->pkt.id == ERROR_PACKET_ID)
        goto syserror;
    if (nc->pkt.ups == NULL)
        goto out2;
    plen = nc->pkt.bodylen;
    stat = tps_unpack_result(nc->pkt.ups, &nr.tpsdoc);
    nc->pkt.ups = NULL;
    if (stat != TIO_READY)
    {
        DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("inputcomplete: stat %d", stat));
  syserror:
        nr.status = RESP_ERROR;
        nr.reply = NULL;
        goto neterror;
    }

    if (nsl_strcmp(te_getname(nr.tpsdoc), "reply") == 0)
    {
        nr.status = COMPLETE;
        get_reply_header_from_tps(nr.tpsdoc, &nr.reply, &nr.reply_headers);
        if (nr.reply == NULL)
        {
            DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("inputcomplete: reply has no children"));
            goto out;
        }
    }
    else if (nsl_strcmp(te_getname(nr.tpsdoc), "iden-receipt") == 0)
    {
        uint32 error = HandleIdenReceipt(pThis, nr.tpsdoc);

        if (error)
        {
            nr.status = RESP_ERROR;
            get_reply_header_from_tps(nr.tpsdoc, &nr.reply, &nr.reply_headers);
            STAILQ_FOREACH_SAFE(nq, &pThis->queries, list, tnq)
            {
                complete(pThis, nq, &nr);
            }
            closeConnection = TRUE;
        }
        goto out;
    }
    else if (nsl_strcmp(te_getname(nr.tpsdoc), "error") == 0)
    {
        nr.status = RESP_ERROR;
        nr.reply = nr.tpsdoc;
    }
    else
    {
        DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("inputcomplete: unexpected top, %s", te_getname(nr.tpsdoc)));
        goto out;
    }

neterror:
    STAILQ_FOREACH_SAFE(nq, &pThis->queries, list, tnq)
    {
        DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("inputcomplete: checking %.*s:%d", nq->targetlen, nq->target, nq->sentid));
        if (nc->pkt.id == ERROR_PACKET_ID)
        {
            NB_Error error = GetServerErrorCodeFromTps(nr.reply);

            NetWriteQaLogErrorExtended(pThis, nq->target, nq->sentid, nr.status, error);
            closeConnection = TRUE;
            complete(pThis, nq, &nr);
            continue;
        }
        if (nq->targetlen != (int)nr.sourcelen)
            continue;
        if (nsl_memcmp(nq->target, nr.source, nr.sourcelen) != 0)
            continue;
        if (nc->pkt.id == nq->sentid)
        {
            DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("inputcomplete: match id=%d", nc->pkt.id));
            if (nr.status == RESP_ERROR)
            {
                closeConnection |= ((nq->failFlags & TN_FF_CLOSE_ERROR) != 0);
            }

            if (nc->pkt.wantBinaryChunk)
            {
                nr.chunkLength = (int)(nc->pkt.chunkQueue.x - nc->pkt.chunkQueue.p);
                nr.binaryChunk = nc->pkt.chunkQueue.r;
            }

            if (nr.status == COMPLETE)
            {
                NetWriteQaLogQueryReplyExtended(pThis, RECORD_ID_NETWORK_REPLY_EXTENDED, nq->target, nq->sentid, (int)plen, nr.status);
            }
            else
            {
                NB_Error error = GetServerErrorCodeFromTps(nr.reply);
                NetWriteQaLogErrorExtended(pThis, nq->target, nq->sentid, nr.status, error);
            }

            complete(pThis, nq, &nr);
            csl_bqfree(&nc->pkt.chunkQueue);
            break;
        }
    }
out:
    if (nr.tpsdoc != NULL)
        te_dealloc(nr.tpsdoc);
    if (nr.reply_headers != NULL)
        te_dealloc(nr.reply_headers);
out2:
    inputreset(pThis);

    if (closeConnection)
    {
        DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("inputcomplete: closing connection"));
        conn_fini(pThis, FALSE);
    }
}

static void
maybeinputcomplete(CSL_Network* pThis)
{
    struct conn *nc = pThis->conn;

    if (nc->pkt.complete)
    {
        csl_bqshrink(&nc->rq);
        inputcomplete(pThis);
    }
}

static void
setinputcomplete(CSL_Network* pThis)
{
    struct conn *nc = pThis->conn;

    nsl_assert(!nc->pkt.complete);
    nc->pkt.complete = 1;
}

static size_t
inputnext(CSL_Network* pThis, const char *data, size_t datalen, size_t i)
{
    if (datalen - i == 0)
        return (i);
    return (i + pThis->conn->pkt.section(pThis, data + i, datalen - i));
}

/*
 * If something went wrong, we just drop the rest of this packet and
 * expect complete to report a failure.
 */
static size_t
#ifndef UNUSED_NDEFINED
input_dropbody(CSL_Network* pThis, const char *data __unused, size_t datalen)
#else
        input_dropbody(CSL_Network* pThis, const char *data, size_t datalen)
#endif
{
    struct conn *nc = pThis->conn;
    size_t i = 0;

    i = datalen;
    if (i > nc->pkt.bodyleft)
        i = nc->pkt.bodyleft;
    nc->pkt.bodyleft -= i;
    if (nc->pkt.bodyleft == 0)
        setinputcomplete(pThis);
    return (i);
}

/*
 * Collect an error message and dump it to the debug log
 */

static size_t
#ifndef UNUSED_NDEFINED
input_error(CSL_Network* pThis, const char *data __unused, size_t datalen)
#else
        input_error(CSL_Network* pThis, const char *data, size_t datalen)
#endif
{
    struct conn *nc = pThis->conn;
    size_t i = 0;

    i = datalen;
    if (i < nc->pkt.bodyleft)
        return 0;
    else
        i = nc->pkt.bodyleft;
    nc->pkt.bodyleft -= i;

    DEBUGLOG(LOG_SS_NET, LOG_SEV_MAJOR, ("net error: %.*s", datalen, data));

    if (nc->pkt.bodyleft == 0)
        setinputcomplete(pThis);
    return (i);
}

struct cl_input_doc_compressed
{
    CSL_Network* network;
    struct bq rq;
};

static NB_Error
decompress_doc(void *arg, const char *data, size_t datalen)
{
    struct cl_input_doc_compressed *ibc = arg;
    struct conn *nc = ibc->network->conn;
    nb_size i;

    if (!csl_bqprep(&ibc->rq, datalen))
        return (NE_NOMEM);
    nsl_memcpy(ibc->rq.w, data, datalen);
    ibc->rq.w += datalen;
    i = tps_unpack_feed(nc->pkt.ups, ibc->rq.r, ibc->rq.w - ibc->rq.r);
    if (i == -1)
        return (NE_INVAL);
    ibc->rq.r += i;
    return (NE_OK);
}

static size_t
input_doc_compressed(CSL_Network* pThis, const char *data, size_t datalen)
{
    struct conn *nc = pThis->conn;
    struct cl_input_doc_compressed idc;
    NB_Error error = NE_OK;
    size_t i;

    /*
     * XXX: This inflate module doesn't support streaming the
     * input, so we have to read the entire document to decompress it.
     * That sucks.
     * The size of uncompressed data is in nc->pkt.ups->packetlen
     */

    DEBUGLOG(LOG_SS_NET, LOG_SEV_INFO, ("input_doc_compressed: datalen=%d target=%.*s bodyleft=%d", datalen, nc->pkt.namelen, nc->pkt.name, nc->pkt.bodyleft));

    if (datalen >= nc->pkt.bodyleft)
    {
        PAL_Error palError = PAL_Ok;
        uint8* destination = NULL;
        size_t decompressedSize = 0;
        uint32 relDecompressedSize = 0;

        idc.network = pThis;
        decompressedSize = tps_packet_len( nc->pkt.ups );

        if (decompressedSize==(size_t)-1)
        {
            error=NE_BADDATA;
        }
        else
        {
            destination = nsl_malloc(decompressedSize);
            if (!destination)
            {
                error = NE_NOMEM;
            }
            else
            {
                nsl_memset(destination, 0, decompressedSize);
                csl_bqinit(&idc.rq);
                palError = PAL_DecompressData((const uint8*)data, (uint32)nc->pkt.bodyleft, destination, (uint32)decompressedSize, &relDecompressedSize);
                if (palError != PAL_Ok)
                {
                    error = NE_BADDATA;
                }
                else
                {
                    if (nc->pkt.wantBinaryChunk)
                    {
                        error = NE_NOMEM;
                        if (csl_bqprep(&nc->pkt.chunkQueue, relDecompressedSize))
                        {
                            nsl_memcpy(nc->pkt.chunkQueue.w, destination, relDecompressedSize);
                            nc->pkt.chunkQueue.w += relDecompressedSize;
                            error = NE_OK;
                        }
                    }
                    error = error ? error : decompress_doc(&idc, (const char*)destination, relDecompressedSize);
                }
                csl_bqfree(&idc.rq);
                nsl_free(destination);
            }
        }
        if (error != NE_OK)
        {
            /*
             * XXX: If the error was generated by inflate
             * (and not by our function as a result of
             * feed returning -1), the unpacker will think
             * that the document is truncated and report a
             * misleading TIO_MALDOC.
             */
            nc->pkt.section = input_dropbody;
            return (inputnext(pThis, data, datalen, 0));
        }
        i = nc->pkt.bodyleft;
        nc->pkt.bodyleft = 0;
        setinputcomplete(pThis);
        return (i);
    }
    return (0);
}

static size_t
input_doc_uncompressed(CSL_Network* pThis, const char *data, size_t datalen)
{
    struct conn *nc = pThis->conn;
    nb_size i;

    i = tps_unpack_feed(nc->pkt.ups, data, datalen);
    if (i == -1)
    {
        nc->pkt.section = input_dropbody;
        return (inputnext(pThis, data, datalen, 0));
    }
    nsl_assert((size_t)i <= nc->pkt.bodyleft);
    nc->pkt.bodyleft -= i;
    if (nc->pkt.wantBinaryChunk)
    {
        struct bq* cq = &nc->pkt.chunkQueue;
        if (csl_bqprep(cq, i))
        {
            nsl_memcpy(cq->w, data, i);
            cq->w += i;
        }
        else
        {
            nc->pkt.section = input_dropbody;
            return (inputnext(pThis, data, datalen, 0));
        }
    }

    if (nc->pkt.bodyleft == 0)
        setinputcomplete(pThis);
    return (i);
}

static size_t
input_doc(CSL_Network* pThis, const char *data, size_t datalen)
{
    struct conn *nc = pThis->conn;
    unsigned long uclen;
    int i;

    if (nc->pkt.rflags & TN_RF_DEFLATE)
    {
        i = uintunpackl(data, datalen, &uclen);
        if (i == -1)
            return (0);
        nc->pkt.reply_unpacking_time = PAL_ClockGetTimeMs();
        nc->pkt.ups = tps_unpack_start(pThis->tl, uclen);
        nc->pkt.section = input_doc_compressed;
    }
    else
    {
        nc->pkt.reply_unpacking_time = PAL_ClockGetTimeMs();
        nc->pkt.ups = tps_unpack_start(pThis->tl, nc->pkt.bodyleft);
        nc->pkt.section = input_doc_uncompressed;
        i = 0;
    }
    nc->pkt.bodyleft -= i;
    if (nc->pkt.ups == NULL)
        nc->pkt.section = input_dropbody;
    return (inputnext(pThis, data, datalen, i));
}

static size_t
input_predoc(CSL_Network* pThis, const char *data, size_t datalen)
{
    struct conn *nc = pThis->conn;

    if (datalen < 2)
        return (0);
    if (nc->pkt.bodyleft < 3)
    {
        nc->pkt.section = input_dropbody;
        return (inputnext(pThis, data, datalen, 0));
    }
    nc->pkt.rflags = (unsigned char)data[0];
    nc->pkt.id = (unsigned char)data[1];
    nc->pkt.bodyleft -= 2;

    {
        NB_NetworkQuery* nq  = NULL;
        NB_NetworkQuery* tnq = NULL;

        STAILQ_FOREACH_SAFE(nq, &pThis->queries, list, tnq)
        {
            if ((unsigned int)nq->targetlen != nc->pkt.namelen)
                continue;
            if (nsl_memcmp(nq->target, nc->pkt.name, nc->pkt.namelen) != 0)
                continue;
            if (nc->pkt.id == nq->sentid)
            {
                nc->pkt.wantBinaryChunk = nq->wantBinaryChunk;
                break;
            }
        }
    }

    if (nc->pkt.id == ERROR_PACKET_ID)
        nc->pkt.section = input_error;
    else
        nc->pkt.section = input_doc;

    return (inputnext(pThis, data, datalen, 2));
}

static size_t
input_bodylen(CSL_Network* pThis, const char *data, size_t datalen)
{
    struct conn *nc = pThis->conn;

    if (datalen < nc->pkt.bodylenlen)
        return (0);
    nc->pkt.bodylen = beunpackl(data, nc->pkt.bodylenlen);
    nc->pkt.bodyleft = nc->pkt.bodylen;
    nc->pkt.reading_body = 1;
    nc->pkt.section = input_predoc;
    return (inputnext(pThis, data, datalen, nc->pkt.bodylenlen));
}

static size_t
input_name(CSL_Network* pThis, const char *data, size_t datalen)
{
    struct conn *nc = pThis->conn;

    if (datalen < nc->pkt.namelen)
        return (0);

    nsl_memset(nc->pkt.name, 0, sizeof(nc->pkt.name));
    nsl_memcpy(nc->pkt.name, data, nc->pkt.namelen);
    nc->pkt.section = input_bodylen;
    return (inputnext(pThis, data, datalen, nc->pkt.namelen));
}

static size_t
input_lengths(CSL_Network* pThis, const char *data, size_t datalen)
{
    struct conn *nc = pThis->conn;
    unsigned char b;

    b = data[0];
    nc->pkt.bodylenlen = (b >> 6) + 1;
    nc->pkt.namelen = (b & 0x3f) + 1;
    nsl_assert(nc->pkt.namelen <= sizeof(nc->pkt.name));
    nc->pkt.section = input_name;
    return (inputnext(pThis, data, datalen, 1));
}

static void
input(CSL_Network* pThis)
{
    struct conn *nc = pThis->conn;
    NB_NetworkQuery* nq = NULL;
    uint32 have = 0;

    if (pThis->conn == NULL)
    {
        return;
    }

    // Report progress before processing stream.
    STAILQ_FOREACH(nq, &pThis->queries, list)
    {
        if (nq->progress == NULL)
            continue;
        if (nq->targetlen != (int)nc->pkt.namelen)
            continue;
        if (nsl_memcmp(nq->target, nc->pkt.name, nc->pkt.namelen) != 0)
            continue;
        if (nq->sentid == nc->pkt.id)
        {
            have = (uint32)(nc->pkt.bodylen - nc->pkt.bodyleft + (nc->rq.w - nc->rq.r));
            nq->progress(nq, FALSE, have, (uint32)nc->pkt.bodylen);
            nq->lastTime = nettime();
            break;
        }
    }

    nc->rq.r += inputnext(pThis, nc->rq.r, nc->rq.w - nc->rq.r, 0);
    maybeinputcomplete(pThis);

    /* connection may get closed on error */
    if (pThis->conn == NULL)
    {
        return;
    }

    if (!nc->pkt.reading_body)
    {
        return;
    }
}

static void
pcinput(CSL_Network* pThis)
{
    struct bq *rq = NULL;
    size_t i, j;

    nsl_assert(pThis->conn != NULL);
    rq = &pThis->conn->rq;
    do
    {
        i = rq->w - rq->r;
        if (i > 0)
        {
            input(pThis);
        }
        j = rq->w - rq->r;
    }
    while (j > 0 && i != j);
}

static enum tpsio_status
output_reader(void *arg, char *doc, size_t doclen)
{
    struct conn *nc = arg;

    if (!csl_bqprep(&nc->wq, doclen))
        return (TIO_NOMEM);
    nsl_memcpy(nc->wq.w, doc, doclen);
    nc->wq.w += doclen;
    if (nc->pending != NULL)
    {
        nc->pending->pendingdata += doclen;
        nc->pending->packetlen += doclen;
    }
    return (TIO_READY);
}

static void
bumpnextid(struct conn *nc)
{
    nc->nextid = (nc->nextid + 1) % ERROR_PACKET_ID;
}

static int
setid(CSL_Network* pThis, NB_NetworkQuery* nq)
{
    struct conn* nc = pThis->conn;
    NB_NetworkQuery* xq = NULL;
    int i = 0;

again:
    /*
     * Make sure that this ID isn't already used. We only have one byte
     * in which to store it, so it's possible that one is being reused
     * and that there is still an outstanding request with the same
     * value.
     */
    STAILQ_FOREACH(xq, &pThis->queries, list)
    {
        if (xq->expecting && xq->sentid == nc->nextid)
        {
            if (++i == ERROR_PACKET_ID)
                return (0); /* No available IDs. */
            bumpnextid(nc);
            goto again;
        }
    }
    nq->sentid = nc->nextid;
    bumpnextid(nc);
    return (1);
}

/*
 * Ready to attempt a write.
 */
static void
trywrite(CSL_Network* pThis)
{
    int count = 0;
    nsl_assert(pThis->conn != NULL);

    count = (int)(pThis->conn->wq.w - pThis->conn->wq.r);
    if (count > 0)
    {
        /* based on protocol, use appropriate PAL send function to write out current bytes */
        switch (pThis->config.protocol)
        {
            case NB_NetworkProtocol_TCP:
            case NB_NetworkProtocol_TCPTLS:
                if (PAL_NetSend(pThis->conn->palConn, (byte*)pThis->conn->wq.r, count) == PAL_Ok)
                {
                    pThis->conn->wq.r += count;
                    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("net: %d bytes queued to send TCP", count));
                }
                break;
            case NB_NetworkProtocol_HTTP:
            case NB_NetworkProtocol_HTTPS:
                if (PAL_NetHttpSend(pThis->conn->palConn, (byte*)pThis->conn->wq.r, count, HTTP_VERB, NULL, HTTP_ACCEPT_TYPES, HTTP_ADDITIONAL_HEADERS, NULL) == PAL_Ok)
                {
                    pThis->conn->wq.r += count;
                    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("net: %d bytes queued to send HTTP/HTTPS", count));
                }
                break;
            default:
                nsl_assert(FALSE); /* error! */
        }
    }
}

static void
output_query(CSL_Network* pThis, NB_NetworkQuery* nq)
{
    struct conn *nc = pThis->conn;
    enum tpsio_status stat = TIO_READY;
    size_t oldw = 0;
    long bodylen = 0;
    char *blp = NULL;
    size_t hlen = 0;
    uint32 start_packing_time;

    nsl_assert(nc != NULL);
    if (!setid(pThis, nq))
        return;
    /*
     * Reset the timeout when we output a request.
     * Even though nothing has actually been written to the socket,
     * this is necessary to prevent a newly queued request from being
     * immediately timed out before it has a chance to be written
     */

    if (pThis->conn != NULL)
        pThis->conn->lastwrite = nettime();

    nq->pendingdata = nc->wq.w - nc->wq.r;
    nq->packetlen = 0;

    /*
     * XXX: We always specify bodylenlen as 4. Doing otherwise
     * would require being able to copy the body back a couple of
     * bytes because the header shrinks. Do we care to do that to
     * save 3 measly bytes?
     */
    oldw = nc->wq.w - nc->wq.r;
    hlen = 1 + nq->targetlen + 6;
    if (!csl_bqprep(&nc->wq, hlen))
        return;
    /* header */
    *nc->wq.w++ = (unsigned char)((3 << 6) | (nq->targetlen - 1));
    /* name */
    nsl_memcpy(nc->wq.w, nq->target, nq->targetlen);
    nc->wq.w += nq->targetlen;
    /* bodylen */
    nc->wq.w += 4;
    /* body */
    *nc->wq.w++ = (unsigned char)nq->qflags;
    *nc->wq.w++ = (unsigned char)nq->sentid;
    nq->packetlen += hlen;
    nq->pendingdata += hlen;
    nc->pending = nq;

    start_packing_time = PAL_ClockGetTimeMs();
    stat = tps_pack(pThis->tl, nq->query, output_reader, nc);
    nq->query_packing_time = PAL_ClockGetTimeMs() - start_packing_time;

    nc->pending = NULL;
    if (stat == TIO_READY)
    {
        blp = nc->wq.r + oldw + 1 + nq->targetlen;
        bodylen = nc->wq.w - blp - 4;
        bepackl(blp, 4, bodylen);

        NetWriteQaLogQueryReplyExtended(pThis, RECORD_ID_NETWORK_QUERY_EXTENDED, nq->target,
                                        nq->sentid, (int)nq->packetlen, NONE);
        trywrite(pThis);
        nq->expecting = 1;
        nq->lastTime = nettime();
    }
    else
    {
        if (stat != TIO_NOMEM)
        {
            failwith(pThis, nq, RESP_ERROR, NE_NOENT);
        }
        nc->wq.w = nc->wq.r + oldw;
    }
}

/*
 * Do whatever is necessary to initialize the connection. This
 * implementation claims an identification instead of doing
 * authentication.
 */
static void
output_auth(CSL_Network* pThis)
{
    struct conn *nc = pThis->conn;
    int targlen = 0;
    enum tpsio_status stat = TIO_READY;
    size_t oldw = 0;
    long bodylen = 0;
    char *blp = NULL;

    targlen = nsl_strlen(IDEN_TARGET);
    oldw = nc->wq.w - nc->wq.r;
    if (!csl_bqprep(&nc->wq, 1 + targlen + 1 + TPSLIB_IDLEN + 6))
    {
        NetWriteQaLogErrorExtended(pThis, "iden", IDEN_PACKET_ID, FAILED, NE_NOMEM);
        conn_fail(pThis, FAILED, NE_OK);
        return;
    }

#if defined(DUMP_ALL_NETWORK_TRAFFIC)
    DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("DUMP_ALL_NETWORK output_auth"));
    dumpelt(pThis->config.iden, 0);
#endif

    /* header */
    *nc->wq.w++ = (unsigned char)((3 << 6) | (targlen - 1));
    /* name */
    nsl_memcpy(nc->wq.w, IDEN_TARGET, targlen);
    nc->wq.w += targlen;
    /* bodylen */
    nc->wq.w += 4;
    /* body */
    tpslib_getid(pThis->tl, nc->wq.w);
    nc->wq.w += TPSLIB_IDLEN;
    nc->pending = NULL;

    stat = tps_pack(pThis->tl, pThis->config.iden, output_reader, nc);
    if (stat == TIO_READY)
    {
        blp = nc->wq.r + oldw + 1 + targlen;
        bodylen = nc->wq.w - blp - 4;
        bepackl(blp, 4, bodylen);
        trywrite(pThis);

        /* The connection is now established, so call the
           callback registered by the user */
        FireNotifyEventCallback(pThis, NB_NetworkNotifyEvent_ConnectionOpened, NULL);
    }
    else
    {
        nc->wq.w = nc->wq.r + oldw;
        NetWriteQaLogErrorExtended(pThis, "iden", IDEN_PACKET_ID, FAILED, NE_NOENT);
        conn_fail(pThis, FAILED, NE_NOENT);
        return;
    }
}

/* :TODO: refactor/share code with output_auth & output_query? */
static void
output_auth_query(CSL_Network* pThis, NB_NetworkQuery* nq)
{
    struct conn *nc = pThis->conn;
    enum tpsio_status stat = TIO_READY;
    size_t oldw = 0;
    long bodylen = 0;
    long blo = 0;
    size_t hlen = 0;
    size_t targlen_iden = 0;
    uint32 start_packing_time;

    nsl_assert(nc != NULL);
    if (!setid(pThis, nq))
        return;

    /*
     * Reset the timeout when we output a request.
     */

    if (pThis->conn != NULL)
        pThis->conn->lastwrite = unixtime();

    nq->pendingdata = nc->wq.w - nc->wq.r;
    nq->packetlen = 0;

    /*
     * Allocate big enough buffer so we don't have to keep reallocate
     */
    /* :TODO: compute needed buffer */
    if (!csl_bqprep(&nc->wq, 1024 * 2))
        return;
    /*
     * XXX: We always specify bodylenlen as 4. Doing otherwise
     * would require being able to copy the body back a couple of
     * bytes because the header shrinks. Do we care to do that to
     * save 3 measly bytes?
     */
    oldw = nc->wq.w - nc->wq.r;

    /*
     * IDEN packet
     */

    /* header */
    targlen_iden = nsl_strlen(IDEN_TARGET);
    *nc->wq.w++ = (unsigned char)((3 << 6) | (targlen_iden - 1));
    /* name */
    nsl_memcpy(nc->wq.w, IDEN_TARGET, targlen_iden);
    nc->wq.w += targlen_iden;
    /* bodylen */
    blo = nc->wq.w - nc->wq.r;
    nc->wq.w += 4;
    /* body */
    tpslib_getid(pThis->tl, nc->wq.w);
    nc->wq.w += TPSLIB_IDLEN;
    nc->pending = NULL;
    start_packing_time = PAL_ClockGetTimeMs();
    stat = tps_pack(pThis->tl, pThis->config.iden, output_reader, nc);
    /* write for iden body length */
    bodylen = nc->wq.w - nc->wq.r - blo - 4;
    bepackl(nc->wq.r + blo, 4, bodylen);

    /*
     * Actual query
     */

    /* header */
    *nc->wq.w++ = (unsigned char)((3 << 6) | (nq->targetlen - 1));
    /* name */
    nsl_memcpy(nc->wq.w, nq->target, nq->targetlen);
    nc->wq.w += nq->targetlen;
    /* bodylen */
    blo = nc->wq.w - nc->wq.r;
    nc->wq.w += 4;
    /* body */
    *nc->wq.w++ = (unsigned char)nq->qflags;
    *nc->wq.w++ = (unsigned char)nq->sentid;
    hlen = 1 + nq->targetlen + 6;
    nq->packetlen += hlen;
    nq->pendingdata += hlen;
    nc->pending = nq;

    stat = tps_pack(pThis->tl, nq->query, output_reader, nc);
    nq->query_packing_time = PAL_ClockGetTimeMs() - start_packing_time;

    nc->pending = NULL;
    if (stat == TIO_READY)
    {
        bodylen = nc->wq.w - nc->wq.r - blo - 4;
        bepackl(nc->wq.r + blo, 4, bodylen);
        trywrite(pThis);
        nq->expecting = 1;
    }
    else
    {
        if (stat != TIO_NOMEM)
        {
            failwith(pThis, nq, RESP_ERROR, NE_NOENT);
        }
        nc->wq.w = nc->wq.r + oldw;
    }
}

static void
pcstart(CSL_Network* pThis)
{
    struct conn *nc = pThis->conn;
    inputreset(pThis);
    nc->connecttime = nettime();
    nc->ready = 1;
    pThis->failedconnect = 0;
    /* send auth packet out for new TCP connection; HTTP/HTTPS sends with each request */
    if (istcp(pThis->config.protocol))
    {
        output_auth(pThis);
    }
    triage(pThis);
}

static void
pcfini(CSL_Network* pThis)
{
    conn_fail(pThis, FAILED, NE_OK);
}

static void
conn_fini(CSL_Network* pThis, boolean fast)
{
    struct conn *nc = pThis->conn;
    NB_NetworkQuery* nq = NULL;
    NB_NetworkQuery* tnq = NULL;
    tpselt te = NULL;

    NET_WRITE_QALOG_EVENT(pThis, "finished");

    DEBUGLOG(LOG_SS_NET, LOG_SEV_INFO, ("net: conn_fini %d", nettime()));

    if(nc == NULL)
    {
        return;
    }

    STAILQ_FOREACH_SAFE(nq, &pThis->queries, list, tnq)
    {
        nq->expecting = 0;
    }

    if (nc->palConn != NULL)
    {
        PAL_NetCloseConnection(nc->palConn);
        PAL_NetDestroyConnection(nc->palConn);
        nc->palConn = NULL;
    }

    csl_bqfree(&nc->wq);
    csl_bqfree(&nc->rq);

    if (nc->pkt.ups != NULL)
        if (tps_unpack_result(nc->pkt.ups, &te) == TIO_READY)
            te_dealloc(te);
    nsl_free(nc);
    pThis->conn = NULL;

    FireNotifyEventCallback(pThis, NB_NetworkNotifyEvent_ConnectionClosed, NULL);
}

static void
conn_fail(CSL_Network* pThis, NB_NetworkResponseStatus status, NB_Error error)
{
    NB_NetworkQuery* nq = NULL;
    NB_NetworkQuery* tnq = NULL;

    NET_WRITE_QALOG_EVENT(pThis, "failed");

    pThis->failedconnect++;

    DEBUGLOG(LOG_SS_NET, LOG_SEV_MAJOR, ("net: conn_fail failedconnect=%d", pThis->failedconnect));

    if (pThis->failedconnect > pThis->connectretry && status == NONE)
    {
        status = TIMEDOUT;
        pThis->failedconnect = 0;
    }

    if (status != NONE)
    {
        DEBUGLOG(LOG_SS_NET, LOG_SEV_INFO, ("net: cancel_queries status=%d", status));

        STAILQ_FOREACH_SAFE(nq, &pThis->queries, list, tnq)
        {
            failwith(pThis, nq, status, error);
        }
    }

    conn_fini(pThis, FALSE);
}

/*! PAL Network connection status notification callback.

  This callback provides notification of status changes of a PAL network connection.

  NOTE: This callback should be invoked from PAL on the thread that the connection was created on.

  @param pData serData (struct net instance) specified at PAL network connection open
  @param status current status of PAL network connection
*/
static void
PalNetStatusNotifyCallback(void* pData, PAL_NetConnectionStatus status)
{
    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("pal net: status %d", (int)status));

    switch (status)
    {
        case PNCS_Connected:
            pcstart(pData);
            break;
        case PNCS_Error:
            NetWriteQaLogErrorExtended(pData, "net", SOCKET_PACKET_ID, RESP_ERROR, NE_NETSVC);
            FireNotifyEventCallback(pData, NB_NetworkNotifyEvent_ConnectionError, NULL);
            pcfini(pData);
            break;
        case PNCS_Failed:
            NetWriteQaLogErrorExtended(pData, "net", SOCKET_PACKET_ID, FAILED, NE_NETSVC);
            FireNotifyEventCallback(pData, NB_NetworkNotifyEvent_ConnectionFailed, NULL);
            pcfini(pData);
            break;
        case PNCS_PppStateChanged:
        {
            PAL_NetPppState state = PNPS_PPP_Invalid;
            CSL_Network* clsNetwork = (CSL_Network*)pData;
            if (clsNetwork && clsNetwork->conn && PAL_NetGetPppState(clsNetwork->conn->palConn, &state) == PAL_Ok)
            {
                char log[20] = {0};
                nsl_sprintf(log, "PPP state=%d", (int)state);
                CSL_QaLogAppState(clsNetwork->qalog, log);
            }
        }
        break;
        default:
            break;
    }
}

/*! PAL TCP network connection data received notification callback.

  This callback provides notification when data has been received on a PAL network connection.

  Data passed into this callback should be copied before exiting.

  NOTE: This callback should be invoked from PAL on the thread that the connection was created on.

  @param pData userData (struct net instance) specified at PAL network connection open
  @param pBytes data bytes received from PAL network connection
  @param count count of data bytes received from PAL network connection
*/
static void
PalNetTcpDataReceivedNotifyCallback(void* pData, const byte* pBytes, uint32 count)
{
    CSL_Network* pThis = (CSL_Network*)pData;
    struct conn *nc = 0;

    nsl_assert(pThis != NULL);
    nc = pThis->conn;

    /* connection may get closed on error */
    if (nc != NULL)
    {
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("pal net: %d bytes received", count));
        nc->lastread = nettime();

        /* write incoming data to the conn's rq (should be on same thread as connection created on) */
        if (!csl_bqprep(&nc->rq, count))
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("clsnetwork: failed to allocate memory"));
            return;
        }
        nsl_memset(nc->rq.w, 0, count);
        nsl_memcpy(nc->rq.w, pBytes, count);
        nc->rq.w += count;
        pcinput(pThis);
    }
}

/*! PAL HTTP network connection data received notification callback.

  This callback provides notification when data has been received on a PAL HTTP network connection.

  Data passed into this callback should be copied before exiting.

  NOTE: This callback should be invoked from PAL on the thread that the connection was created on.

  @param pData userData (struct net instance) specified at PAL network connection open
  @param pBytes data bytes received from PAL network connection
  @param count count of data bytes received from PAL network connection
  @param request reference passed during call to PAL_NetHttpSend
*/
static void
PalNetHttpDataReceivedNotifyCallback(void* pData, void* pRequestData, PAL_Error errorCode, const byte* pBytes, uint32 count)
{
    struct conn *nc = 0;
    CSL_Network* pThis = pData;


    nsl_assert(pThis != NULL);
    nc = pThis->conn;

    if (count == 0)
    {
        // HTTP and HTTPS will send a 0 byte packet to signal that the input data has been exhausted
        // TPS over HTTP includes the total response length so we can safely ignore this notification
        return;
    }

    /* connection may get closed on error */
    if (nc != NULL)
    {
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("pal net: %d bytes received", count));

        /* write incoming data to the conn's rq (should be on same thread as connection created on) */
        if (!csl_bqprep(&nc->rq, count))
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("clsnetwork: failed to allocate memory"));
            return;
        }
        nsl_memcpy(nc->rq.w, pBytes, count);
        nc->rq.w += count;

        pcinput(pThis);
    }
}

/*! PAL network connection data sent notification callback.

  This callback provides notification when data that was previously queued to be sent has been
  actually sent on a PAL network connection.

  Data passed into this callback should be copied before exiting.

  NOTE: This callback should be invoked from PAL on the thread that the connection was created on.

  @param pData userData (struct net instance) specified at PAL network connection open
  @param pBytes data bytes sent on PAL network connection
  @param count count of data bytes sent on PAL network connection
*/
static void
PalNetDataSentNotifyCallback(void* pData, const byte* pBytes, uint32 count)
{
    CSL_Network* pThis = pData;
    struct conn *nc = 0;

    nsl_assert(pThis != NULL);
    nc = pThis->conn;

    /* connection may get closed on error */
    if (nc != NULL)
    {
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("pal net: %d bytes sent", count));

        if (nc->wq.r == nc->wq.w)
        {
            csl_bqshrink(&nc->wq);
        }
    }
}

/*! PAL network DNS result notification callback.

  This callback provides notification when data that was previously queued to be sent has been
  actually sent on a PAL network connection.

  Data passed into this callback should be copied before exiting.

  NOTE: This callback should be invoked from PAL on the thread that the connection was created on.

  @param userData caller-supplied reference for this connection
  @param hostName host name
  @param resultCode result code
  @param addresses resolved addresses
  @param addressCount address count
*/
static void
PalNetDnsResultNotifyCallback(void* userData, PAL_Error errorCode, const char* hostName, uint32 const* const* addresses, uint32 addressCount)
{
    CSL_Network* pThis = userData;
    NetWriteQaLogDnsResult(pThis, errorCode, hostName, addresses, addressCount);
}

static void
conn_open_pal(CSL_Network* pThis)
{
    PAL_Error err = PAL_Ok;

    PAL_NetConnectionProtocol protocol = PNCP_Undefined;
    switch (pThis->config.protocol)
    {
        case NB_NetworkProtocol_TCP:
            protocol = PNCP_TCP;
            break;
        case NB_NetworkProtocol_TCPTLS:
            protocol = PNCP_TCPTLS;
            break;
        case NB_NetworkProtocol_HTTP:
            protocol = PNCP_HTTP;
            break;
        case NB_NetworkProtocol_HTTPS:
            protocol = PNCP_HTTPS;
            break;
        default:
            err = PAL_ErrBadParam;
            break;
    }

    if (err == PAL_Ok)
    {
        err = PAL_NetCreateConnection(pThis->pal, protocol, &pThis->conn->palConn);
        if (err == PAL_Ok)
        {
            PAL_NetConnectionConfig netCfg;

            nsl_memset(&netCfg, 0, sizeof(netCfg));
            netCfg.netStatusCallback = PalNetStatusNotifyCallback;
            netCfg.netDataSentCallback = PalNetDataSentNotifyCallback;

            if (istcp((NB_NetworkProtocol)protocol))
            {
                netCfg.netDataReceivedCallback = PalNetTcpDataReceivedNotifyCallback;
            }
            else if (PNCP_HTTP == protocol || PNCP_HTTPS == protocol)
            {
                netCfg.netHttpDataReceivedCallback = PalNetHttpDataReceivedNotifyCallback;
            }

            netCfg.netDnsResultCallback = PalNetDnsResultNotifyCallback;
            netCfg.userData = pThis;
            COPY_TLSCONFIG(netCfg.tlsConfig, pThis->config.tlsConfig);

            err = PAL_NetOpenConnection(pThis->conn->palConn, &netCfg, pThis->config.hostname, pThis->config.port);
            if (err != PAL_Ok)
            {
                PAL_NetDestroyConnection(pThis->conn->palConn);
                pThis->conn->palConn = NULL;
            }
        }
    }
}

static void
triage_timer(PAL_Instance* pal, void* arg, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired)
    {
        CSL_Network* pThis = arg;

        //DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("net: triage_timer"));

        PAL_TimerSet(pal, TRIAGE_TIMER_PERIOD * 1000, triage_timer, pThis);
        triage(pThis);
    }
}

static int
conn_open_real(CSL_Network* pThis)
{
    struct conn *nc = NULL;
    PAL_NetConnectionStatus status = PNCS_Undefined;

    NET_WRITE_QALOG_EVENT(pThis, "open");

    nsl_assert(pThis->conn == NULL);
    pThis->conn = nc = nsl_malloc(sizeof(*pThis->conn));
    if (nc == NULL)
        return (0);

    nc->palConn = 0;
    csl_bqinit(&nc->rq);
    csl_bqinit(&nc->wq);

    nc->nextid = 0;
    nc->ready = 0;
    nc->emptysince = nc->lastwrite = nc->lastread = nettime();
    nsl_memset(&nc->pkt, 0, sizeof(nc->pkt));
    nc->connecttime = 0;
    conn_open_pal(pThis);
    PAL_TimerSet(pThis->pal, TRIAGE_TIMER_PERIOD * 1000, triage_timer, pThis);

    if (pThis && pThis->conn && pThis->conn->palConn)
    {
        status = PAL_NetGetStatus(pThis->conn->palConn);
    }

    return (status == PNCS_Created ||
            status == PNCS_Initialized ||
            status == PNCS_Connecting ||
            status == PNCS_Resolving ||
            status == PNCS_Connected ||
            status == PNCS_ProxyAuthRequired);
}

static void
conn_open(CSL_Network* pThis)
{
    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("net: conn_open %d", nettime()));

    if (!conn_open_real(pThis))
    {
        conn_fini(pThis, FALSE);
    }
}

static void
triage(CSL_Network* pThis)
{
    NB_NetworkQuery* nq = NULL;
    NB_NetworkQuery* tnq = NULL;
    nb_unixTime curtime = nettime();
    struct conn *nc = pThis->conn;

    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("net: triage - curtime=%d", curtime));
    if (STAILQ_EMPTY(&pThis->queries))
    {
        if (pThis->conn != NULL)
        {
            if (pThis->conn->emptysince == -1)
            {
                pThis->conn->emptysince = curtime;
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("net: triage - setting emptysince=%d", pThis->conn->emptysince));
            }

            if (curtime >= (pThis->conn->emptysince + pThis->config.triageTimeoutSeconds))
            {
                conn_fini(pThis, FALSE);
            }
        }
        else
        {
            PAL_TimerCancel(pThis->pal, triage_timer, pThis);
        }

        return;
    }
    else if (pThis->conn != NULL)
    {
        pThis->conn->emptysince = -1;
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("net: triage - setting emptysince=%d",
                                               pThis->conn->emptysince));
    }

    if (pThis->conn == NULL)
    {
        conn_open(pThis);
        return;
    }
    else if (!pThis->conn->ready)
    {
        return;
    }

    nb_boolean checkTimeStamp = FALSE;
    STAILQ_FOREACH_SAFE(nq, &pThis->queries, list, tnq)
    {
        if (nq->expecting)
        {
            checkTimeStamp = TRUE;
            break;
        }
    }

    /* haven't receive any data in specified time, report timed_out for current query, and
       reset network */
    if (checkTimeStamp &&
        curtime >= (pThis->conn->lastread + pThis->config.triageTimeoutSeconds))
    {
        STAILQ_FOREACH_SAFE(nq, &pThis->queries, list, tnq)
        {
            failwith(pThis, nq, TIMEDOUT, NE_OK);
        }

        DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("net: triage - closing connection"));
        conn_fini(pThis, FALSE);
        return;
    }

    STAILQ_FOREACH_SAFE(nq, &pThis->queries, list, tnq)
    {
        if (!nq->expecting)
        {
            if (pThis->config.protocol == NB_NetworkProtocol_HTTP ||
                pThis->config.protocol == NB_NetworkProtocol_HTTPS)
            {
                output_auth_query(pThis, nq);
            }
            else
            {
                output_query(pThis, nq);
            }
        }
    }
}

CSL_DEF NB_Error
CSL_NetworkCreate(PAL_Instance* pal, NB_NetworkConfiguration* configuration, CSL_Network** network)
{
    CSL_Network* pThis = NULL;

    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("CSL_NetworkCreate: %d", nettime()));

    if (!pal || !configuration || !configuration->iden || !network)
    {
        return NE_INVAL;
    }
    *network = NULL;

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        te_dealloc(configuration->iden);
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->pal = pal;
    pThis->connectretry = 1;     /// @todo Provide a way to change this
    nsl_strncpy(pThis->config.hostname, configuration->hostname, sizeof(pThis->config.hostname));
    pThis->config.port = configuration->port;
    pThis->config.protocol = configuration->protocol;
    pThis->config.iden = te_clone(configuration->iden);
    if (!pThis->config.iden)
    {
        nsl_free(pThis);
        return NE_NOMEM;
    }

    pThis->tl = tpslib_preload(configuration->tpslibData, configuration->tpslibLength);
    if (!pThis->tl)
    {
        te_dealloc(pThis->config.iden);
        nsl_free(pThis);
        return NE_NOMEM;
    }

    STAILQ_INIT(&pThis->queries);
    pThis->failedconnect = 0;
    pThis->qalog = 0;

    if (pThis->config.tlsConfig.name != NULL)
    {
        nsl_free((void*)pThis->config.tlsConfig.name);
        pThis->config.tlsConfig.name = NULL;
    }

    if (configuration->tlsConfig.name != NULL)
    {
        pThis->config.tlsConfig.name = nsl_strdup(configuration->tlsConfig.name);
    }
    pThis->config.tlsConfig.override = configuration->tlsConfig.override;

    /* Check the configuration triage timeout and set it to default if not valid */
    if (configuration->triageTimeoutSeconds > 0)
    {
        pThis->config.triageTimeoutSeconds = configuration->triageTimeoutSeconds;
    }
    else
    {
        pThis->config.triageTimeoutSeconds = TRIAGE_TIMEOUT_SEC;
    }

    *network = pThis;

    return NE_OK;
}

/* See header file for description */
CSL_DEF NB_Error
CSL_NetworkCreateByCopy(CSL_Network* sourceNetwork, NB_NetworkConfiguration* configuration, CSL_Network** network)
{
    NB_Error result = NE_OK;
    CSL_Network* newNetwork = NULL;

    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("CSL_NetworkCreateByCopy: %d", nettime()));

    if (!sourceNetwork || !network)
    {
        return NE_INVAL;
    }

    /* Create a new CSL_Network object by calling the function CSL_NetworkCreate. And the network
       configuration of source CSL_Network object (sourceNetwork->config) could not be used to create
       a new CSL_Network object. Because it is not complete. The parameter 'configuration' should be used.
    */
    result = CSL_NetworkCreate(sourceNetwork->pal, configuration, &newNetwork);
    if (result != NE_OK)
    {
        return result;
    }

    // Set the CSL_QaLog object for new CSL_Network object.
    CSL_NetworkSetQaLog(newNetwork, sourceNetwork->qalog);

    // Copy the notify callbacks from source CSL_Network object to new CSL_Network object.
    {
        int i = 0;
        int eventCallbackCount = sourceNetwork->eventCallbackCount;

        for (i = 0; i < eventCallbackCount; ++i)
        {
            result = CSL_NetworkAddEventNotifyCallback(newNetwork, &(sourceNetwork->eventCallbacks[i]));
            if (result != NE_OK)
            {
                CSL_NetworkDestroy(newNetwork);
                newNetwork = NULL;

                return result;
            }
        }
    }

    *network = newNetwork;

    // Notify of connection reset.
    FireNotifyEventCallback(newNetwork, NB_NetworkNotifyEvent_ConnectionReset, NULL);
    return result;
}

CSL_DEF NB_Error
CSL_NetworkDestroy(NB_Network* pThis)
{
    nb_boolean tearDownFast = FALSE;
    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("CSL_NetworkDestroy: %d", nettime()));

    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->conn)
    {
        conn_fini(pThis, tearDownFast);
    }

    PAL_TimerCancel(pThis->pal, triage_timer, pThis);
    while (!STAILQ_EMPTY(&pThis->queries))
    {
        failwith(pThis, STAILQ_FIRST(&pThis->queries), CANCELLED, NE_OK);
    }

    if (pThis->config.iden)
    {
        te_dealloc(pThis->config.iden);
    }

    if (pThis->tl)
    {
        tpslib_dealloc(pThis->tl);
    }
    nsl_free(pThis);

    if (pThis->config.tlsConfig.name != NULL)
    {
        nsl_free((void*)pThis->config.tlsConfig.name);
        pThis->config.tlsConfig.name = NULL;
    }

    return NE_OK;
}

CSL_DEF void
CSL_NetworkRequest(CSL_Network* pThis, NB_NetworkQuery* request)
{
    NB_NetworkResponse nr;
    tpselt qe, qeh, ce;
    boolean fireCallback = FALSE;
    char dbgtarget[64];
    tpselt invocationContext = NULL;

    if (!pThis || !request)
    {
        goto fail;
    }

    if (request->targetlen == -1)
    {
        request->targetlen = (int)nsl_strlen(request->target);
    }

    nsl_memset(dbgtarget, 0, sizeof(dbgtarget));
    nsl_strlcpy(dbgtarget, "query: ", sizeof(dbgtarget));
    nsl_memcpy(dbgtarget + 7, request->target, MIN((int)(sizeof(dbgtarget) - nsl_strlen(dbgtarget)), request->targetlen));
    NET_WRITE_QALOG_EVENT(pThis, dbgtarget);

    qe = te_new("query");
    qeh = te_new("query-headers");
    if (!qe || !qeh)
    {
        if(qe) te_dealloc(qe);
        if(qeh) te_dealloc(qeh);
        goto fail;
    }

    ce = te_clone(te_getchild(pThis->config.iden, "formatted-iden-parameters"));
    if (!ce || !te_attach(qeh, ce))
    {
        te_dealloc(qe);
        te_dealloc(qeh);
        if(ce) te_dealloc(ce);
        goto fail;
    }

    // For include the invocation context
    if (request->invocation && request->invocation->vt)
    {
        invocationContext = request->invocation;
    }
    else
    {
        invocationContext = GetDefaultInvocationContext(pThis);
    }

    if (!invocationContext || !te_attach(qeh, invocationContext))
    {
        te_dealloc(qeh);
        te_dealloc(qe);
        goto fail;
    }

    if (!te_attach(qe, qeh))
    {
        te_dealloc(qe);
        te_dealloc(qeh);
        goto fail;
    }

    if (!te_attach(qe, request->query))
    {
        te_dealloc(qe);
        goto fail;
    }

    if (STAILQ_EMPTY(&pThis->queries))
    {
        fireCallback = TRUE;
    }

    request->query = qe;

#if !defined(NDEBUG) && defined(DUMP_ALL_NETWORK_TRAFFIC)
    DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("DUMP_ALL_NETWORK net_query"));
    dumpelt(request->query, 0);
#endif

    STAILQ_INSERT_TAIL(&pThis->queries, request, list);
    request->expecting = 0;
    request->lastTime = nettime();

    if (fireCallback)
    {
        nb_boolean value = TRUE;
        FireNotifyEventCallback(pThis, NB_NetworkNotifyEvent_DataPending, &value);
    }

    triage(pThis);
    return;

fail:
    nr.status = FAILED;
    if (request)
    {
        request->action(request, &nr);
    }
}

CSL_DEF void
CSL_NetworkCancel(CSL_Network* pThis, NB_NetworkQuery* query)
{
    failwith(pThis, query, CANCELLED, NE_OK);
    triage(pThis);
}

CSL_DEF void
CSL_NetworkFreeResponse(NB_NetworkResponse* response)
{
    if (response && (response->status == COMPLETE || response->status == RESP_ERROR) && response->tpsdoc != NULL)
    {
        te_dealloc(response->tpsdoc);
        response->tpsdoc = NULL;
    }
}

CSL_DEF void
CSL_NetworkSetQaLog(CSL_Network* pThis, CSL_QaLog* qalog)
{
    if (pThis)
    {
        pThis->qalog = qalog;
    }
}

CSL_DEF NB_Error
CSL_NetworkAddEventNotifyCallback(CSL_Network* pThis, NB_NetworkNotifyEventCallback* callback)
{
    if (!pThis || !callback)
    {
        return NE_INVAL;
    }

    if (pThis->eventCallbackCount == MAX_EVENT_CALLBACKS)
    {
        return NE_RANGE;
    }

    pThis->eventCallbacks[pThis->eventCallbackCount] = *callback;
    pThis->eventCallbackCount++;

    return NE_OK;
}

CSL_DEF NB_Error
CSL_NetworkRemoveEventNotifyCallback(CSL_Network* pThis, NB_NetworkNotifyEventCallback* callback)
{
    int i = 0;
    nb_boolean found = FALSE;

    for (i = 0; i < MAX_EVENT_CALLBACKS && i < pThis->eventCallbackCount; i++)
    {
        if (!found && pThis->eventCallbacks[i].callback == callback->callback && pThis->eventCallbacks[i].callbackData == callback->callbackData)
        {
            found = TRUE;
            pThis->eventCallbackCount--;
        }

        if (found && i + 1 < MAX_EVENT_CALLBACKS)
        {
            pThis->eventCallbacks[i] = pThis->eventCallbacks[i + 1];
        }
    }

    return found ? NE_OK : NE_NOENT;
}

CSL_DEF NB_Error
CSL_NetworkSuspend(CSL_Network* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    return NE_NOSUPPORT;
}

CSL_DEF NB_Error
CSL_NetworkResume(CSL_Network* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    return NE_NOSUPPORT;
}

/* See header file for description */
CSL_DEF NB_Error
CSL_NetworkSetIden(CSL_Network* pThis, NB_NetworkConfiguration* configuration)
{
    tpselt newIden = NULL;

    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("CSL_NetworkSetIden: %d", nettime()));

    if (!pThis || !configuration || !(configuration->iden))
    {
        return NE_INVAL;
    }

    // Clone the 'iden' of parameter 'configuration'.
    newIden = te_clone(configuration->iden);
    if (!newIden)
    {
        return NE_NOMEM;
    }

    // Destroy current 'iden' if it is not NULL.
    if (pThis->config.iden)
    {
        te_dealloc(pThis->config.iden);
        pThis->config.iden = NULL;
    }

    // Set the 'iden'.
    pThis->config.iden = newIden;

    return NE_OK;
}

/* See header file for description */
CSL_DEF NB_Error
CSL_NetworkGetClientGuid(CSL_Network* pThis,
                         NB_NetworkNotifyEventClientGuid* clientGuid)
{
    char* guid = 0;
    size_t guidLength = 0;
    tpselt fi, ui;

    if ((!pThis) || (!clientGuid))
    {
        return NE_INVAL;
    }

    // Get the client guid from iden.
    fi = te_getchild(pThis->config.iden, "formatted-iden-parameters");
    if (fi != NULL)
    {
        ui = te_getchild(fi, "user-info");
        if (ui)
        {
            if ((te_getattr(ui, "client-guid", &guid, &guidLength)) && (guidLength != 0))
            {
                // Return the client guid.
                clientGuid->guidData = (const uint8*) guid;
                clientGuid->guidDataLength = (nb_size) guidLength;
                return NE_OK;
            }
        }
    }
    return NE_NOENT;
}

NB_Error
NetWriteQaLogEvent(CSL_Network* pThis, const char* event, const char* function, const char* file, int line)
{
    NB_Error err = NE_OK;

    if (pThis && pThis->qalog)
    {
        CSL_QaRecord* record = 0;
        err = CSL_QaRecordCreate(gpstime(), RECORD_ID_NET_EVENT, 250, &record);
        if (!err)
        {
            err = err ? err : CSL_QaRecordWriteText(record, event, MAX_FILE_NAME_LEN);
            err = err ? err : CSL_QaRecordWriteFileLocation(record, function, file, line);

            err = err ? err : CSL_QaLogWrite(pThis->qalog, record);

            CSL_QaRecordDestroy(record);
        }
    }

    return err;
}

NB_Error
WriteQaLogDnsRecord(CSL_Network* pThis, uint16 recordID, PAL_Error errorCode,
                    const char* hostName, uint32 const* const* addressList, uint32 addressCount)
{
    NB_Error err = NE_OK;
    uint16 lowerBound = recordID == RECORD_ID_DNS_RESULT ? 0 : IPV4_DNS_RESULT_ADDRESSES;
    uint16 upperBound = recordID == RECORD_ID_DNS_RESULT ? IPV4_DNS_RESULT_ADDRESSES : MAX_DNS_RESULT_ADDRESSES;

    // BREW error codes currently used for qa log
#define QA_LOG_AEE_NET_ETIMEDOUT   18
#define QA_LOG_AEE_NET_BADDOMAIN   50
#define QA_LOG_AEE_NET_UNKDOMAIN   51
#define QA_LOG_AEE_NET_BADRESPONSE 52

    if (pThis && pThis->qalog)
    {
        CSL_QaRecord* record = 0;
        err = CSL_QaRecordCreate(gpstime(), recordID, 250, &record);
        if (!err)
        {
            uint32 n = 0;

            err = err ? err : CSL_QaRecordWriteText(record, hostName, HOST_NAME_LEN);

            if (errorCode == PAL_Ok)
            {
                err = err ? err : CSL_QaRecordWriteUint32(record, addressCount);
            }
            else
            {
                uint32 qaLogErrorCode = QA_LOG_AEE_NET_BADRESPONSE;
                switch (errorCode)
                {
                    case PAL_ErrNetTimeout:
                        qaLogErrorCode = QA_LOG_AEE_NET_ETIMEDOUT;
                        break;
                    case PAL_ErrNetDnsMalformedHostName:
                        qaLogErrorCode = QA_LOG_AEE_NET_BADDOMAIN;
                        break;
                    case PAL_ErrNetDnsUnknownHostName:
                        qaLogErrorCode = QA_LOG_AEE_NET_UNKDOMAIN;
                        break;
                    case PAL_ErrNetGeneralFailure:
                    default:
                        qaLogErrorCode = QA_LOG_AEE_NET_BADRESPONSE;
                        break;
                }
                err = err ? err : CSL_QaRecordWriteUint32(record, qaLogErrorCode);
            }

            for (n = lowerBound; n < upperBound && err == NE_OK; n++)
            {
                if (recordID == RECORD_ID_DNS_RESULT)     // This is IPv4
                {
                    err = err ? err : ((n < addressCount && addressList[n])  ?
                                       CSL_QaRecordWriteUint32(record, addressList[n][0]) :
                                       CSL_QaRecordWriteUint32(record, 0));
                }
                else
                {
                    char ipv6Address[IPV6_ADDRESS_LEN] = {0};
                    byte* address = (byte*)addressList[n];
                    uint8 i = 0;
                    for (i = 0; i < 4 * sizeof(uint32); i++)
                    {
                        char tmp[6] = {0};
                        byte part1 = (n < addressCount && addressList[n]) ? address[i] : 0;
                        byte part2 = (n < addressCount && addressList[n]) ? address[++i] : 0;
                        nsl_sprintf(tmp, "%2.2X%2.2X:", part1, part2);
                        nsl_strlcat(ipv6Address, tmp, IPV6_ADDRESS_LEN);
                    }
                    err = err ? err : CSL_QaRecordWriteText(record, ipv6Address, IPV6_ADDRESS_LEN);
                }
            }

            err = err ? err : CSL_QaLogWrite(pThis->qalog, record);

            CSL_QaRecordDestroy(record);
        }
    }

    return err;
}

NB_Error
NetWriteQaLogDnsResult(CSL_Network* pThis, PAL_Error errorCode, const char* hostName, uint32 const* const* addressList, uint32 addressCount)
{
    NB_Error err = WriteQaLogDnsRecord(pThis, RECORD_ID_DNS_RESULT, errorCode, hostName, addressList, addressCount);
    err = err ? err : WriteQaLogDnsRecord(pThis, RECORD_ID_DNS_IPV6_RESULT, errorCode, hostName, addressList, addressCount);

    return err;
}

NB_Error
NetWriteQaLogQueryReplyExtended(CSL_Network* pThis, uint16 id, const char* name, int queryId, int size, NB_NetworkResponseStatus status)
{
    NB_Error err = NE_OK;

    if (pThis && pThis->qalog)
    {
        CSL_QaRecord* record = 0;
        err = CSL_QaRecordCreate(gpstime(), id, 250, &record);
        if (!err)
        {
            err = err ? err : CSL_QaRecordWriteText(record, name, MAX_FILE_NAME_LEN);
            err = err ? err : CSL_QaRecordWriteUint32(record, queryId);
            err = err ? err : CSL_QaRecordWriteUint32(record, size);
            err = err ? err : CSL_QaRecordWriteUint32(record, status);

            err = err ? err : CSL_QaLogWrite(pThis->qalog, record);

            CSL_QaRecordDestroy(record);
        }
    }

    return err;
}

NB_Error
NetWriteQaLogErrorExtended(CSL_Network* pThis, const char* name, int queryId, NB_NetworkResponseStatus status, NB_Error error)
{
    NB_Error err = NE_OK;

    if (pThis && pThis->qalog)
    {
        CSL_QaRecord* record = 0;
        err = CSL_QaRecordCreate(gpstime(), RECORD_ID_NETWORK_ERROR_EXTENDED2, 250, &record);
        if (!err)
        {
            err = err ? err : CSL_QaRecordWriteText(record, name, MAX_FILE_NAME_LEN);
            err = err ? err : CSL_QaRecordWriteUint32(record, queryId);
            err = err ? err : CSL_QaRecordWriteUint32(record, status);
            err = err ? err : CSL_QaRecordWriteUint32(record, error);

            err = err ? err : CSL_QaLogWrite(pThis->qalog, record);

            CSL_QaRecordDestroy(record);
        }
    }

    return err;
}

// iden procedure might return different kind of errors(e.g. rate limit).
// We need to cancel all pending requests on iden error, so we passes the error to caller.
uint32 HandleIdenReceipt(CSL_Network* pThis, tpselt receipt)
{
    tpselt ce = 0;
    uint32 result = 0;

#if defined(DUMP_ALL_NETWORK_TRAFFIC)
    DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("DUMP_ALL_NETWORK iden-receipt"));
    dumpelt(receipt, 0);
#endif

    ce = te_nextchild(receipt, NULL);
    if (ce)
    {
        if (nsl_strcmp(te_getname(ce), "error") == 0)
        {
            NB_NetworkNotifyEventError error = { 0 };
            const char* code = te_getattrc(ce, "code");

            if (code)
            {
                error.code = (uint32)nsl_strtoul(code, NULL, 10);
                result = error.code;
            }
            error.description = te_getattrc(ce, "description");

            FireNotifyEventCallback(pThis, NB_NetworkNotifyEvent_IdenError, &error);
        }
        else
        {
            // Should never get here
            nsl_assert(FALSE);
        }
    }
    else
    {
        char* guid = 0;
        size_t guidLength = 0;
        tpselt fi, ui;

        NB_NetworkNotifyEventClientGuid callbackGuid = { 0 };

        if (te_getattr(receipt, "client-guid", &guid, &guidLength))
        {
            tpselt ce = te_getchild(pThis->config.iden, "want-client-guid-assigned");
            /* This is used for a special condition if the protocol is HTTP or HTTPS. Every request is
               sent with the single IDEN in the network configuration. If there is no client-guid in the
               configuration, the child want-client-guid-assigned is sent with every requests. If one
               request is returned, want-client-guid-assigned is unlinked from the single IDEN.

               @todo: Should the pending requests wait for the IDEN request?
            */
            if (ce)
            {
                // Remove the element asking for a guid to be assigned
                te_unlink(ce);
                te_dealloc(ce);

                // Add the guid to the iden
                fi = te_getchild(pThis->config.iden, "formatted-iden-parameters");
                if (fi != NULL)
                {
                    ui = te_getchild(fi, "user-info");
                    if (ui)
                    {
                        te_setattr(ui, "client-guid", guid, guidLength);

                        // Send guid to listeners
                        callbackGuid.guidData = (const uint8*)guid;
                        callbackGuid.guidDataLength = (nb_size)guidLength;
                        FireNotifyEventCallback(pThis, NB_NetworkNotifyEvent_ClientGuidAssigned, &callbackGuid);
                    }
                }
            }
        }
    }

    return result;
}

static void FireNotifyEventCallback(CSL_Network* pThis, NB_NetworkNotifyEvent event, void* data)
{
    int i = 0;

    for (i = 0; i < pThis->eventCallbackCount; i++)
    {
        if (pThis->eventCallbacks[i].callback)
        {
            (pThis->eventCallbacks[i].callback)(event, data, pThis->eventCallbacks[i].callbackData);
        }
    }
}

NB_Error
GetServerErrorCodeFromTps(tpselt te)
{
    NB_Error error = NE_UNEXPECTED;

    if (te)
    {
        error = (NB_Error)nsl_atoi(te_getattrc(te, "code"));
    }

    return error;
}


tpselt GetDefaultInvocationContext(CSL_Network* pThis)
{
    static const char* children[] = { "source-module", "screen-id", "input-source",
                                      "invocation-method", NULL };
    const char** name = NULL;
    if (!pThis->defaultInvocationContext)
    {
        pThis->defaultInvocationContext = te_new("invocation-context");
        if (!pThis->defaultInvocationContext)
        {
            return NULL;
        }

        name = &children[0];
        while (*name)
        {
            te_setattrc(pThis->defaultInvocationContext, *name, "default");
            name++;
        }
    }

    return te_clone(pThis->defaultInvocationContext);
}

NB_TransactionInfo* TransactionInfoCreate()
{
    static NB_TransactionInfo transactionEvent = {0};
    return &transactionEvent;
}

void TransactionInfoDestroy(NB_TransactionInfo* transactionEvent)
{
    if (transactionEvent)
    {
        if (transactionEvent->query_name)
        {
            nsl_free((void*)transactionEvent->query_name);
        }
        if (transactionEvent->target_name)
        {
            nsl_free((void*)transactionEvent->target_name);
        }
        memset(transactionEvent, 0, sizeof(NB_TransactionInfo));
    }
}

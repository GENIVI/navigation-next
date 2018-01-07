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

@file cslnetwork.h
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

/*
* Interaction with the Tesla server.
*
* Supports the mux and TPS protocols.
*/

#ifndef CSLNETWORK_H
#define	CSLNETWORK_H

#include "paltypes.h"
#include "pal.h"
#include "tpselt.h"
#include "bsdqueue.h"
#include "cslqalog.h"
#include "cslutil.h"
#include "nbnetworkconfiguration.h"

/*!
@}
@addtogroup cslnetwork
@{
*/

typedef struct NB_TransactionInfo
{
    const char*         transaction_id;
    const char*         target_name;
    const char*         query_name;
    uint32              transaction_time;
    uint32              total_processing_time;
    uint32              query_packing_time;
    uint32              reply_unpacking_time;
} NB_TransactionInfo;

typedef enum
{
    NONE,
    COMPLETE,
    CANCELLED,
    FAILED,
    TIMEDOUT,
    RESP_ERROR
} NB_NetworkResponseStatus;

typedef struct NB_NetworkResponse
{
    NB_NetworkResponseStatus    status;
    /* The following are only valid if status is COMPLETE or REPLY_ERROR. */
    const char*                 source;
    size_t                      sourcelen;
    tpselt                      reply;
    tpselt                      reply_headers;
    tpselt                      tpsdoc;

    /* Following fields should be set only when wantBinaryChunk is set */
    const char*                 binaryChunk;
    int                         chunkLength;
} NB_NetworkResponse;

/* Query flags. */
#define	TN_QF_DEFLATE		    (1 << 0)
/* Reply flags. */
#define	TN_RF_DEFLATE		    (1 << 0)

/* Failure flags */
#define TN_FF_NOCLOSE		    (0)
#define TN_FF_CLOSE_TIMEOUT	    (1 << 0)
#define TN_FF_CLOSE_ERROR	    (1 << 1)
#define TN_FF_CLOSE_ANY		    (TN_FF_CLOSE_TIMEOUT | TN_FF_CLOSE_ERROR)

typedef struct NB_NetworkQuery NB_NetworkQuery;

struct NB_NetworkQuery
{
    void (*action)(NB_NetworkQuery*, NB_NetworkResponse*);
    void (*progress)(NB_NetworkQuery*, boolean up, uint32 have, uint32 total);

    /* invocation-context */
    tpselt invocation;

    uint32  query_packing_time;

    /* Query parameters. */
    tpselt query;
    char target[64];
    int targetlen;
    int qflags;

    int failFlags;

    /* Internal processing status. */
    STAILQ_ENTRY(NB_NetworkQuery) list;
    int expecting;
    int sentid;
    size_t packetlen;		/* the length of this packet */
    size_t pendingdata;		/* the amount of data that must be written
                            before this packet has been sent (i.e.
                            the amount of data pending before this
                            packet is written + the length of this
                            packet. */
    nb_unixTime lastTime;   /* The last time that data associated with
                            the query was sent or received */
    nb_boolean wantBinaryChunk; /* If this flag is set, cslnetwork should not
                                unpack tps binary chunk, but return binary
                                chunk to caller directly.*/
};

typedef struct CSL_Network CSL_Network;

/*! Construct a CSL_Network instance.

@param pal A PAL_Instance
@param configuration The configuration settings for this network
@param network On success, a newly created CSL_Network object.  A valid object must be destroyed with CSL_NetworkDestroy
@return NB_Error
@see CSL_NetworkDestroy
*/
CSL_DEC NB_Error CSL_NetworkCreate(PAL_Instance* pal, NB_NetworkConfiguration* configuration, CSL_Network** network);

/*! Create a CSL_Network instance by a copy.

@param sourceNetwork Source CSL_Network object to copy
@param configuration The configuration settings for this network
@param network On success, returns pointer to new CSL_Network object
@return NE_OK if success
@see CSL_NetworkDestroy
*/
CSL_DEC NB_Error CSL_NetworkCreateByCopy(CSL_Network* sourceNetwork, NB_NetworkConfiguration* configuration, CSL_Network** network);

/*! Destroy a CSL_Network instance
@param network The network instance to destroy
@return NB_Error
*/
CSL_DEC NB_Error CSL_NetworkDestroy(CSL_Network* network);

/*! Issue a request over the network.

@param network Pointer to a CSL_Network interface created with ::CSL_NetworkCreate.
@param query Object describing the request to be issued.
*/
CSL_DEC void		CSL_NetworkRequest(CSL_Network* network, NB_NetworkQuery* query);

/*! Cancel an existing network request created with CSL_NetworkRequest.

@param network Pointer to a CSL_Network interface created with ::CSL_NetworkCreate.
@param query Object describing the request to be canceled.
*/
CSL_DEC void		CSL_NetworkCancel(CSL_Network* network, NB_NetworkQuery* query);

/*! Free response data from a completed query.

@param response Pointer to a network query response to be freed.
*/
CSL_DEC void        CSL_NetworkFreeResponse(NB_NetworkResponse* response);

/*! Set the QA Log instance
@param network A CSL_Network instance
@param qalog A QA Log instance
*/
CSL_DEC void        CSL_NetworkSetQaLog(CSL_Network* network, CSL_QaLog* qalog);

/*! Add a notify event callback
@param network A CSL_Network instance
@param callback A pointer to a NB_NetworkNotifyEventCallback that defines the callback to invoke
@return NB_Error
*/
CSL_DEC NB_Error    CSL_NetworkAddEventNotifyCallback(CSL_Network* network, NB_NetworkNotifyEventCallback* callback);

/*! Remove a previously added notify event callback
@param network A CSL_Network instance
@param callback A pointer to a NB_NetworkNotifyEventCallback that defines the callback to invoke.
The contents of this structure must exactly match the contents of a structure previously passed to CSL_NetworkAddEventNotifyCallback
@return NB_Error
*/
CSL_DEC NB_Error    CSL_NetworkRemoveEventNotifyCallback(CSL_Network* network, NB_NetworkNotifyEventCallback* callback);

/*! Temporarily suspend the network
@param network A CSL_Network instance
@return NB_Error
*/
CSL_DEC NB_Error    CSL_NetworkSuspend(CSL_Network* network);

/*! Resume the network after a suspend
@param network A CSL_Network instance
@return NB_Error
*/
CSL_DEC NB_Error    CSL_NetworkResume(CSL_Network* network);

/*! Set the 'iden' of the 'CSL_Network' instance

@param network The 'CSL_Network' instance
@param configuration The configuration settings for this network

@return NB_Error
*/
CSL_DEC NB_Error CSL_NetworkSetIden(CSL_Network* network, NB_NetworkConfiguration* configuration);

/*! Get the client guid from the context

This function cannot be called too frequently. Because it gets the client guid
from a dictionary.

@return NE_OK if success, NE_NOENT if the client guid has not been assigned.
*/
CSL_DEC NB_Error
    CSL_NetworkGetClientGuid(CSL_Network* network,                      /*!< Network instance to get the client guid */
    NB_NetworkNotifyEventClientGuid* clientGuid/*!< On return client guid if success */
    );

/*! @} */

#endif

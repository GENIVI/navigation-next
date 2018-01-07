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

@file nbnetwork.h
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

#ifndef NBNETWORK_H
#define NBNETWORK_H

#include "paltypes.h"
#include "palnet.h"

/*!
@addtogroup nbnetwork
@{
*/

/*! Network protocol to use for a network instance
*/
typedef enum
{
    NB_NetworkProtocol_Undefined = 0,
    NB_NetworkProtocol_TCP,
    NB_NetworkProtocol_HTTP,
    NB_NetworkProtocol_HTTPS,
    NB_NetworkProtocol_TCPTLS,
} NB_NetworkProtocol;

/*! Current status of a network instance
*/
typedef enum
{
    NB_NetworkRequestStatus_Success,
    NB_NetworkRequestStatus_Canceled,
    NB_NetworkRequestStatus_Failed,
    NB_NetworkRequestStatus_TimedOut,
    NB_NetworkRequestStatus_Progress
} NB_NetworkRequestStatus;

/*! Network event notification
*/
typedef enum
{
    NB_NetworkNotifyEvent_ConnectionOpened = 0,
    NB_NetworkNotifyEvent_ConnectionClosed,
    NB_NetworkNotifyEvent_DataPending,
    NB_NetworkNotifyEvent_IdenError,
    NB_NetworkNotifyEvent_ClientGuidAssigned,
    NB_NetworkNotifyEvent_ConnectionError,
    NB_NetworkNotifyEvent_ConnectionFailed,
    NB_NetworkNotifyEvent_TransactionInfo,
    NB_NetworkNotifyEvent_ConnectionReset
} NB_NetworkNotifyEvent;

typedef struct CSL_Network NB_Network;

typedef struct NB_NetworkConfiguration NB_NetworkConfiguration;

/*! Network notify callback function

The network notify callback is invoked whenever "something interesting" occurs on the network.
@param event The notification event that occured
@param data Event-specific data.
This value is not used for NB_NetworkNotifyEvent_ConnectionOpened or NB_NetworkNotifyEvent_ConnectionClosed.
For NB_NetworkNotifyEvent_DataPending, data is a pointer to an nb_boolean whose value is non-zero if the client is waiting for a server request to finish; zero, otherwise.
For NB_NetworkNotifyEvent_IdenError, data is a pointer to an NB_NetworkNotifyEventError.
For NB_NetworkNotifyEvent_ClientGuidAssigned, data is a pointer to an NB_NetworkNotifyEventClientGuid.
@param userData Opaque user data registered with the callback
*/
typedef void (*NB_NetworkNotifyEventCallbackFunction)(NB_NetworkNotifyEvent event, void* data, void* userData);

/*! Defines the notify callback to invoke and specifies the user data to be supplied to the callback
*/
typedef struct
{
    NB_NetworkNotifyEventCallbackFunction   callback;
    void*                                   callbackData;
} NB_NetworkNotifyEventCallback;

/*! error information
*/
typedef struct
{
    uint32          code;               /*!< Error code */
    const char*     description;        /*!< Internal description - never display to user */
} NB_NetworkNotifyEventError;

/*! Client GUID information
*/
typedef struct
{
    const uint8*    guidData;           /*!< Server assigned GUID */
    nb_size         guidDataLength;     /*!< Length of GUID data */
} NB_NetworkNotifyEventClientGuid;

/*! @} */

#endif

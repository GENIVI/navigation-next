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

    @file     nbcontext.h

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

#ifndef NBCONTEXT_H
#define NBCONTEXT_H

#include "pal.h"
#include "paltypes.h"
#include "nberror.h"
#include "nbexp.h"
#include "nbnetwork.h"
#include "nbpersistentdata.h"
#include "cslcache.h"

/*!
    @addtogroup nbcontext
    @{
*/


/*! @struct NB_Context
The NAVBuilder context maintains the overall state of the library
*/
typedef struct NB_Context NB_Context;


typedef void (*NB_ContextCallbackFunction)(void* userData);

typedef struct
{
    NB_ContextCallbackFunction    callback;
    void*                         callbackData;
} NB_ContextCallback;


typedef struct 
{
    char* cacheName;                                /*!< Name of cache, will be used as the directory name for the cache */
    uint32  maximumItemsInMemoryCache;              /*!< Maximum number of items in memory cache */
    uint32  maximumItemsInPersistentCache;          /*!< Maximum number of items in persistent (file or database) cache */
    byte* obfuscateKey;
    size_t obfuscateKeySize;
    CSL_CacheConfig* configuration;                 /*!< Optional. Set to NULL to use file-cache as persistent cache. */

} NB_CacheConfiguration;

/*! Create NAVBuilder context object

@param pal An instance of a Platform Adaptation Layer (PAL) object
@param network A NetworkConfiguration object used to create and initialize the network connection for this context
@param callback Optional callback to receive network event notifications
@param voiceCacheConfiguration Configuration for voice cache. Optional, set to NULL if not used.
@param rasterTileCacheConfiguration Configuration for raster tile manager cache. Optional, set to NULL if not used.

@param persistentData A persistent data object used to allow calling app to serialize data for 
                      use across sessions.  The object will be destroyed during NB_ContextDestroy().
                      This parameter is currently required.
                      @todo: (BUG 55773) Make the raster tile manager cache optional.

@param context On success, a newly created NB_Context object; NULL otherwise.  A valid object must be destroyed using NB_ContextDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_ContextCreate(PAL_Instance* pal, 
                                 NB_NetworkConfiguration* network, 
                                 NB_NetworkNotifyEventCallback* callback, 
                                 NB_CacheConfiguration* voiceCacheConfiguration, 
                                 NB_CacheConfiguration* rasterTileCacheConfiguration, 
                                 NB_PersistentData* persistentData,
                                 NB_Context** context);

/*! Destroy a previously created Context object

@param context A NB_Context object created with NB_ContextCreate
@returns NB_Error
*/
NB_DEC NB_Error NB_ContextDestroy(NB_Context* context);

/*! Resets the network object in the NB_Context object

@param context A NB_Context object created with NB_ContextCreate
@param network A NetworkConfiguration object used to create and initialize the network connection for this context
@returns NB_Error
*/
NB_DEC NB_Error NB_ContextNetworkReset(NB_Context* context, NB_NetworkConfiguration* networkConfig, NB_NetworkNotifyEventCallback* callback);

/*! Set the 'iden' of the network object in the NB_Context object

This function should be called when some info of 'iden' is changed and the event of 'NB_NetworkNotifyEvent_ConnectionClosed' is received.
User could receive the event of 'NB_NetworkNotifyEvent_ConnectionClosed' in 'NB_NetworkNotifyEventCallback'. So this function should be
called in 'NB_NetworkNotifyEventCallback'.

@param context A NB_Context object created with NB_ContextCreate
@param networkConfig A NetworkConfiguration object contained the 'iden'

@returns NB_Error
*/
NB_DEC NB_Error NB_ContextNetworkSetIden(NB_Context* context, NB_NetworkConfiguration* networkConfig);

/*! Return Iden error

@param context A NB_Context object created with NB_ContextCreate
@returns NB_Error
*/
NB_DEC NB_Error NB_ContextGetIdenError(NB_Context* context);

/*! Initialize the generic HTTP download manager in the NB_Context object

    @todo: This is temporary method for the generic HTTP download manager until integrating
           the 'ContextBaseSingleton' from hybrid.

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_ContextInitializeGenericHttpDownloadManager(NB_Context* context,                 /*!< Pointer to current context */
                                               uint32 httpDownloadConnectionCount   /*!< Count of connections for the generic
                                                                                         HTTP download manager */
                             );

/*! Calls Master Clear for all the objects that have registered callback with NB_Context

@param context A NB_Context object created with NB_ContextCreate
@returns NB_Error
*/
NB_DEC NB_Error
NB_ContextSendMasterClear(NB_Context* context /*!< Pointer to current context */);

/*! Register NB_Context callback function

@param callback function pointer
@returns NB_Error
@The callback will be triggered in NB_ContextDestroy.
@After sending callback list, NB_Context instance will remove all registered callbacks
*/
NB_DEC NB_Error NB_ContextRegisterCallback(NB_Context* context, const NB_ContextCallback* callback);

/*! Set locale setting to NB_Context.

@param context A NB_Context object created with NB_ContextCreate.
@param locale The locale setting
@returns NB_Error
*/
NB_DEC NB_Error NB_ContextSetLocale(NB_Context* context, const char* local);

/*! Get locale setting from NB_Context.

@param context A NB_Context object created with NB_ContextCreate.
@returns locale setting const char pointer.
*/
NB_DEC const char* NB_ContextGetLocale(NB_Context* context);

/*! Get API key or token from from NB_Context.
 
 @param context A NB_Context object created with NB_ContextCreate.
 @returns NB_Error by filling the APIkey ot token value in the cred pointer.
 */

NB_DEC NB_Error NB_ContextGetCredential(NB_Context* context, char **cred);

/*! @} */

#endif

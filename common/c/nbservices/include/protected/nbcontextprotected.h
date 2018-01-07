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

    @file     nbcontextprotected.h
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

#ifndef NBCONTEXTPROTECTED_H
#define NBCONTEXTPROTECTED_H


#include "cslqalog.h"
#include "nbcontext.h"
#include "datautil.h"
#include "nbanalytics.h"
#include "data_search_cookie.h"
#include "nbwifiprobes.h"
#include "paltestlog.h"

// @todo (BUG 55774) finalize how thread validity checking should be done
#if 0
#define NB_ASSERT_VALID_THREAD(context) (void)PAL_AssertPalThread(NB_ContextGetPal(context))
#else
#define NB_ASSERT_VALID_THREAD(context)
#endif


#ifndef NDEBUG
#define NB_ASSERT_CCC_THREAD(context)                                   \
    do                                                                  \
    {                                                                   \
        if (!PAL_IsPalThread(NB_ContextGetPal(context)))                \
        {                                                               \
            PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,              \
                        "Check thread -- %s(%d) - %s returns false",    \
                        __FILE__, __LINE__,  __FUNCTION__);             \
            abort();                                                    \
        }                                                               \
    } while (0)
#else
#define NB_ASSERT_CCC_THREAD(context)
#endif

/*!
    @addtogroup nbcontext
    @{
*/

/*! Get the PAL instance from the context

@param context The context to get the PAL from
@returns The PAL associated with this context
*/
NB_DEC PAL_Instance* NB_ContextGetPal(NB_Context* context);


/*! Get the NB_Network instance from the context

@param context The context to get the network from
@returns The network associated with this context
*/
NB_DEC NB_Network* NB_ContextGetNetwork(NB_Context* context);


/*! Get the data_util_state from the context

@param context The context to get the data state from
@returns The data state associated with this context
*/
NB_DEC data_util_state* NB_ContextGetDataState(NB_Context* context);


/*! Associate a QA Log instance with the context

Associate a QA Log instance with the context.  Once set, ownership is
transferred to the Context and the QA Log will be destroyed during
NB_ContextDestroy.  If a QA Log has already been set and this function
is called again, the previous QA Log instance will be destroyed.  To
disable QA Logging, pass a NULL QA Log

@param context The context to set the QA Log for
@param log Optional. The QA Log to set
@returns NB_Error
*/
NB_DEC NB_Error NB_ContextSetQaLog(NB_Context* context, CSL_QaLog* log);


/*! Get the QA Log from the context.

This function can be called to check if QA logging is enabled or not. If QA logging requires additional 
calculation then this function should be called to check first if QA logging is enabled and the 
calculation is necessary or not.

@param context The context to get the QA Log from
@returns The QA Log associated with this context or NULL if QA logging is disabled
*/
NB_DEC CSL_QaLog* NB_ContextGetQaLog(const NB_Context* context);

/*! Set the last search cookie received

Sets search cookie for the context.  If a search cookie was previously
set, it will be freed.  If NULL passed in for searchCookie, previous
cookie will be cleared for subsequent calls to NB_ContextGetSearchCookie().

@param context The context to set the search cookie for
@param searchCookie The search cookie to copy and store within the context.  If NULL, previous cookie is cleared.
@returns The search cookie associated with this context
*/
NB_DEF NB_Error NB_ContextSetSearchCookie(NB_Context* context, data_search_cookie* searchCookie);


/*! Get the last search cookie received

Returns pointer to last search cookie set for the context, or NULL if none currently set.

@param context The context to get the search cookie from
@returns The search cookie associated with this context, or NULL if none currently set
*/
NB_DEF const data_search_cookie* NB_ContextGetSearchCookie(NB_Context* context);


/*! Get a target mapping from the context

@param context The context to get the QA Log from
@param target The name of the target
@returns The mapping for the target, or the original target if no mapping defined
*/
NB_DEC const char* NB_ContextGetTargetMapping(NB_Context* context, const char* target);

/*! Get the client guid from the context

    This function cannot be called too frequently. Because it gets the client guid
    from a dictionary.

    @return NE_OK if success, NE_NOENT if the client guid has not been assigned.
*/
NB_Error
NB_ContextGetClientGuid(NB_Context* context,                        /*!< Context to get the client guid */
                        NB_NetworkNotifyEventClientGuid* clientGuid /*!< On return client guid if success */
                        );

/*! Get the generic download manager from the context

    @return A pointer to the generic download manager. User should convert
            this pointer to type of 'nbmap::DownloadManager*'.
*/
void*
NB_ContextGetGenericHttpDownloadManager(NB_Context* context /*!< The context to get the generic HTTP download manager */
                                        );

/*! Register NB_Context MasterClear callback function

@param context The context to get the QA Log from
@param target The name of the target
@returns The mapping for the target, or the original target if no mapping defined
*/
NB_DEC const char* NB_ContextGetTargetMapping(NB_Context* context, const char* target);

/*! Get the client guid from the context

    This function cannot be called too frequently. Because it gets the client guid
    from a dictionary.

    @return NE_OK if success, NE_NOENT if the client guid has not been assigned.
*/
NB_Error
NB_ContextGetClientGuid(NB_Context* context,                        /*!< Context to get the client guid */
                        NB_NetworkNotifyEventClientGuid* clientGuid /*!< On return client guid if success */
                        );

/*! Get the generic download manager from the context

    @return A pointer to the generic download manager. User should convert
            this pointer to type of 'nbmap::DownloadManager*'.
*/
void*
NB_ContextGetGenericHttpDownloadManager(NB_Context* context /*!< The context to get the generic HTTP download manager */
                                        );

/*! Register NB_Context MasterClear callback function

The callback will be triggered when client calls NB_ContextSendMasterClear.
NB_Context will call MasterClear callback for the registered functions.
In NB_ContextDestroy, NB_Context instance will remove all registered callbacks

@param context An instance of NB_Context
@param callback function pointer
@returns NB_Error
*/
NB_DEC NB_Error NB_ContextRegisterMasterClearCallback(NB_Context* context, const NB_ContextCallback* callback);

/*! Unregister NB_Context MasterClear callback function

 @param context An instance of NB_Context
 @param callback function pointer
 @returns NB_Error
 */
NB_DEC NB_Error NB_ContextUnregisterMasterClearCallback(NB_Context* context, const NB_ContextCallback* callback);

/*! Update active session of this context.

  @param context An instance of NB_Context
  @param sessionId new active session id to be used by this context.
  @param length length of sessionId.
  @returns None
*/
NB_DEC void
NB_ContextUpdateActiveSession(NB_Context* context, const char* sessionId, uint32 length);


/*! Register cache path for this context.

  The registered cache will be removed completely after master clear, don't call this
  interface if you want to clear it yourself.

  @param context An instance of NB_Context
  @param path Null terminated path of cache.
  @returns None
*/
NB_DEC void
NB_ContextAddCachePath(NB_Context* context, const char* path);

/*! @} */

#endif

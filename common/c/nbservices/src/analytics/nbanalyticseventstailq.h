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

    @file   nbanalyticseventstailq.h
    @brief  Helper data structure and functions for handling Analytics events
            BSD tailq.
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

#ifndef NBANALYTICSEVENTSTAILQ_H_
#define NBANALYTICSEVENTSTAILQ_H_

#include "bsdqueue.h"
#include "datautil.h"
#include "data_analytics_event.h"

/*!
    @addtogroup nbanalyticseventstailq
    @{
*/

/*! Analytics Events TailQ entry.
    This is the double linked elements in an Analytics Events TailQ.
    see bsdqueue.h for detail.
*/
typedef struct NB_AnalyticsEventsTailQEntry
{
    data_analytics_event*                       event;
    TAILQ_ENTRY(NB_AnalyticsEventsTailQEntry)   tailQEntry;
} NB_AnalyticsEventsTailQEntry;

/*! Analytics Events TailQ head.
    This is the head of an Analytics Events TailQ.
    see bsdqueue.h for detail.
*/
typedef TAILQ_HEAD(NB_AnalyticsEventsTailQ, NB_AnalyticsEventsTailQEntry)
    NB_AnalyticsEventsTailQ;

/*! Create an Analytics Events TailQ Entry.
*/
NB_AnalyticsEventsTailQEntry* NB_AnalyticsEventsTailQEntryCreate(void);

/*! Destroy an Analytics Events TailQ Entry.
    The entry will be properly destroyed. This entry should have been already
    removed from any Analytics Events TailQ.
*/
void NB_AnalyticsEventsTailQEntryDestroy(
        data_util_state* pds, NB_AnalyticsEventsTailQEntry* pEntry);

/*! Free all events in an Analytics Events TailQ
    The Analytics Events TailQ will be empty afterward, and the TailQ itself
    is not freed.
*/
void NB_AnalyticsEventsTailQFreeAllEvents(
        data_util_state* pds, NB_AnalyticsEventsTailQ* tailq);

/* the following are Analytics Events TailQ wrapper macro of the
   bSD tailq macros.
   see bsdqueue.h for detail
*/
#define NB_AE_TAILQ_CONCAT(head1, head2) \
    TAILQ_CONCAT(head1, head2, tailQEntry)

#define NB_AE_TAILQ_EMPTY(head) \
    TAILQ_EMPTY(head)

#define NB_AE_TAILQ_FIRST(head) \
    TAILQ_FIRST(head)

#define NB_AE_TAILQ_FOREACH(var, head) \
    TAILQ_FOREACH(var, head, tailQEntry)

#define NB_AE_TAILQ_FOREACH_SAFE(var, head, tvar) \
    TAILQ_FOREACH_SAFE(var, head, tailQEntry, tvar)

#define NB_AE_TAILQ_FOREACH_REVERSE(var, head) \
    TAILQ_FOREACH_REVERSE(var, head, NB_AnalyticsEventsTailQ, tailQEntry)

#define NB_AE_TAILQ_FOREACH_REVERSE_SAFE(var, head, tvar) \
    TAILQ_FOREACH_REVERSE_SAFE(var, head, NB_AnalyticsEventsTailQ, tailQEntry, \
                               tvar)

#define	NB_AE_TAILQ_INIT(head) \
    TAILQ_INIT(head)

#define	NB_AE_TAILQ_INSERT_AFTER(head, listelm, elm) \
    TAILQ_INSERT_AFTER(head, listelm, elm, tailQEntry)

#define	NB_AE_TAILQ_INSERT_BEFORE(listelm, elm) \
    TAILQ_INSERT_BEFORE(listelm, elm, tailQEntry)

#define	NB_AE_TAILQ_INSERT_HEAD(head, elm) \
    TAILQ_INSERT_HEAD(head, elm, tailQEntry)

#define	NB_AE_TAILQ_INSERT_TAIL(head, elm) \
    TAILQ_INSERT_TAIL(head, elm, tailQEntry)

#define	NB_AE_TAILQ_LAST(head) \
    TAILQ_LAST(head, NB_AnalyticsEventsTailQ)

#define	NB_AE_TAILQ_NEXT(elm) \
    TAILQ_NEXT(elm, tailQEntry)

#define	NB_AE_TAILQ_PREV(elm) \
    TAILQ_PREV(elm, NB_AnalyticsEventsTailQ, tailQEntry)

#define	NB_AE_TAILQ_REMOVE(head, elm) \
    TAILQ_REMOVE(head, elm, tailQEntry)

/* @} */
#endif


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

/*
* Copyright (c) 2008 Networks In Motion, Inc.
* All rights reserved. This file and associated materials are the
* trade secrets, confidential information, and copyrighted works of
* Networks In Motion, Inc.
*
* This intellectual property is for the internal use only by Networks
* In Motion, Inc. This source code contains proprietary information
* of Networks In Motion, Inc., and shall not be used, copied by, or
* disclosed to, anyone other than a Networks In Motion, Inc.,
* certified employee that has written authorization to view or modify
* said source code.
*
*  Created by Dat Nguyen on 11/14/08.
*/

#ifndef TIMERIMPL_H
#define TIMERIMPL_H

#include <vector>

#include "palimpl.h"
#include "paltimer.h"
#include "pallock.h"
#include "wm_global.h"
#include "windows.h"
#include "paltaskqueue.h"

typedef struct _timer timer_t;

typedef struct _timer_entry
{
    timer_t              *timer;
    PAL_TimerCallback    *user_cb;
    void                 *user_cbdata;
    int                  timerID;
    uint32               ownerID;
    PAL_QueueHandle      handle;
} timer_entry_t;

typedef int (timer_entry_compare_t)(timer_entry_t *lhs, timer_entry_t *rhs);
timer_entry_t* timer_entry_create(timer_t *timer, PAL_TimerCallback *cb, void *cbdata, PAL_QueueHandle handle);
void timer_entry_free(timer_t *timer, timer_entry_t *te);


typedef struct _timer_list
{
    std::vector<timer_entry_t*> *list;
} timer_list_t;

void timer_list_ctor(timer_list_t *list);
void timer_list_dtor(timer_list_t *list);

PAL_Error timer_list_add(timer_list_t *list, timer_entry_t *te);
PAL_Error timer_list_remove(timer_list_t *list, timer_entry_t *te);

// index, -1 not found
int timer_list_find(timer_list_t *list, timer_entry_compare_t *cmp, timer_entry_t *te);

// timer, NULL if index not in range
timer_entry_t* timer_list_get(timer_list_t *tl, int index);


struct _timer
{
    PAL_Instance  *pal;
    PAL_Lock      *lock;
    timer_list_t  *timerList;
} ;

void timer_ctor(PAL_Instance *pal);
void timer_dtor(PAL_Instance *pal);

PAL_Error timer_schedule(timer_t *timer, timer_entry_t *te, int32 millisecs);
PAL_Error timer_cancel(timer_t *timer, timer_entry_t *te);

#endif

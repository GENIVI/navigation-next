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

#include <vector>
#include <windows.h>

#include "palimpl.h"
#include "palstdlib.h"
#include "timerimpl.h"
#include "finishfunctionnormal.h"
#include "paldebug.h"

timer_entry_t* timer_list_getat(timer_list_t *list, int idx);
timer_entry_t* timer_list_removeat(timer_list_t *list, int idx);
static void CALLBACK WinTimerCB(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);

using namespace std;

void 
timer_list_ctor(timer_list_t *tl)
{
    tl->list = new vector<timer_entry_t*>;
}

void 
timer_list_dtor(timer_list_t *tl)
{
    for (vector<timer_entry_t*>::const_iterator iterator = tl->list->begin(); iterator != tl->list->end(); ++iterator)
    {
        timer_entry_free(NULL /* Not used */, *iterator);
    }

    delete tl->list;
}

PAL_Error 
timer_list_add(timer_list_t *tl, timer_entry_t *te)
{
    tl->list->push_back(te);
    return PAL_Ok;
}


timer_entry_t *
timer_list_getat(timer_list_t *list, int idx)
{
    return (timer_entry_t*)(list->list->at(idx));
}

timer_entry_t *
timer_list_removeat(timer_list_t *list, int idx)
{
    timer_entry_t *obs = (timer_entry_t *)timer_list_getat(list, idx);
    vector<timer_entry_t*>::iterator current;

    current = list->list->begin();
    current += idx;
    list->list->erase(current);

    return obs;
}

PAL_Error 
timer_list_remove(timer_list_t *tl, timer_entry_t *te)
{
    vector<timer_entry_t*> *list  = tl->list;
    int count = list->size();
    int index;

    for (index = 0; index < count; index++)
    {
        timer_entry_t *temp = (timer_entry_t *)list->at(index);
        if (temp == te)
        {
            timer_list_removeat(tl, index);
            break;
        }
    }

    return PAL_Ok;
}

// index, -1 not found
int 
timer_list_find(timer_list_t *tl, timer_entry_compare_t *cmp, timer_entry_t *te)
{
    int returnIndex = -1;
    vector<timer_entry_t*> *list = tl->list;
    int count = list->size();
    int index;

    for (index = 0; index < count; index++)
    {
        timer_entry_t *temp = (timer_entry_t *)list->at(index);
        if (cmp(temp, te) == 0)
        {
            returnIndex = index;
            break;
        }
    }

    return returnIndex;
}

timer_entry_t *
timer_list_find_by_id(timer_list_t *tl, int timerID)
{
    vector<timer_entry_t*> *list = tl->list;
    int count = list->size();
    int index;

    for (index = 0; index < count; index++)
    {
        timer_entry_t *temp = (timer_entry_t *)list->at(index);
        if (temp->timerID == timerID) 
        {           
            return temp;
        }
    }
    return NULL;
}

// timer_entry, NULL if index not in range
timer_entry_t * 
timer_list_get(timer_list_t *tl, int index)
{
    timer_entry_t *returnTimerEntry = NULL;
    vector<timer_entry_t*> *list = tl->list;
    int count = list->size();

    if (0 <= index && index < count)
    {
        returnTimerEntry = (timer_entry_t *)list->at(index);
    }

    return returnTimerEntry;
}

timer_entry_t* 
timer_entry_create(timer_t *timer, PAL_TimerCallback *cb, void *cbdata, PAL_QueueHandle handle)
{
    timer_entry_t* te = reinterpret_cast<timer_entry_t*>(nsl_malloc(sizeof(timer_entry_t)));
    if (te == NULL)
    {
        return NULL;
    }
    te->timer = timer;
    te->user_cb = cb;
    te->user_cbdata = cbdata;
    te->timerID = -1;
    te->handle = handle;
    return te;
}

void 
timer_entry_free(timer_t *timer, timer_entry_t *te) 
{
    if (te != NULL)
    {
        nsl_free(te);
    }
}

static int 
timer_entry_compare(timer_entry_t *lhs, timer_entry_t *rhs)
{
    if (lhs->user_cb == rhs->user_cb && lhs->user_cbdata == rhs->user_cbdata)
        return 0;

    return -1;
}

void 
timer_ctor(PAL_Instance *pal)
{
    timer_t* timer = reinterpret_cast<timer_t*>(nsl_malloc(sizeof(timer_t)));
    if (!timer)
    {
        return;
    }
    nsl_memset(timer, 0, sizeof(timer_t));

    timer_list_t *tl = reinterpret_cast<timer_list_t*>(nsl_malloc(sizeof(timer_list_t)));
    if (!tl)
    {
        nsl_free(timer);
        return;
    }
    timer_list_ctor(tl);
    timer->timerList = tl;

    timer->pal = pal;
    pal->timer = timer;
    PAL_LockCreate(pal, &(pal->timer->lock));
}

void 
timer_dtor(PAL_Instance *pal)
{
    timer_list_dtor(pal->timer->timerList);

    nsl_free(pal->timer->timerList);
    PAL_LockDestroy(pal->timer->lock);
    nsl_free(pal->timer);

    pal->timer = NULL;
}

PAL_Error 
timer_schedule(timer_t *timer, timer_entry_t *te, int32 millisecs)
{
    // TODO, need to bundle timer and te together to support threading
    te->timerID = SetTimer(NULL, 0, millisecs, WinTimerCB);
    timer_cancel(timer, te);  // if any
    timer_list_add(timer->timerList, te);

    return PAL_Ok;
}

PAL_Error 
timer_cancel(timer_t *timer, timer_entry_t *te)
{
    timer_entry_t *original = NULL;
    int index = timer_list_find(timer->timerList, timer_entry_compare, te);
    if (index != -1)
    {
        original = timer_list_get(timer->timerList, index);
        timer_list_remove(timer->timerList, original);
        timer->pal->functionManager->DeleteScheduledFunctions(original->ownerID);
    }

    if (original != NULL)
    {     
        KillTimer(NULL, original->timerID);
        original->user_cb(timer->pal, original->user_cbdata, PTCBR_TimerCanceled);
        timer_entry_free(timer, original);
    }

    return PAL_Ok;
}


void
timer_finishfunc(void* pParam)
{
    timer_entry_t *te = (timer_entry_t *)pParam;

    PAL_LockLock(te->timer->lock);
    timer_list_remove(te->timer->timerList, te);
    PAL_LockUnlock(te->timer->lock);

    te->user_cb(te->timer->pal, te->user_cbdata, PTCBR_TimerFired);
    timer_entry_free(te->timer, te);
}

/* Callback for SetTimer */
static void CALLBACK
WinTimerCB(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    PAL_Instance* pal = Get_PAL_NBI_InstPointer();
    if (!pal)
    {
        return;
    }

    PAL_LockLock(pal->timer->lock);

    timer_entry_t *te = timer_list_find_by_id(pal->timer->timerList, idEvent);
    if (te)
    {
        te->ownerID = pal->functionManager->RegisterOwner();
        FinishFunctionNormal* function = new FinishFunctionNormal(timer_finishfunc, te);
        pal->functionManager->Schedule(te->ownerID, function, te->handle, TRUE);
    }
    KillTimer(NULL, idEvent);

    PAL_LockUnlock(pal->timer->lock);
}

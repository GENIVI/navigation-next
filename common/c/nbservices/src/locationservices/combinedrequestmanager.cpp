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

    @file     CombinedRequestManager.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    CombinedRequestManager class implementation

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */


#include "locationcontext.h"
#include "combinedlocationrequest.h"
#include "combinedrequestmanager.h"
#include "statemanager.h"
#include "nbqalog.h"

CombinedRequestManager::CombinedRequestManager(NB_LocationContext* locationContext)
    : m_locationContext(locationContext)
{

    m_list = new ListenerList();
    m_deleteList = new ListenerList();

    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Combined request manager instance created", 1));
}

CombinedRequestManager::~CombinedRequestManager(void)
{
    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
            StaticPALTimerCallback, (void*)this);

    CleanListData(m_list);
    CleanListData(m_deleteList);
    delete m_list;
    delete m_deleteList;


}

NB_LS_LocationState CombinedRequestManager::GetState(void)
{

    int count = m_list->Count();


    NB_LS_LocationState state = NBPGS_Initialized;
    if (count > 0)
    {
        state = NBPGS_Oneshot;
    }
    return state;
}

NB_Error CombinedRequestManager::RequestStart(NB_LS_LocationCallback callback, void* userData,
                                                  NB_LS_FixType fixType, NB_Context* nbContext)
{
    CombinedLocationRequest* request = new CombinedLocationRequest(m_locationContext, nbContext);

    NB_LS_Listener* listener = (NB_LS_Listener*)nsl_malloc(sizeof(NB_LS_Listener));
    RequestData* reqData = (RequestData*)nsl_malloc(sizeof(RequestData));
    if (request == NULL || listener == NULL || reqData == NULL)
    {
        nsl_free(reqData);
        nsl_free(listener);
        delete request;

        return NE_NOMEM;
    }

    reqData->me = this;
    reqData->request = request;
    reqData->userData = userData;

    listener->callback = callback;
    listener->replyTime = 0;
    listener->userData = reqData;


    m_list->Append(listener);
    int count = m_list->Count();


    NB_Error retStatus = request->RequestStart((NB_LS_LocationCallback)Static_CombinedRequestCallback, listener, fixType);

    if ((retStatus == NE_OK) && (count == 1)) // We've just added first combined request, so our state turned to active
    {
        m_locationContext->m_stateManager->SetOneFixState(NBPGS_Oneshot);
    }

    return retStatus;
}

void CombinedRequestManager::RequestCancel(NB_LS_LocationCallback callback, void* userData)
{

    NB_LS_Listener* listener = m_list->First();
    while (listener != NULL)
    {
        RequestData* reqData = (RequestData*)listener->userData;
        // We either remove all listeners for the same callback (if userData is NULL),
        // or remove the listener with the same pair "callback, userData" if userData is not NULL.
        if (((listener->callback == callback) && (userData == NULL)) ||
            ((listener->callback == callback) && (userData == reqData->userData)))
        {
            reqData->request->RequestCancel();

            m_list->Remove(listener);
            FreeListener(listener);
        }
        listener = m_list->Next();
    }

    int count = m_list->Count();


    if (count == 0) // No more requests, combined request manager is inactive
    {
        m_locationContext->m_stateManager->SetOneFixState(NBPGS_Initialized);
    }
}

void CombinedRequestManager::Static_CombinedRequestCallback(void* appData, NB_LocationContext* context, const NB_LS_Location* location, NB_Error error)
{
    NB_LS_Listener* listener = (NB_LS_Listener*)appData;
    if (listener != NULL)
    {
        RequestData* reqData = (RequestData*)listener->userData;
        if (reqData != NULL)
        {
            reqData->me->CombinedRequestCallback(listener, location, error);
        }
    }
}

void CombinedRequestManager::CombinedRequestCallback(NB_LS_Listener* listener, const NB_LS_Location* location, NB_Error error)
{
    RequestData* reqData = (RequestData*)listener->userData;

    NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_StartCallback, __FUNCTION__, "NB_LS_LocationCallback");
    listener->callback(reqData->userData, m_locationContext, location, error);
    NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_CallbackReturn, __FUNCTION__, "NB_LS_LocationCallback");

    // remove listener from the list

    m_list->Remove(listener);
    // we cannot destroy combined request object here, because this callback is called from the combined request method.
    // add this listener to the m_deleteList. It will be destroyed later.
    m_deleteList->Append(listener);
    int count = m_list->Count();


    if (count == 0) // No more requests, combined request manager is inactive
    {
        m_locationContext->m_stateManager->SetOneFixState(NBPGS_Initialized);
    }

    PAL_TimerSet(m_locationContext->m_PAL_Instance, 0,
            StaticPALTimerCallback, (void*)this);
}

void CombinedRequestManager::FreeListener(NB_LS_Listener* listener)
{
    if (listener != NULL)
    {
        RequestData* reqData = (RequestData*)listener->userData;
        delete reqData->request;
        nsl_free(reqData);
        nsl_free(listener);
    }
}

void CombinedRequestManager::CleanListData(ListenerList* list)
{
    NB_LS_Listener* listener = list->First();
    while (listener != NULL)
    {
        FreeListener(listener);
        listener = list->Next();
    }
}

void CombinedRequestManager::StaticPALTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    CombinedRequestManager* me = (CombinedRequestManager*)userData;
    if (me != NULL && reason == PTCBR_TimerFired)
    {
        me->TimerCallback();
    }
}

void CombinedRequestManager::TimerCallback()
{
    // remove all from the m_deleteList

    CleanListData(m_deleteList);
    NB_LS_Listener* listener = m_deleteList->First();
    while (listener != NULL)
    {
        m_deleteList->Remove(listener);
        listener = m_deleteList->Next();
    }

}

/*! @} */

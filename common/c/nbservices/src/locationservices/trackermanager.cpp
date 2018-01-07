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

    @file     TrackerManager.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    TrackerManager class implementation

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
#include "locationtracker.h"
#include "trackermanager.h"
#include "statemanager.h"

TrackerManager::TrackerManager(NB_LocationContext* locationContext)
    : m_locationContext(locationContext),
      m_trackingInfo(NULL)
{

    m_list = new ListenerList();

    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Tracker manager instance created", 1));
}

TrackerManager::~TrackerManager(void)
{
    CleanListData();
    delete m_list;


}

void TrackerManager::EmulationEnabled(bool isEnabled)
{
    /* TODO!!!
    m_tracker->EmulationEnabled(isEnabled);
    */
}

LocationTracker* TrackerManager::GetNewTracker(NB_Context* nbContext)
{
    // in current implementation we use one tracker for all listeners, because they all
    // start tracking with the same parameters. If in the future we need to start tracking sessions
    // with different parameters this function will create new tracker for the new listener.
    return new LocationTracker(m_locationContext, nbContext);
}

NB_Error TrackerManager::BeginTracking(NB_LS_LocationCallback callback, void* userData, NB_Context* nbContext)
{
    LocationTracker* tracker = GetNewTracker(nbContext);
    if (tracker == NULL)
    {
        return NEGPS_GENERAL;
    }
    NB_LS_Listener* listener = (NB_LS_Listener*)nsl_malloc(sizeof(NB_LS_Listener));
    RequestData* reqData = (RequestData*)nsl_malloc(sizeof(RequestData));
    if (listener == NULL || reqData == NULL)
    {
        nsl_free(reqData);
        nsl_free(listener);
        delete tracker;
        return NE_NOMEM;
    }

    reqData->me = this;
    reqData->userData = userData;
    reqData->tracker = tracker;

    listener->callback = callback;
    listener->replyTime = 0;
    listener->userData = reqData;

    m_list->Append(listener);

    NB_Error err = NE_OK;
    if (tracker->GetState() == NBPGS_Initialized)
    {
        err = tracker->BeginTracking(callback, userData, &m_trackingInfo);
    }

    return err;
}

void TrackerManager::EndTracking(NB_LS_LocationCallback callback, void* userData)
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
            // Stop the tracking and get rid of the tracker
            reqData->tracker->EndTracking(m_trackingInfo);
            delete reqData->tracker;
            reqData->tracker = NULL;

            // Now get rid of the listener
            m_list->Remove(listener);
            FreeListener(listener);
        }
        listener = m_list->Next();
    }

    if (m_list->Count() == 0)
    {
        // No more trackers launched
        this->m_locationContext->m_stateManager->SetTrackerState(NBPGS_Initialized);
    }
}

void TrackerManager::FreeListener(NB_LS_Listener* listener)
{
    if (listener != NULL)
    {
        nsl_free(listener->userData);
        nsl_free(listener);
    }
}

void TrackerManager::CleanListData(void)
{
    if (m_list)
    {
        NB_LS_Listener* listener = m_list->First();
        while (listener != NULL)
        {
            // Stop the tracking and get rid of the tracker
            RequestData* reqData = (RequestData*)listener->userData;
            reqData->tracker->EndTracking(m_trackingInfo);
            delete reqData->tracker;
            reqData->tracker = NULL;

            // Now get rid of the listener
            FreeListener(listener);
            listener = m_list->Next();
        }
    }
}

/*! @} */

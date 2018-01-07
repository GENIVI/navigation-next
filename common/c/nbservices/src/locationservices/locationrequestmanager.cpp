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

    @file     LocationRequestManager.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    @todo Needs description

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


#include "locationrequestmanager.h"
#include "locationcontext.h"
//#include "NBCoreCrossPlatform.h"

extern "C" {
  #include "palclock.h"
}

LocationRequestManager::LocationRequestManager(NB_LocationContext* context)
    : m_locationContext(context)
{

}

LocationRequestManager::~LocationRequestManager()
{
    // Go to the head of the list
    NB_LS_Listener* current = First();

    // Find the appropriate listeners and remove them
    while (current)
    {
        Remove(current);
        nsl_free(current);
        current = Next();
    }
}

NB_Error LocationRequestManager::AddListener(NB_LS_LocationCallback callback, void* appData, int timeout)
{
    NB_Error retStatus = NE_OK;

    if (!CheckForPresence(callback, appData))  // Search for the listener with the same properties
    {
        NB_LS_LocalTime replyTime = PAL_ClockGetGPSTime() + timeout;
        NB_LS_Listener* newListener = (NB_LS_Listener*)nsl_malloc(sizeof(NB_LS_Listener));

        if (newListener == NULL)
        {
            retStatus = NE_NOMEM;
        }
        else
        {
            newListener->callback = callback;
            newListener->userData = appData;
            newListener->replyTime = replyTime;
            Append(newListener);
        }
    }
    else
    {
        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Request already exists", 1));
        retStatus = NE_BUSY;
    }

    // Currently, if we already got this exact listener in the queue, we do nothing, and return error.
    // So request will not proceed further.
    // This logic could be changed later, e.g. to extend the request's timeout instead.



    return retStatus;
}

bool LocationRequestManager::CheckForPresence(NB_LS_LocationCallback callback, void* appData)
{
    bool retStatus = FALSE;

    // Go to the head of the list
    NB_LS_Listener* current = First();

    // Search for the listener with the same properties
    while (current)
    {
        if ((current->callback == callback) && (current->userData == appData))
        {
            retStatus = TRUE;
            break;
        }
        current = Next();
    }

    return retStatus;
}

void LocationRequestManager::RemoveListeners(NB_LS_LocationCallback callback, void* appData)
{


    // Go to the head of the list
    NB_LS_Listener* current = First();

    // Find the appropriate listeners and remove them
    while (current)
    {
        if ((current->callback == callback) && (current->userData == appData))
        {
            Remove(current);
            nsl_free(current);
        }
        current = Next();
    }


}

void LocationRequestManager::NotifyAndRemoveAll(NB_Error error, const NB_LS_Location* location)
{
    ListenerList guysToBeNotified;

    // First, we remove everyone from our protected queue,
    // and move them to a local queue where we don't care about critical sections


    // Go to the head of the list
    NB_LS_Listener* current = First();

    // Find the appropriate listeners and remove them
    while (current)
    {
        Remove(current);
        guysToBeNotified.Append(current);
        current = Next();
    }



    // Now let's do notifications
    NB_LS_Listener* currentNotified = guysToBeNotified.First();

    // Find the appropriate listeners and remove them
    while (currentNotified)
    {
        guysToBeNotified.Remove(currentNotified);
        if (currentNotified->callback)
        {
            currentNotified->callback(currentNotified->userData, m_locationContext, location, error);
        }
        nsl_free(currentNotified);
        currentNotified = guysToBeNotified.Next();
    }

}

void LocationRequestManager::NotifyTimeout()
{
    ListenerList guysToBeNotified;

    // First, we remove everyone from our protected queue,
    // and move them it to a local queue where we don't care about critical sections


    NB_LS_LocalTime timeThreshold = PAL_ClockGetGPSTime() + NBTIME_LAG_VALUE;

    // Go to the head of the list
    NB_LS_Listener* current = First();

    // Find the appropriate listeners and remove them
    while (current)
    {
        if (current->replyTime <= timeThreshold)
        {
            Remove(current);
            guysToBeNotified.Append(current);
        }
        current = Next();
    }



    // Now let's do notifications
    NB_LS_Listener* currentNotified = guysToBeNotified.First();

    // Find the appropriate listeners and remove them
    while (currentNotified)
    {
        guysToBeNotified.Remove(currentNotified);
        if (currentNotified->callback)
        {
            currentNotified->callback(currentNotified->userData, m_locationContext, NULL, NEGPS_TIMEOUT);
        }
        nsl_free(currentNotified);
        currentNotified = guysToBeNotified.Next();
    }

}

NB_LS_LocalTime LocationRequestManager::GetMinimalReplyTime()
{


    // Go to the head of the list
    NB_LS_LocalTime retVal = First()->replyTime;
    NB_LS_Listener* current = Next();

    // Find the appropriate listeners and remove them
    while (current)
    {
        retVal = (current->replyTime < retVal) ? current->replyTime : retVal;
        current = Next();
    }



    return retVal;
}

/*! @} */

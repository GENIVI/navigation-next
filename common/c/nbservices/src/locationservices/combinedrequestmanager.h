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

    @file     combinedrequestmanager.h
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    CombinedRequestManager class declaration
        This class handles the list of combined requests.

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

#pragma once

extern "C" {
  #include "paltimer.h"
}

#include "listenerlist.h"

class CombinedLocationRequest;

class CombinedRequestManager : public Base
{
public:
    CombinedRequestManager(NB_LocationContext* locationContext);
    ~CombinedRequestManager(void);

    NB_Error RequestStart(NB_LS_LocationCallback callback, void* userData, NB_LS_FixType fixType, NB_Context* nbContext);
    void RequestCancel(NB_LS_LocationCallback callback, void* userData);
    NB_LS_LocationState GetState(void);

private:
    struct RequestData
    {
        CombinedRequestManager*  me;
        CombinedLocationRequest* request;
        void* userData;
    };

    static void Static_CombinedRequestCallback(void* appData, NB_LocationContext* context, const NB_LS_Location* location, NB_Error error);
    void CombinedRequestCallback(NB_LS_Listener* listener, const NB_LS_Location* location, NB_Error error);
    static void StaticPALTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
    void TimerCallback();

    void FreeListener(NB_LS_Listener* listener);
    void CleanListData(ListenerList* list);

private:
    NB_LocationContext*        m_locationContext;
    ListenerList*            m_list;
    ListenerList*            m_deleteList;
};

/*! @} */

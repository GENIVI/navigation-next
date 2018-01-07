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
 
 @file     QaLog.h
 @date     09/21/2010
 
 
 This is the implementation of the startupcontroller interface.
 StartupController handles all the logic of the app startup sequence.
 
 */
/*
 (C) Copyright 2010 by TeleCommunication Systems
 
 The information contained herein is confidential, proprietary
 to Networks In Motion, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef __QALOG_H
#define __QALOG_H

#include "smartpointer.h"
#include "pallock.h"
#include "abqaloghandler.h"
#include "abqaloginformation.h"
#include <string>
#include <vector>
#include "nbnetwork.h"
#include "nbuitaskqueue.h"
#include "abqalog.h"

class QaLogListenerInterface;

typedef enum
{
    QLE_UploadStarted =0,
    QLE_UploadCompleted,
    QLE_UploadCanceled,
    QLE_UploadProgress,
    QLE_UploadError,
    QLE_LogFileEmpty
}QaLogEvent;

class QaLog
{
public:
    QaLog(NB_Context* context);
    virtual ~QaLog();
    NB_Context* GetContext(){return m_nbContext;};

    virtual bool Initialize(NB_NetworkConfiguration* networkConfig, PAL_Instance* pal);
    virtual void SetQaLogConfiguration(const AB_QaLogHeader* header, const char* fileName);
    virtual NB_Error StartLogging (bool verbose);
    virtual NB_Error UploadLogFile ();
    virtual void ClearLogFile(uint32 sessionCount);
    virtual void CancelUpload ();

    virtual void RegisterListener (QaLogListenerInterface* listener);
    virtual void ReleaseListener (QaLogListenerInterface* listener);

public:
    static shared_ptr<QaLog> GetInstance(NB_Context* context);

private:
    //Private Functions

    //These 2 copy functions are deeply copy.
    void CopyNetworkConfiguration(const NB_NetworkConfiguration* networkConfig);
    void CopyQaLogHeader(const AB_QaLogHeader* header);

    void NotifyListener (QaLogEvent event, void* userData);

    // CCC functions
    void CCC_EnableLogging();
    void CCC_StartUploadRequest ();
    void CCC_CancelUpload ();
    void CCC_UploadCompleted ();

    //Task queue callback function
    void static TaskQueue_EnableLogging (PAL_Instance* palInstance, void* userData);
    void static TaskQueue_UploadRequest (PAL_Instance* palInstance, void* userData);
    void static TaskQueue_CancelUploadRequest (PAL_Instance* palInstance, void* userData);
    void static TaskQueue_ClearLogFile(PAL_Instance* palInstance, void* userData);

    // callback of taskqueue reponse, CCC callback
    static void QaLogRequestHandlerCallback (void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
    //Private Memebers
    NB_Context*         m_nbContext;
    // Pal Lock
    PAL_Lock*			m_palLock;
    // task queue id
    uint32				m_taskId;
    //Pointer to request handler
    AB_QaLogHandler*		m_handler;
    //Pointer to information object
    AB_QaLogInformation*	m_information;
    //Log file name
    std::string				m_fileName;
    //log file identifier
    std::string				m_identifier;
    //pointer to listener
    std::vector<QaLogListenerInterface*> m_listeners;
    NB_NetworkConfiguration* m_networkConfiguration;
    AB_QaLogHeader m_qaLogHeader;
    bool m_enableQaLog;
    uint32         m_qalogClearSessionCount;
    shared_ptr<UiTaskQueue> m_UITaskQueue;
};

typedef shared_ptr<QaLog> QaLogPtr;

#endif

/*! @} */


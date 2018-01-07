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

 @file     QaLog.cpp
 @date     09/17/2010
 @defgroup QA_LOG class
 */
/*
 See header file for description.

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

#include "QaLog.h"
#include "Lock.h"
#include "abqalogparameters.h"
#include "paltaskqueue.h"
#include "QaLogListenerInterface.h"
#include "palfile.h"
#include "nbcontextprotected.h"
#include "paltestlog.h"
#include "contextbasedsingleton.h"
#include "nbnetworkconfiguration.h"

QaLog::QaLog(NB_Context* context) :
    m_nbContext(context),
    m_palLock(NULL),
    m_taskId(0),
    m_handler(NULL),
    m_information(NULL),
    m_networkConfiguration(new NB_NetworkConfiguration()),
    m_enableQaLog(false)
{
    nsl_memset(&m_qaLogHeader, 0, sizeof(AB_QaLogHeader));
}

QaLog::~QaLog()
{
    if (m_palLock)
    {
        PAL_LockDestroy(m_palLock);
    }

    if (m_handler)
    {
        AB_QaLogHandlerDestroy(m_handler);
    }

    if (m_information)
    {
        AB_QaLogInformationDestroy(m_information);
    }

    if(m_networkConfiguration && m_networkConfiguration->tpslibData)
    {
        delete[] (m_networkConfiguration->tpslibData);
    }

    if(m_networkConfiguration && m_networkConfiguration->tlsConfig.name)
    {
        delete[] (m_networkConfiguration->tlsConfig.name);
    }

    if(m_networkConfiguration)
    {
        delete m_networkConfiguration;
    }

    if(m_qaLogHeader.productName)
    {
        delete[] m_qaLogHeader.productName;
    }
}

bool
QaLog::Initialize(NB_NetworkConfiguration* networkConfig, PAL_Instance* pal)
{
    if(networkConfig)
    {
        CopyNetworkConfiguration(networkConfig);
    }
    m_UITaskQueue = shared_ptr<UiTaskQueue>(new UiTaskQueue(pal));
    PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::Initialize[OK]!");
    return true;
}

void
QaLog::CopyNetworkConfiguration(const NB_NetworkConfiguration* config)
{
    if(!config)
    {
        return;
    }

    *m_networkConfiguration = *config;
    if(config->tpslibData && config->tpslibLength >0)
    {
        char* tmp = new char[config->tpslibLength];
        if(tmp)
        {
            nsl_memset(tmp, 0, config->tpslibLength);
            nsl_memcpy(tmp, config->tpslibData, config->tpslibLength);
            m_networkConfiguration->tpslibData = tmp;
        }
    }

    if(config->tlsConfig.name)
    {
        size_t len = nsl_strlen(config->tlsConfig.name);
        if(len > 0)
        {
            char* tmp = new char[len + 1];
            if(tmp)
            {
                nsl_memset(tmp, 0, len + 1);
                nsl_memcpy(tmp, config->tlsConfig.name, len);
                m_networkConfiguration->tlsConfig.name = tmp;
            }
        }
    }

    m_networkConfiguration->triageTimeoutSeconds = 120;
}

void
QaLog::CopyQaLogHeader(const AB_QaLogHeader* header)
{
    if(!header)
    {
        return;
    }

    nsl_memcpy(&m_qaLogHeader, header, sizeof(AB_QaLogHeader));
    if(header->productName)
    {
        size_t len = nsl_strlen(header->productName);
        if(len > 0)
        {
            char* tmp = new char[len + 1];
            if(tmp)
            {
                nsl_memset(tmp, 0, len + 1);
                nsl_memcpy(tmp, header->productName, len);
                m_qaLogHeader.productName = tmp;
            }
        }
    }
}

void
QaLog::SetQaLogConfiguration(const AB_QaLogHeader* header, const char* fileName)
{
    PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::SetQaLogConfiguration fileName = %s", fileName);
    if(header)
    {
        CopyQaLogHeader(header);
    }

    if(fileName)
    {
        m_fileName.assign(fileName);
    }
}

NB_Error
QaLog::StartLogging (bool verbose)
{
    PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::StartLogging verbose = %d", verbose);
    m_enableQaLog = verbose;
    PAL_EventTaskQueueAdd(NB_ContextGetPal(m_nbContext), TaskQueue_EnableLogging, this, &m_taskId);
    return NE_OK;
}

NB_Error
QaLog::UploadLogFile ()
{
    PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::UploadLogFile");
    //upload the log file, add request to task queue
    PAL_EventTaskQueueAdd(NB_ContextGetPal(m_nbContext), TaskQueue_UploadRequest, this, &m_taskId);
    return NE_OK;
}

void
QaLog::CancelUpload ()
{
    //upload the log file, add request to task queue
    PAL_EventTaskQueueAdd(NB_ContextGetPal(m_nbContext), TaskQueue_CancelUploadRequest, this, &m_taskId);
}

void
QaLog::TaskQueue_EnableLogging (PAL_Instance* palInstance, void* userData)
{
    QaLog* pThis = static_cast<QaLog*> (userData);
    pThis->CCC_EnableLogging();
}

void
QaLog::TaskQueue_UploadRequest (PAL_Instance* palInstance, void* userData)
{
    PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::TaskQueue_UploadRequest");
    QaLog* pThis = static_cast<QaLog*> (userData);
    pThis->CCC_StartUploadRequest();
}

void
QaLog::TaskQueue_CancelUploadRequest (PAL_Instance* palInstance, void* userData)
{
    QaLog* pThis = static_cast<QaLog*> (userData);
    if (pThis->m_handler)
    {
        AB_QaLogHandlerCancelRequest(pThis->m_handler);
    }
}

void
QaLog::CCC_EnableLogging()
{
    AB_QaLogCreate(m_nbContext, &m_qaLogHeader, m_fileName.c_str(), m_enableQaLog);
}

void
QaLog::CCC_StartUploadRequest()
{
    NB_Error result = NE_OK;
    AB_QaLogParameters* parameters = NULL;
    PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::CCC_StartUploadRequest");

    try
    {
        //check if qalog file exists
        uint32 filesize =0;
        PAL_Error palResult = PAL_FileGetSize(NB_ContextGetPal(m_nbContext),  m_fileName.c_str(), &filesize);
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::m_fileName = %s", m_fileName.c_str());

        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::filesize = %d", filesize);
        if (filesize  == 0 || palResult != PAL_Ok)
        {
            NotifyListener (QLE_LogFileEmpty, NULL);
            return;
        }

        if (m_handler == NULL)
        {
            NB_RequestHandlerCallback callbackData = { &QaLogRequestHandlerCallback, this};
            result = AB_QaLogHandlerCreate(m_nbContext, &callbackData, &m_handler);
            if (result != NE_OK)
            {
                throw result;
            }
        }
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::AB_QaLogHandlerCreate");

        if (m_information)
        {
            AB_QaLogInformationDestroy(m_information);
        }
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::AB_QaLogInformationDestroy");

        if (m_fileName.empty())
        {
            result = NE_INVAL;
            throw result;
        }
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::m_fileName.empty()");

        result = AB_QaLogParametersCreate(m_nbContext, m_fileName.c_str(), m_networkConfiguration, &parameters);
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::AB_QaLogParametersCreate");
        if (result != NE_OK)
        {
            throw result;
        }

        //start the request
        result = AB_QaLogHandlerStartRequest(m_handler, parameters);
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::AB_QaLogHandlerStartRequest parameters = %p", parameters);
        if (result != NE_OK)
        {
            throw result;
        }

        if (result != NE_OK)
        {
            throw result;
        }

        NotifyListener (QLE_UploadStarted, NULL);
    }
    catch (NB_Error error)
    {
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::catch (NB_Error error)");
        NotifyListener(QLE_UploadError, &error);
    }

    if (parameters)
    {
        AB_QaLogParametersDestroy(parameters);
    }
}

void
QaLog::QaLogRequestHandlerCallback (void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::QaLogRequestHandlerCallback status = %d, err = %d, up = %d, percent = %d", status, err, up, percent);
    QaLog* pThis = static_cast<QaLog*> (userData);
    NB_Error result = NE_OK;

    char* identifier;
    if (status == NB_NetworkRequestStatus_Success)
    {
        //if successful, get the identifier for uploaded file
        result = AB_QaLogHandlerGetInformation(pThis->m_handler, &pThis->m_information);
        if (result != NE_OK)
        {
            pThis->NotifyListener (QLE_UploadError, &result);
            return;
        }

        result = AB_QaLogInformationGetIdentifier(pThis->m_information, &identifier);

        //store the identifier
        pThis->m_identifier.assign(identifier, strlen(identifier));
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::NB_NetworkRequestStatus_Success LogID[%s]!", pThis->m_identifier.c_str());

        //release identifier
        nsl_free(identifier);

        pThis->NotifyListener (QLE_UploadCompleted, NULL);

        //empty the log file
        PAL_File* palFile = NULL;

        PAL_Error error = PAL_FileExists(NB_ContextGetPal(pThis->m_nbContext), pThis->m_fileName.c_str());

        if (error == PAL_Ok)
        {
            error = PAL_FileOpen(NB_ContextGetPal(pThis->m_nbContext), pThis->m_fileName.c_str(), PFM_Create, &palFile);
            if (error == PAL_Ok)
            {
                PAL_FileClose(palFile);
            }
        }
    }
    else if (status == NB_NetworkRequestStatus_Progress)
    {
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::NB_NetworkRequestStatus_Progress");
        pThis->NotifyListener (QLE_UploadProgress, &percent);
    }
    else if (status == NB_NetworkRequestStatus_Canceled)
    {
        if (pThis->m_handler)
        {
            PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::NB_NetworkRequestStatus_Canceled");
            AB_QaLogHandlerCancelRequest(pThis->m_handler);
        }
    }
    else if (status == NB_NetworkRequestStatus_TimedOut)
    {
        //if there is a timeout report error code
        //@todo - check why do we need to set this value, it should be coming from CCC
        err = NESERVER_TIMEDOUT;
        pThis->NotifyListener (QLE_UploadError, &err);
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::NB_NetworkRequestStatus_TimedOut");
    }
    else if (status == NB_NetworkRequestStatus_Failed)
    {
        //@todo - check why do we need to set this value, it should be coming from CCC
        err = NESERVER_FAILED;
        pThis->NotifyListener (QLE_UploadError, &err);
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "QaLog::NB_NetworkRequestStatus_Failed");
    }
}

void
QaLog::RegisterListener (QaLogListenerInterface* listener)
{
    nsl_assert(listener);

    if (std::find(m_listeners.begin(), m_listeners.end(), listener) == m_listeners.end())
    {
        m_listeners.push_back(listener);
    }
}

void
QaLog::ReleaseListener (QaLogListenerInterface* listener)
{
    nsl_assert(listener);

    std::vector<QaLogListenerInterface*>::iterator found = m_listeners.end();
    found = std::find(m_listeners.begin(), m_listeners.end(), listener);

    if (found != m_listeners.end())
    {
        m_listeners.erase(found);
    }
}

/*! Basic used to add to task queue by QaLog, refer to class Task for more description */
class NotifyListenerTask : public Task
{
public:
    NotifyListenerTask(vector<QaLogListenerInterface*>& listeners, QaLogEvent event, const string& logid, int* percentage, NB_Error* error):
    m_listeners(listeners), m_event(event), m_logid(logid)
    {
        if(percentage)
        {
            m_percentage = *percentage;
        }
        if(error)
        {
            m_error = *error;
        }
    }

    virtual ~NotifyListenerTask(){};

    // Refer to class Task for description.
    virtual void Execute()
    {
        //messages are complete
        //forward the notification to the message listeners
        std::vector<QaLogListenerInterface*>::iterator iter;

        for (iter = m_listeners.begin(); iter != m_listeners.end(); iter++)
        {
            QaLogListenerInterface* listener = *iter;

            switch(m_event)
            {
                case QLE_UploadStarted:
                    listener->UploadStarted();
                    break;
                case QLE_UploadCompleted:
                    listener->UploadCompleted(m_logid);
                    break;
                case QLE_UploadProgress:
                    listener->UploadProgress(m_percentage);
                    break;
                case QLE_LogFileEmpty:
                    listener->LogFileEmpty();
                    break;
                case QLE_UploadError:
                    listener->UploadFailed(m_error);
                    break;
                default:
                    break;
            }
        }
        delete this;
    }


private:
    vector<QaLogListenerInterface*>& m_listeners;
    QaLogEvent  m_event;
    string      m_logid;
    int         m_percentage;
    NB_Error    m_error;
};

void
QaLog::NotifyListener (QaLogEvent event, void* userData)
{
    if (m_UITaskQueue->AddTask(new NotifyListenerTask(m_listeners, event, m_identifier, (int*)userData, (NB_Error*)userData)) != PAL_Ok)
    {
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "m_UITaskQueue->AddTask(new NotifyListenerTask() != PAL_Ok");
    }
}

QaLogPtr
QaLog::GetInstance(NB_Context *context)
{
    return ContextBasedSingleton<QaLog>::getInstance(context);
}

void QaLog::ClearLogFile(uint32 sessionCount)
{
    m_qalogClearSessionCount = sessionCount;
    PAL_EventTaskQueueAdd(NB_ContextGetPal(m_nbContext), TaskQueue_ClearLogFile, this, &m_taskId);
}

void QaLog::TaskQueue_ClearLogFile(PAL_Instance* palInstance, void* userData)
{
    QaLog* pThis = static_cast<QaLog*> (userData);
    if (pThis->m_nbContext)
    {
        AB_QaLogClear(pThis->m_nbContext, pThis->m_qalogClearSessionCount);
    }
}
/*! @} */

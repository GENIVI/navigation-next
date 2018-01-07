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
   @file         QaLogTask.cpp
*/
/*
   (C) Copyright 2014 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#include "QaLogTask.h"
#include "nbqalog.h"

extern "C"
{
#include "nbcontextprotected.h"
}

/*! Task to write qalog data. */
class QALogWriteDataTask : public Task
{
public:
    QALogWriteDataTask(NB_Context* nbContext, const uint8* data, uint32 dataSize):m_context(nbContext), m_data(NULL), m_dataSize(dataSize)
    {
        m_data = (uint8*)nsl_malloc(sizeof(uint8) * m_dataSize);
        if (m_data)
        {
            nsl_memset(m_data, 0, m_dataSize);
            nsl_memcpy(m_data, data, m_dataSize);
        }
    }

    virtual ~QALogWriteDataTask(){};

    virtual void Execute()
    {
        if (m_data)
        {
            NB_QaLogWriteBuffer(m_context, m_data, m_dataSize);
            nsl_free(m_data);
        }
        delete this;
    }
private:
    NB_Context* m_context;
    uint8*  m_data;
    uint32  m_dataSize;
};

QaLogTask::QaLogTask(NB_Context* context)
    : m_context(context)
{
    // Get CCC task queue
    m_eventQueue = shared_ptr<EventTaskQueue>(new EventTaskQueue(NB_ContextGetPal(m_context)));
}

QaLogTask::~QaLogTask()
{
}

/* See description in header file. */
void
QaLogTask::WriteBuffer(const uint8* data, uint32 dataSize)
{
    m_eventQueue->AddTask(new QALogWriteDataTask(m_context, data, dataSize));
}

/*! @} */

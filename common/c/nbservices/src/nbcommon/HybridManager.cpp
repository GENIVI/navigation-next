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

#include "HybridManager.h"
#include "nbcontextprotected.h"
#include "contextbasedsingleton.h"

namespace nbcommon {

void NotifyHybridModeChanged(HybridTask* pTask)
{
    list<HybridModeListener*>::iterator iter = pTask->m_listeners.begin();
    list<HybridModeListener*>::iterator end = pTask->m_listeners.end();
    for(; iter != end; iter++)
    {
        (*iter)->HybridModeChanged(pTask->m_mode);
    }
}

shared_ptr<HybridManager> HybridManager::GetInstance(NB_Context* nbContext)
{
    return ContextBasedSingleton<HybridManager>::getInstance(nbContext);
}

void HybridManager::OnHybridModeChanged()
{
    HybridMode mode = m_pHybridStrategy->getMode();
    if(m_currentMode != mode)
    {
        m_currentMode = mode;
        m_eventQueue->AddTask(new HybridTask(&NotifyHybridModeChanged, m_hybridModeListeners, mode));
    }
}

void HybridManager::SetHybridStrategy(shared_ptr<HybridStrategy> hybridStrategy)
{
    m_pHybridStrategy = hybridStrategy;
    OnHybridModeChanged();
}

HybridMode HybridManager::GetMode() const
{
    return m_currentMode;
}

void HybridManager::AddListener(HybridModeListener* hybridModeListener)
{
    if(hybridModeListener)
    {
        m_hybridModeListeners.push_back(hybridModeListener);
    }
}

void HybridManager::RemoveListener(HybridModeListener *hybridModeListener)
{
    if (hybridModeListener)
    {
        list<HybridModeListener*>::iterator iter = m_hybridModeListeners.begin();
        list<HybridModeListener*>::iterator end = m_hybridModeListeners.end();
        for(; iter != end; iter++)
        {
            if(*iter == hybridModeListener)
            {
                m_hybridModeListeners.erase(iter);
                break;
            }
        }
    }
}

HybridManager::HybridManager(NB_Context* nbContext)
    :m_nbContext(nbContext),
      m_currentMode(HBM_OFFBOARD_ONLY)
{
    m_eventQueue = shared_ptr<EventTaskQueue>(new EventTaskQueue(NB_ContextGetPal(nbContext)));
}

HybridManager::~HybridManager()
{
    m_hybridModeListeners.clear();
}

NB_Context *HybridManager::GetContext()
{
    return m_nbContext;
}

void HybridTask::Execute()
{
    m_function(this);
    delete this;
}

HybridModeListener::HybridModeListener(NB_Context *nbContext)
    :m_nbContext(nbContext)
{
    HybridManager::GetInstance(nbContext)->AddListener(this);
}

HybridModeListener::~HybridModeListener()
{
    HybridManager::GetInstance(m_nbContext)->RemoveListener(this);
}

}

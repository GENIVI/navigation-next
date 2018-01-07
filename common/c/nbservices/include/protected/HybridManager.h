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
   (C) Copyright 2016 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/
#ifndef HYBRIDMANAGER_H
#define HYBRIDMANAGER_H
#include "LayerManager.h"
#include <functional>
#include "nbtaskqueue.h"
#include "HybridStrategy.h"
#include <list>

namespace nbcommon {

class HybridTask;
class HybridStrategy;
class HybridModeListener;

typedef void (*HybridTaskFunction)(HybridTask*);

class HybridManager
{
public:
    static shared_ptr<HybridManager> GetInstance(NB_Context* nbContext);
    HybridManager(NB_Context* nbContext);
    ~HybridManager();

public:
    NB_Context* GetContext();
    void OnHybridModeChanged();
    void SetHybridStrategy(shared_ptr<HybridStrategy> hybridStrategy);
    HybridMode GetMode() const;
    void AddListener(HybridModeListener* hybridModeListener);
    void RemoveListener(HybridModeListener* hybridModeListener);

private:
    std::list<HybridModeListener*> m_hybridModeListeners;
    shared_ptr<EventTaskQueue> m_eventQueue;
    shared_ptr<HybridStrategy> m_pHybridStrategy;
    NB_Context* m_nbContext;
    HybridMode m_currentMode;
};

class HybridTask : public Task
{
public:
    HybridTask(HybridTaskFunction function, std::list<HybridModeListener*> listeners, HybridMode mode)
        :m_listeners(listeners),m_mode(mode),m_function(function){}
    virtual ~HybridTask(){}

    // Refer to class Task for description.
    virtual void Execute(void);

public:
    std::list<HybridModeListener*> m_listeners;
    HybridMode m_mode;
protected:
    shared_ptr<bool> m_isValid;         /*!< Flag to indicate if MapView instance is valid. */

private:
    HybridTaskFunction m_function;
};

class HybridModeListener
{
public:
    HybridModeListener(NB_Context* nbContext);
    ~HybridModeListener();

public:
    virtual void HybridModeChanged(HybridMode mode) = 0;

private:
    NB_Context* m_nbContext;
};

}

#endif // HYBRIDMANAGER_H

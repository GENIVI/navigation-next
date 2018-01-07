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
 @file     NativeRenderContext.cpp
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "NativeRenderContext.h"
#include <stdio.h>
#ifdef Q_OS_UNIX
#include <sched.h>
#endif
#include <QDebug>
#include "paltestlog.h"
#include "renderthreadqt.h"

// porting to telematics es1
NativeRenderContext::NativeRenderContext(void* widget, NBGM_RenderSystemType renderSystemType)
    : m_widget((RenderThreadQt*)widget)
    , mRenderSystemType(renderSystemType)
    , mIsBackground(false)
    , mSuspend(false)
    , mRenderBallOwner(Nobody)
{}

NativeRenderContext::~NativeRenderContext()
{}

const void* NativeRenderContext::GetConfiguration(const char* name)
{
    return (void*)(&mRenderSystemType);
}

void NativeRenderContext::Initialize()
{
    if (mIsBackground)
    {
        return;
    }
    m_widget->makeCurrent();
}

void NativeRenderContext::Finalize()
{
}

int NativeRenderContext::GetRenderTarget()
{
    return m_widget->displayRenderTargetId();
}

bool NativeRenderContext::BeginRender()
{
    mRenderBallMutex.lock();
    while (mRenderBallOwner == GUI)
        mRenderBallAvailableCondition.wait(&mRenderBallMutex);
    mRenderBallOwner = Renderer;
    mRenderBallMutex.unlock();
    //qWarning() << "NBUI Renderer has the render ball";

    m_widget->makeCurrent();
    return true;
}

bool NativeRenderContext::EndRender()
{
    m_widget->swapBuffers();


    mRenderBallMutex.lock();
    mRenderBallOwner = Nobody;
    bool yield = mSuspend;
    //qWarning() << "NBUI Renderer releases the render ball";
    mRenderBallAvailableCondition.wakeAll();
    mRenderBallMutex.unlock();

#ifdef Q_OS_UNIX
    if (yield)
        sched_yield();
#endif
    return true;
}

void NativeRenderContext::Reset()
{
    //do nothing now
}

bool NativeRenderContext::SwitchTo()
{
    if (mIsBackground)
    {
        return false;
    }
    m_widget->makeCurrent();
    return true;
}

//void NativeRenderContext::UpdateSurface(QGLWidget* widget)
//{
//    //m_widget = widget;
//}

void NativeRenderContext::EnterBackground(bool val)
{
    if (val)
    {
        // Take the render ball away from the render thread.
        mRenderBallMutex.lock();
        mSuspend = true;
        while (mRenderBallOwner == Renderer)
            mRenderBallAvailableCondition.wait(&mRenderBallMutex);
        mRenderBallOwner = GUI;
        mRenderBallMutex.unlock();
        qWarning() << "NBUI GUI has the render ball";
    }
    else
    {
        mRenderBallMutex.lock();
        if (mRenderBallOwner == GUI)
        {
            mRenderBallOwner = Nobody;
            mSuspend = false;
            qWarning() << "NBUI GUI releases the render ball";
            mRenderBallAvailableCondition.wakeAll();
        }
        mRenderBallMutex.unlock();
    }
}

/*! @} */

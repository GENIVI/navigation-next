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

 @file NativeRenderContext.h

 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret
 as defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly prescribing
 the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef _NATIVE_RENDER_CONTEXT_H_
#define _NATIVE_RENDER_CONTEXT_H_

#include "nbgmrendersystemtype.h"
#include "nbgmrendercontext.h"
#include <QMutex>
#include <QWaitCondition>

class QGLWidget;
class RenderThreadQt;
/*
    Implement NativeRenderContext on Qt
*/
class NativeRenderContext : public NBGM_RenderContext
{
public:
    NativeRenderContext(void* widget, NBGM_RenderSystemType renderSystemType);
    virtual ~NativeRenderContext();

public:
    int GetRenderTarget();
    /*! Initialize the 3D API driver: create swap chain, create default frame buffer, etc. */
    void Initialize();
    /*! Close this context. Clean up all render resource. */
    void Finalize();
    /*! Prepare to render a frame. */
    bool BeginRender();
    /*! End a frame. */
    bool EndRender();
    /*! Reset render state to default. Including type, size, and the format of the swap chain. */
    void Reset();
    /*! Switch current native render context to this context. */
    bool SwitchTo();
    /*! Get render context attributes. */
    const void* GetConfiguration(const char* name);
    /*!
     Before app entering background, we should call this function to inform ccc to stop rendering.
     */
    void EnterBackground(bool val);
    /*! Update OpenGL Render Surface. */
    //void UpdateSurface(QGLWidget* window);

private:
    RenderThreadQt* m_widget;
    NBGM_RenderSystemType mRenderSystemType;
    bool mIsBackground;

    bool mSuspend;
    QMutex mRenderBallMutex;
    QWaitCondition mRenderBallAvailableCondition;
    enum { Nobody, GUI, Renderer } mRenderBallOwner;
};


#endif //_NATIVE_RENDER_CONTEXT_H_
/*! @} */

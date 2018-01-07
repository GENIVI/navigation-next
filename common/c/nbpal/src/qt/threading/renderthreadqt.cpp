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

    @file     threadqt.cpp
    @date     02/15/2015
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for Qt thread.

    Qt implementation for PAL thread.
*/
/*
    (C) Copyright 2015 by Telecommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Telecommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Telecommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "renderthreadqt.h"
#include "palstdlib.h"
#include "taskqueue.h"
#include "task.h"
#include <QThread>
#include <QOpenGLFunctions>

RenderThreadQt::RenderThreadQt(PAL_Instance* pal, nimpal::taskqueue::TaskQueue* taskQueue, nimpal::taskqueue::ThreadStateListener* listener, const char* name)
    : ThreadQt(pal, taskQueue, listener, name)
    , surface(0)
    , context(0)
    , m_renderFbo(0)
    , m_displayFbo(0)
//        , m_logoRenderer(0)
    , m_size(1024,1024)
{
    //ThreadRenderer::threads << this;
}

void RenderThreadQt::renderNext()
{
    context->makeCurrent(surface);

    if (!m_renderFbo) {
        // Initialize the buffers and renderer
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        m_renderFbo = new QOpenGLFramebufferObject(m_size, format);
        m_displayFbo = new QOpenGLFramebufferObject(m_size, format);
//            m_logoRenderer = new LogoRenderer();
//            m_logoRenderer->initialize();
    }

    //m_renderFbo->bind();
    //context->functions()->glViewport(0, 0, m_size.width(), m_size.height());

//        m_logoRenderer->render();

    // We need to flush the contents to the FBO before posting
    // the texture to the other thread, otherwise, we might
    // get unexpected results.
    //context->functions()->glFlush();

    m_renderFbo->bindDefault();
    qSwap(m_renderFbo, m_displayFbo);

    emit textureReady(m_displayFbo->texture(), m_size);
}

void RenderThreadQt::shutDown()
{
    context->makeCurrent(surface);
    delete m_renderFbo;
    delete m_displayFbo;
//        delete m_logoRenderer;
    context->doneCurrent();
    delete context;

    // schedule this to be deleted only after we're done cleaning up
    surface->deleteLater();

    // Stop event processing, move the thread to GUI and make sure it is deleted.
    exit();
    moveToThread(QGuiApplication::instance()->thread());
}

PAL_Error RenderThreadQt::StartThread(void)
{
    return ThreadQt::StartThread();
}

void RenderThreadQt::makeCurrent()
{
    context->makeCurrent(surface);

    if (!m_renderFbo) {
        qDebug("RenderThreadQt::makeCurrent() init FBOs");
        // Initialize the buffers and renderer
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        m_renderFbo = new QOpenGLFramebufferObject(m_size, format);
        m_displayFbo = new QOpenGLFramebufferObject(m_size, format);
//            m_logoRenderer = new LogoRenderer();
//            m_logoRenderer->initialize();
    }else
    {
        for (int i = 0; i < m_fbosToDelete.size(); ++i)
        {
            delete m_fbosToDelete[i];
        }
        m_fbosToDelete.clear();
        if (m_size != m_renderFbo->size())
        {
            delete m_renderFbo;
            delete m_msaaFbo;
            m_fbosToDelete.push_back(m_displayFbo);

            QOpenGLFramebufferObjectFormat format;
            format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            m_renderFbo = new QOpenGLFramebufferObject(m_size, format);
            m_displayFbo = new QOpenGLFramebufferObject(m_size, format);
            format.setSamples(4);
            m_msaaFbo = new QOpenGLFramebufferObject(m_size, format);
        }
    }
    m_renderFbo->bind();
}

void RenderThreadQt::swapBuffers()
{
    glFinish();
    m_renderFbo->bindDefault();
    qSwap(m_renderFbo, m_displayFbo);

    emit textureReady(m_displayFbo->texture(), m_size);
}

int RenderThreadQt::displayRenderTargetId()
{
   return (int)m_renderFbo->handle();
}

void RenderThreadQt::setSize(int width, int height)
{
   m_size.setWidth(width);
   m_size.setHeight(height);
}

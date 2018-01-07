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
 
 @file NBUI_GLESDrawContext.m
 
 See file description in header file.
 
 */
/*
 (C) Copyright 2010 by TeleCommunication Systems, Inc.
 
 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret
 as defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly prescribing
 the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/

/*! @{ */

#import "NBUI_GLESFrameBuffer.h"
#include "palgl2.h"

@implementation NBUI_GLESFrameBuffer

@synthesize frameBufferID;
@synthesize renderBufferID;
@synthesize width;
@synthesize height;

- (NBUI_GLESFrameBuffer*)init:(NBUI_GLESDrawContext*)glesContext withView:(UIView*)uiView withFrameScale:(int)scale
{
    context = [glesContext retain];
    [EAGLContext setCurrentContext:glesContext.context];

    // Create default framebuffer object.
    glGenFramebuffersOES( 1, &frameBufferID );
    glBindFramebufferOES(GL_FRAMEBUFFER, frameBufferID );

    // Create color render buffer and allocate backing store.
    glGenRenderbuffersOES( 1, &renderBufferID );
    glBindRenderbufferOES( GL_RENDERBUFFER, renderBufferID );

    // Set the scale factor and DON'T resize the UIView here, this could have unexpected side effects!
    uiView.contentScaleFactor = scale;

    [glesContext.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:( id<EAGLDrawable> )uiView.layer];

    glGetRenderbufferParameterivOES( GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width );
    glGetRenderbufferParameterivOES( GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height );

    glFramebufferRenderbufferOES( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBufferID);

    glGenRenderbuffersOES( 1, &depthBufferID );
    glBindRenderbufferOES( GL_RENDERBUFFER, depthBufferID );
    glRenderbufferStorageOES( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height );
    glFramebufferRenderbufferOES( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID );

    if ( glCheckFramebufferStatusOES( GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES( GL_FRAMEBUFFER ) );

    return self;
}

- (void)dealloc
{
    [EAGLContext setCurrentContext:context.context];

    glDeleteFramebuffersOES( 1, &frameBufferID );
    glDeleteRenderbuffersOES( 1, &renderBufferID );
    glDeleteRenderbuffersOES( 1, &depthBufferID );

    [context release];
    [super dealloc];
}

@end

/*! @} */


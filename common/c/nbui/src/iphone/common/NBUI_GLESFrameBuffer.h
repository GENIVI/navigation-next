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
 
 @file NBUI_GLESFrameBuffer.h

 
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

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import "NBUI_GLESDrawContext.h"

@class NBUI_GLESDrawContext;

/*! GLESFrameBuffer class.
 
 This class is used to generate, set up and bind frame buffer, render buffer, depth buffer. 
 Get storage for all buffers and attach the frame buffer to the render buffer. 
 
 */

@interface NBUI_GLESFrameBuffer : NSObject {
@private
    NBUI_GLESDrawContext* context;   // the draw context
    UIView* view;               // view associated with the buffer
    GLuint frameBufferID;       // frame buffer id
    GLuint renderBufferID;      // render buffer id
    GLuint depthBufferID;       // depth buffer id
    int frameScale;
    GLint width;
    GLint height;
}

@property (nonatomic, readonly)GLuint frameBufferID;
@property (nonatomic, readonly)GLuint renderBufferID;
@property (nonatomic, readonly)GLint width;
@property (nonatomic, readonly)GLint height;

- (NBUI_GLESFrameBuffer*)init:(NBUI_GLESDrawContext*)glesContext withView:(UIView*)uiView withFrameScale:(int)scale;

@end

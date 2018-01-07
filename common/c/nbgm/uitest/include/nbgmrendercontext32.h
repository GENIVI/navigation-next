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
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#ifndef _NBGM_RenderContextW32_H
#define _NBGM_RenderContextW32_H
#include "nbgmrendercontext.h"
#include "NBGMTestView.h"
#include "nbgmrendersystemtype.h"
#include "d3d9.h"

class NBGM_RenderContextW32:public NBGM_RenderContext
{
public:
    NBGM_RenderContextW32(CNBGMTestView* nbgmNativeView, const NBGM_RenderSystemType& renderSystemType);
    ~NBGM_RenderContextW32();

public:
    void AssignD3DDevice(IDirect3D9* d3D9, IDirect3DDevice9* d3D9Device);

public:
    virtual void Finalize();
    virtual void Initialize();
    virtual bool EndRender();
    virtual const void* GetConfiguration( const char* name );
    virtual bool BeginRender();
    virtual void Reset();
    virtual bool SwitchTo();
    virtual void EnterBackground(bool val){}

private:
    CNBGMTestView* mNBGMNativeView;
    NBGM_RenderSystemType mRenderSystemType;
    IDirect3D9* mD3D9;
    IDirect3DDevice9* mD3D9Device;
};

#endif

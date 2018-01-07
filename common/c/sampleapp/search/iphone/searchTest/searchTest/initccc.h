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

//
//  initccc.h
//  NavUIKitTest
//

#ifndef NavUIKitTest_initccc_h
#define NavUIKitTest_initccc_h

#include "nbcontext.h"
#include <vector>

@protocol CCCEventListener <NSObject>

- (void) initCCCComplete;

@end

class CCCComponent
{
public:
    CCCComponent(void);
    ~CCCComponent(void);

public:
    void Initialize(void);

    void AddListener(void* pListener);

    PAL_Instance* GetPalInstance(void) const;
    NB_Context* GetContext(void) const;

private:
    static void CCC_Initialize(PAL_Instance* pal, void* userData);
    nb_boolean CCC_CreateContext(PAL_Instance* pal, const char* credential, const char* hostname, NB_NetworkProtocol protocol, uint16 port, NB_Context** context);
    
    
    static void UI_InitComplete(PAL_Instance* pal, void* userData);
    void UI_InitComplete(void);

private:
    PAL_Instance*                       m_pPal;
    NB_Context*                         m_pContext;
    std::vector<void*>                  m_listenerVector;
};

uint8 CreatePalAndContext(PAL_Instance**pal, NB_Context** context);

#endif

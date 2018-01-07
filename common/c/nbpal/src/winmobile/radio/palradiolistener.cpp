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

    @file     palradiolistener.cpp
    @date     11/09/2010
    @defgroup PAL_RADIO PAL Radio Functions

    @brief    Classes for PAL Radio base listeners.

    Windows implementation for PAL Radio base listeners.
*/
/*
(C) Copyright 2010 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "palradiolistener.h"
#include "palwindow.h"
#include "finishfunctionnormal.h"

namespace nimpal
{
    namespace radio
    {
        PAL_Instance* PalRadioListener::m_pal;

        PalRadioListener::PalRadioListener()
        {
            m_ownerId = PALWindow::Instance()->GetFinishFunctionManager()->RegisterOwner();
        }
        
        PalRadioListener::~PalRadioListener()
        {
            if (m_ownerId > 0)
            {
                PALWindow::Instance()->GetFinishFunctionManager()->DeleteScheduledFunctions(m_ownerId);
            }
        }

        void PalRadioListener::CallInPalThread(PalRadioListener* pThis)
        {
            FinishFunctionNormal* function = new FinishFunctionNormal(Static_PalThreadCallerFunc, pThis);
            PALWindow::Instance()->GetFinishFunctionManager()->Schedule(0, function, NULL);
        }

        void PalRadioListener::Static_PalThreadCallerFunc(void* params)
        {
            PalRadioListener* pThis = (PalRadioListener*)params;
            pThis->PalThreadCallerFunc();
            pThis->m_ownerId = 0;
        }

        void PalRadioListener::DestroyListenerAsync()
        {
            PAL_TimerSet(m_pal, 1, Static_DestroyTimerCb, this);
        }

        void PalRadioListener::Static_DestroyTimerCb(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
        {
            PalRadioListener* pThis = (PalRadioListener*)userData;
            delete pThis;
        }
    }
}

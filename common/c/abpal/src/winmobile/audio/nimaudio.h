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

@file     NimAudio.h
@date     03/02/2009
@defgroup PAL Audio API Base Class

@brief    Audio Base Class for Windows Mobile

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/


/*! @( */

#ifndef NIMAUDIO_H
#define NIMAUDIO_H

#pragma once

#include "abpalaudio.h"
#include "windows.h"

typedef enum
{
    Mono = 1,
    Stereo
} AudioChannels;

class NIMAudio
{
public:
    NIMAudio(PAL_Instance* pPal, ABPAL_AudioFormat format);
    virtual ~NIMAudio();

    ABPAL_AudioState    GetState() { return m_State; }
    ABPAL_AudioFormat   GetFormat() { return m_Format; }
    void                SetFormat(ABPAL_AudioFormat format) { m_Format = format; }

protected:
    void                SetState(ABPAL_AudioState state) { m_State = state; }

    PAL_Instance*       m_pPal;

    HANDLE              m_MediaEventHandle;

    ABPAL_AudioState    m_State;
    ABPAL_AudioFormat   m_Format;

private:

};

#endif	// NIMAUDIO_H


/*! @} */

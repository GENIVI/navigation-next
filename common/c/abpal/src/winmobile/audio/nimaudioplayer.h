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

    @file     nimaudioplayer.cpp
    @date     02/27/2009
    @defgroup PAL Audio API Player

    @brief    Audio Player for Windows Mobile

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

#ifndef __NIMAUDIOPLAYER_H__
#define __NIMAUDIOPLAYER_H__

#pragma once

#include "nimaudio.h"

#include "dshow.h"
#include "soundfile.h"
#include "mmsystem.h"


class NIMAudioPlayer : public NIMAudio
{
public:
	NIMAudioPlayer(PAL_Instance *pPal, ABPAL_AudioFormat format);
	~NIMAudioPlayer(void);

    void SetVolumeLevel(ABPAL_AudioVolumeLevel level);
    ABPAL_AudioVolumeLevel GetVolumeLevel(void);
    
	PAL_Error Play(byte *buffer, int bufferSize, nb_boolean takeOwnership, ABPAL_AudioPlayerCallback* callback, void* callbackUserData);
	nb_boolean Stop(void);

protected:
    void SetState(ABPAL_AudioState state);
    
private:
    HRESULT SetupInterfaces(void);
    void TearDownInterfaces(void);
    void DeleteTemporaryFile(void);
    
    void GetCurrentDeviceVolume(void);
    void ResetDeviceVolume(void);

    nb_boolean SetDeviceVolumeLevel(void);
    
    HRESULT Render(const byte *buffer, uint32 bufferSize);

    static const TCHAR* GetAudioFormatFileExtension(ABPAL_AudioFormat format);

    static DWORD _stdcall NotifyThreadProc(LPVOID pParameter);
    void NotifyThread(void);

    ABPAL_AudioVolumeLevel      m_volumeLevel;
           
    // DirectShow interfaces
    IGraphBuilder*              m_pGraph;
    IMediaControl*              m_pMediaControl;
    IMediaEvent*                m_pEvent;
    IBasicAudio*                m_pBasicAudio;
    
    // previous waveout device info
    nb_boolean                  m_bVolumeReset;
    uint32			            m_oldVolume;
    int				            m_oldSoundType;

    // audio buffer    
    byte*                       m_Buffer;
    int                         m_BufferSize;
	TCHAR                       m_TempFileName[MAX_PATH + 1];
	
	// callback
	ABPAL_AudioPlayerCallback*  m_Callback; 
	void*                       m_CallbackUserData;

    // notify thread
    HANDLE                      m_NotifyThread;
    HANDLE                      m_NotifyThreadStopEvent;
};

#endif	// __NIMAUDIOPLAYER_H__


/*! @} */

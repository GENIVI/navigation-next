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
    @file         PreservedAudioAnnouncement.cpp
    @defgroup     nkui
*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary to
    TeleCommunication Systems, Inc., and considered a trade secret as defined
    in section 499C of the penal code of the State of California. Use of this
    information by anyone other than authorized employees of TeleCommunication
    Systems is granted only under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
    --------------------------------------------------------------------------
*/

/*! @{ */

#include "PreservedAudioAnnouncement.h"
#include "abpalaudio.h"
#include "palfile.h"

using namespace nkui;
using namespace nbnav;

static const char* s_audioDataType[AudioDataType_Num] = {
    "audio/aac"
    , "ipa"
};

PreservedAudioAnnouncement::PreservedAudioAnnouncement(PAL_Instance* pal, const char* text, const char* filePath, AudioDataType type)
    : m_audioData(NULL, 0)
{
    if (text != NULL)
    {
        m_text = text;
    }
    if (type >= AudioDataType_Num)
    {
        type = AudioDataType_TTS;
    }
    m_audioMimeType = s_audioDataType[type];
    if (filePath != NULL)
    {
        ABPAL_AudioCombiner* pCombiner = NULL;
        if (ABPAL_AudioCombinerCreate(pal, ABPAL_AudioFormat_AAC, &pCombiner) == PAL_Ok && pCombiner != NULL)
        {
            // Read audio data
            void* pData = NULL;
            size_t dataSize = 0;
            if (PAL_FileLoadFile(pal, filePath, (unsigned char**)&pData, (uint32*)&dataSize) == PAL_Ok)
            {
                if (pData && dataSize > 0)
                {
                    (void)ABPAL_AudioCombinerAddBuffer(pCombiner, (byte*)pData, (int)dataSize, true);
                    nsl_free(pData);
                }
            }

            // Envelope data to buffer
            byte* buffer = NULL;
            int bufferSize = 0;
            (void) ABPAL_AudioCombinerGetData(pCombiner, &buffer, &bufferSize);
            if (buffer)
            {
                BinaryBuffer tempBuffer(buffer, bufferSize);
                m_audioData = tempBuffer;
                nsl_free(buffer);
            }
            (void) ABPAL_AudioCombinerDestroy(pCombiner);
        }
    }
}

PreservedAudioAnnouncement::~PreservedAudioAnnouncement()
{
}

BinaryBuffer PreservedAudioAnnouncement::GetAudio() const
{
    return m_audioData;
}

void PreservedAudioAnnouncement::OnAnnouncementStart() const
{
}

void PreservedAudioAnnouncement::OnAnnouncementComplete()
{
}

std::string PreservedAudioAnnouncement::GetText() const
{
    return m_text;
}

std::string PreservedAudioAnnouncement::GetAudioMimeType() const
{
    return m_audioMimeType;
}

const std::vector<AudioClip> PreservedAudioAnnouncement::GetAudioClip() const
{
    return m_audioClip;
}

nbnav::SpeechSynthesis& PreservedAudioAnnouncement::GetSpeechSynthesis()
{
    return m_speech;
}
/*! @} */

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
   @file        abpalttsengine.h
   @defgroup    abpal
*/
/*
   (C) Copyright 2014 by TeleCommunication Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#ifndef _ABPALTTSENGINE_H_
#define _ABPALTTSENGINE_H_

#include <string>
#include "smartpointer.h"
#include "paltypes.h"
#include "abpalaudio.h"

namespace abpal
{

/*! Enumerations of behaviours of how tts engine should interact with other
    audios sources.
 */
typedef enum _EngineOutputType
{
    EOT_MuteOthers,     /*!< Mute others */
    EOT_MixWithOthers,  /*!< Mix with others  */
    EOT_DuckOthers,     /*!< Duck others  */
    EOT_Invalid,        /*!< Helper ID to check whether EOT is valid or not */
} EngineOutputType;

/*! Enumerations of voice styles. */
typedef enum _EngineVoiceType
{
    EVT_Unspecified,    /*!< Unspecified, Engine can pick one randomly. */
    EVT_Male,           /*!< Male voice */
    EVT_Female,         /*!< Female voice */
    EVT_Invalid,        /*!< Helper ID to check whether EOT is valid or not */
} EngineVoiceType;

/*! Enumerations of Engine Errors. */
typedef enum _EngineError
{
    EE_NotInitialized,   /*!< Not Initialize  */
    EE_Canceled,        /*!< Canceled  */
    EE_InternalError,   /*!< Internal Error  */
    EE_NoMemory,        /*!< No Memory  */
} EngineError;


/*! A representation of data used by TTS Engine to play announcements.

    TTSEngine needs to get proper data to play announcements. You can choose
    to use this class directly, or inherit from this class and implement on
    your own.
 */
class TTSData
{
public:
    /*! Constructor */
    TTSData(const std::string& mimeType,   /*!< Mime type of this phonetic. */
            const std::string& phonetics,  /*!< Phonetics in TTSData. TTSEngine should use
                                                phonetics to speak if possible. */
            const std::string& rawText,    /*!< Raw text of this TTSData, TTSEngine can use
                                                raw data if it does not support phonetics. */
            int32 dataID                   /*!< The ID of current data which can be used with
                                                TTSEngineListenter callback. */
            )
            : m_mimeType(mimeType),
              m_phonetics(phonetics),
              m_rawText(rawText),
              m_dataID(dataID)
    {
    }

    virtual ~TTSData()
    {
    }

    const std::string& GetMimeType() const
    {
        return m_mimeType;
    }

    const std::string& GetPhonetics() const
    {
        return m_phonetics;
    }

    const std::string& GetRawText() const
    {
        return m_rawText;
    }

    const int32 GetDataID() const
    {
        return m_dataID;
    }

private:
    std::string m_mimeType;
    std::string m_phonetics;
    std::string m_rawText;
    int32       m_dataID;
};

typedef shared_ptr<TTSData> TTSDataPtr;


class TTSEngine;
typedef shared_ptr<TTSEngine> TTSEnginePtr;

/*! Listeners for TTSEngine. */
class TTSEngineListenter
{
public:
    virtual ~TTSEngineListenter(){}

    /*! Notify listener completed in playing successfully */
    virtual void OnPlayCompleted(int32 dataID) = 0;

    /*! Notify listener error occurred during play */
    virtual void OnPlayError(int32 dataID, EngineError error) = 0;
};

/*! Platform dependent TTSEngine. */
class TTSEngine
{
public:
    /*! Create an TTSEngine instance with given paramters. */
    static TTSEnginePtr
    CreateInstance(const char*      language,   /*!< Language Tag, follow BCP-47 standard. */
                   EngineOutputType outputType, /*!< Specify style of interacting with other sources*/
                   EngineVoiceType   voiceType  /*!< Voice style of this desired engine. */
                   );

    virtual ~TTSEngine(){}

    virtual void SetListener(TTSEngineListenter* listener) = 0;

    /*! Stop playing */
    virtual void Stop() = 0;

    /*! Play new TTSData */
    virtual void Play(TTSDataPtr data,      /*!< Data to be played */
                      bool preempt = true   /*!< Flag to indicate whether to preempt or not.
                                                 If set to yes, TTSEngine will stop playing
                                                 current data, and play the new one. */
                      ) = 0;

    /*! Check if it is playing or not. */
    virtual bool IsPlaying() const = 0;

    /*! Set volume. Allowed values are in the range from 0.0 (silent) to 1.0 (loudest). The default volume is 1.0.*/
    virtual void SetVolume(ABPAL_AudioVolumeLevel volume) = 0;
};

}

#endif /* _ABPALTTSENGINE_H_ */

/*! @} */

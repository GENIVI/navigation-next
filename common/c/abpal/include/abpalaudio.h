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
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
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

/*!--------------------------------------------------------------------------

 @file     palaudio.h
 @date     01/16/2009
 @defgroup PAL Audio API

 @brief    Platform-independent Audio API

 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.           

 The information contained herein is confidential, proprietary 
 to TeleCommunication Systems, Inc., and considered a trade secret as 
 defined in section 499C of the penal code of the State of     
 California. Use of this information by anyone other than      
 authorized employees of TeleCommunication Systems is granted only    
 under a written non-disclosure agreement, expressly           
 prescribing the scope and manner of such use.                 

 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PALAUDIO_H
#define PALAUDIO_H

#include "pal.h"
#include "paltypes.h"
#include "palerror.h"
#include "abpalexp.h"

/*! Audio formats. */
typedef enum {
    ABPAL_AudioFormat_UNK = 0,        /* Unknown Format */
    ABPAL_AudioFormat_AMR,
    ABPAL_AudioFormat_CMX,
    ABPAL_AudioFormat_QCP,
    ABPAL_AudioFormat_WAV,
    ABPAL_AudioFormat_AU,
    ABPAL_AudioFormat_GSM,
    ABPAL_AudioFormat_WVE,
    ABPAL_AudioFormat_RAW,
    ABPAL_AudioFormat_AAC,
    ABPAL_AudioFormat_MP3,
    ABPAL_AudioFormat_OGG,
    ABPAL_AudioFormat_SPX

} ABPAL_AudioFormat;

/*! Audio states. */
typedef enum {
    ABPAL_AudioState_Unknown = 0,
    ABPAL_AudioState_Init,
    ABPAL_AudioState_Playing,
    ABPAL_AudioState_Recording,
    ABPAL_AudioState_Ended,
    ABPAL_AudioState_Error,
    ABPAL_AudioState_Stopped,
    ABPAL_AudioState_Paused,
    ABPAL_AudioState_Cancel,
    ABPAL_AudioState_ReadyToRecord,
    ABPAL_AudioState_StreamRecordBufferAvailable
} ABPAL_AudioState;

/*! Audio parameters. */
typedef enum {
    ABPAL_AudioParameter_Output = 0,
    ABPAL_AudioParameter_Volume,
    ABPAL_AudioParameter_SoundDevice,
    ABPAL_AudioParameter_BT,
    ABPAL_AudioParameter_BT_SpeakerVolume,
    ABPAL_AudioParameter_BT_MicrophoneVolume,
    ABPAL_AudioParameter_AudioSession,                 /*! param1 passed to SetParameter method should be one from ABPAL_AudioSessionType. */
    ABPAL_AudioParameter_StreamRecordingMode,          /*! param1 passed to SetParameter used to customize buffer duration in msecs. It can be 0, to keep default value. */
    ABPAL_AudioParameter_LoudnessLevel,                /*! Used to get current loudness level (0 - 100) for audio receiver first channel. param1 used as output and should points to int32. */
    ABPAL_AudioParameter_SampleRate,                   /*! Used to get sample rate. param1 used as output and should points to int32. */
    ABPAL_AudioParameter_BitsPerSample                 /*! Used to get bits per sample. param1 used as output and should points to int32. */
} ABPAL_AudioParameter;

/*! Audio volume level. */
typedef enum {
    ABPAL_AudioVolumeLevel_Off = 0,
    ABPAL_AudioVolumeLevel_Low,
    ABPAL_AudioVolumeLevel_Medium,
    ABPAL_AudioVolumeLevel_High,
    ABPAL_AudioVolumeLevel_VeryHigh
} ABPAL_AudioVolumeLevel;

/*! Audio session type. */
typedef enum {
    ABPAL_AudioSession_Silent = 0,
    ABPAL_AudioSession_Play,
    ABPAL_AudioSession_Record,
    ABPAL_AudioSession_PlayAndRecord  /*! Allows playing and recording simultaneously. This session is not interrupted with ABPAL_AudioSession_Play and ABPAL_AudioSession_Record. */
} ABPAL_AudioSessionType;

/*! AudioPlayer Interface Definition */
typedef struct ABPAL_AudioPlayer ABPAL_AudioPlayer;

/*! AudioRecorder Interface Definition */
typedef struct ABPAL_AudioRecorder ABPAL_AudioRecorder;

/*! AudioCombiner Interface Definition */
typedef struct ABPAL_AudioCombiner ABPAL_AudioCombiner;

/*! AudioSession Interface Definition */
typedef void* ABPAL_AudioSession;

/*! Callback used for AudioPlayer functions */
typedef void (ABPAL_AudioPlayerCallback)(
                                       void* userData, /*!< pointer to data provided by user */
                                       ABPAL_AudioState state /*!< current ABPAL_AudioState */
                                       );

/*! Callback used for AudioRecorder functions */
typedef void (ABPAL_AudioRecorderCallback)(
                                        void* userData,     /*!< pointer to data provided by user */
                                        ABPAL_AudioState state    /*!< current ABPAL_AudioState */
                                        );

/*! Creates an instance of the AudioPlayer

Call this function first before any other ABPAL_AudioPlayerXXX API functions to create the AudioPlayer instance. 
The caller is responsible for saving the pointer pointed to by the OUT parameter ppplayer.
This ABPAL_AudioPlayer pointer is used on all subsequent calls to PAL AudioPlayer API.
When finished with the player, the user must call ABPAL_AudioPlayerDestroy to close and free the resources used 
by the ABPAL_AudioPlayer.

@return PAL_Error on error or PAL_Ok if the ABPAL_AudioPlayer is created successfully

@see ABPAL_AudioFormat
@see ABPAL_AudioPlayerDestroy
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioPlayerCreate(
        PAL_Instance* pal,                                  /*!< PAL instance */
        ABPAL_AudioFormat format,                           /*!< Audio format to use for this player */
        ABPAL_AudioPlayer** player                          /*!< [OUT parameter] that will return a pointer to the ABPAL_AudioPlayer.
                                                                 The caller must pass this player to every ABPAL_AudioPlayerXXXX API */
    );

/*! Release the AudioPlayer instance

This function stops play, closes the AudioPlayer and releases all associated resources.

@return nothing

@see ABPAL_AudioPlayerCreate
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioPlayerDestroy(
                       ABPAL_AudioPlayer* player            /*!< Pointer to the player returned on call to ABPAL_AudioPlayerCreate */
                       );

/*! Plays the specified audio data

This function plays the audio data supplied in buffer.  If the data is not of the correct ABPAL_AudioFormat specified in the ABPAL_AudioPlayerCreate
function, it will return an error.

@return PAL_Error or PAL_Ok if successfully destroyed

@see ABPAL_AudioPlayerCreate
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioPlayerPlay(
                    ABPAL_AudioPlayer* player,              /*!< Pointer to the player returned on call to ABPAL_AudioPlayerCreate */
                    byte* buffer,                           /*!< Pointer to the data to play */
                    int bufferSize,                         /*!< Size of the data specified in buffer */
                    nb_boolean takeOwnership,               /*!< Flag to specify that the ownership of the buffer will be the responsibility 
                                                                 of the ABPAL_AudioPlayer.  If this flag is set to true, the ABPAL_AudioPlayer will 
                                                                 free the memory associated with the buffer when the ABPAL_AudioPlayer is destroyed.  
                                                                 The buffer will also be freed if the ABPAL_AudioPlayerPlay function is called again.*/
                    ABPAL_AudioPlayerCallback* callback,    /*!< Callback function called for any audio events */
                    void* callbackData                      /*!< Data passed back to the callback function specified in callback */
                    );

/*! Plays the specified combined audio data

This function plays the audio data supplied in combiner.

@return PAL_Error

@see ABPAL_AudioPlayerCreate
@see ABPAL_AudioCombiner
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioPlayerPlayCombined(
                    ABPAL_AudioPlayer* player,        /*!< Pointer to the player returned on call to ABPAL_AudioPlayerCreate */
                    ABPAL_AudioCombiner* combiner,    /*!< Pointer to the combiner returned on call to ABPAL_AudioCombinerCreate */
                    ABPAL_AudioPlayerCallback* callback,/*!< Callback function called for any audio events */
                    void* userData                /*!< Data passed back to the callback function specified in callback */
                    );

/*! Cancels the current playing audio data

This function cancels the current audio data that is playing.

@return PAL_Error

@see ABPAL_AudioPlayerPlay
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioPlayerCancel(
                      ABPAL_AudioPlayer* player        /*!< Pointer to the player returned on call to ABPAL_AudioPlayerCreate */
                      );

/*! Gets the current state of the ABPAL_AudioPlayer

This function returns the current state of the ABPAL_AudioPlayer

@return ABPAL_AudioState

@see ABPAL_AudioPlayerCreate
@see ABPAL_AudioState
*/
ABPAL_DEC
ABPAL_AudioState
ABPAL_AudioPlayerGetState(
                        ABPAL_AudioPlayer* player        /*!< Pointer to the player returned on call to ABPAL_AudioPlayerCreate */
                        );

/*! Set an audio parameter for the ABPAL_AudioPlayer

This function sets the specified parameter to the specified value

@return PAL_Error

@see ABPAL_AudioPlayerCreate
@see ABPAL_AudioParameter
*/

ABPAL_DEC 
PAL_Error
ABPAL_AudioPlayerSetParameter(
                        ABPAL_AudioPlayer* player,        /*!< Pointer to the player returned on call to ABPAL_AudioPlayerCreate */
                        ABPAL_AudioParameter  audioParam,  /*!< The audio parameter to set */
                        int32 param1,                    /*!< First parameter to set */
                        int32 param2                   /*!< Second parameter to set */
                        );

/*! Get an audio parameter for the ABPAL_AudioPlayer

This function gets the specified parameter and returns it in specified parameter pointers

@return PAL_Error

@see ABPAL_AudioPlayerCreate
@see ABPAL_AudioParameter
*/

ABPAL_DEC
PAL_Error
ABPAL_AudioPlayerGetParameter(
                        ABPAL_AudioPlayer* player,        /*!< Pointer to the player returned on call to ABPAL_AudioPlayerCreate */
                        ABPAL_AudioParameter  audioParam,  /*!< The audio parameter to get */
                        int32 *param1,                    /*!< [OUT] First parameter to return */
                        int32 *param2                   /*!< [OUT] Second parameter to return */
                        );

/*! Creates an instance of the AudioRecorder

Call this function first before any other ABPAL_AudioRecorderXXX API functions to create the AudioRecorder instance. 
The caller is responsible for saving the pointer pointed to by the OUT parameter pprecorder.
This ABPAL_AudioRecorder pointer is used on all subsequent calls to PAL AudioRecorder API.
When finished with the player, the user must call ABPAL_AudioRecorderDestroy to close and free the resources used 
by the ABPAL_AudioRecorder.

@return PAL_Error on error or PAL_Ok if the ABPAL_AudioRecorder is created successfully

@see ABPAL_AudioFormat
@see ABPAL_AudioRecorderDestroy
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioRecorderCreate(
                        PAL_Instance* pal,                    /*!< PAL instance */
                        ABPAL_AudioFormat format,                /*!< Audio format to use for recording data */
                        ABPAL_AudioRecorder** recorder        /*!< Pointer to the recorder returned on call to ABPAL_AudioRecorderCreate */
                        );

/*! Release the AudioRecorder instance

This function stops recording, closes the AudioRecorder and releases all associated resources.

@return PAL_Error

@see ABPAL_AudioRecorderCreate
*/
ABPAL_DEC 
PAL_Error
ABPAL_AudioRecorderDestroy(
                        ABPAL_AudioRecorder* recorder            /*!< Pointer to the recorder returned by ABPAL_AudioRecorderCreate */
                        );

/*! Starts recording 

This function starts recording and places the audio data in the pointer supplied in buffer. Call this method implicitly calls ABPAL_AudioRecorderPrepareToRecord. Recording will end when the data uses
all of the buffer space specified in bufferSize.

@return PAL_Error or PAL_Ok if recording started.

@see ABPAL_AudioRecorderStop
@see ABPAL_AudioRecorderGetRecordedData
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioRecorderRecord(
                        ABPAL_AudioRecorder* recorder,        /*!< Pointer to the recorder returned by ABPAL_AudioRecorderCreate */
                        byte** buffer,                        /*!< Buffer to use to store recorded data */
                        int bufferSize,                        /*!< Size of the buffer specified in buffer */
                        ABPAL_AudioRecorderCallback* callback,        /*!< Callback function called for any audio events */
                        void* userData                        /*!< Data passed back to the callback function specified in callback */
                        );

/*! Prepare to record

 This function prepares for recording. Call ABPAL_AudioRecorderRecord to start recording.

 @return PAL_Error or PAL_Ok if success.

 @see ABPAL_AudioRecorderRecord
 */
ABPAL_DEC
PAL_Error
ABPAL_AudioRecorderPrepareToRecord(
                          ABPAL_AudioRecorder* recorder        /*!< Pointer to the recorder returned by ABPAL_AudioRecorderCreate */
                          );

/*! Set an audio parameter for the ABPAL_AudioRecorder

 This function sets the specified parameter to the specified value

 @return PAL_Error

 @see ABPAL_AudioRecorderCreate
 @see ABPAL_AudioParameter
 */

ABPAL_DEC
PAL_Error
ABPAL_AudioRecorderSetParameter(
                              ABPAL_AudioRecorder* recorder,     /*!< Pointer to the recorder returned on call to ABPAL_AudioRecorderCreate */
                              ABPAL_AudioParameter audioParam,   /*!< The audio parameter to set */
                              int32 param1,                      /*!< First parameter to set */
                              int32 param2                       /*!< Second parameter to set */
                              );

/*! Get an audio parameter for the ABPAL_AudioRecorder

 This function gets the specified parameter and returns it in specified parameter pointers

 @return PAL_Error

 @see ABPAL_AudioRecorderCreate
 @see ABPAL_AudioParameter
 */

ABPAL_DEC
PAL_Error
ABPAL_AudioRecorderGetParameter(
                              ABPAL_AudioRecorder* recorder,     /*!< Pointer to the recorder returned on call to recorderCreate */
                              ABPAL_AudioParameter audioParam,   /*!< The audio parameter to get */
                              int32* param1,                     /*!< [OUT] First parameter to return */
                              int32* param2                      /*!< [OUT] Second parameter to return */
                              );

/*! Get recorded data 
 Call this function for getting record data buffer, and the record data size. 
 Notice that the caller need free the memory of buffer if buffer not null
 @return PAL_Error or PAL_Ok if getting recorded data.
 */
ABPAL_DEC
PAL_Error
ABPAL_AudioRecorderGetRecordedData(
                                   ABPAL_AudioRecorder* recorder,        /*!< Pointer to the recorder returned by ABPAL_AudioRecorderCreate */
                                   byte** buffer,                        /*!< recorded buffer data */
                                   uint32* size                          /*!< recorder data size when record stop*/
                                    );

/*! Stops recording 

This function stops recording.  The data is the then ready to play or save.

@return PAL_Error or PAL_Ok if successfully stopped

@see ABPAL_AudioRecorderRecord
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioRecorderStop(
                      ABPAL_AudioRecorder* recorder            /*!< Pointer to the recorder returned by ABPAL_AudioRecorderCreate */
                      );

/*! Gets the current state

This function returns the current state of the ABPAL_AudioRecorder

@return ABPAL_AudioState

@see ABPAL_AudioRecorderCreate
*/
ABPAL_DEC
ABPAL_AudioState
ABPAL_AudioRecorderGetState(
                          ABPAL_AudioRecorder* recorder        /*!< Pointer to the recorder returned by ABPAL_AudioRecorderCreate */
                          );

/*! Creates an instance of the ABPAL_AudioCombiner

Call this function first before any other ABPAL_AudioCombinerXXX API functions to create the AudioCombiner instance. 
The caller is responsible for saving the pointer pointed to by the OUT parameter ppcombiner.
This ABPAL_AudioCombiner pointer is used on all subsequent calls to PAL AudioCombiner API.
When finished with the player, the user must call ABPAL_AudioCombinerDestroy to close and free the resources used 
by the ABPAL_AudioCombiner.

@return PAL_Error on error or PAL_Ok if the ABPAL_AudioCombiner is created successfully

@see ABPAL_AudioFormat
@see ABPAL_AudioCombinerDestroy
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioCombinerCreate(
                        PAL_Instance* pal,                    /*!< PAL instance */
                        ABPAL_AudioFormat format,                /*!< Audio format to use for combining data */
                        ABPAL_AudioCombiner** combiner        /*!< Pointer to the combiner returned on call to ABPAL_AudioCombinerCreate */
                        );

/*! Release the AudioCombiner instance

This function cancels any pending operations and releases all associated resources.

@return PAL_Error

@see ABPAL_AudioCombinerCreate
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioCombinerDestroy(
                         ABPAL_AudioCombiner* combiner        /*!< Pointer to the combiner returned by ABPAL_AudioCombinerCreate */
                         );

/*! Adds a data buffer to the current data buffer

This function adds the specified audio data specified by buf to the data currently stored in the ABPAL_AudioCombiner.
The data specified in buf must be the same format as the format specified in ABPAL_AudioCombinerCreate.

@return PAL_Error

@see ABPAL_AudioCombinerCreate
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioCombinerAddBuffer(
                           ABPAL_AudioCombiner* combiner,    /*!< Pointer to the combiner returned by ABPAL_AudioCombinerCreate */
                           byte* buffer,                /*!< A pointer to the audio data to combine with any existing data */
                           int bufferSize,                /*!< The size of the buffer */
                           nb_boolean copyNeeded        /*!< Specify whether a copy needs to be made */
                           );

/*! Copies the combined data into the buffer

This function copies the data currently stored in the ABPAL_AudioCombiner into the buffer.
The bufferSize must be at least as big as the total size of the combined data in ABPAL_AudioCombiner.
To optain the size of the buffer needed to get data from this function, first call the function with a 
NULL pointer to buffer.  This will cause the function to return the size of the buffer needed in the
bufferSize variable.

@return PAL_Error

@see ABPAL_AudioCombinerAddBuffer
*/
ABPAL_DEC
PAL_Error
ABPAL_AudioCombinerGetData(
                         ABPAL_AudioCombiner* combiner,        /*!< Pointer to the combiner returned by ABPAL_AudioCombinerCreate */
                         byte** buffer,                    /*!< Pointer of the buffer to copy the combined data to */
                         int* bufferSize                    /*!< [IN/OUT]If buffer is NULL returns the size of buffer needed.
                                                                 If buffer is a valid pointer it should contain the size of the buffer.
                                                                 In this case, the return value will be the number of bytes copied to the buffer. */
                         );

/*! Creates an instance of the AudioSession

 The caller is responsible for saving the pointer pointed to by the OUT parameter audioSession.
 This ABPAL_AudioSession pointer is used on all subsequent calls to PAL AudioSession API.
 User must call ABPAL_AudioSessionDestroy to close audio session and free the resources used 
 by the ABPAL_AudioSession.

 @return PAL_Error on error or PAL_Ok if the ABPAL_AudioSession is created successfully

 @see ABPAL_AudioSessionType
 @see ABPAL_AudioSessionDestroy
 */
ABPAL_DEC
PAL_Error
ABPAL_AudioSessionCreate(
                         PAL_Instance* pal,                          /*!< PAL instance */
                         ABPAL_AudioSessionType audioSessionType,    /*!< Ausio session to be setup */
                         ABPAL_AudioSession** audioSession           /*!< [OUT parameter] that will return a pointer to the ABPAL_AudioSession.
                                                                          The caller must pass this player to every ABPAL_AudioSessionXXXX API */
                         );

/*! Release the AudioSession instance

 This function closes AudioSession and releases all associated resources.

 @return PAL_Error

 @see ABPAL_AudioSessionCreate
 */
ABPAL_DEC
PAL_Error
ABPAL_AudioSessionDestroy(
                         ABPAL_AudioSession* audioSession            /*!< Pointer to the ABPAL_AudioSession returned on call to ABPAL_AudioSessionCreate */
                         );

/*! Get audio duration. 

 This function closes AudioSession and releases all associated resources.
 @return PAL_Error
 @see ABPAL_AudioGetEstimatedDuration
 */
ABPAL_DEC
PAL_Error
ABPAL_AudioGetEstimatedDuration(byte* buf,        /*!< audio buffer */
                                uint32 bufSize,   /*!< audio buffer length */
                                double* duration  /*!< output duration */
                         );


#endif

/*! @} */

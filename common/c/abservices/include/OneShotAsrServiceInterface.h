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
 
 @file     OneShotAsrServiceInterface.h
 @date     02/09/2012
 @defgroup AB One Shot Asr Service Public API
 
 @brief    AB One Shot Asr Service Public API
 
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

#ifndef ONESHOTASRSERVICEINTERFACE_H
#define ONESHOTASRSERVICEINTERFACE_H


#include "abexp.h"
#include "nbcontext.h"
#include "nberror.h"
#include "base.h"
#include "OneShotAsrListenerInterface.h"
#include "OneShotAsrParametersInterface.h"
#include "OneShotAsrInformationInterface.h"
#include "abpalaudio.h"


namespace aboneshotasr
{

/*! Speech detection algorithm
 */
enum SpeechDetectionAlgorithm
{
    SDA_Unknown = 0,
    SDA_Amplitude,              /*! Amplitude based voice detection algorithm. */
    SDA_Spectrum                /*! Spectrum based voice detection algorithm. */
};


/*! One Shot Asr config. For any field zero value allowed, and default value will be used in this case.
 */
struct OneShotAsrConfig
{
    uint32                    uploadDataSize;                /*! Chunks upload data size to server in bytes. 32 kb is default value. */
    uint32                    uploadInterval;                /*! Chunks upload interval to server in msec. 260 msec is default. */
    uint32                    beginOfSpeechTimeout;          /*! If there is no first audible chunk during this timeout no speech is declared. By default is disabled. */
    uint32                    maxRecordTime;                 /*! Max record time in msec. Default is 10 sec. */
    uint32                    noSpeechTimeout;               /*! No speech is declared when there is no speech during this timeout since last audible chunk was detected in msec. Default is 1 sec. */
    SpeechDetectionAlgorithm  speechDetectionAlgorithm;      /*! Speech detection algorithm. */
    uint32                    recorderSampleRate;            /*! Sample rate for recorder audio format. Default is 8 kHz */
    uint32                    recorderBitsPerSample;         /*! Bits per sample for recorder audio format. Default is 8 bits. */
    uint32                    recorderBufferDuration;        /*! Recorder audio buffer duration in msec. This value allows client customize speech detection response. Default value is 200 msec. */
    ABPAL_AudioFormat         audioEncoderFormat;            /*! Encoder audio format. Only ABPAL_AudioFormat_SPX supported for now. */
};


/*! One Shot Asr Loudness Level data structure
 */
struct OneShotAsrLoudnessLevel
{
    uint32   loudnessLevel;   // 0  - 100: Loudness level for first audio receiver channel.
};


/*! One Shot Asr Service Public Interface
 */
class OneShotAsrServiceInterface : public Base
{
public:
/*! Destroys previously created object belong to OneShotAsrServiceInterface

 @returns none
*/
    virtual void Release(void) = 0;


/*! Starts asr. Next flow will be launched: Play announce -> Start Recording with continuos frame handling. Be ready to handle events inside listener.

 @param parameters OneShotAsrParameters object specifying the parameters.

 @returns NB_Error
*/
    virtual NB_Error StartOneShotAsr(OneShotAsrParametersInterface* parameters) = 0;


/*! Cancel a previously started one shot asr request.

 @returns NB_Error
*/
    virtual NB_Error Cancel(void) = 0;


/*! Stops recording by client request. If speech was detected than recognition will be started(client will be notified with correspondent event),
    otherwise request will be canceled without client notification.

 @returns NB_Error
*/
    virtual NB_Error StopRecording(void) = 0;


/*! Returns One Shot Asr Information contained stored data. Client become owner of returned information and should free is by self.

 @param information output OneShotAsrInformationInterface

 @returns NB_Error
*/
    virtual NB_Error GetInformation(OneShotAsrInformationInterface** information) = 0;


/*! Returns OneShotAsrLoudnessLevel contained loudness level data. Can be accessed from different thread.

 @param information output OneShotAsrLoudnessLevel

 @returns NB_Error
*/
    virtual NB_Error GetLoudnessLevel(OneShotAsrLoudnessLevel* loudnessLevel) = 0;

protected:
    virtual ~OneShotAsrServiceInterface(void) {};
};


/*! Creates an instance of the One Shot Asr Service

 @param context NB_Context
 @param config OneShotAsrServiceConfig* pointer to configuration data
 @param listener OneShotAsrListenerInterface listener to handle service events
 @param asrService output pointer to newly created One Shot Asr Service object

 @return NB_Error
 */
AB_DEC NB_Error CreateOneShotAsrService(NB_Context* context,
                                        OneShotAsrConfig* config,
                                        OneShotAsrListenerInterface* listener,
                                        OneShotAsrServiceInterface** asrService
                                        );

} //aboneshotasr

#endif //ONESHOTASRSERVICEINTERFACE_H

/*! @} */

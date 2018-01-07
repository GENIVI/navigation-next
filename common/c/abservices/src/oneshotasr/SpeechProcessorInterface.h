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
 
 @file     SpeechProcessorInterface.h
 @date     02/18/2012
 @defgroup AB Speech Processor Private API
 
 @brief    AB Speech Processor Private API
 
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

#ifndef SPEECHPROCESSORINTERFACE_H
#define SPEECHPROCESSORINTERFACE_H


#include "abexp.h"
#include "nbcontext.h"
#include "nberror.h"
#include "base.h"


namespace abspeechprocessor
{

/*! Speech Processor Result
 */
enum SpeechProcessorResult
{
    SPR_GeneralError = 0,
    SPR_NoSpeechDetected,
    SPR_SpeechDetected
};


/*! Speech Processor Available Algorithms
 */
enum SpeechProcessorAlgorithm
{
    SPA_Unknown = 0,
    SPA_Amplitude,
    SPA_Spectrum
};


/*! Speech Processor Interface
 */
class SpeechProcessorInterface : public Base
{
public:
/*! Destroys previously created object belong to SpeechProcessorInterface

 @returns none
*/
    virtual void Release(void) = 0;


/*! Process passed buffer with cpeech detection algorithm

 @param buffer byte* pointer to data
 @param bufferSize uint32 data lenght

 @returns SpeechProcessorResult
 */
    virtual SpeechProcessorResult ProcessBuffer(byte* buffer, uint32 bufferSize) = 0;


/*! Resets speech processor history. Should be called on start processing new audio stream.
 */
    virtual void ResetHistory(void) = 0;

protected:
    virtual ~SpeechProcessorInterface(void) {};
};


/*! Creates an instance of the Speech Processor object

 @param context NB_Context
 @param algorithm SpeechProcessorAlgorithm speech processor algorithm
 @param bitsPerSample uint32 input audio bits per sample
 @param sampleRate uint32 input audio sample rate
 @param bigEndian nb_boolean big endian data format
 @param speechProcessor output pointer to newly created Speech Processor object

 @return NB_Error
 */
AB_DEC NB_Error CreateSpeechProcessor(NB_Context* context,
                                      SpeechProcessorAlgorithm algorithm,
                                      uint32 bitsPerSample,
                                      uint32 sampleRate,
                                      nb_boolean bigEndian,
                                      SpeechProcessorInterface** speechProcessor
                                      );

} //abspeechprocessor

#endif //SPEECHPROCESSORINTERFACE_H

/*! @} */

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
 
 @file     OneShotAsrListenerInterface.h
 @date     02/09/2012
 @defgroup AB One Shot Asr Listener Public API
 
 @brief    AB One Shot Asr Listener Public API
 
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

#ifndef ONESHOTASRLISTENERINTERFACE_H
#define ONESHOTASRLISTENERINTERFACE_H


#include "nberror.h"
#include "paltypes.h"
#include "base.h"


namespace aboneshotasr
{

/*! One Shot Asr Event enum
 */
enum OneShotAsrEvent
{
    OSASR_Event_ResultsAvailable = 0,    /*! Have success feedback from server with recognition results available. Results can be retrieved thru OneShotAsrServiceInterface::GetInformation(). ASR service resets to idle state. */
    OSASR_Event_Recording,               /*! Recording successfully started. Recorded chunks continuously sends to server. */
    OSASR_Event_Listening,               /*! Begin of speech detected, recording and chunks sending are in progress. */
    OSASR_Event_Recognition,             /*! End of speech detected, recorder is stopped. Sending last chunk to server and waiting feedback. */
    OSASR_Event_GeneralError,            /*! General SW error occured. Additional param filled with NB_Error. ASR service resets to idle state. */
    OSASR_Event_ServerError,             /*! Server returned error. Additional param filled with ServerCompletionCode. ASR service resets to idle state.*/
    OSASR_Event_BeginOfSpeechTimeout     /*! Begin of speech timeout. ASR service resets to idle state. */
};


/*! One Shot Asr Announce enum
 */
enum OneShotAsrAnnounce
{
    OSASR_Announce_RecordingTone = 0,          /*! Recording is started tone. */
    OSASR_Announce_RecognitionTone,            /*! Recognition is started tone. */
    OSASR_Announce_ResultsAvailableTone,       /*! Results available tone. */
    OSASR_Announce_BeginOfSpeechTimeoutTone,   /*! Begin of speech timeout tone. */
    OSASR_Announce_NoSpeechWasDetectedTone,    /*! No speech was detected in the input audio tone. */
    OSASR_Announce_NoValidResultsTone,         /*! Recognition did not return any valid result tone. */
    OSASR_Announce_ParserReturnNoResultsTone,  /*! Parser does not return any result tone. */
    OSASR_Announce_OtherServerErrorTone,       /*! Other server errors tone. */
    OSASR_Announce_GeneralErrorTone,           /*! General SW error tone. */
    OSASR_Announce_End
};


/*! One Shot Asr Listener Public Interface
 */
class OneShotAsrListenerInterface
{
public:
/*! Handler for different kind events from One Shot Asr

 @param event OneShotAsrEvent
 @param param uint32 additional parameter that depends from event value

 @returns none
*/
    virtual void OneShotAsrHandleEvent(OneShotAsrEvent event, uint32 param) = 0;


/*! Data provider for announce tone audio

 @param announce OneShotAsrAnnounce announce type to setup tone data
 @param data byte** [out] client need setup this pointer to data filled with tone audio. One Shot Asr Service will take ownership of the data, so client should not free the data.
 @param size uint32* [out] client need setup this pointer to the data size

 @returns NB_Error
*/
    virtual NB_Error OneShotAsrGetAnnounceData(OneShotAsrAnnounce announce, byte** data, uint32* dataSize) = 0;

protected:
    virtual ~OneShotAsrListenerInterface(void) {};
};

} //aboneshotasr

#endif //ONESHOTASRLISTENERINTERFACE_H

/*! @} */

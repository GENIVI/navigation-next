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
 
 @file     OneShotAsrInformationInterface.h
 @date     02/09/2012
 @defgroup AB One Shot Asr Information Public API
 
 @brief    AB One Shot Asr Information Public API
 
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

#ifndef ONESHOTASRINFORMATIONINTERFACE_H
#define ONESHOTASRINFORMATIONINTERFACE_H


#include "nberror.h"
#include "paltypes.h"
#include "base.h"
#include <string>
#include <vector>


namespace aboneshotasr
{

/*! Server completion code
*/
enum ServerCompletionCode
{
    ServerCompletionCode_Success =                     0,   /*! Standart response for success. */
    ServerCompletionCode_ServerError =             11001,   /*! Speech recognition server error. */
    ServerCompletionCode_ServerTimeout =           11002,   /*! Speech recognition server timeout. */
    ServerCompletionCode_AudioIsTooLong =          11003,   /*! The input audio provided is too long. */
    ServerCompletionCode_NoSpeechWasDetected =     11004,   /*! No speech was detected in the input audio. */
    ServerCompletionCode_NoValidResults =          11005,   /*! Recognition did not return any valid result. */
    ServerCompletionCode_InvalidAudioFormat =      11006,   /*! Invalid audio format. */
    ServerCompletionCode_ParserReturnNoResults =   11007    /*! Parser does not return any result. */
};

/*! One Shot Asr Results data structure
 */
struct OneShotAsrResults
{
    ServerCompletionCode       completionCode;     /*! Server completion code. */
    std::string                sessionID;          /*! ASR session ID. This is the same session ID value sent in parameters. */
    std::string                providerSessionID;  /*! Nuance session ID. Should be passed to single search request. */
    std::vector<std::string>   results;            /*! Vector of recognized strings. */

    OneShotAsrResults(void) :
        completionCode(ServerCompletionCode_Success)
    {
        sessionID.clear();
        providerSessionID.clear();
        results.clear();
    }
};


/*! One Shot Asr Information Public Interface
 */
class OneShotAsrInformationInterface : public Base
{
public:
/*! Destroys previously created object belong to OneShotAsrInformationInterface

 @returns none
*/
    virtual void Release(void) = 0;


/*! Returns One Shot Asr results. Passed results will be filled with returned data if no errors occured.

 @param results output OneShotAsrResults User should pass valid pointer to struct belong to OneShotAsrResults.
 @see DestroyResults

 @returns NB_Error
*/
    virtual NB_Error GetResults(OneShotAsrResults* results) = 0;

protected:
    virtual ~OneShotAsrInformationInterface(void) {};
};

} //aboneshotasr

#endif //ONESHOTASRINFORMATIONINTERFACE_H

/*! @} */

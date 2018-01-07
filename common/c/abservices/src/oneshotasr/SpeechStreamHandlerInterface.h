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
 
 @file     SpeechStreamHandlerInterface.h
 @date     02/28/2012
 @defgroup AB Speech Stream Handler API
 
 @brief    AB Speech Stream Handler API
 
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

#ifndef SPEECHSTREAMHANDLERINTERFACE_H
#define SPEECHSTREAMHANDLERINTERFACE_H


#include "abexp.h"
#include "nbcontext.h"
#include "nberror.h"
#include "base.h"
#include "SpeechStreamParametersInterface.h"
#include "SpeechStreamInformation.h"
#include "SpeechStreamListenerInterface.h"


namespace aboneshotasr
{

/*! Speech Stream Handler Interface
 */
class SpeechStreamHandlerInterface : public Base
{
public:
/*! Destroys previously created object belong to SpeechStreamHandlerInterface

 @returns none
*/
    virtual void Release(void) = 0;


/*! Starts request with tps extracted from passed parameters

 @param parameters SpeechStreamParametersInterface pointer to SpeechStreamParametersInterface

 @returns NB_Error
*/
    virtual NB_Error StartRequest(const SpeechStreamParametersInterface* parameters) = 0;


/*! Cancels active request

 @param parameters OneShotAsrParameters parameters object

 @returns NB_Error
*/
    virtual NB_Error CancelRequest(void) = 0;


/*! Returns reply information. Client become owner of the returned data and should free it by self.

 @param speechStreamInformation SpeechStreamInformation** output information

 @returns NB_Error
*/
    virtual NB_Error GetReplyInformation(SpeechStreamInformation** speechStreamInformation) = 0;

protected:
    virtual ~SpeechStreamHandlerInterface(void) {};
};


/*! Creates an instance of the Speech Stream Handler

 @param context NB_Context
 @param listener SpeechStreamListenerInterface listener
 @param speechStreamHandler output pointer to newly created Speech Stream Handler object

 @return NB_Error
 */
AB_DEC NB_Error CreateSpeechStreamHandler(NB_Context* context,
                                          SpeechStreamListenerInterface* listener,
                                          SpeechStreamHandlerInterface** speechStreamHandler
                                         );

} //aboneshotasr

#endif //SPEECHSTREAMHANDLERINTERFACE_H

/*! @} */

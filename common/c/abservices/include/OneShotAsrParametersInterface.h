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
 
 @file     OneShotAsrParametersInterface.h
 @date     02/09/2012
 @defgroup AB One Shot Asr Parameters Public API
 
 @brief    AB One Shot Asr Parameters Public API
 
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

#ifndef ONESHOTASRPARAMETERSINTERFACE_H
#define ONESHOTASRPARAMETERSINTERFACE_H


#include "abexp.h"
#include "nbcontext.h"
#include "nberror.h"
#include "nbgpstypes.h"
#include "base.h"
#include <string>


namespace aboneshotasr
{

/*! One Shot Asr entry point enum to track feature use from individual screens.
 */
enum OneShotAsrEntryPoint
{
    OSASR_EntryPoint_None = 0,
    OSASR_EntryPoint_Carousel,
    OSASR_EntryPoint_Map,
    OSASR_EntryPoint_EnterAddress,
    OSASR_EntryPoint_Places
};


/*! One Shot Asr Parameters Public Interface
 */
class OneShotAsrParametersInterface : public Base
{
public:
/*! Destroys previously created object belong to OneShotAsrParametersInterface

 @returns none
*/
    virtual void Release(void) = 0;


/*! Initializates common fields for One Shot Asr Parameters

 @param sessionID string uniq session ID
 @param language string lnguage in Iden format. Only  en-US supported.
 @param screen OneShotAsrEntryPoint entry point for asr feature
 @param location NB_GpsLocation gps location

 @returns NB_Error
*/
    virtual NB_Error Init(const std::string& sessionID, const std::string& language, OneShotAsrEntryPoint screen, const NB_GpsLocation* location) = 0;

protected:
    virtual ~OneShotAsrParametersInterface(void) {};
};


/*! Creates an instance of the One Shot Asr Parameters

 @param context NB_Context
 @param parameters output pointer to newly created asr parameters object

 @return NB_Error
 */
AB_DEC NB_Error CreateOneShotAsrParameters(NB_Context* context,
                                           OneShotAsrParametersInterface** parameters
                                           );

} //aboneshotasr

#endif //ONESHOTASRPARAMETERSINTERFACE_H

/*! @} */

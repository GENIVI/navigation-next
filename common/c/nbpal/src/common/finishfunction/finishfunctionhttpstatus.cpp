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

@file     finishfunctionhttpstatus.cpp
*/
/*
(C) Copyright 2012 by Networks In Motion, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "finishfunctionhttpstatus.h"
//#include <wininet.h>

FinishFunctionHttpStatus::FinishFunctionHttpStatus(FinishFunction_HttpStatus function, uint32 status, void* userData, void *requestData, uint32 contentLength) :
    FinishFunctionBase(userData),
    m_function(function),
    m_status(status),
    m_requestData(requestData),
    m_contentLength(contentLength)
{
}


void FinishFunctionHttpStatus::InvokeFunction()
{
    PAL_Error errorCode = PAL_ErrNetGeneralFailure;

    switch (m_status)
    {
        case 400://HTTP_STATUS_BAD_REQUEST:
        case 404://HTTP_STATUS_NOT_FOUND:
            errorCode = PAL_ErrNoData;
            break;
        case 200://HTTP_STATUS_OK:
            errorCode = PAL_Ok;
            break;
    }

    (m_function)(errorCode, m_userData, m_requestData, m_contentLength);
}

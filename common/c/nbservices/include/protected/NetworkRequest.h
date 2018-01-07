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
    @file     NetworkRequest.h

    Description: NetworkRequest is the request wrapper.
*/
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */
#ifndef _NETWORK_REQUEST_
#define _NETWORK_REQUEST_

#include "paltypes.h"
#include "smartpointer.h"
#include <string>

class MutableTpsElement;

class NetworkRequest
{
public:
    NetworkRequest();
    NetworkRequest(shared_ptr<string> target, MutableTpsElement* tpsElement, MutableTpsElement* invocationContextTpsElement, uint32& id);
    ~NetworkRequest();

    shared_ptr<string> GetTarget() { return m_target; }
    MutableTpsElement* GetMutableTpsElement() { return m_tpsElement; }
    MutableTpsElement* GetInvocationContextTpsElement() { return m_invocationContextTpsElement; }
    uint32 GetID() { return m_id; }

private:
    shared_ptr<string> m_target;             /*!< tps target.*/
    MutableTpsElement* m_tpsElement;         /*!< TPS element.*/
    MutableTpsElement* m_invocationContextTpsElement;         /*!< TPS element.*/
    uint32 m_id;                             /*!< Unique identification of the request.*/
};

#endif //_NETWORK_REQUEST_
/*! @} */

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
    @file     AsyncCallback.h
    @defgroup nbmap

*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems, is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __ASYNC_CALLBACK__
#define __ASYNC_CALLBACK__

#include <vector>
#include "smartpointer.h"
#include "nberror.h"

namespace nbmap{
/*! Used for asynchronousl notificaiton
 	
	This class is used by clients when they need to be asynchronously 
    notified by CCC that a request has been processed.
 	Client will inherit AsyncCallback and specialize the class with the
    request and response parameters. Once CCC has processed the request
    CCC will notify the client using Success (or Error). Success will contain
    the response parameter and Error will contain the error information.
*/
template <class Response>
class AsyncCallback
{
public:
    virtual ~AsyncCallback() {}
	
	/*! If Request Succeeds

	 Client should write code in this function
	 to handle a successful callback and process
	 the response parameter
	*/
	virtual void Success(Response response) = 0;
	
	/*! If there is an error in processing the request
	
	 Client should write code in this function
	 to handle an error condition.
	*/
	virtual void Error(NB_Error error) = 0;

	/*! To indicate progress of a callback

	 Client should write code to handle 
	 progress reports for the callback.
	 @return bool false if the request needs to be cancelled.	
	*/
    virtual bool Progress(int /*percentage*/)
    {
        return true;
    }
};

template <class Request, class Response>
class AsyncCallbackWithRequest
{
public:
    virtual ~AsyncCallbackWithRequest() {}

	/*! If Request Succeeds

	 Client should write code in this function
	 to handle a successful callback and process
	 the response parameter. The Request Parameter 
	 will help determine which request the respone belongs to.
	*/
	virtual void Success(Request request,Response response) = 0;

	/*! If there is an error in processing the request
	
	 Client should write code in this function
	 to handle an error condition.
	*/
	virtual void Error(Request request, NB_Error error) = 0;

	/*! To indicate progress of a callback

	 Client should write code to handle 
	 progress reports for the callback.
	 @return bool false if the request needs to be cancelled.	
	*/
    virtual bool Progress(int /*percentage*/)
    {
        return true;
    }
};

}
#endif //__ASYNC_CALLBACK__
/*! @} */

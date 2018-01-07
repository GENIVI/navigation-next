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
   @file        finishfunctionhttpresponseheaders.h

   Description:

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of Networks In
   Motion is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

--------------------------------------------------------------------------*/


#ifndef _FINISHFUNCTIONHTTPRESPONSEHEADERS_H_
#define _FINISHFUNCTIONHTTPRESPONSEHEADERS_H_

/*! @{ */

#include "finishfunctionbase.h"
#include "palerror.h"

/*! Http response header function prototype */
typedef void (*FinishFunction_HttpResponseHeaders)(void* userData, void* requestData,
                                                   const char* responseHeaders);


class FinishFunctionHttpResponseHeaders : public FinishFunctionBase
{
  public:
    /*! FinishFunctionHttpResponseHeaders constructor

      This function takes ownership of the parameter headers.
    */
    FinishFunctionHttpResponseHeaders(FinishFunction_HttpResponseHeaders function,    /*!< A pointer to callback function */
                                      void* userData,                                 /*!< User data of callback function */
                                      void* requestData,                              /*!< Data set when user calls 'PAL_NetHttpSend' */
                                      char* headers                                   /*!< String of the response headers. It is taken
                                                                                           ownership by this function. */
        );

    /*! FinishFunctionHttpResponseHeaders destructor */
    virtual ~FinishFunctionHttpResponseHeaders();

    /*! Function for callback

      This function inherits from class 'Callback'.

      @return None
    */
    virtual void InvokeFunction();

private:
    // Private members ..........................................................................

    FinishFunction_HttpResponseHeaders m_function;          /*!<  Real function to be called.  */
    void*                              m_requestData;       /*!< Data set when user calls 'PAL_NetHttpSend' */
    char*                              m_responseHeaders;   /*!< Entire string of response headers */
};


/*! @} */

#endif /* _FINISHFUNCTIONHTTPRESPONSEHEADERS_H_ */

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

#include "ltkerror.h"

namespace locationtoolkit
{

LTKError::LTKError()
{
    set(LTK_OK);
}


LTKError::LTKError(int code)
{
    set(code);
}


LTKError::LTKError(int code, const QString &des)
{
    set(code, des);
}

void LTKError::set(int code)
{
    errorCode = code;
    CreateDescription();
}

void LTKError::set(int code, const QString &des)
{
    errorCode = code;
    errorDescription = des;
}

static const char* OKDescription = "OK";


static const char* GeneralErrorDescription[] = {
    "General Failure",
    "Bad Parameter",
    "Not enough memory",
    "Data is NULL",
    "Interface has not been initialized",
    "Behavior not supported",
    "parameter is out of range",
    "Result can't be found",
    "Wrong format",
    "Over flow",
    "Already exist",
    "EOF Error",
    "Not ready"
};

static const char* ServerErrorDescription[] = {
    "Server Unknown Error",
    "Server Failed",
    "Server Timedout"
};

static const char* NavigateRouteErrorDescription[] =
{
    "No Error",
    "Request Time Out",
    "Bad Destination",
    "Bad Origin",
    "Can not Route, Server or map data error",
    "Empty Route",
    "Net Error",
    "UnknownError",
    "No Match",
    "Server Error",
    "No Detour",
    "PedRoute Too Long",
    "Origin Country Unsuppoted",
    "Destination Country Unsupported",
    "Unauthorized",
    "BicycleRoute Too Long",
    "Route Too Long",
    "GPS Timeout",
    "Cancelled",
    "Internal"
};

void LTKError::CreateDescription()
{
    if(errorCode == LTK_OK)
    {
        errorDescription = OKDescription;
    }
    else if(errorCode>=LTK_Failed && errorCode<=LTK_ErrNotReady)
    {
        errorDescription = GeneralErrorDescription[errorCode - LTK_Failed];
    }
    else if(errorCode>=LTK_ServerErrorUnknown && errorCode<=LTK_ServerErrorTimedout)
    {
        errorDescription = ServerErrorDescription[errorCode - LTK_ServerErrorUnknown];
    }
    else if(errorCode>=LTK_NavigationErrorNone && errorCode<=LTK_NavigationErrorInternal)
    {
         errorDescription = NavigateRouteErrorDescription[errorCode - LTK_NavigationErrorNone];
    }
    else
    {
        errorDescription = GeneralErrorDescription[LTK_Failed];
    }
}

}

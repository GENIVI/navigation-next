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

/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _LTK_ERROR_H_
#define _LTK_ERROR_H_
#include <QString>

namespace locationtoolkit
{
/*! LTK Error codes */
class LTKError
{
public:
    enum
    {
        LTK_OK              = 0,
        LTK_Failed          = 1000,               /*!< LTK General Failure */
        LTK_ErrBadParam,                            /*!< LTK Bad Parameter */
        LTK_ErrNoMem,                               /*!< LTK Not enough memory */
        LTK_ErrNoData,                              /*!< LTK Data is NULL */
        LTK_ErrNoInit,                              /*!< LTK Interface has not been initialized */
        LTK_ErrUnsupported,                         /*!< LTK Behavior not supported */
        LTK_ErrRange,                               /*!< LTK parameter is out of range */
        LTK_ErrNotFound,                            /*!< LTK Result can't be found */
        LTK_ErrWrongFormat,                         /*!< LTK Wrong format */
        LTK_ErrOverflow,                            /*!< LTK Over flow */
        LTK_ErrAlreadyExist,                        /*!< LTK Already exist */
        LTK_ErrEOF,                                 /*!< LTK EOF Error */
        LTK_ErrNotReady,                            /*!< LTK Not ready */

        LTK_ServerErrorUnknown      = 2000,
        LTK_ServerErrorFailed,
        LTK_ServerErrorTimedout,

        LTK_NavigationErrorNone     = 3000,
        LTK_NavigationErrorTimedOut,
        LTK_NavigationErrorBadDestination,
        LTK_NavigationErrorBadOrigin,
        LTK_NavigationErrorCannotRoute,
        LTK_NavigationErrorEmptyRoute,
        LTK_NavigationErrorNetError,
        LTK_NavigationErrorUnknownError,
        LTK_NavigationErrorNoMatch,
        LTK_NavigationErrorServerError,
        LTK_NavigationErrorNoDetour,
        LTK_NavigationErrorPedRouteTooLong,
        LTK_NavigationErrorOriginCountryUnsuppoted,
        LTK_NavigationErrorDestinationCountryUnsupported,
        LTK_NavigationErrorUnauthorized,
        LTK_NavigationErrorBicycleRouteTooLong,
        LTK_NavigationErrorRouteTooLong,
        LTK_NavigationErrorGPSTimeout,
        LTK_NavigationErrorCancelled,
        LTK_NavigationErrorInternal,

        LTK_SearchErrorNone         = 4000,
        LTK_MapErrorNone            = 5000,
    };

public:
    LTKError();
    LTKError(int code);
    LTKError(int code, const QString& des);

public:
    void set(int code);
    void set(int code, const QString& des);

    inline int code() const {return errorCode;}
    inline QString description() const {return errorDescription;}

private:
    void CreateDescription();

private:
    int errorCode;
    QString errorDescription;
};

}
#endif //_LTK_ERROR_H_

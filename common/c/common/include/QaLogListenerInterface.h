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

 @file     searchlistenerinterface.h
 @date     09/07/2010
 @defgroup SERVICE_CONTROLLERS		Service Controllers

 Using this interface, we can get the feedback of search.

 */
/*
 (C) Copyright 2010 by TeleCommunication Systems

 The information contained herein is confidential, proprietary
 to Networks In Motion, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef __QALOG_LISTENER_INTERFACE_H
#define __QALOG_LISTENER_INTERFACE_H

#include <string>
#include "nberror.h"

class QaLogListenerInterface
{
public:
    QaLogListenerInterface() {};
    virtual ~ QaLogListenerInterface () {};

    virtual void UploadStarted() = 0;

    virtual void UploadCanceled() = 0;

    virtual void UploadCompleted(const std::string& logid) = 0;

    virtual void UploadProgress(int percentage) = 0;

    virtual void LogFileEmpty() = 0;

    virtual void UploadFailed (NB_Error error) = 0;
};



#endif

/*! @} */

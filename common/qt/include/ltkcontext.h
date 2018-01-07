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

 @file ltkcontext.h
 @date 08/04/2014
 @addtogroup ltkcommon

 */
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#ifndef LTKCONTEXT_H_
#define LTKCONTEXT_H_

#include "qaloglistenerdelegate.h"
#include <QString>

class QtContext;

namespace locationtoolkit
{
/**
 * Contains internal data for LTK.
 */
class LTKContext
{
public:
    /**
     * Constructs a new LTKContext instance.
     *
     * @param credential Credential key
     * @param dpi set dpi for screen.
     */
    static LTKContext* CreateInstance( const QString& credential, float dpi);

    /**
     * Constructs a new LTKContext instance.
     *
     * @param credential Credential key
     * @param language Language code
     * @param dpi set dpi for screen.
     */
    static LTKContext* CreateInstance( const QString& credential,
                                       const QString& language,
                                       float dpi);

    /**
     * Constructs a new LTKContext instance.
     *
     * @param credential Credential key
     * @param countryCode Country code
     * @param language Language code
     * @param dpi set dpi for screen.
     */
    static LTKContext* CreateInstance( const QString& credential,
                                       const QString& language,
                                       const QString& countryCode,
                                       float dpi);

    /**
     * Constructs a new LTKContext instance.
     *
     * @param credential Credential key
     * @param countryCode Country code
     * @param language Language code
     * @param mdn Client's mdn code
     * @param productName Name of the production
     * @param pushMessageGUID Push Message GUID
     * @param dpi set dpi for screen.
     */
    static LTKContext* CreateInstance( const QString& credential,
                                       const QString& countryCode,
                                       const QString& language,
                                       qint64 mdn,
                                       const QString& productName,
                                       const QString& pushMessageGUID,
                                       float dpi);

    virtual ~LTKContext() {};

    /**
     * Clear all network cache data.
     */
    virtual void MasterClear() = 0;

    /**
     * Start Qa log.
     */
    virtual void StartQaLog( bool verbose = true ) = 0;

    /**
     * Upload Qa log.
     */
    virtual void UploadQaLog() = 0;

    /**
     * Get Qa log delegate.
     */
    virtual QaLogListenerDelegate* GetQaLogDelegate() = 0;

    /**
     * Get version of LTK SDK.
     */
    static QString GetVersion();

    /**
     * Get version of CCC SDK.
     */
    static QString GetCCCVersion();

    /**
     * Get version of NCDB SDK.
     */
    static QString GetNCDBVersion();
};
}

#endif //LTKCONTEXT_H_

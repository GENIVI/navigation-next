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

 @file ltkcontextimpl.h
 @date 08/04/2014
 @addtogroup common

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

#ifndef LTKCONTEXTIMPL_H_
#define LTKCONTEXTIMPL_H_

#include "ltkcontext.h"
#include "palnet.h"
#include "nbcontext.h"
#include "nbnetworkoptions.h"
#include "QaLogListenerInterface.h"
#include "qaloglistenerdelegate.h"

namespace locationtoolkit
{
/**
 * @author
 * @version
 * @created
 */
class LTKContextImpl: public LTKContext
{
public:
	LTKContextImpl(const QString credential,
							const QString countryCode,
							const QString language,
							const quint64 mdn,
							const QString productName,
							const QString pushMessageGUID,
							float dpi);

    ~LTKContextImpl();

public:
    PAL_Instance* GetPalInstance() const;
    NB_Context* GetNBContext() const;

    virtual void MasterClear();
    virtual void StartQaLog( bool verbose = true );
    virtual void UploadQaLog();
    virtual QaLogListenerDelegate* GetQaLogDelegate();
    virtual const QString& GetLanguage() const;

private:
    bool InitializeNBContext(const QString credential,
							const QString countryCode,
							const QString language,
							const quint64 mdn,
							const QString productName,
							const QString pushMessageGUID,
							float dpi);
    void CreateNBContext();
    bool CreateContext(NB_NetworkOptionsDetailed& networkOptions);
    void ClearCache();
    void ClearLocalGUID();

private:
    NB_NetworkConfiguration* GetNetworkConfiguration(NB_NetworkOptionsDetailed& options);

private:
    PAL_Instance*  mPALInstance;
    NB_Context*    mContext;
    QaLogListenerInterface* mQalogListener;
    QaLogListenerDelegate*  mQaLogDelegate;
    QString        mVersion;
    QString        mLanguage;
};
}

#endif // LTKCONTEXTIMPL_H_

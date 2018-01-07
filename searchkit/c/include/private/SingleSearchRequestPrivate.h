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
 @file     SingleSearchRequestPrivate.h
 @defgroup nbsearch_private
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __SINGLESEARCHREQUESTPRIVATE_H__
#define __SINGLESEARCHREQUESTPRIVATE_H__

#include "SingleSearchListener.h"
#include "SingleSearchConfiguration.h"
#include "SingleSearchParameters.h"
#include "SingleSearchHandler.h"
#include "pal.h"
#include "CommonTypes.h"
#include "nbtaskqueue.h"
#include "nbuitaskqueue.h"
#include "nbsinglesearchhandler.h"
#include "AsyncCallback.h"

namespace nbsearch
{

class SingleSearchRequest;
class SingleSearchTask;
class SingleSearchState;

class SingleSearchRequestPrivate
{
public:

    /*!
     *  public constructor
     *  the created instance will take ownership of parameter
     */
    static SingleSearchRequestPrivate* NewInstance(NB_Context* context,
                                                   const SingleSearchRequest* user,
                                                   const std::shared_ptr<SingleSearchListener>& listener,
                                                   const std::shared_ptr<SingleSearchParameters>& param);
    /*!
     *  public destrctor
     */
    void Destroy();

    void Request();
    void Cancel();

    /*! See the SingleSearchRequest header.
     */
    void SetInvocationContext(const char* inputSource, const char* invocationMethod, const char* screenId);

    /*!
     *  this is a blocking function, take care when call
     */
    bool IsRequestInProgress();

    bool IsRequestAlive();

private:
    void CCC_Request(const SingleSearchTask* pTask);
    void CCC_Cancel(const SingleSearchTask* pTask);
    void CCC_CheckInProgress(const SingleSearchTask* pTask);
    void CCC_Destroy(const SingleSearchTask* pTask);

    SingleSearchRequestPrivate(NB_Context* context,
                               const SingleSearchRequest* user = NULL,
                               const std::shared_ptr<SingleSearchListener>& listener = NULL,
                               const std::shared_ptr<SingleSearchParameters>& param = NULL);
    ~SingleSearchRequestPrivate();

    NB_Context* m_context;
    PAL_Instance* m_palInstance;
    bool m_inProgress;
    bool m_initSearch;

    /*! CCC worker thread */
    shared_ptr<TaskQueue> m_eventQueue;
    SingleSearchHandler*  m_handler;           /*!< used in CCC thread only */
    std::shared_ptr<SingleSearchState> m_state;
};



}

#endif /* __SINGLESEARCHREQUESTPRIVATE_H__ */

/*! @} */

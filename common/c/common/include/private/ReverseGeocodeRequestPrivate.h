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
 @file     ReverseGeocodeRequestPrivate.h
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
#ifndef __REVERSEGEOCODEREQUESTPRIVATE_H__
#define __REVERSEGEOCODEREQUESTPRIVATE_H__

#include "ReverseGeocodeParameter.h"
#include "ReverseGeocodeHandler.h"
#include "ReverseGeocodeListener.h"
#include "pal.h"
#include "nbcontext.h"
#include "nbtaskqueue.h"
#include "nbuitaskqueue.h"
#include "smartpointer.h"

namespace nbcommon
{
class ReverseGeocodeRequest;
class ReverseGeocodeTask;

class ReverseGeocodeRequestPrivate
{
public:
    /*!
     *  public constructor
     *  the created instance will take ownership of parameter
     */
    static ReverseGeocodeRequestPrivate* NewInstance(NB_Context* context,
                                                     const ReverseGeocodeRequest* user,
                                                     const std::shared_ptr<ReverseGeocodeListener>& listener,
                                                     const std::shared_ptr<ReverseGeocodeParameter>& sparam);
    /*!
     *  public destrctor
     */
    void Destroy();

    /*! See the ReverseGeocodeRequest header.
     */
    void SetInvocationContext(const char* inputSource, const char* invocationMethod, const char* screenId);

    void Request();
    void Cancel();

    /*!
     *  this is a blocking function, take care when call
     */
    bool IsRequestInProgress();

    void NotifySuccess(ReverseGeocodeInformation* info);
    void NotifyError(NB_Error error);
    void NotifyProgress(int percentage);

private:

    void CCC_Request(const ReverseGeocodeTask* pTask);
    void CCC_Cancel(const ReverseGeocodeTask* pTask);
    void CCC_CheckInProgress(const ReverseGeocodeTask* pTask);
    void CCC_Destroy(const ReverseGeocodeTask* pTask);
    void UI_NotifySuccess(const ReverseGeocodeTask* pTask);
    void UI_NotifyError(const ReverseGeocodeTask* pTask);
    void UI_NotifyProgress(const ReverseGeocodeTask* pTask);

    bool IsRequestAlive();

    ReverseGeocodeRequestPrivate(NB_Context* context,
                                 const ReverseGeocodeRequest* user,
                                 const std::shared_ptr<ReverseGeocodeListener>& listener,
                                 const std::shared_ptr<ReverseGeocodeParameter>& sparam);
    ~ReverseGeocodeRequestPrivate();

    NB_Context* m_context;
    PAL_Instance* m_palInstance;
    ReverseGeocodeHandlerInterface*  m_handler;  /*!< used in CCC thread only */
    std::shared_ptr<ReverseGeocodeListener>  m_listener;         /*!< used in UI thread only */
    const ReverseGeocodeRequest*  m_userRequest; /*!< used in UI thread only */
    bool m_inProgress;
    /*! CCC worker thread */
    shared_ptr<TaskQueue> m_eventQueue;
    /*! UI task queue */
    shared_ptr<UiTaskQueue> m_UITaskQueue;
};

/*! SingleSearchAsyncCallback

 Refer to AsyncCallbackWithRequest for more description.
 */
class ReverseGeocodeAsyncCallback : public nbmap::AsyncCallbackWithRequest<protocol::ReverseGeocodeParametersSharedPtr,
                                                                           protocol::ReverseGeocodeInformationSharedPtr>
{
public:
    ReverseGeocodeAsyncCallback(ReverseGeocodeRequestPrivate* requestPrivate);
    virtual ~ReverseGeocodeAsyncCallback() {}
    void Success(protocol::ReverseGeocodeParametersSharedPtr request,
                 protocol::ReverseGeocodeInformationSharedPtr response);
    void Error(protocol::ReverseGeocodeParametersSharedPtr request, NB_Error error);
    bool Progress(int percentage);

    /*! Reset this call back to invalid state */
    void Reset();

private:
    ReverseGeocodeRequestPrivate* m_requestPrivate;
};

} /* nbcommon */

#endif //__REVERSEGEOCODEREQUESTPRIVATE_H__
/*! @} */

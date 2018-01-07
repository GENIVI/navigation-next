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
    @file     SingleSearchOffboardHandler.h
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

#ifndef __SEARCH_SINGLE_SEARCH_OFFBOARD_HANDLER_H__
#define __SEARCH_SINGLE_SEARCH_OFFBOARD_HANDLER_H__

#include "smartpointer.h"
#include "SingleSearchHandler.h"
#include "TpsAtlasbookProtocol.h"
#include "Handler.h"
#include "NBProtocolSingleSearchSourceInformation.h"
#include "NBProtocolSingleSearchSourceParameters.h"
#include "SingleSearchParameters.h"
#include "SingleSearchTask.h"
#include "SingleSearchHandlerPrivate.h"

namespace nbsearch
{

class SingleSearchOffboardHandler;
class OffboardSingleSearchTask;
class SingleSearchState;

typedef void (SingleSearchOffboardHandler::*OffboardTaskFunction)(const OffboardSingleSearchTask* pTask);

class OffboardSingleSearchTask : public Task
{
public:
    OffboardSingleSearchTask(SingleSearchOffboardHandler* handler, OffboardTaskFunction function, const std::shared_ptr<SingleSearchState>& state)
        :m_handler(handler), m_function(function), m_state(state) {}
    virtual ~OffboardSingleSearchTask() {}

    // Override
    virtual void Execute(void) {
        if (m_state->isValid())
        {
            if (m_handler) (m_handler->*m_function)(this);
        }
        delete this;
    }

private:
    SingleSearchOffboardHandler* m_handler;
    OffboardTaskFunction         m_function;
    std::shared_ptr<SingleSearchState> m_state;
};

class OffboardSingleSearchUITaskSuccess : public OffboardSingleSearchTask
{
public:
    OffboardSingleSearchUITaskSuccess(SingleSearchOffboardHandler* handler, OffboardTaskFunction function, SingleSearchInformation* information, const std::shared_ptr<SingleSearchState>& state)
    : OffboardSingleSearchTask(handler, function, state), m_information(information) {}

    ~OffboardSingleSearchUITaskSuccess() {}

    SingleSearchInformation* m_information;
};

class OffboardSingleSearchUITaskError : public OffboardSingleSearchTask
{
public:
    OffboardSingleSearchUITaskError(SingleSearchOffboardHandler* handler, OffboardTaskFunction function, NB_Error error, const std::shared_ptr<SingleSearchState>& state)
    : OffboardSingleSearchTask(handler, function, state), m_error(error) {}

    ~OffboardSingleSearchUITaskError() {}

    NB_Error m_error;
};

class OffboardSingleSearchUITaskProgress : public OffboardSingleSearchTask
{
public:
    OffboardSingleSearchUITaskProgress(SingleSearchOffboardHandler* handler, OffboardTaskFunction function, int percentage, const std::shared_ptr<SingleSearchState>& state)
    : OffboardSingleSearchTask(handler, function, state), m_percentage(percentage) {}

    ~OffboardSingleSearchUITaskProgress() {}

    int m_percentage;
};

/*! SingleSearchAsyncCallback

    Refer to AsyncCallbackWithRequest for more description.
 */
class SingleSearchAsyncCallback : public nbmap::AsyncCallbackWithRequest<protocol::SingleSearchSourceParametersSharedPtr,
                                                                         protocol::SingleSearchSourceInformationSharedPtr>
{
public:
    SingleSearchAsyncCallback(SingleSearchOffboardHandler* offboardHandler);
    virtual ~SingleSearchAsyncCallback();
    void Success(protocol::SingleSearchSourceParametersSharedPtr request,
                 protocol::SingleSearchSourceInformationSharedPtr response);
    void Error(protocol::SingleSearchSourceParametersSharedPtr request, NB_Error error);
    bool Progress(int percentage);

    /*! Reset this call back to invalid state */
    void Reset();

private:
    SingleSearchOffboardHandler* m_handler;
};


class SingleSearchOffboardHandler : public SingleSearchHandler
{
public:
    explicit SingleSearchOffboardHandler(
            SingleSearchHandlerPrivate* outer,
            NB_Context* context,
            const SingleSearchRequest* user,
            const std::shared_ptr<SingleSearchListener>& listener,
            const std::shared_ptr<SingleSearchParameters>& param,
            const std::shared_ptr<SingleSearchState>& state);

    /*!
     *  SingleSearchOffboardHandler has to be deleted in CCC thread
     */
    ~SingleSearchOffboardHandler();

    NB_Error Request(NB_Context* context);
    void Cancel();
    bool IsInProgress();

    /*! See the SingleSearchRequest header.
     */
    void SetInvocationContext(const char* inputSource, const char* invocationMethod, const char* screenId);


    void NotifySuccess(protocol::SingleSearchSourceParametersSharedPtr param, protocol::SingleSearchSourceInformationSharedPtr information);
    void NotifyError(NB_Error error);
    void NotifyProgress(int percentage);

private:
    SingleSearchOffboardHandler(SingleSearchOffboardHandler const& copy);
    SingleSearchOffboardHandler& operator= (SingleSearchOffboardHandler const& copy);

    void UI_NotifySuccess(const OffboardSingleSearchTask* pTask);
    void UI_NotifyError(const OffboardSingleSearchTask* pTask);
    void UI_NotifyProgress(const OffboardSingleSearchTask* pTask);

    /*!< Single search handler instance */
    shared_ptr<protocol::Handler<protocol::SingleSearchSourceParameters,
               protocol::SingleSearchSourceInformation> > m_handler;
    /*!< Single search callback */
    shared_ptr<SingleSearchAsyncCallback> m_singleSearchAsyncCallback;
    /*!< Single search parameter */
    shared_ptr<SingleSearchParameters> m_param;
    std::string m_inputSource;
    std::string m_invocationMethod;
    std::string m_screenId;
    NB_Context* m_context;
    SingleSearchHandlerPrivate* m_outer;

    /*! UI task queue */
    shared_ptr<UiTaskQueue> m_UITaskQueue;
    std::shared_ptr<SingleSearchListener>  m_listener;         /*!< used in UI thread only */
    const SingleSearchRequest*  m_userRequest; /*!< used in UI thread only */
    std::shared_ptr<SingleSearchState> m_state;
};

}
#endif //__SEARCH_SINGLE_SEARCH_OFFBOARD_HANDLER_H__

/*! @} */

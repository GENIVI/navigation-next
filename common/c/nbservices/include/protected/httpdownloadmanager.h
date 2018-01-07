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

    @file       httpdownloadmanager.h

    Class 'HttpDownloadManager' inherits from 'DownloadManager' interface.
    A 'HttpDownloadManager' object is used to request or cancel data of HTTP
    protocol.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef HTTPDOWNLOADMANAGER_H
#define HTTPDOWNLOADMANAGER_H

/*!
    @addtogroup nbcommon
    @{
*/

extern "C"
{
    #include "nbcontext.h"
}

#include "base.h"
#include "downloadmanager.h"
#include "downloadrequest.h"
#include "nblist.h"
#include "smartpointer.h"
#include <string>

namespace nbcommon
{

// Types ........................................................................................

/*! Download manager for HTTP downloading */
class HttpDownloadManager : public DownloadManager,
                            public Base
{
public:
    // Public functions .........................................................................

    /*! HttpDownloadManager constructor */
    HttpDownloadManager();

    /*! HttpDownloadManager constructor with initialization

        This function combines default constructor and Initialize. Because constructor cannot
        return an error and C++ exception cannot be used. User could check if initialization
        succeeds by function IsInitialized. And user should not call function Initialize after
        calling this constructor.
    */
    HttpDownloadManager(NB_Context* context,            /*!< Pointer to current context */
                        uint32 downloadConnectionCount  /*!< Count of download connection */
                        );

    /*! HttpDownloadManager destructor */
    virtual ~HttpDownloadManager();

    /*! Initialize a 'HttpDownloadManager' object

        This function should be called before user requests or cancels data of HTTP protocol.

        @return NE_OK if success
    */
    NB_Error
    Initialize(NB_Context* context,             /*!< Pointer to current context */
               uint32 downloadConnectionCount   /*!< Count of download connection */
               );

    /*! Check if this 'HttpDownloadManager' object is initialized

        This function should be called in all public functions of class 'HttpDownloadManager'.

        @return Flag if this 'HttpDownloadManager' object is initialized
    */
    bool
    IsInitialized() const;

    /*! Get a pointer to current context

        Class ContextBasedSingleton calls this function. Cannot compile successfully without
        this function.

        @return A pointer to current context
    */
    NB_Context*
    GetContext();

    /* See description in 'downloadmanager.h' */
    virtual NB_Error RequestData(const std::vector<DownloadRequestPtr>& requests,
                                 uint32 levelId,
                                 std::vector<uint32>* duplicatedLevelIds);
    virtual NB_Error CancelRequest(DownloadRequestPtr request);
    virtual NB_Error ClearRequests(uint32 levelId,
                                   bool inProgress);
    virtual NB_Error RemoveLevel(uint32 levelId);
    virtual NB_Error PauseDownloading(uint32 levelId,
                                      bool inProgress);
    virtual NB_Error ResumeDownloading(uint32 levelId);
    virtual bool IsEmpty(uint32 levelId);
    virtual bool IsPaused(uint32 levelId);


private:
    // Private types ............................................................................

    /*! Download entry in download queue */
    class DownloadEntry : public Base
    {
    public:
        /* See source file for description */

        DownloadEntry(uint32 levelId,
                      shared_ptr<bool> isPaused,
                      DownloadRequestPtr request);
        virtual ~DownloadEntry();
        uint32 GetLevelId() const;
        bool IsPaused() const;
        DownloadRequestPtr GetRequest();

    private:
        // Copy constructor and assignment operator are not supported.
        DownloadEntry(const DownloadEntry& entry);
        DownloadEntry& operator=(const DownloadEntry& entry);

        uint32 m_levelId;                   /*!< Download level to download the request */
        shared_ptr<bool> m_isPaused;        /*!< Is the download level paused? */
        DownloadRequestPtr m_request;       /*!< A request to download */
    };
    typedef shared_ptr<DownloadEntry> DownloadEntryPtr;

    /*! Download level */
    class DownloadLevel : public Base
    {
    public:
        /* See source file for description */

        DownloadLevel(uint32 levelId);
        virtual ~DownloadLevel();
        NB_Error Initialize();
        uint32 GetLevelId() const;
        bool IsPaused() const;
        void SetPaused(bool isPaused);
        shared_ptr<bool> GetPausedFlag();
        int GetEntryCount() const;
        NB_Error AddEntry(int index,
                          shared_ptr<ListNode<DownloadEntryPtr> > entry);
        NB_Error FindEntry(DownloadRequestPtr request,
                           int* index);
        shared_ptr<ListNode<DownloadEntryPtr> > GetEntry(int index);
        shared_ptr<ListNode<DownloadEntryPtr> > RemoveEntry(int index);
        std::vector<shared_ptr<ListNode<DownloadEntryPtr> > > GetEntriesInProgress();
        std::vector<shared_ptr<ListNode<DownloadEntryPtr> > > RemoveEntriesNotInProgress();

    private:
        // Copy constructor and assignment operator are not supported.
        DownloadLevel(const DownloadLevel& downloadLevel);
        DownloadLevel& operator=(const DownloadLevel& downloadLevel);

        /* See source file for description */

        uint32 m_levelId;                                                   /*!< The ID of the download level */
        shared_ptr<bool> m_isPaused;                                        /*!< Is the download level paused? */
        std::vector<shared_ptr<ListNode<DownloadEntryPtr> > > m_entries;    /*!< List nodes of download entries in
                                                                                 download queue */
    };
    typedef shared_ptr<DownloadLevel> DownloadLevelPtr;

    /*! Download connection */
    class DownloadConnection : public Base
    {
    public:
        /* See source file for description */

        DownloadConnection(HttpDownloadManager& httpDownloadManager);
        virtual ~DownloadConnection();
        NB_Error Initialize(NB_Context* context);
        bool IsRequestInProgress() const;
        NB_Error RequestData(DownloadRequestPtr request,
                             uint32 levelId,
                             uint32 currentRetryTimes);
        NB_Error CancelRequest(DownloadRequestPtr request,
                               uint32* levelId);
        NB_Error CancelRequestByLevel(uint32 levelId);

    private:
        // Copy constructor and assignment operator are not supported.
        DownloadConnection(const DownloadConnection& downloadConnection);
        DownloadConnection& operator=(const DownloadConnection& downloadConnection);

        /* See source file for description */

        bool IsInitialized() const;
        NB_Error CreatePalConnectionIfNotExist(DownloadRequestPtr request);
        void DestroyPalConnectionToRequest();
        void ProcessRequest();
        void NotifyRequestError(NB_Error errorCode);
        shared_ptr<std::string> StandardizeHostname(DownloadRequestPtr request,
                                                    DownloadProtocol& protocol);
        shared_ptr<std::string> StandardizeUrl(DownloadRequestPtr request);
        static NB_Error TranslateHttpPalErrorToNeError(PAL_Error palError);
        static void StatusCallback(void* userData,
                                   PAL_NetConnectionStatus status);
        static void ResponseStatusCallback(PAL_Error errorCode,
                                           void* userData,
                                           void* requestData,
                                           uint32 contentLength);
        static void DataReceivedCallback(void* userData,
                                         void* requestData,
                                         PAL_Error errorCode,
                                         const byte* bytes,
                                         uint32 count);
        static void ResponseHeadersCallback(void* userData,
                                            void* requestData,
                                            const char* responseHeaders);

        uint16 m_currentPort;                               /*!< Current port of this HTTP connection. Only valid
                                                                 when the HTTP connection is connected. */
        uint32 m_currentRetryTimes;                         /*!< Current retry times */
        uint32 m_levelIdInProgress;                         /*!< The download level which 'm_requestInProgress'
                                                             belongs to. Only valid when 'm_requestInProgress'
                                                             is not NULL. */
        DownloadProtocol m_protocol;                        /*!< Current protocol of this connection */
        HttpDownloadManager& m_httpDownloadManager;         /*!< 'HttpDownloadManager' object */
        NB_Context* m_context;                              /*!< Pointer to current context */
        PAL_NetConnection* m_httpConnection;                /*!< The HTTP connection of PAL */
        shared_ptr<std::string> m_currentHostname;          /*!< Current hostname of this HTTP connection. Only
                                                                 valid when the HTTP connection is connected. */
        DownloadRequestPtr m_requestInProgress;             /*!< The item is downloading in progress. This item is
                                                                 also contained in the download queue of level
                                                                 'm_levelIdInProgress'. */
    };
    typedef shared_ptr<DownloadConnection> DownloadConnectionPtr;


private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    HttpDownloadManager(const HttpDownloadManager& httpDownloadManager);
    HttpDownloadManager& operator=(const HttpDownloadManager& httpDownloadManager);

    /* See source file for description */

    void Reset();
    NB_Error FindLevel(uint32 levelId,
                       int* position);
    NB_Error CreateLevel(uint32 levelId,
                         int position);
    NB_Error AddRequest(DownloadRequestPtr request,
                        DownloadLevelPtr targetLevel,
                        std::vector<DownloadLevelPtr>& duplicatedLevels);
    NB_Error AddRequestToLevel(DownloadRequestPtr request,
                               DownloadLevelPtr level);
    NB_Error RemoveRequestFromLevel(uint32 levelId,
                                    DownloadRequestPtr request);
    NB_Error RemoveRequestByIndex(int index,
                                  DownloadRequestPtr request,
                                  DownloadLevelPtr level);
    NB_Error ProcessRequests();

    static int CompareRequest(DownloadRequestPtr request,
                              DownloadRequestPtr anotherRequest);

    void ScheduleProcessTasks();

    static void ScheduledTasksCallback(PAL_Instance* pal, void* userData);

    // Private members ..........................................................................

    NB_Context* m_context;                                              /*!< Pointer to current context */
    std::vector<DownloadConnectionPtr> m_downloadConnections;           /*!< A vector of 'DownloadConnection' items */
    std::vector<DownloadLevelPtr> m_downloadLevels;                     /*!< A vector of 'DownloadLevel' items */
    List<DownloadEntryPtr> m_downloadQueue;                             /*!< Download queue used to schedule
                                                                             sequence of the requests */
    uint32 m_scheduleTaskId;            /*!< Identifier of scheduled tasks. */
    bool   m_taskScheduled;             /*!< Flag to indicate if there is tasks scheduled. */
};

};  // namespace nbmap

/*! @} */

#endif  // HTTPDOWNLOADMANAGER_H

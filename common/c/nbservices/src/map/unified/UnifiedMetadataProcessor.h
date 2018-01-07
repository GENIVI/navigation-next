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
   @file        UnifiedMetadataProcessor.h
   @defgroup    nbmap

   Description: UnifiedMetadataProcessor is responsible for the processing of
   metadata, including: request, serializing, desrializing, retry, and so on.
   This class should be singleton for a NB_Context, and is shared by several
   objects that need to know about metadata, such as UnifiedLayerProvder,
   UnifiedLayerManager and so on.
*/
/*
   (C) Copyright 2015 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */

#ifndef _UnifiedMetadataProcessor_H_
#define _UnifiedMetadataProcessor_H_

extern "C"
{
}

#include "base.h"
#include "AsyncCallback.h"
#include "NBProtocolMetadataSourceInformation.h"
#include "NBProtocolMetadataSourceParameters.h"
#include "MetadataConfiguration.h"

class NB_Context;

namespace nbmap
{

// UnifiedMetadataProcessor .......................................................................

class UnifiedMetadataProcessor : public Base
{
public:
    UnifiedMetadataProcessor(NB_Context* context);
    virtual ~UnifiedMetadataProcessor(void);

public:
    /*! Checks whether metadata is changed.

        This function will check whether it is necessary to communicate with server to check
        the metadata changes. A AsyncCallback must be provided in order for
        UnifiedMetadataProcessor to tell the user about the changes.

        @return None.
    */
    virtual void
    CheckMetadataChanges(shared_ptr<AsyncCallback<bool> > callback,     /*!< callback to be invoked */
                         shared_ptr<MetadataConfiguration> config,      /*!< Configuration used to retrieve metadtadata from server.
                                                                            Set it to empty to use default values. */
                         bool skipTimeInterval,                          /*!< Flag, when set to true, will
                                                                            check with server no matter the
                                                                            tile stamp of last sync time.  */
                         bool forceUpdate                                /*!< Flag to force update */
                         );

    /*! Return MetadataSourceInformation stored in this processor.

        User must call CheckMetadataChanges() before calling this function.

        @return protocol::MetadataSourceInformationSharedPtr
    */
    virtual protocol::MetadataSourceInformationSharedPtr GetMetadataSourceInformation(void);

    /*! Reset some statistic information.

        @return None.
    */
    virtual void Reset(void);

public:
    // Public functions for callbacks. Re-design these functions later

    /*! Return current NB_Context, used by ContextBasedSingleton.

        @return Pointer to NB_Context instance.
    */
    NB_Context* GetContext(void);

    /*! Set retry time of Metadata.

        @return None.
    */
    void SetMetadataRetryTimes(int number);

    /*! Set persistent metadata path.

        This path will be used to store serialized metadata.

        @return NE_OK if succeeded.
    */
    NB_Error SetPersistentMetadataPath(shared_ptr<string> metadataPath);

    /*! Create a copy of current MetadataConfiguration.

      @return shared pointer of crated copy.
    */
    shared_ptr<MetadataConfiguration> CopyCurrentMetadataConfig(void);

protected:
    virtual bool IsNeedUpdateMetadataForTime(shared_ptr<MetadataConfiguration> config,
                                             bool skipTimeInterval);

    virtual void StartMetadataRequestWithCallback(shared_ptr<AsyncCallback<bool> > callback) = 0;

    /*! Get the metadata source information from the persistent data of specified file

      This function gets the persistent data in the file specifed by 'm_metadataPath' and
      parse the data to a MetadataSourceInformation object 'm_metadataInformation'.

      @return NE_OK if succeeded.
    */
    NB_Error GetMetadataInformationFromPersistentData(shared_ptr<std::string> metadataPath);

    /*! Store the metadata source information to the persistent data of specified file

      @return NE_OK if success
    */
    NB_Error
    SetMetadataInformationToPersistentData(protocol::MetadataSourceInformationSharedPtr metadataInformation);

    /*! Converts internal MetadataSouceParamter into a MetadtaConfiguration.

      @return MetadataConfigurationPtr which can be used by client of CoreSDK.
    */
    MetadataConfigurationPtr SourceParameterToConfiguration();

    /*! Converts an MetadataConfiguration to MetadataSourceParameter.

      If everything goes OK, it will store the converted parameter into
      m_pMetadataSourceParams, and return NE_OK.

      @return NE_OK if succeeded, or other value otherwise.
    */
    NB_Error ConfigurationToSourceParameter(MetadataConfigurationPtr config);

protected:
    // Protected member fields ...............................................................
    NB_Context* m_pContext;             /*!< Pointer of NB_Context instance */

    bool   m_metadataRequestInProgress; /*!< Flag to indicate metadata request is in progress  */
    int    m_currentRetryTimes;         /*!< Number of current retry times  */
    int    m_metadataMaxRetryTime;      /*!< Number of m metadata max retry time  */
    uint32  m_lastSuccessTime;          /*!< Time stamp of last successful metadata query */

    /*!< MetadataSource parameters */
    protocol::MetadataSourceParametersSharedPtr m_pMetadataSourceParams;

    shared_ptr<string> m_metadataPath;    /*!< Full path of persistent metadata. It
                                               contains both path and filename. */
    shared_ptr<MetadataConfiguration> m_pConfig;

    /*! Persistent metadata information */
    protocol::MetadataSourceInformationSharedPtr m_metadataInformation;
};

typedef shared_ptr<UnifiedMetadataProcessor> UnifiedMetadataProcessorPtr;

/*! Callback to check whether metadata is changed.

    Refer to AsyncCallback for more description.
*/
template <typename T>
class CheckMetadataChangesCallback : public AsyncCallback<bool>,
                                     public Base
{
public:
    CheckMetadataChangesCallback(T* instance, void (T::*callback)(bool, NB_Error))
            : m_instance(instance),
              m_callback(callback)
    {
    }

    virtual ~CheckMetadataChangesCallback()
    {
    }

    virtual void Success(bool changed)
    {
        if (m_instance && m_callback)
        {
            (m_instance->*m_callback)(changed, NE_OK);
        }
    }

    virtual void Error (NB_Error error)
    {
        // NE_BUSY means another request is being processed, and this callback will be
        // invoked later.
        if (m_instance && m_callback && error != NE_BUSY)
        {
           (m_instance->*m_callback)(false, error);
        }
    }

    /*! Set this callback into invalid state.

      @return None.
    */
    void SetInvalid()
    {
        m_instance = NULL;
    }

private:
    T* m_instance;
    void (T::*m_callback)(bool, NB_Error);
};

}

#endif /* _UnifiedMetadataProcessor_H_ */

/*! @} */

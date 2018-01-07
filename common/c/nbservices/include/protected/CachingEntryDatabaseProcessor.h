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

    @file       CachingEntryDatabaseProcessor.h

    Class CachingEntryDatabaseProcessor inherits from
    CachingEntryProcessor interface. A CachingEntryDatabaseProcessor
    object is used to get and save the CachingEntry objects from a
    database file. 'PAL_DB*' functions are used to operate the database
    file which saves the caching entries.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef CACHINGENTRYDATABASEPROCESSOR_H
#define CACHINGENTRYDATABASEPROCESSOR_H

/*!
    @addtogroup nbcommon
    @{
*/

extern "C"
{
    #include "paldb.h"
    #include "nberror.h"
    #include "nbcontext.h"
}

#include "CachingEntryProcessor.h"
#include "CachingEntry.h"

namespace nbcommon
{

// Types ........................................................................................

/*! Processor for operating the database file which saves the caching entries */
class CachingEntryDatabaseProcessor : public Base,
                                      public CachingEntryProcessor
{
public:
    // Public functions .........................................................................

    /*! CachingEntryDatabaseProcessor constructor */
    CachingEntryDatabaseProcessor();

    /*! CachingEntryDatabaseProcessor destructor */
    virtual ~CachingEntryDatabaseProcessor();

    /*! Initialize a CachingEntryDatabaseProcessor object

        This function should be called before user calls other public functions.

        @return NE_OK if success
    */
    NB_Error
    Initialize(PAL_Instance* pal,                       /*!< A PAL instance */
               shared_ptr<std::string> databaseFullPath /*!< A full path of the database file which
                                                             saves the caching entries */
               );

    /* See description in CachingEntryProcessor.h */
    virtual CachingEntryPtr GetEntry(shared_ptr<std::string> type,
                                     shared_ptr<std::string> name);
    virtual void GetAllEntries(std::vector<CachingEntryPtr>& entries);
    virtual uint32 GetEntryCount();
    virtual NB_Error SetEntry(CachingEntryPtr entry);
    virtual void RemoveEntry(shared_ptr<std::string> type,
                             shared_ptr<std::string> name);
    virtual bool IsEntryExisting(shared_ptr<std::string> type,
                                 shared_ptr<std::string> name);
    virtual void SetEntryProtected(shared_ptr<std::string> type,
                                   shared_ptr<std::string> name,
                                   bool entryProtected);


private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    CachingEntryDatabaseProcessor(const CachingEntryDatabaseProcessor& processor);
    CachingEntryDatabaseProcessor& operator=(const CachingEntryDatabaseProcessor& processor);

    /* See source file for description */

    bool IsInitialized() const;
    void Reset();
    NB_Error CreateDatabaseConnection();
    NB_Error CreateEntryTable();
    NB_Error InitializeAllEntries();
    NB_Error ReadEntryFromDatabase(PAL_DBReader* reader, CachingEntryPtr& entry);
    NB_Error SetEntryToDatabase(CachingEntryPtr entry,
                                bool added);
    NB_Error RemoveEntryFromDatabase(shared_ptr<std::string> type,
                                     shared_ptr<std::string> name);
    NB_Error UpdateProtectedFlagToDatabase(shared_ptr<std::string> type,
                                           shared_ptr<std::string> name,
                                           bool entryProtected);
    NB_Error SerializeFromAdditionalData(shared_ptr<std::map<std::string, shared_ptr<std::string> > > additionalData,
                                         uint8** data,
                                         nb_size* dataSize);
    NB_Error DeserializeToAdditionalData(const uint8* data,
                                         nb_size dataSize,
                                         shared_ptr<std::map<std::string, shared_ptr<std::string> > >& additionalData);
    NB_Error SetStringToPersistentData(NB_PersistentData* persistentData,
                                       const std::string& key,
                                       const std::string& data);


private:
    // Private members ..........................................................................

    PAL_Instance* m_pal;                                /*!< A PAL instance */
    PAL_DBConnect* m_dbConnection;                      /*!< Database connection */
    shared_ptr<std::string> m_databaseFullPath;         /*!< A full path of the database file which saves the caching entries */
    std::map<std::string, shared_ptr<std::map<std::string, CachingEntryPtr> > > m_entries;  /*!< Caching entries indexed by the type and name */
};

typedef shared_ptr<CachingEntryDatabaseProcessor> CachingEntryDatabaseProcessorPtr;

};  // namespace nbcommon

/*! @} */

#endif  // CACHINGENTRYPROCESSOR_H

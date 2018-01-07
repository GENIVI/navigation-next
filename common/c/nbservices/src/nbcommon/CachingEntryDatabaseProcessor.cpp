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

    @file       CachingEntryDatabaseProcessor.cpp

    See header file for description.
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

extern "C"
{
    #include "paldberror.h"
    #include "palfile.h"
    #include "nbpersistentdataprotected.h"
}

#include "CachingEntryDatabaseProcessor.h"

/*! @{ */

using namespace std;
using namespace nbcommon;

// Local Constants ..............................................................................

#define MAX_DATABASE_FULL_PATH_LENGTH 512

static const int TRUE_TO_INTEGER = 1;
static const int FALSE_TO_INTEGER = 0;

static const char DB_FIELD_PROTECTED[] = "Protected";
static const char DB_FIELD_TYPE[] = "Type";
static const char DB_FIELD_NAME[] = "Name";
static const char DB_FIELD_ADDITIONAL_DATA[] = "AdditionalData";

static const char DB_PARAMETER_PROTECTED[] = "@Protected";
static const char DB_PARAMETER_TYPE[] = "@Type";
static const char DB_PARAMETER_NAME[] = "@Name";
static const char DB_PARAMETER_ADDITIONAL_DATA[] = "@AdditionalData";

static const char CREATE_ENTRY_TABLE_SQL[] =
"CREATE TABLE IF NOT EXISTS 'entry' ( \
'Id' INTEGER PRIMARY KEY NOT NULL, \
'Protected' INTEGER, \
'Type' VARCHAR, \
'Name' VARCHAR, \
'AdditionalData' BLOB);";

static const char GET_ALL_ENTRIES_SQL[] =
"SELECT * FROM entry;";

static const char INSERT_ENTRY_SQL[] =
"INSERT INTO entry ( \
'Protected', \
'Type', \
'Name', \
'AdditionalData' \
) VALUES ( \
@Protected, \
@Type, \
@Name, \
NULL);";

static const char INSERT_ENTRY_WITH_ADDITIONAL_DATA_SQL[] =
"INSERT INTO entry ( \
'Protected', \
'Type', \
'Name', \
'AdditionalData' \
) VALUES ( \
@Protected, \
@Type, \
@Name, \
@AdditionalData);";

static const char UPDATE_ENTRY_SQL[] =
"UPDATE entry SET \
Protected = @Protected, \
AdditionalData = NULL \
WHERE Type = @Type AND Name = @Name;";

static const char UPDATE_ENTRY_WITH_ADDITIONAL_DATA_SQL[] =
"UPDATE entry SET \
Protected = @Protected, \
AdditionalData = @AdditionalData \
WHERE Type = @Type AND Name = @Name;";

static const char UPDATE_PROTECTED_SQL[] =
"UPDATE entry SET Protected = @Protected WHERE Type = @Type AND Name = @Name;";

static const char DELETE_ALL_ENTRIES_SQL[] =
"DELETE FROM entry;";

static const char DELETE_ENTRIES_BY_TYPE_SQL[] =
"DELETE FROM entry WHERE Type = @Type;";

static const char DELETE_ENTRY_BY_TYPE_AND_NAME_SQL[] =
"DELETE FROM entry WHERE Type = @Type AND Name = @Name;";


// Public functions .............................................................................

/* See header file for description */
CachingEntryDatabaseProcessor::CachingEntryDatabaseProcessor() : m_pal(NULL),
                                                                 m_dbConnection(NULL),
                                                                 m_databaseFullPath(),
                                                                 m_entries()
{
    // Nothing to do here.
}

/* See header file for description */
CachingEntryDatabaseProcessor::~CachingEntryDatabaseProcessor()
{
    Reset();
}

/* See header file for description */
NB_Error
CachingEntryDatabaseProcessor::Initialize(PAL_Instance* pal,
                                          shared_ptr<string> databaseFullPath)
{
    NB_Error nbError = NE_OK;
    PAL_DB_Error dbError = PAL_DB_OK;

    if ((!pal) || (!databaseFullPath))
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (IsInitialized())
    {
        return NE_EXIST;
    }

    Reset();

    // Initialize the members of this object.
    m_pal = pal;
    m_databaseFullPath = databaseFullPath;

    // Create the database connection.
    nbError = CreateDatabaseConnection();
    if (nbError != NE_OK)
    {
        Reset();
        return nbError;
    }

    if (!m_dbConnection)
    {
        Reset();
        return NE_UNEXPECTED;
    }

    // Start a Transaction to create the caching entry table and get all caching entries.
    dbError = PAL_DB_Transaction_Begin(m_dbConnection);
    if(dbError != PAL_DB_OK)
    {
        Reset();
        return NE_BADDATA;
    }

    nbError = CreateEntryTable();
    nbError = nbError ? nbError : InitializeAllEntries();

    if (nbError == NE_OK)
    {
        dbError = PAL_DB_Transaction_Commit(m_dbConnection);
        if (dbError != PAL_DB_OK)
        {
            nbError = NE_BADDATA;
        }
    }

    if (nbError != NE_OK)
    {
        // Ignore the returned error.
        PAL_DB_Transaction_Rollback(m_dbConnection);
        Reset();
        return nbError;
    }

    return NE_OK;
}

/* See description in CachingEntryProcessor.h */
CachingEntryPtr
CachingEntryDatabaseProcessor::GetEntry(shared_ptr<string> type,
                                        shared_ptr<string> name)
{
    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return CachingEntryPtr();
    }

    if ((!type) || (!name))
    {
        return CachingEntryPtr();
    }

    // Try to find the map of the caching entries by the type.
    map<string, shared_ptr<map<string, CachingEntryPtr> > >::iterator foundType = m_entries.find(*type);
    if (foundType == m_entries.end())
    {
        return CachingEntryPtr();
    }

    if (!(foundType->second))
    {
        return CachingEntryPtr();
    }

    map<string, CachingEntryPtr>& entriesOfTypeRef = *(foundType->second);

    // Try to find the caching entry by the name.
    map<string, CachingEntryPtr>::iterator foundName = entriesOfTypeRef.find(*name);
    if (foundName == entriesOfTypeRef.end())
    {
        return CachingEntryPtr();
    }

    return foundName->second;
}

/* See description in CachingEntryProcessor.h */
void
CachingEntryDatabaseProcessor::GetAllEntries(vector<CachingEntryPtr>& entries)
{
    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return;
    }

    map<string, shared_ptr<map<string, CachingEntryPtr> > >::const_iterator typeIterator = m_entries.begin();
    map<string, shared_ptr<map<string, CachingEntryPtr> > >::const_iterator typeEnd = m_entries.end();

    // Loop for each type.
    for (; typeIterator != typeEnd; ++typeIterator)
    {
        if (!(typeIterator->second))
        {
            continue;
        }

        map<string, CachingEntryPtr>& entriesOfType = *(typeIterator->second);
        map<string, CachingEntryPtr>::const_iterator nameIterator = entriesOfType.begin();
        map<string, CachingEntryPtr>::const_iterator nameEnd = entriesOfType.end();

        // Loop for each name.
        for (; nameIterator != nameEnd; ++nameIterator)
        {
            if (!(nameIterator->second))
            {
                continue;
            }

            // Add the caching entry to the returned vector.
            entries.push_back(nameIterator->second);
        }
    }
}

/* See description in CachingEntryProcessor.h */
uint32
CachingEntryDatabaseProcessor::GetEntryCount()
{
    uint32 entryCount = 0;
    map<string, shared_ptr<map<string, CachingEntryPtr> > >::const_iterator iterator = m_entries.begin();
    map<string, shared_ptr<map<string, CachingEntryPtr> > >::const_iterator end = m_entries.end();

    for (; iterator != end; ++iterator)
    {
        const shared_ptr<map<string, CachingEntryPtr> >& entriesOfType = iterator->second;

        if (entriesOfType)
        {
            entryCount += (uint32) (entriesOfType->size());
        }
    }

    return entryCount;
}

/* See description in CachingEntryProcessor.h */
NB_Error
CachingEntryDatabaseProcessor::SetEntry(CachingEntryPtr entry)
{
    NB_Error error = NE_OK;

    if (!entry)
    {
        return NE_INVAL;
    }

    // Get the type and name of the caching entry.
    shared_ptr<string> type = entry->GetType();
    shared_ptr<string> name = entry->GetName();

    if ((!type) || (!name))
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    // Find the map of the caching entries by the type.
    map<string, shared_ptr<map<string, CachingEntryPtr> > >::iterator typeIterator = m_entries.find(*type);
    if (typeIterator == m_entries.end())
    {
        shared_ptr<map<string, CachingEntryPtr> > entriesOfType(new map<string, CachingEntryPtr>());
        if (!entriesOfType)
        {
            return NE_NOMEM;
        }

        typeIterator = m_entries.insert(pair<string, shared_ptr<map<string, CachingEntryPtr> > >(*type, entriesOfType)).first;
        if (typeIterator == m_entries.end())
        {
            return NE_UNEXPECTED;
        }
    }

    if (!(typeIterator->second))
    {
        return NE_UNEXPECTED;
    }

    map<string, CachingEntryPtr>& entriesOfTypeReference = *(typeIterator->second);
    pair<map<string, CachingEntryPtr>::iterator, bool> insertedResult = entriesOfTypeReference.insert(pair<string, CachingEntryPtr>(*name, entry));

    if (insertedResult.first == entriesOfTypeReference.end())
    {
        return NE_UNEXPECTED;
    }

    if (insertedResult.second)
    {
        // Add the caching entry to the database. Because the caching entry is added successfully in memory.
        error = SetEntryToDatabase(entry, true);
        if (error != NE_OK)
        {
            // Remove the added caching entry. Because an error occurs.
            entriesOfTypeReference.erase(insertedResult.first);
            return error;
        }
    }
    else
    {
        // The caching entry is already existing and update the caching entry to the database.
        error = SetEntryToDatabase(entry, false);
        if (error != NE_OK)
        {
            return error;
        }

        // Update the caching entry in memory.
        insertedResult.first->second = entry;
    }

    return error;
}

/* See description in CachingEntryProcessor.h */
void
CachingEntryDatabaseProcessor::RemoveEntry(shared_ptr<string> type,
                                           shared_ptr<string> name)
{
    NB_Error error = NE_OK;

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return;
    }

    if (type)
    {
        // Try to find the map of the caching entries by the type.
        map<string, shared_ptr<map<string, CachingEntryPtr> > >::iterator foundType = m_entries.find(*type);
        if (foundType == m_entries.end())
        {
            return;
        }

        if (name)
        {
            if (!(foundType->second))
            {
                return;
            }

            map<string, CachingEntryPtr>& entriesOfType = *(foundType->second);

            // Try to find the caching entry by the name.
            map<string, CachingEntryPtr>::iterator foundName = entriesOfType.find(*name);
            if (foundName == entriesOfType.end())
            {
                return;
            }

            // Remove the caching entry by the type and name.
            error = RemoveEntryFromDatabase(type, name);
            if (error != NE_OK)
            {
                return;
            }

            // Remove the caching entry in memory.
            entriesOfType.erase(foundName);
        }
        else
        {
            // Remove the caching entries by the type.
            error = RemoveEntryFromDatabase(type, name);
            if (error != NE_OK)
            {
                return;
            }

            // Remove the caching entries of the type in memory.
            m_entries.erase(foundType);
        }
    }
    else
    {
        // Remove all caching entries.
        error = RemoveEntryFromDatabase(type, name);
        if (error != NE_OK)
        {
            return;
        }

        // Remove all caching entries in memory.
        m_entries.clear();
    }
}

/* See description in CachingEntryProcessor.h */
bool
CachingEntryDatabaseProcessor::IsEntryExisting(shared_ptr<string> type,
                                               shared_ptr<string> name)
{
    CachingEntryPtr entry = GetEntry(type, name);

    return (entry != NULL);
}

/* See description in CachingEntryProcessor.h */
void
CachingEntryDatabaseProcessor::SetEntryProtected(shared_ptr<string> type,
                                                 shared_ptr<string> name,
                                                 bool entryProtected)
{
    NB_Error error = NE_OK;

    if ((!type) || (!name))
    {
        return;
    }

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return;
    }

    // Check if the caching entry exists.
    CachingEntryPtr entry = GetEntry(type, name);
    if (!entry)
    {
        return;
    }

    // Update the protected flag of the caching entry to the database.
    error = UpdateProtectedFlagToDatabase(type, name, entryProtected);
    if (error != NE_OK)
    {
        return;
    }

    // Update the protected flag of the found caching entry in memory.
    entry->SetProtected(entryProtected);
}


// Private functions ............................................................................

/*! Check if this object is initialized

    This function should be called in all public functions.

    @return true if this object is initialized, false otherwise.
*/
bool
CachingEntryDatabaseProcessor::IsInitialized() const
{
    if (m_pal && m_dbConnection && m_databaseFullPath)
    {
        return true;
    }

    return false;
}

/*! Reset this object to be uninitialized

    Clear all members in this object.

    @return None
*/
void
CachingEntryDatabaseProcessor::Reset()
{
    m_entries.clear();

    // Close the database connection.
    if (m_dbConnection)
    {
        PAL_DBClose(m_dbConnection);
        m_dbConnection = NULL;
    }

    // Clear other members in this object.
    m_databaseFullPath.reset();
    m_pal = NULL;
}

/*! Create the database connection if it is NULL

    @return NB_OK if success
*/
NB_Error
CachingEntryDatabaseProcessor::CreateDatabaseConnection()
{
    PAL_Error palError = PAL_Ok;
    PAL_DB_Error dbError = PAL_DB_OK;
    PAL_DBConnect* connection = NULL;
    char pathWithoutFile[MAX_DATABASE_FULL_PATH_LENGTH] = {0};

    // Return NE_OK if the database connection has already been created.
    if (m_dbConnection)
    {
        return NE_OK;
    }

    if ((!m_pal) || (!m_databaseFullPath) || (m_databaseFullPath->empty()))
    {
        return NE_NOTINIT;
    }

    nsl_memset(pathWithoutFile, 0, sizeof(pathWithoutFile));
    nsl_strlcpy(pathWithoutFile, m_databaseFullPath->c_str(), sizeof(pathWithoutFile));

    palError = PAL_FileRemovePath(m_pal, pathWithoutFile);
    palError = palError ? palError : PAL_FileCreateDirectoryEx(m_pal, pathWithoutFile);
    if (palError != PAL_Ok)
    {
        return NE_FSYS;
    }

    dbError = PAL_DBOpen(m_pal, PAL_DB_Sqlite, m_databaseFullPath->c_str(), &connection);
    if (dbError != PAL_DB_OK)
    {
        return NE_BADDATA;
    }

    m_dbConnection = connection;

    return NE_OK;
}

/*! Create the caching entry table if not exists

    @return NB_OK if success
*/
NB_Error
CachingEntryDatabaseProcessor::CreateEntryTable()
{
    PAL_DB_Error dbError = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!m_dbConnection)
    {
        return NE_NOTINIT;
    }

    // Create the caching entry table if not exists.
    dbError = PAL_DBInitCommand(m_dbConnection, CREATE_ENTRY_TABLE_SQL, &command);
    if (dbError != PAL_DB_OK)
    {
        return NE_BADDATA;
    }

    dbError = PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);
    command = NULL;

    return (dbError == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/*! Get all caching entries from the database

    This function gets all caching entries from the database and saves them to the
    member 'm_entries'.

    @return NB_OK if success
*/
NB_Error
CachingEntryDatabaseProcessor::InitializeAllEntries()
{
    NB_Error nbError = NE_OK;
    PAL_DB_Error dbError = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;

    if (!m_dbConnection)
    {
        return NE_NOTINIT;
    }

    // Get all caching entries.
    dbError = PAL_DBInitCommand(m_dbConnection, GET_ALL_ENTRIES_SQL, &command);
    if (dbError != PAL_DB_OK)
    {
        return NE_BADDATA;
    }

    dbError = PAL_DBExecuteReader(command, &reader);
    if (dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, reader);
        command = NULL;
        reader = NULL;
        return NE_BADDATA;
    }

    if (!reader)
    {
        PAL_DBCommandFinalize(command, reader);
        command = NULL;
        reader = NULL;
        return NE_UNEXPECTED;
    }

    // Read the caching entries from the database one by one.
    map<string, shared_ptr<map<string, CachingEntryPtr> > > entries;
    while (nbError == NE_OK)
    {
        CachingEntryPtr entry;

        nbError = ReadEntryFromDatabase(reader, entry);
        if (nbError == NE_NOENT)
        {
            // There are no more entries.
            nbError = NE_OK;
            break;
        }
        else if (nbError != NE_OK)
        {
            break;
        }

        if (!entry)
        {
            // There are no more entries.
            break;
        }

        shared_ptr<string> type = entry->GetType();
        shared_ptr<string> name = entry->GetName();
        if ((!type) || (!name))
        {
            nbError = NE_UNEXPECTED;
            break;
        }

        // Add this caching entry to the map 'entries'.
        map<string, shared_ptr<map<string, CachingEntryPtr> > >::iterator found = entries.find(*type);
        if (found == entries.end())
        {
            shared_ptr<map<string, CachingEntryPtr> > entriesOfType(new map<string, CachingEntryPtr>());
            if (!entriesOfType)
            {
                nbError = NE_NOMEM;
                break;
            }

            found = entries.insert(pair<string, shared_ptr<map<string, CachingEntryPtr> > >(*type, entriesOfType)).first;
            if (found == entries.end())
            {
                nbError = NE_UNEXPECTED;
                break;
            }
        }

        if (!(found->second))
        {
            nbError = NE_UNEXPECTED;
            break;
        }

        // Ignore the returned error.
        found->second->insert(pair<string, CachingEntryPtr>(*name, entry));
    }

    if (nbError == NE_OK)
    {
        // Set the caching entries.
        m_entries.swap(entries);
    }

    PAL_DBCommandFinalize(command, reader);
    command = NULL;
    reader = NULL;

    return nbError;
}

/*! Read a caching entry from the database

    @return NB_OK if success,
            NE_BADDATA if a database error occurs,
            NE_NOENT if there are no more caching entries.
*/
NB_Error
CachingEntryDatabaseProcessor::ReadEntryFromDatabase(PAL_DBReader* reader,  /*!< A database reader */
                                                     CachingEntryPtr& entry /*!< On return a cachng entry if success */
                                                     )
{
    NB_Error nbError = NE_OK;
    PAL_DB_Error dbError = PAL_DB_OK;

    if (!reader)
    {
        return NE_INVAL;
    }

    dbError = PAL_DBReaderRead(reader);

    if (dbError == PAL_DB_ROW)
    {
        int entryProtected = 0;
        int dataSize = 0;
        byte* data = NULL;
        char* typeChar = NULL;
        char* nameChar = NULL;
        shared_ptr<string> type;
        shared_ptr<string> name;
        shared_ptr<map<string, shared_ptr<string> > > additionalData;

        // Reset the error of database from PAL_DB_ROW to PAL_DB_OK.
        dbError = PAL_DB_OK;

        // Read the protected flag.
        dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBReaderGetInt(reader, DB_FIELD_PROTECTED, &entryProtected);

        // Read the type.
        dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBReaderGetstring(reader, DB_FIELD_TYPE, &typeChar);
        if ((dbError == PAL_DB_OK) && typeChar)
        {
            type.reset(new string(typeChar));
            nsl_free(typeChar);
            typeChar = NULL;
        }

        // Read the name.
        dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBReaderGetstring(reader, DB_FIELD_NAME, &nameChar);
        if ((dbError == PAL_DB_OK) && nameChar)
        {
            name.reset(new string(nameChar));
            nsl_free(nameChar);
            nameChar = NULL;
        }

        if (dbError != PAL_DB_OK)
        {
            return NE_BADDATA;
        }

        // Read the additional data.
        dbError = PAL_DBReaderGetblob(reader, DB_FIELD_ADDITIONAL_DATA, &data, &dataSize);
        if ((dbError == PAL_DB_OK) && data && (dataSize > 0))
        {
            nbError = DeserializeToAdditionalData((const uint8*) data, (nb_size) dataSize, additionalData);
            nsl_free(data);
            data = NULL;
            dataSize = 0;
            if (nbError != NE_OK)
            {
                return nbError;
            }
        }
        else
        {
            // Ignore the error when read the additional data. Because additional data can be set to empty.
            dbError = PAL_DB_OK;
        }

        if (dbError == PAL_DB_OK)
        {
            if ((!type) || (!name))
            {
                return NE_UNEXPECTED;
            }

            CachingEntryPtr newEntry(new CachingEntry(type, name, additionalData));
            if (!newEntry)
            {
                return NE_NOMEM;
            }

            newEntry->SetProtected((entryProtected == FALSE_TO_INTEGER) ? false : true);

            entry = newEntry;
            return NE_OK;
        }
    }
    else if (dbError == PAL_DB_DONE)
    {
        return NE_NOENT;
    }

    return NE_BADDATA;
}

/*! Add or update a caching entry to the database

    @return NB_OK if success, NE_BADDATA if a database error occurs.
*/
NB_Error
CachingEntryDatabaseProcessor::SetEntryToDatabase(CachingEntryPtr entry,    /*!< A caching entry to add to the database */
                                                  bool added                /*!< Is the caching entry added or updated? */
                                                  )
{
    NB_Error nbError = NE_OK;
    PAL_DB_Error dbError = PAL_DB_OK;
    nb_size serializedDataSize = 0;
    uint8* serializedData = NULL;
    PAL_DBCommand* command = NULL;

    if (!entry)
    {
        return NE_INVAL;
    }

    // Get the members of the caching entry.
    int entryProtected = (entry->GetProtected()) ? TRUE_TO_INTEGER : FALSE_TO_INTEGER;
    shared_ptr<string> type = entry->GetType();
    shared_ptr<string> name = entry->GetName();
    shared_ptr<map<string, shared_ptr<string> > > additionalData = entry->GetAdditionalData();

    if ((!type) || (!name))
    {
        return NE_INVAL;
    }

    if (!m_dbConnection)
    {
        return NE_NOTINIT;
    }

    if (additionalData && (additionalData->size() > 0))
    {
        // Serialize the additional data.
        nbError = SerializeFromAdditionalData(additionalData, &serializedData, &serializedDataSize);
        if (nbError != NE_OK)
        {
            return nbError;
        }

        if ((!serializedData) || (serializedDataSize <= 0))
        {
            nsl_free(serializedData);
            return NE_UNEXPECTED;
        }

        // Set the caching entry with the serialized additional data.
        dbError = PAL_DBInitCommand(m_dbConnection,
                                    added ? INSERT_ENTRY_WITH_ADDITIONAL_DATA_SQL : UPDATE_ENTRY_WITH_ADDITIONAL_DATA_SQL,
                                    &command);
        if (dbError != PAL_DB_OK)
        {
            nsl_free(serializedData);
            serializedData = NULL;
            serializedDataSize = 0;

            return NE_BADDATA;
        }

        dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindBlobParam(command,
                                                                         DB_PARAMETER_ADDITIONAL_DATA,
                                                                         (void*) serializedData,
                                                                         (int) serializedDataSize);
        nsl_free(serializedData);
        serializedData = NULL;
        serializedDataSize = 0;
    }
    else
    {
        // Set the caching entry and clear the additional data.
        dbError = PAL_DBInitCommand(m_dbConnection,
                                    added ? INSERT_ENTRY_SQL : UPDATE_ENTRY_SQL,
                                    &command);
        if (dbError != PAL_DB_OK)
        {
            return NE_BADDATA;
        }
    }

    dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindIntParam(command, DB_PARAMETER_PROTECTED, entryProtected);
    dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindStringParam(command, DB_PARAMETER_TYPE, type->c_str());
    dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindStringParam(command, DB_PARAMETER_NAME, name->c_str());

    dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return ((dbError == PAL_DB_OK) ? NE_OK : NE_BADDATA);
}

/*! Remove a caching entry from the database

    @return NB_OK if success, NE_BADDATA if a database error occurs.
*/
NB_Error
CachingEntryDatabaseProcessor::RemoveEntryFromDatabase(shared_ptr<string> type, /*!< Type to remove from the database */
                                                       shared_ptr<string> name  /*!< Name to remove from the database */
                                                       )
{
    PAL_DB_Error dbError = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!m_dbConnection)
    {
        return NE_NOTINIT;
    }

    if (type)
    {
        if (name)
        {
            // Remove the caching entry by the type and name.
            dbError = PAL_DBInitCommand(m_dbConnection, DELETE_ENTRY_BY_TYPE_AND_NAME_SQL, &command);
            if (dbError != PAL_DB_OK)
            {
                return NE_BADDATA;
            }

            dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindStringParam(command, DB_PARAMETER_TYPE, type->c_str());
            dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindStringParam(command, DB_PARAMETER_NAME, name->c_str());

            dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBExecuteNonQueryCommand(command);
            PAL_DBCommandFinalize(command, NULL);
            command = NULL;
        }
        else
        {
            // Remove the caching entries by the type.
            dbError = PAL_DBInitCommand(m_dbConnection, DELETE_ENTRIES_BY_TYPE_SQL, &command);
            if (dbError != PAL_DB_OK)
            {
                return NE_BADDATA;
            }

            dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindStringParam(command, DB_PARAMETER_TYPE, type->c_str());

            dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBExecuteNonQueryCommand(command);
            PAL_DBCommandFinalize(command, NULL);
            command = NULL;
        }
    }
    else
    {
        // Remove all caching entries.
        dbError = PAL_DBInitCommand(m_dbConnection, DELETE_ALL_ENTRIES_SQL, &command);
        if (dbError != PAL_DB_OK)
        {
            return NE_BADDATA;
        }

        dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBExecuteNonQueryCommand(command);
        PAL_DBCommandFinalize(command, NULL);
        command = NULL;
    }

    return ((dbError == PAL_DB_OK) ? NE_OK : NE_BADDATA);
}

/*! Update the protected flag of the caching entry to the database

    @return NB_OK if success, NE_BADDATA if a database error occurs.
*/
NB_Error
CachingEntryDatabaseProcessor::UpdateProtectedFlagToDatabase(shared_ptr<string> type,   /*!< Type of the caching entry to update */
                                                             shared_ptr<string> name,   /*!< Name of the caching entry to update */
                                                             bool entryProtected        /*!< Protected flag to update to the database */
                                                             )
{
    PAL_DB_Error dbError = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if ((!type) || (!name))
    {
        return NE_INVAL;
    }

    if (!m_dbConnection)
    {
        return NE_NOTINIT;
    }

    // Initialize a SQL command to update the protected flag.
    dbError = PAL_DBInitCommand(m_dbConnection, UPDATE_PROTECTED_SQL, &command);
    if (dbError != PAL_DB_OK)
    {
        return NE_BADDATA;
    }

    dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindIntParam(command, DB_PARAMETER_PROTECTED, entryProtected ? TRUE_TO_INTEGER : FALSE_TO_INTEGER);
    dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindStringParam(command, DB_PARAMETER_TYPE, type->c_str());
    dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindStringParam(command, DB_PARAMETER_NAME, name->c_str());

    dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);
    command = NULL;

    return ((dbError == PAL_DB_OK) ? NE_OK : NE_BADDATA);
}

/*! Serialize from a map of additional data to the serialized data

    @return NE_OK if success
*/
NB_Error
CachingEntryDatabaseProcessor::SerializeFromAdditionalData(shared_ptr<map<string, shared_ptr<string> > > additionalData,    /*!< A map of additional data */
                                                           uint8** data,        /*!< On return derialized data */
                                                           nb_size* dataSize    /*!< On return size of serialized data */
                                                           )
{
    NB_Error error = NE_OK;
    nb_size serializedDataSize = 0;
    uint8* serializedData = NULL;
    NB_PersistentData* persistentData = NULL;

    if ((!additionalData) || (!data) || (!dataSize))
    {
        return NE_INVAL;
    }

    error = NB_PersistentDataCreate(NULL, 0, NULL, &persistentData);
    if (error != NE_OK)
    {
        return error;
    }

    map<string, shared_ptr<string> >::const_iterator iterator = additionalData->begin();
    map<string, shared_ptr<string> >::const_iterator end = additionalData->end();

    for (; iterator != end; ++iterator)
    {
        // Use an empty string if the shared pointer to value is NULL.
        string value("");

        if (iterator->second)
        {
            value = *(iterator->second);
        }

        error = SetStringToPersistentData(persistentData, iterator->first, value);
        if (error != NE_OK)
        {
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;
            return error;
        }
    }

    // Serialize the persistent data.
    error = NB_PersistentDataSerialize(persistentData,
                                       (const uint8**) (&serializedData),
                                       &serializedDataSize);
    NB_PersistentDataDestroy(persistentData);
    persistentData = NULL;
    if (error != NE_OK)
    {
        return error;
    }

    *data = serializedData;
    *dataSize = serializedDataSize;
    return NE_OK;
}

/*! Deserialize from the serialized data to a map of additional data

    @return NE_OK if success
*/
NB_Error
CachingEntryDatabaseProcessor::DeserializeToAdditionalData(const uint8* data,   /*!< Serialized data */
                                                           nb_size dataSize,    /*!< Size of serialized data */
                                                           shared_ptr<map<string, shared_ptr<string> > >& additionalData    /*!< On return a map of additional data */
                                                           )
{
    NB_Error error = NE_OK;
    int i = 0;
    NB_PersistentData* persistentData = NULL;

    if ((!data) || (dataSize <= 0))
    {
        // There is no data to deserialize.
        return NE_INVAL;
    }

    error = NB_PersistentDataCreate(data, dataSize, NULL, &persistentData);
    if (error != NE_OK)
    {
        return error;
    }

    // Create a new map of additional data.
    shared_ptr<map<string, shared_ptr<string> > > newAdditionalData(new map<string, shared_ptr<string> >());
    if (!newAdditionalData)
    {
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;
        return NE_NOMEM;
    }

    // Get the key and value from the persistent data.
    i = 0;
    while (error == NE_OK)
    {
        const char* keyChar = NULL;
        uint8* valueData = NULL;
        nb_size valueDataSize = 0;

        // Get the next persistent element.
        error = NB_PersistentDataNext(persistentData,
                                      &i,
                                      &keyChar,
                                      &valueData,
                                      &valueDataSize);
        if (error != NE_OK)
        {
            break;
        }

        if ((!keyChar) || (!valueData) || (valueDataSize <= 0))
        {
            error = NE_UNEXPECTED;
            break;
        }

        shared_ptr<string> value(new string((const char*) valueData));
        nsl_free(valueData);
        valueData = NULL;
        valueDataSize = 0;
        if (!value)
        {
            error = NE_NOMEM;
            break;
        }

        // Ignore the returned error.
        newAdditionalData->insert(pair<string, shared_ptr<string> >(keyChar, value));
    }

    // Ignore the error NE_NOENT.
    error = (error == NE_NOENT) ? NE_OK : error;

    if (persistentData)
    {
        // Ignore the returned error.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;
    }

    // Return the map of additional data.
    if (error == NE_OK)
    {
        additionalData = newAdditionalData;
    }

    return error;
}

/*! Set a string to the persistent data

    @return NE_OK if success
*/
NB_Error
CachingEntryDatabaseProcessor::SetStringToPersistentData(NB_PersistentData* persistentData, /*!< A persistent data instance */
                                                         const string& key,                 /*!< A persistent key */
                                                         const string& data                 /*!< A persistent data of type string */
                                                         )
{
    nb_size dataSize = 0;

    if (!persistentData)
    {
        return NE_INVAL;
    }

    /* Add the data even if the parameter 'data' is "". Because we should save the key
       with empty data.
    */
    dataSize = (nb_size) (data.size() + 1);

    /*  Storing length of string plus 1 is simple to deserialize. It could be directly
        converted to 'char*'. This is same as below.
    */
    return NB_PersistentDataSet(persistentData,
                                key.c_str(),
                                (const uint8*) (data.c_str()),
                                dataSize,
                                FALSE,
                                FALSE);
}

/*! @} */

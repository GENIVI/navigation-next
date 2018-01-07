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

    @file     analyticsdbstorage.cpp
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2014 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */


#include "analyticsdbstorage.h"
#include "nbcontextprotected.h"
#include "paldb.h"
#include "paldberror.h"

using namespace nbcommon;

static const char QUERY_SIZE[]                 = "select sum(Size) as TotalSize from events";
static const char QUERY_COUNT[]                = "select count(*) as CategoryCount from events where Category=@category";
static const char QUERY_OLDEST_TIME[]          = "select min(Timestamp) as OldestTime from events where Category=@category";
static const char QUERY_EVENTS_DESCEND[]       = "select * from Events where Category=@category order by Timestamp desc limit @count";
static const char QUERY_EVENTS_ASCEND[]        = "select * from Events where Category=@category order by Timestamp asc limit @count";
static const char DELETE_EVENTS_BY_EVENTS[]    = "delete from Events where Category=@category and Timestamp=@timeStamp "
                                                 "and Name=@name and Event=@event";
static const char DELETE_EVENTS_BY_CATEGORY[]  = "delete from Events where Category=@category";
static const char DELETE_EVENTS_BY_TIMESTAMP[] = "delete from Events where Timestamp<@timeStamp";
static const char SAVE_EVENTS[]                = "insert into Events(Category,Timestamp,Name,Size,Event) values(@category,@timeStamp,"
                                                 "@name,@size,@event)";
static const char CREATETABLE[]                = "create table if not exists events(Category int not null, Timestamp int64 not null, "
                                                 "Name text not null, Size int not null, Event text not null)";
static const char CLEARTABLE[]                 = "delete from Events";

#define ANALYTICS_DB_CATEGORY             "Category"
#define ANALYTICS_DB_TIMESTAMP            "Timestamp"
#define ANALYTICS_DB_NAME                 "Name"
#define ANALYTICS_DB_EVENTBODY            "Event"
#define ANALYTICS_DB_SIZE                 "Size"
#define ANALYTICS_DB_TOTALSIZE            "TotalSize"
#define ANALYTICS_DB_CATEGORY_COUNT       "CategoryCount"
#define ANALYTICS_DB_OLDEST_TIME          "OldestTime"
#define ANALYTICS_DB_CATEGORY_SYMBOL      "@category"
#define ANALYTICS_DB_TIMESTAMP_SYMBOL     "@timeStamp"
#define ANALYTICS_DB_NAME_SYMBOL          "@name"
#define ANALYTICS_DB_EVENTBODY_SYMBOL     "@event"
#define ANALYTICS_DB_SIZE_SYMBOL          "@size"
#define ANALYTICS_DB_COUNT_SYMBOL         "@count"

NB_Error AnalyticsDBStorage::Initialize(NB_Context* context, const std::string& filePath)
{
    if(m_bInitialized)
    {
        return NE_UNEXPECTED;
    }

    m_pal = NULL;
    m_connection = NULL;
    m_context = context;
    if (!context || filePath.empty())
    {
        return NE_INVAL;
    }
    m_pal = NB_ContextGetPal(m_context);
    if(!m_pal)
    {
        return NE_NOENT;
    }
    PAL_DB_Error dbError = PAL_DBOpen(m_pal, PAL_DB_Sqlite,
                                      filePath.c_str(),
                                      &m_connection);
    if(dbError!=PAL_DB_OK)
    {
        return NE_NOENT;
    }

    PAL_DBCommand* command;
    dbError = PAL_DBInitCommand(m_connection, CREATETABLE, &command);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBClose(m_connection);
        m_connection = NULL;
        return NE_UNEXPECTED;
    }
    dbError = PAL_DBExecuteNonQueryCommand(command);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, NULL);
        PAL_DBClose(m_connection);
        m_connection = NULL;
        return NE_UNEXPECTED;
    }
    PAL_DBCommandFinalize(command, NULL);
    m_bInitialized = true;
    return NE_OK;
}

NB_Error AnalyticsDBStorage::AppendEvents(const std::vector<AnalyticsEvent>& eventsQueue)
{
    if(!m_bInitialized)
    {
        return NE_NOTINIT;
    }

    NB_Error error = NE_OK;
    PAL_DBCommand* command = NULL;

    std::vector<AnalyticsEvent>::const_iterator iter = eventsQueue.begin();
    while (iter != eventsQueue.end())
    {
        AnalyticsEvent event = *iter;
        PAL_DB_Error dbError = PAL_DBInitCommand(m_connection, SAVE_EVENTS, &command);
        if (dbError != PAL_DB_OK)
        {
            error = NE_UNEXPECTED;
            break;
        }
        dbError = PAL_DBBindIntParam(command, ANALYTICS_DB_CATEGORY_SYMBOL, event.category);
        dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindStringParam(command, ANALYTICS_DB_EVENTBODY_SYMBOL, event.eventBody.c_str());
        dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindLongParam(command, ANALYTICS_DB_TIMESTAMP_SYMBOL, event.timeStamp);
        dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindStringParam(command, ANALYTICS_DB_NAME_SYMBOL, event.name.c_str());
        dbError = (dbError != PAL_DB_OK) ? dbError : PAL_DBBindIntParam(command, ANALYTICS_DB_SIZE_SYMBOL, event.eventBody.length());
        if(dbError != PAL_DB_OK)
        {
            error = NE_UNEXPECTED;
            break;
        }

        dbError = PAL_DBExecuteNonQueryCommand(command);
        if(dbError != PAL_DB_OK)
        {
            error = NE_UNEXPECTED;
            break;
        }
        ++iter;
        PAL_DBCommandFinalize(command, NULL);
        command = NULL;
    }
    if(command)
    {
        PAL_DBCommandFinalize(command, NULL);
    }
    return error;

}

NB_Error AnalyticsDBStorage::ReadEvents(uint32 category, bool oldFirst, int readLimit,
                                        std::vector<AnalyticsEvent>& eventsQueue,  int& readCount)
{
    if(!m_bInitialized)
    {
        return NE_NOTINIT;
    }

    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    PAL_DB_Error dbError;

    const char* commandString = NULL;
    if (oldFirst)
    {
        commandString = QUERY_EVENTS_ASCEND;
    }
    else
    {
        commandString = QUERY_EVENTS_DESCEND;
    }
    dbError = PAL_DBInitCommand(m_connection, commandString, &command);
    if (dbError != PAL_DB_OK)
    {
        return NE_UNEXPECTED;
    }

    dbError = PAL_DBBindIntParam(command, ANALYTICS_DB_CATEGORY_SYMBOL, (int)category);
    dbError = (dbError != PAL_DB_OK)? dbError : PAL_DBBindIntParam(command, ANALYTICS_DB_COUNT_SYMBOL, readLimit);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, NULL);
        return NE_UNEXPECTED;
    }

    dbError = PAL_DBExecuteReader(command, &reader);

    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, reader);
        return NE_UNEXPECTED;
    }

    int readStatus = PAL_DBReaderRead(reader);
    NB_Error error = NE_OK;
    readCount = 0;
    while (readStatus == PAL_DB_ROW)
    {
        char* event = NULL;
        char* name = NULL;
        uint32 category = 0;
        int64 timeStamp = 0;

        AnalyticsEvent eventStruct;
        dbError = PAL_DBReaderGetstring(reader, ANALYTICS_DB_EVENTBODY, &event);
        if(dbError != PAL_DB_OK)
        {
            error = NE_UNEXPECTED;
            break;
        }
        eventStruct.eventBody = event;
        nsl_free(event);

        dbError = PAL_DBReaderGetstring(reader, ANALYTICS_DB_NAME, &name);
        if(dbError != PAL_DB_OK)
        {
            error = NE_UNEXPECTED;
            break;
        }
        eventStruct.name = name;
        nsl_free(name);

        dbError = PAL_DBReaderGetInt(reader, ANALYTICS_DB_CATEGORY, (int*)&category);
        if(dbError != PAL_DB_OK)
        {
            error = NE_UNEXPECTED;
            break;
        }
        eventStruct.category = category;

        dbError = PAL_DBReaderGetLong(reader, ANALYTICS_DB_TIMESTAMP, &timeStamp);
        if(dbError != PAL_DB_OK)
        {
            error = NE_UNEXPECTED;
            break;
        }
        eventStruct.timeStamp = timeStamp;

        eventsQueue.push_back(eventStruct);
        ++readCount;
        readStatus = PAL_DBReaderRead(reader);
    }
    PAL_DBCommandFinalize(command, reader);
    return error;
}

NB_Error AnalyticsDBStorage::DeleteEvents(const std::vector<AnalyticsEvent>& eventsQueue)
{
    if(!m_bInitialized)
    {
        return NE_NOTINIT;
    }

    NB_Error error = NE_OK;
    PAL_DBCommand* command = NULL;
    std::vector<AnalyticsEvent>::const_iterator iter = eventsQueue.begin();

    while (iter != eventsQueue.end())
    {
        AnalyticsEvent event = *iter;
        PAL_DB_Error dbError = PAL_DBInitCommand(m_connection, DELETE_EVENTS_BY_EVENTS, &command);
        if (dbError != PAL_DB_OK)
        {
            error = NE_UNEXPECTED;
            break;
        }

        dbError = PAL_DBBindIntParam(command, ANALYTICS_DB_CATEGORY_SYMBOL, (int)event.category);
        dbError = dbError?dbError:PAL_DBBindStringParam(command, ANALYTICS_DB_EVENTBODY_SYMBOL, event.eventBody.c_str());
        dbError = dbError?dbError:PAL_DBBindLongParam(command, ANALYTICS_DB_TIMESTAMP_SYMBOL, event.timeStamp);
        dbError = dbError?dbError:PAL_DBBindStringParam(command, ANALYTICS_DB_NAME_SYMBOL, event.name.c_str());
        if(dbError != PAL_DB_OK)
        {
            error = NE_UNEXPECTED;
            break;
        }

        dbError = PAL_DBExecuteNonQueryCommand(command);
        if(dbError != PAL_DB_OK)
        {
            error = NE_UNEXPECTED;
            break;
        }
        ++iter;

        PAL_DBCommandFinalize(command, NULL);
        command = NULL;
    }
    if(command)
    {
        PAL_DBCommandFinalize(command, NULL);
    }
    return error;
}

NB_Error AnalyticsDBStorage::DeleteExpireEvents(int64 timeStamp)
{
    if(!m_bInitialized)
    {
        return NE_NOTINIT;
    }

    PAL_DBCommand* command = NULL;

    PAL_DB_Error dbError = PAL_DBInitCommand(m_connection, DELETE_EVENTS_BY_TIMESTAMP, &command);
    if (dbError != PAL_DB_OK)
    {
        return NE_UNEXPECTED;
    }

    dbError = PAL_DBBindLongParam(command, ANALYTICS_DB_TIMESTAMP_SYMBOL, timeStamp);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, NULL);
        return NE_UNEXPECTED;
    }

    dbError = PAL_DBExecuteNonQueryCommand(command);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, NULL);
        return NE_UNEXPECTED;
    }

    PAL_DBCommandFinalize(command, NULL);
    return NE_OK;
}

int AnalyticsDBStorage::ReadTotalLength()
{
    if(!m_bInitialized)
    {
        return -1;
    }

    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    PAL_DB_Error dbError;

    dbError = PAL_DBInitCommand(m_connection, QUERY_SIZE, &command);
    if (dbError != PAL_DB_OK)
    {
        return -1;
    }

    dbError = PAL_DBExecuteReader(command, &reader);

    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, reader);
        return -1;
    }

    dbError = PAL_DBReaderRead(reader);
    if(dbError != PAL_DB_ROW)
    {
        PAL_DBCommandFinalize(command, reader);
        return -1;
    }

    int length = 0;
    dbError = PAL_DBReaderGetInt(reader, ANALYTICS_DB_TOTALSIZE, &length);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, reader);
        return -1;
    }
    PAL_DBCommandFinalize(command, reader);
    return length;
}

AnalyticsDBStorage::~AnalyticsDBStorage()
{
    if(!m_connection)
    {
        return;
    }
    PAL_DBClose(m_connection);
    m_connection = NULL;
}

nbcommon::AnalyticsDBStorage::AnalyticsDBStorage():
    m_context(NULL),
    m_pal(NULL),
    m_connection(NULL),
    m_bInitialized(false)
{
}

int AnalyticsDBStorage::EventCount(uint32 category)
{
    if(!m_bInitialized)
    {
        return -1;
    }

    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    PAL_DB_Error dbError;

    dbError = PAL_DBInitCommand(m_connection, QUERY_COUNT, &command);
    if (dbError != PAL_DB_OK)
    {
        return -1;
    }

    dbError = PAL_DBBindIntParam(command, ANALYTICS_DB_CATEGORY_SYMBOL, (int)category);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, NULL);
        return -1;
    }

    dbError = PAL_DBExecuteReader(command, &reader);

    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, reader);
        return -1;
    }

    dbError = PAL_DBReaderRead(reader);
    if(dbError != PAL_DB_ROW)
    {
        PAL_DBCommandFinalize(command, reader);
        return -1;
    }

    int length = 0;
    dbError = PAL_DBReaderGetInt(reader, ANALYTICS_DB_CATEGORY_COUNT, &length);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, reader);
        return -1;
    }
    PAL_DBCommandFinalize(command, reader);
    return length;
}

NB_Error AnalyticsDBStorage::DeleteEvents(uint32 category)
{
    if(!m_bInitialized)
    {
        return NE_NOTINIT;
    }

    PAL_DBCommand* command = NULL;

    PAL_DB_Error dbError = PAL_DBInitCommand(m_connection, DELETE_EVENTS_BY_CATEGORY, &command);
    if (dbError != PAL_DB_OK)
    {
        return NE_UNEXPECTED;
    }

    dbError = PAL_DBBindIntParam(command, ANALYTICS_DB_CATEGORY_SYMBOL, (int)category);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, NULL);
        return NE_UNEXPECTED;
    }

    dbError = PAL_DBExecuteNonQueryCommand(command);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, NULL);
        return NE_UNEXPECTED;
    }

    PAL_DBCommandFinalize(command, NULL);
    return NE_OK;
}

NB_Error AnalyticsDBStorage::ClearTable()
{
    if(!m_bInitialized)
    {
        return NE_NOTINIT;
    }

    PAL_DBCommand* command = NULL;

    PAL_DB_Error dbError = PAL_DBInitCommand(m_connection, CLEARTABLE, &command);
    if (dbError != PAL_DB_OK)
    {
        return NE_UNEXPECTED;
    }

    dbError = PAL_DBExecuteNonQueryCommand(command);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, NULL);
        return NE_UNEXPECTED;
    }

    PAL_DBCommandFinalize(command, NULL);
    return NE_OK;
}
int64 AnalyticsDBStorage::GetOldestEventTimeStamp(uint32 category)
{
    if(!m_bInitialized)
    {
        return -1;
    }

    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    PAL_DB_Error dbError;

    dbError = PAL_DBInitCommand(m_connection, QUERY_OLDEST_TIME, &command);
    if (dbError != PAL_DB_OK)
    {
        return -1;
    }

    dbError = PAL_DBBindIntParam(command, ANALYTICS_DB_CATEGORY_SYMBOL, (int)category);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, NULL);
        return -1;
    }

    dbError = PAL_DBExecuteReader(command, &reader);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, reader);
        return -1;
    }

    dbError = PAL_DBReaderRead(reader);
    if(dbError != PAL_DB_ROW)
    {
        PAL_DBCommandFinalize(command, reader);
        return -1;
    }

    int64 oldest = 0;
    dbError = PAL_DBReaderGetLong(reader, ANALYTICS_DB_OLDEST_TIME, &oldest);
    if(dbError != PAL_DB_OK)
    {
        PAL_DBCommandFinalize(command, reader);
        return -1;
    }
    PAL_DBCommandFinalize(command, reader);
    return oldest;
}
/*!
    @}
*/

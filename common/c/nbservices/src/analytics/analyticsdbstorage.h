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

@file     analyticsdbstorage.h

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/



/*!

    @{

*/
#ifndef __ANALYTICS_DB_STORAGE_H__
#define __ANALYTICS_DB_STORAGE_H__

#include "palstdlib.h"
#include "nberror.h"
#include <string>
#include <vector>

struct NB_Context;
struct PAL_Instance;
struct PAL_DBConnect;

namespace nbcommon
{

/*! @struct AnalyticsEvent

\brief  The Event information package. There are four elements inside:event body, event category, event timeStamp, event name
*/
struct AnalyticsEvent
{
    AnalyticsEvent()
    {
    }

    AnalyticsEvent(const AnalyticsEvent& other):
        category(other.category),
        timeStamp(other.timeStamp),
        name(other.name),
        eventBody(other.eventBody)
    {

    }

    ~AnalyticsEvent(){}

    AnalyticsEvent& operator=(const AnalyticsEvent& other)
    {
        if(this != &other)
        {
            this->category = other.category;
            this->eventBody = other.eventBody;
            this->timeStamp = other.timeStamp;
            name = other.name;
        }
        return *this;
    }

    bool operator==(const AnalyticsEvent& other) const
    {
        return (category == other.category) &&
               (timeStamp == other.timeStamp) &&
               (eventBody == other.eventBody) &&
               (name == other.name);
    }

    uint32      category;               /*! The event category*/
    int64       timeStamp;              /*! The event timeStamp*/
    std::string name;                   /*! The event name*/
    std::string eventBody;              /*!The event body*/
};

/*!@class AnalyticsDBStorage

/brief Database storage of analytics events
*/
class AnalyticsDBStorage
{
public:
    AnalyticsDBStorage();
    ~AnalyticsDBStorage();

    /*!Initialize the AnalyticsDBStorage class.

    @param context NB_Context handler
    @param filePath The db storage full path.
    @returns NB_Error Whether initialize is successful
    */
    NB_Error Initialize(NB_Context* context, const std::string& filePath);

    /*!Store event packages at the tail of the persistent media.

    @param eventsQueue The event packages contains in a vector needed to be stored.
    @returns NB_Error Whether storage is successful.
    */
    NB_Error AppendEvents(const std::vector<AnalyticsEvent>& eventsQueue);

    /*!Read event packages out which fit the category in time order.

    @param category The category of events user want to read out
    @param oldFirst To specify the order of read events out
    @param eventsQueue A vector contains event packages. Events read out are pulled at the tail of this vector.
    @param readLimit How many events you want to read out
    @param readCount How many events are actually read out
    @returns NB_Error Whether read is successful
    */
    NB_Error ReadEvents(uint32 category, bool oldFirst, int readLimit, std::vector<AnalyticsEvent>& eventsQueue, int& readCount);

    /*!Delete events fit the AnalyticsEvents in the vector.

    @param eventsQueue Events you want to delete from persistent media.
    @returns NB_Error Whether delete is successful
    */
    NB_Error DeleteEvents(const std::vector<AnalyticsEvent>& eventsQueue);

    /*! Delete events that belongs to category

    @param category The category user want to delete
    @returns NB_Error error code that operation returns
    */
    NB_Error DeleteEvents(uint32 category);

    /*!Delete expire events.

    @param timeStamp The time limit of delete
    @returns NB_Error Whether delete is successful
    */
    NB_Error DeleteExpireEvents(int64 timeStamp);

    /*! Delete all events in database

    @returns NB_Error Error status in deleting
    */
    NB_Error ClearTable();

    /*! The number of one category of events

    @param category The event category
    @returns int the count of this category, returns -1 if something is wrong
    */
    int EventCount(uint32 category);

    /*! Read total length of database contents out

    @returns int, the total length
    */
    int ReadTotalLength();

    /*! Get oldest event timestamp

    @category The event category
    @returns int64, the oldest timestamp in database
    */
    int64 GetOldestEventTimeStamp(uint32 category);
private:
    NB_Context*    m_context;            /*!The NB_Context handler*/
    PAL_Instance*  m_pal;                /*!The PAL_Instance handler*/
    PAL_DBConnect* m_connection;         /*!The PAL_DBConnect handler*/
    bool           m_bInitialized;       /*!Whether this class has been initialized*/
};

}

#endif /* __ANALYTICS_DB_STORAGE_H__ */

/*!

    @}

*/

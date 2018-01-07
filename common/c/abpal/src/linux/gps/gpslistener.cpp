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

    @file     gpslistener.h
    @date     04/03/2012
    @defgroup GPSLISTENER_H Gps listener

    Gps listener
*/
/*
    See file description in header file.

    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "gpslistener.h"
#include "gpscontext.h"
#include "paltaskqueue.h"
#include "palatomic.h"
#include "paltestlog.h"

GpsListener::GpsListener(const ABPAL_GpsCriteria* criteria,
                         ABPAL_GpsLocationCallback* callBack,
                         void * data,
                         uint32 trackID,
                         PAL_Instance* pal,
                         uint32 trackType):
    m_pPal(pal),
    m_callBack(callBack),
    m_data(data),
    m_trackID(trackID),
    m_invalid(0),
    m_pLock(NULL),
    m_tracktype(trackType),
    m_gpsCriteria(*criteria)
{
    PAL_LockCreate(pal, &m_pLock);
}

GpsListener::~GpsListener()
{
    deactivate();
    PAL_LockDestroy(m_pLock);
}

uint32 GpsListener::getTrackID()
{
    return m_trackID;
}

uint32 GpsListener::getTrackType(){
	return m_tracktype;
}

void GpsListener::deactivate()
{
    PAL_AtomicIncrement(&m_invalid);
    PAL_LockLock(m_pLock);
    std::list<ListGpsEntry*>::iterator i = m_list.begin();
    while (i != m_list.end())
    {
        // delete only if task is in the queue
        if (PAL_Ok == PAL_EventTaskQueueRemove(m_pPal, (*i)->taskId, FALSE))
        {
            delete(*i);
        }
        i = m_list.erase(i);
    }
    PAL_LockUnlock(m_pLock);
}

void GpsListener::TaskQueue_GpsCallback(PAL_Instance* pal, void* userData)
{
	GpsListener::ListGpsEntry* latestEntry = NULL;
	GpsListener::ListGpsEntry* entry = static_cast<GpsListener::ListGpsEntry*>(userData);
	GpsListener* gpsListener = entry->pThis;
	int size = 0;
	PAL_LockLock(gpsListener->m_pLock);
	size = gpsListener->m_list.size();
	if (size == 0)
	{
		PAL_LockUnlock(gpsListener->m_pLock);
		delete entry;
		return;
	}
    //only get latest location, ignored other's
	latestEntry = gpsListener->m_list.back();
	gpsListener->m_list.clear();

	PAL_LockUnlock(gpsListener->m_pLock);
    if(size > 1)
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "abpal gps queue blocked size = %d", (int)(gpsListener->m_list.size()));
    }
	if (!gpsListener->m_invalid)
	{
		gpsListener->m_callBack(gpsListener->m_data, latestEntry->error, &latestEntry->location, NULL);
	}
	delete entry;
}

void GpsListener::Notify(PAL_Error error, const ABPAL_GpsLocation& location, bool isGpsFix)
{
    if (m_gpsCriteria.desiredPositionSource != PPS_sourceMostAccurateOnly || isGpsFix)
    {
        if (m_callBack && !m_invalid)
        {
            ListGpsEntry* entry = new ListGpsEntry;
            if (!entry)
            {
                return;
            }
            nsl_memcpy(&entry->location, &location, sizeof(ABPAL_GpsLocation));
            entry->error = error;
            entry->pThis = this;
            PAL_LockLock(m_pLock);
            m_list.push_back(entry);
            PAL_LockUnlock(m_pLock);
            PAL_EventTaskQueueAdd(m_pPal,&GpsListener::TaskQueue_GpsCallback,static_cast<void*>(entry),&entry->taskId);
        }
    }
}

/*! @} */

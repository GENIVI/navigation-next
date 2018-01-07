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

    @file     gpsprovider.cpp
    @date     09/11/2012
    @defgroup GPSPROVIDER_H GPS Provider

    @brief    GPS Provider

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

#include "gpsprovider.h"

GpsProvider::GpsProvider(PAL_Instance* pal):
    m_bEnabled(FALSE),
    m_Period(0),
    m_pLock(NULL),
    m_pList()
{
    PAL_LockCreate(pal, &m_pLock);
}

GpsProvider::~GpsProvider()
{
    PAL_LockLock(m_pLock);
    m_pList.clear();
    PAL_LockUnlock(m_pLock);

    PAL_LockDestroy(m_pLock);
    m_pLock = NULL;
}

void GpsProvider::AddListner(LocationListner* listner)
{
    PAL_LockLock(m_pLock);
    m_pList.push_front(listner);
    PAL_LockUnlock(m_pLock);
}

void GpsProvider::RemoveListner(LocationListner* listner)
{
    PAL_LockLock(m_pLock);
    m_pList.remove(listner);
    PAL_LockUnlock(m_pLock);
}

void GpsProvider::NotifyListeners(PAL_Error error, const ABPAL_GpsLocation& location)
{
    PAL_LockLock(m_pLock);
    std::list<LocationListner*>::iterator listIterator;
    for (listIterator = m_pList.begin(); listIterator != m_pList.end(); ++listIterator)
    {
        (*listIterator)->LocationCallback(error, location);
    }
    PAL_LockUnlock(m_pLock);
}

void GpsProvider::RemoveInstance()
{
    delete this;
}

nb_boolean GpsProvider::IsEnable()
{
    return m_bEnabled;
}

void GpsProvider::setPeriod(uint32 period)
{
    m_Period = period;
}

/*! @} */

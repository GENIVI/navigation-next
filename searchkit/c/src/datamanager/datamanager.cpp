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

    @file datamanager.cpp

    See header file for description.

*/
/*
    (C) Copyright 2014 by TeleCommunication Systems
 
    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#include "datamanager.h"
#include "recentsearchdao.h"
#include "poidao.h"

#ifndef DEBUG
#define DMASSERT(check)
#else
#define DMASSERT(check) nsl_assert(check)
#endif

DataManager::DataManager(shared_ptr<DBManager> manager)
{
    m_dbmanager = manager;
    m_pDAOFactory = DAOFactory::CreateFactory( DAO_FACTORY_TYPE_SQLITE );
}

DataManager::~DataManager()
{
    if ( m_pDAOFactory != NULL )
    {
        delete m_pDAOFactory;
    }
}

/* See header file for description */
IPOIDAO* 
DataManager::GetPOIDAO()
{
    if ( m_pDAOFactory == NULL )
    {
        return NULL;
    }
    return m_pDAOFactory->GetPOIDAO(m_dbmanager);
}

/* See header file for description */
IRecentSearchDAO*
DataManager::GetRecentSearchDAO()
{
    if ( m_pDAOFactory == NULL )
    {
        return NULL;
    }
    return m_pDAOFactory->GetRecentSearchDAO(m_dbmanager);
}

/* See header file for description */
IShortcutDAO*
DataManager::GetShortcutDAO()
{
    if ( m_pDAOFactory == NULL )
    {
        return NULL;
    }
    return m_pDAOFactory->GetShortcutDAO(m_dbmanager);
}

/* See header file for description */
ISuggestListDAO*
DataManager::GetSuggestListDAO()
{
    if ( m_pDAOFactory == NULL )
    {
        return NULL;
    }
    return m_pDAOFactory->GetSuggestListDAO(m_dbmanager);
}

/*! @} */

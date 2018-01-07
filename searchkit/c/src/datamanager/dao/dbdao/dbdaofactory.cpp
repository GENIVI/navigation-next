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

    @file dbdaofactory.cpp
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

#include "dbdaofactory.h"
#include "poidbdao.h"
#include "recentsearchdbdao.h"
#include "shortcutdbdao.h"
#include "suggestlistdbdao.h"


template<>
void Serialize <std::map<std::string, std::string> >(const std::map<std::string, std::string>& map, void*& data, int& size )
{
    size = 0;
    size += sizeof(uint32);
    uint32 count = 0;
    for (std::map<std::string, std::string>::const_iterator iter = map.begin(); iter != map.end(); iter++)
    {
        size+=(nsl_strlen(iter->first.c_str())+1);
        size+=(nsl_strlen(iter->second.c_str())+1);
        count++;
    }
    data = nsl_malloc(size);
    if (data == NULL)
    {
        return;
    }
    nsl_memset(data, 0, size);
    char* ptr = (char*)data;
    *(uint32*)ptr = count;
    ptr += sizeof(uint32);
    for (std::map<std::string, std::string>::const_iterator iter = map.begin(); iter != map.end(); iter++)
    {
        nsl_strcpy(ptr, iter->first.c_str());
        ptr+=(nsl_strlen(iter->first.c_str())+1);
        nsl_strcpy(ptr, iter->second.c_str());
        ptr+=(nsl_strlen(iter->second.c_str())+1);
    }
}

template<>
void Deserialize<std::map<std::string, std::string> >(std::map<std::string, std::string>& map, void* data, int size)
{
    map.clear();
    char* ptr = (char*)data;
    uint32 count = *(uint32*)ptr;
    ptr += sizeof(uint32);
    while (count > 0 && (ptr - (char*)data) < size)
    {
        std::string key = ptr;
        ptr+=(nsl_strlen(key.c_str()) + 1);
        std::string value = ptr;
        ptr+=(nsl_strlen(value.c_str()) + 1);
        map[key] = value;
    }
}

DBDAOFactory::DBDAOFactory()
{
}

DBDAOFactory::~DBDAOFactory()
{
}

/* See header file for description */
IPOIDAO* 
DBDAOFactory::GetPOIDAO(shared_ptr<DBManager> manager)
{
    if ( m_pPOIDAO == NULL )
    {
        m_pPOIDAO = new POIDBDAO(manager);
    }
    return m_pPOIDAO;
}

/* See header file for description */
IRecentSearchDAO*
DBDAOFactory::GetRecentSearchDAO(shared_ptr<DBManager> manager)
{
    if ( m_pRecentSearchDAO == NULL )
    {
        m_pRecentSearchDAO = new RecentSearchDBDAO(manager);
    }
    return m_pRecentSearchDAO;
}

/* See header file for description */
IShortcutDAO*
DBDAOFactory::GetShortcutDAO(shared_ptr<DBManager> manager)
{
    if ( m_pShortcutDAO == NULL )
    {
        m_pShortcutDAO = new ShortcutDBDAO(manager);
    }
    return m_pShortcutDAO;
}

/* See header file for description */
ISuggestListDAO*
DBDAOFactory::GetSuggestListDAO(shared_ptr<DBManager> manager)
{
    if ( m_pSuggestListDAO == NULL )
    {
        m_pSuggestListDAO = new SuggestListDBDAO(manager);
    }
    return m_pSuggestListDAO;
}

/*! @} */

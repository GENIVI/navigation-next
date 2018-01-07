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

    @file recentsearchdao.h
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

#ifndef RECENTSEARCHDAO_H
#define RECENTSEARCHDAO_H

#include <vector>
#include <map>
#include "datamanagerdefs.h"
#include "offboard/SingleSearchOffboardInformation.h"

typedef enum
{
    RSDT_KeyWordText        = 0,
    RSDT_SuggestionData     = 1,
    RSDT_KeyValueMap        = 2
} RecentSearchDataType;

class RecentSearchData
{
public:
    RecentSearchData():m_suggestion(NULL),m_dataType(RSDT_KeyWordText),m_id(0){};
    ~RecentSearchData()
    {
        delete m_suggestion;
        m_suggestion = NULL;
    }
    nbsearch::SuggestionMatchImpl* m_suggestion;
    std::string m_keyWord;
    std::map<std::string, std::string> m_map;//used to save addtional data if type is suggestionData
    RecentSearchDataType m_dataType;
    uint32 m_id;
};

class IRecentSearchDAO
{
public:
    virtual ~IRecentSearchDAO() {}

    // Interfaces ............................................................

    /*! Initialize for the new table 'RecentSearch'

        @return PAL_DB_OK if success
    */
    virtual PAL_DB_Error
    Initialize() PURE;

    /*! Get all 'RecentSearchData' objects in descending order.
        @return PAL_DB_OK if success
    */
    virtual PAL_DB_Error
    GetAllRecentSearches(std::vector<RecentSearchData*>& recentSearches   /*!< User should                                                                             delete all objects of vector when they are useless. */
                    ) PURE;

    /*! Add (do not update) a 'RecentSearchData' object and update the ID of this
        recent search.

        @return PAL_DB_OK if success
    */
    virtual PAL_DB_Error
    AddRecentSearch(RecentSearchData* recentSearch                        /*!< 'RecentSearchData' object to add */
                    ) PURE;

    /*! Remove a 'DMRecentSearch' object by id.

        @return PAL_DB_OK if success
    */
    virtual PAL_DB_Error
    RemoveRecentSearch(int recentSearchId                               /*!< ID of 'DMRecentSearch' object to remove */
                    ) PURE;

    /*! Remove all recent searches.

        @return PAL_DB_OK if success
    */
    virtual PAL_DB_Error
    RemoveAllRecentSearches() PURE;
};

#endif

/*! @} */

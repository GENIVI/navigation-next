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
 @file     SearchHistoryManager.cpp
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "SearchHistoryManager.h"
//#include "SingleSearchInformationImpl.h"
#include "contextbasedsingleton.h"
#include "datamanager.h"
#include "dbmanager.h"

using namespace nbsearch;

inline RecentSearchData* CreateRecentSearchDataFromSearchHistoryData(const SearchHistoryData* data)
{
    RecentSearchData* result = new RecentSearchData();
    switch (data->GetDataType())
    {
        case nbsearch::SHDT_SearchKeyword:
            result->m_dataType = RSDT_KeyWordText;
            result->m_keyWord = data->GetSearchKeyWordData();
            break;
        case nbsearch::SHDT_SuggestionMatch:
            result->m_dataType = RSDT_SuggestionData;
            result->m_suggestion = new SuggestionMatchImpl(*data->GetSuggestionMatchData());
            result->m_map = data->GetKeyValueData();
            break;
        case nbsearch::SHDT_UserData:
            result->m_dataType = RSDT_KeyValueMap;
            result->m_map = data->GetKeyValueData();
            break;
        default:
            break;
    }
    return result;
}

inline SearchHistoryData* CreateSearchHistoryDataFromRecentSearchData(const RecentSearchData* data)
{
    SearchHistoryData* result = NULL;
    switch (data->m_dataType)
    {
        case RSDT_KeyWordText:
            result = new SearchHistoryData(data->m_keyWord);
            break;
        case RSDT_SuggestionData:
            result = new SearchHistoryData(data->m_suggestion, data->m_map);
            break;
        case RSDT_KeyValueMap:
            result = new SearchHistoryData(data->m_map);
            break;
        default:
            break;
    }
    return result;
}

SearchHistoryData::SearchHistoryData(const std::string& keyword):m_searchKeyWord(keyword), m_suggestionMatch(NULL), m_dataType(SHDT_SearchKeyword)
{
}

SearchHistoryData::SearchHistoryData(const SuggestionMatch* suggestion):m_dataType(SHDT_SuggestionMatch)
{
    m_suggestionMatch = new SuggestionMatchImpl(*suggestion);
}

SearchHistoryData::SearchHistoryData(const SuggestionMatch* suggestion, const std::map<std::string, std::string>& keyValueData):m_dataType(SHDT_SuggestionMatch)
{
    m_suggestionMatch = new SuggestionMatchImpl(*suggestion);
    m_userData = keyValueData;
}

SearchHistoryData::SearchHistoryData(const map<std::string, std::string>& userData):m_suggestionMatch(NULL), m_dataType(SHDT_UserData)
{
    m_userData = userData;
}

SearchHistoryData::SearchHistoryData(const SearchHistoryData& copy):
    m_searchKeyWord(copy.m_searchKeyWord),
    m_suggestionMatch(NULL),
    m_dataType(copy.m_dataType)
{
    if (copy.m_suggestionMatch != NULL)
    {
        m_suggestionMatch = new SuggestionMatchImpl(*copy.m_suggestionMatch);
    }
}

SearchHistoryData::~SearchHistoryData()
{
    delete m_suggestionMatch;
    m_suggestionMatch = NULL;
}

SearchHistoryManager::SearchHistoryManager(NB_Context* context):
        m_context(context)
{
    m_dbmanager = ContextBasedSingleton<DBManager>::getInstance(context);
    m_datamanager = shared_ptr<DataManager>(new DataManager(m_dbmanager));
}

SearchHistoryManager::~SearchHistoryManager()
{
    m_context = NULL;
}

void SearchHistoryManager::InitializeDatabase(const std::string &dbFile)
{
    m_dbmanager->SetupDB(DBT_SearchHistory, dbFile.c_str());
    m_datamanager->GetRecentSearchDAO()->Initialize();
}

shared_ptr<SearchHistoryManager> SearchHistoryManager::GetInstance(NB_Context *context)
{
    return ContextBasedSingleton<SearchHistoryManager>::getInstance(context);
}

void SearchHistoryManager::AddSearchHistoryData(const nbsearch::SearchHistoryData *data)
{
    RecentSearchData* rsdata = CreateRecentSearchDataFromSearchHistoryData(data);
    m_datamanager->GetRecentSearchDAO()->AddRecentSearch(rsdata);
    delete rsdata;

    std::vector<RecentSearchData*> result;
    m_datamanager->GetRecentSearchDAO()->GetAllRecentSearches(result);
    m_cachedData.clear();
    for (std::vector<RecentSearchData*>::iterator iter = result.begin() ; iter != result.end(); iter++)
    {
        m_cachedData.push_back(shared_ptr<SearchHistoryData>(CreateSearchHistoryDataFromRecentSearchData(*iter)));
        delete *iter;
    }
}

std::vector<shared_ptr<SearchHistoryData> > SearchHistoryManager::GetAllSearchHistoryData()
{
    if (m_cachedData.size() == 0)
    {
        std::vector<RecentSearchData*> result;
        m_datamanager->GetRecentSearchDAO()->GetAllRecentSearches(result);
        m_cachedData.clear();
        for (std::vector<RecentSearchData*>::iterator iter = result.begin() ; iter != result.end(); iter++)
        {
            m_cachedData.push_back(shared_ptr<SearchHistoryData>(CreateSearchHistoryDataFromRecentSearchData(*iter)));
            delete *iter;
        }
    }
    return m_cachedData;
}

void SearchHistoryManager::RemoveAllSearchHistoryData()
{
    m_cachedData.clear();
    m_datamanager->GetRecentSearchDAO()->RemoveAllRecentSearches();
}

/*! @} */

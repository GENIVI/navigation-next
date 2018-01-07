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
 @file     SearchHistoryManager
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

#ifndef __SearchHistoryManager__
#define __SearchHistoryManager__

#include "nbcontext.h"
#include "smartpointer.h"
#include <vector>
#include <string>
#include <map>

template <typename T> class ContextBasedSingleton;
class DataManager;
class DBManager;
namespace nbsearch
{
class SuggestionMatch;
typedef enum
{
    SHDT_SearchKeyword      = 0,    /*!< search history data is string */
    SHDT_SuggestionMatch    = 1,    /*!< search history data is suggestion match */
    SHDT_UserData           = 2     /*!< search history data is key-value pair*/
} SearchHistoryDataType;    ///search history data type

/*! SearchHistory Data
 *
 *  This class is used to access search history. Can be constructed from search keyword   or suggestion match.
 */
class SearchHistoryData
{
public:
    SearchHistoryData(const SearchHistoryData& copy);
    SearchHistoryData(const std::string& searchKeyword);
    SearchHistoryData(const SuggestionMatch* suggestionMatch);
    SearchHistoryData(const SuggestionMatch* suggestionMatch, const std::map<std::string, std::string>& additionalData);
    SearchHistoryData(const std::map<std::string, std::string>& keyValueData);
    ~SearchHistoryData();

    SearchHistoryDataType GetDataType() const{return m_dataType;}
    const SuggestionMatch* GetSuggestionMatchData() const{return m_suggestionMatch;}
    const std::string GetSearchKeyWordData() const{return m_searchKeyWord;}
    const std::map<std::string, std::string> GetKeyValueData() const{return m_userData;};
private:
    std::string m_searchKeyWord;
    SuggestionMatch* m_suggestionMatch;
    std::map<std::string, std::string> m_userData;
    SearchHistoryDataType m_dataType;
};

/*! Search History Manager
 * 
 *  This class is used to storage user search history inlcuding keyword and suggestion match. There's an internal record count limit, which is 100 record. Once reach the limit, elder record will be removed.
 */
class SearchHistoryManager
{
public:
    /*! Get instance
     *
     *  Get a instance binded with the nbcontext
     *  @param context nbcontext binded with the instance
     *  @return instance of SearchHistoryManager
     */
    static shared_ptr<SearchHistoryManager> GetInstance(NB_Context* context);

    /*! Initialize the database
     *
     *  Initialize the database (create tables if not exist)
     *  @param dbFile database file to load from, create one if not exist
     *  @note user should not share the this db file with other classes
     *  @return None.
     */
    void InitializeDatabase(const std::string& dbFile);

    /*! Add a search history data
     *
     *  @param data search history data to add
     *  @return None.
     */
    void AddSearchHistoryData(const SearchHistoryData* data);

    /*! Get all search history data
     *
     *  @return all search history data
     */
    std::vector<shared_ptr<SearchHistoryData> > GetAllSearchHistoryData();

    /*! Remove all search history data
     *
     *  @return None.
     */
    void RemoveAllSearchHistoryData();

private:
    friend class ContextBasedSingleton<SearchHistoryManager>;
    /// Functions for ContextBasedSingleton
    NB_Context* GetContext(){return m_context;}
    SearchHistoryManager(NB_Context* context);
    ~SearchHistoryManager();

    NB_Context* m_context;
    PAL_Instance* m_palInstance;
    shared_ptr<DataManager> m_datamanager;
    shared_ptr<DBManager> m_dbmanager;
    std::vector<shared_ptr<SearchHistoryData> > m_cachedData;
};
}
#endif /* defined(__SearchHistoryManager__) */
/*! @} */
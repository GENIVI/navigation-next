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
 @file     ShortcutManager.cpp
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

#include "dbmanager.h"
#include "datamanager.h"
#include "contextbasedsingleton.h"
#include "NBProtocolSuggestListSerializer.h"
#include "shortcutdao.h"
#include "offboard/SingleSearchOffboardInformation.h"
#include "ShortcutManagerPrivate.h"
#include "ShortcutManager.h"

namespace nbsearch
{

ShortcutManagerPrivate::ShortcutManagerPrivate(NB_Context* context): m_context(context)
{
    m_dbmanager = ContextBasedSingleton<DBManager>::getInstance(context);
    m_datamanager.reset(new DataManager(m_dbmanager));
}

ShortcutManagerPrivate::~ShortcutManagerPrivate()
{
}

void ShortcutManagerPrivate::InitializeDatabase(const std::string& dbFile)
{
    m_dbmanager->SetupDB(DBT_Shortcut, dbFile.c_str());
    m_datamanager->GetShortcutDAO()->Initialize();
}

bool ShortcutManagerPrivate::UpdateAllShortcutData(const std::vector<SuggestionList*>& allData)
{
    std::vector<ShortcutData> allShortcutData(CreateDBDataVectorBySuggestionListVector<ShortcutData>(allData));
    if (allShortcutData.empty())
        return false;

    PAL_DB_Error err = m_datamanager->GetShortcutDAO()->UpdateAll(allShortcutData);
    return err == PAL_DB_OK ? true : false;
}

bool ShortcutManagerPrivate::RemoveAllShortcutData()
{
    PAL_DB_Error err = m_datamanager->GetShortcutDAO()->RemoveAll();
    return err == PAL_DB_OK ? true : false;
}

std::vector<SuggestionList*> ShortcutManager::GetAllDefaultData()
{
    SuggestListBuilder shortcutList;
    shortcutList.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("ACC", "category", "Gas Stations", "XCF").Protocol());
    shortcutList.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("MM", "category", "Movies", "movie").Protocol());
    shortcutList.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AEE", "category", "Coffee", "XEABM").Protocol());
    shortcutList.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AE", "category", "Restaurants & Bars", "XE").Protocol());
    shortcutList.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AAA", "category", "ATMs", "XAA").Protocol());

    std::vector<SuggestionList*> suggestionLists;
    suggestionLists.push_back(new SuggestionListImpl(shortcutList.Protocol()));

    return suggestionLists;
}

std::vector<SuggestionList*> ShortcutManagerPrivate::GetAllShortcutData() const
{
    std::vector<ShortcutData> allShortcutData;
    PAL_DB_Error err = m_datamanager->GetShortcutDAO()->GetAll(allShortcutData);
    (void)err;

    std::vector<SuggestionList*> allData(CreateSuggestionListVectorByDBDataVector(allShortcutData));
    return allData;
}

ShortcutManager::ShortcutManager(NB_Context* context)
{
    m_private.reset(new ShortcutManagerPrivate(context));
}

ShortcutManager::~ShortcutManager()
{
}

NB_Context* ShortcutManager::GetContext() const
{
    return m_private->GetContext();
}

shared_ptr<ShortcutManager> ShortcutManager::GetInstance(NB_Context* context)
{
    return ContextBasedSingleton<ShortcutManager>::getInstance(context);
}

void ShortcutManager::InitializeDatabase(const std::string& dbFile)
{
    m_private->InitializeDatabase(dbFile);
}

bool ShortcutManager::UpdateAllShortcutData(const std::vector<SuggestionList*>& allData)
{
    return m_private->UpdateAllShortcutData(allData);
}

bool ShortcutManager::RemoveAllShortcutData()
{
    return m_private->RemoveAllShortcutData();
}

std::vector<SuggestionList*> ShortcutManager::GetAllShortcutData() const
{
    return m_private->GetAllShortcutData();
}

SearchFilter* ShortcutManager::SerializeSearchFilter(SearchFilter* filter)
{
    protocol::ResultStyleSharedPtr resultStyle(new protocol::ResultStyle());
    resultStyle->SetKey(CHAR_PTR_TO_UTF_STRING_PTR(filter->GetResultStyle()));

    protocol::SearchFilterSharedPtr filterPtr(new protocol::SearchFilter());
    filterPtr->SetResultStyle(resultStyle);

    std::vector<StringPair> pairs = filter->GetPairs();
    std::vector<StringPair>::const_iterator pair = pairs.begin();
    for (; pair != pairs.end(); pair++)
    {
        protocol::PairSharedPtr pairPtr(new protocol::Pair());
        pairPtr->SetKey(CHAR_PTR_TO_UTF_STRING_PTR((*pair).GetKey()));
        pairPtr->SetValue(CHAR_PTR_TO_UTF_STRING_PTR((*pair).GetValue()));
        filterPtr->GetSearchKeyArray()->push_back(pairPtr);
    }

    return new SearchFilter(SearchFilterUtil::toSearchFilter(filterPtr));
}

}
/*! @} */

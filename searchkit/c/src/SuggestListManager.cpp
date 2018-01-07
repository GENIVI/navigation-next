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
 @file     SuggestListManager.cpp
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
#include "suggestlistdao.h"
#include "offboard/SingleSearchOffboardInformation.h"
#include "SuggestListManager.h"
#include "SuggestListManagerPrivate.h"

namespace nbsearch
{

SuggestListManagerPrivate::SuggestListManagerPrivate(NB_Context* context): m_context(context)
{
    m_dbmanager = ContextBasedSingleton<DBManager>::getInstance(context);
    m_datamanager.reset(new DataManager(m_dbmanager));
}

SuggestListManagerPrivate::~SuggestListManagerPrivate()
{
}

void SuggestListManagerPrivate::InitializeDatabase(const std::string& dbFile)
{
    m_dbmanager->SetupDB(DBT_SuggestList, dbFile.c_str());
    m_datamanager->GetSuggestListDAO()->Initialize();
}

bool SuggestListManagerPrivate::UpdateAllSuggestListData(const std::vector<SuggestionList*>& allData)
{
    std::vector<SuggestListData> allSuggestListData(CreateDBDataVectorBySuggestionListVector<SuggestListData>(allData));
    if (allSuggestListData.empty())
        return false;

    PAL_DB_Error err = m_datamanager->GetSuggestListDAO()->UpdateAll(allSuggestListData);
    return err == PAL_DB_OK ? true : false;
}

bool SuggestListManagerPrivate::RemoveAllSuggestListData()
{
    PAL_DB_Error err = m_datamanager->GetSuggestListDAO()->RemoveAll();
    return err == PAL_DB_OK ? true : false;
}

std::vector<SuggestionList*> SuggestListManagerPrivate::GetAllSuggestListData() const
{
    std::vector<SuggestListData> allSuggestListData;
    PAL_DB_Error err = m_datamanager->GetSuggestListDAO()->GetAll(allSuggestListData);
    (void)err;

    std::vector<SuggestionList*> allData(CreateSuggestionListVectorByDBDataVector(allSuggestListData));
    return allData;
}

std::vector<SuggestionList*> SuggestListManager::GetAllDefaultData()
{
    SuggestListBuilder essentials;
    essentials.SetName("Essentials");
    essentials.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AHC", "category", "Hotels & Motels", "XHC,XHA,XHF").Protocol());
    essentials.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("ACC", "category", "Gas Stations", "XCF").Protocol());
    essentials.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("ALA", "category", "Airports", "XLB").Protocol());
    essentials.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("ACJ", "category", "Car Wash", "XCE").Protocol());
    essentials.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("ANC", "category", "Pharmacy", "XKK").Protocol());
    essentials.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AFD", "category", "Post Office", "XDC").Protocol());
    essentials.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AKD", "category", "Grocery Stores", "XKH").Protocol());
    essentials.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AAA", "category", "ATMs", "XAA").Protocol());
    essentials.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("ANB", "category", "Hospitals", "XGE").Protocol());

    SuggestListBuilder foodandmore;
    foodandmore.SetName("Food & More");
    foodandmore.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AEK", "category", "Pizza", "XEACO").Protocol());
    foodandmore.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AEE", "category", "Coffee Shops", "XEABM").Protocol());
    foodandmore.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AE", "category", "Restaurants & Bars", "XE").Protocol());
    foodandmore.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AEF", "category", "Fast Food", "XEAAJ").Protocol());
    foodandmore.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AEOC", "category", "Sushi", "XEACD").Protocol());

    SuggestListBuilder thingstodo;
    thingstodo.SetName("Things to Do");
    thingstodo.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AKH", "category", "Malls", "XKS").Protocol());
    thingstodo.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AK", "category", "Shopping", "XKD,XKS,XKT").Protocol());
    thingstodo.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("ABP", "category", "Parks", "XBL").Protocol());
    thingstodo.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("ABB", "category", "Casinos", "XBD").Protocol());
    thingstodo.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("AIU", "category", "Churches", "XJA").Protocol());
    thingstodo.AddSuggestMatch(SuggestMatchBuilder::CreateByMatch("ABD", "category", "Golf Courses", "XBG").Protocol());

    SuggestListBuilder popluarbrands;
    popluarbrands.SetName("Popluar Brands");
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0001", "category", "Walmart", "Walmart", "XKU,XKT,XKK,XKL,XKR,XKD,XKF,XKH,XKI").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0002", "category", "Starbucks", "Starbucks", "XEACA,XEAAA,XEAAJ,XE").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0003", "category", "McDonald's", "McDonald's::Mc Donald's", "XEACA,XEAAA,XEAAJ,XE").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0004", "category", "The Home Depot", "The Home Depot", "XKR,XKI").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0005", "category", "Target", "Target", "XKU,XKT,XKL,XKD,XKH").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0007", "category", "Bank of America", "Bank of America::Bank Of America", "XAB,XAA").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0006", "category", "Walgreens", "Walgreens", "XKK").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0011", "category", "Lowe's", "Lowe's::Lowe's Home Improvement", "XKR,XKG,XKI").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0015", "category", "Marriott", "Marriott", "XHC,XEABI").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0016", "category", "Costco", "Costco", "XECAC,XKZ,XKX,XKV,XKU,XKT,XEABC,XCF,XCH,XKK,XKL,XKC,XKD,XKG,XKH").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("poi", "category", "Kohl's", "Kohl's", "XKL,XKD,XKV,XKT").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("poi", "category", "Verizon", "Verizon", "XKL").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("poi", "category", "UPS Store", "UPS Store::Ups Store", "XDA,XKX,XDB,XDC,XKL,XD-UPS Store").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("poi", "category", "FedEx", "FedEx::Fedex Office Print & Ship Ctr", "XKL,XD,XDA,XDB,XKX").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0038", "category", "Barnes & Noble", "Barnes & Noble::Barnes & Noble Booksellers", "XKB").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0010", "category", "Burger King", "Burger King::BURGER KING", "XEACA,XEAAA,XEAAJ,XE").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0008", "category", "Holiday Inn", "Holiday Inn", "XEABI,XHC").Protocol());
    popluarbrands.AddSuggestMatch(SuggestMatchBuilder::CreateByBrand("B0012", "category", "Subway", "Subway::SUBWAY", "XEAAJ,XEABC,XE").Protocol());

    std::vector<SuggestionList*> suggestionLists;
    suggestionLists.push_back(new SuggestionListImpl(essentials.Protocol()));
    suggestionLists.push_back(new SuggestionListImpl(foodandmore.Protocol()));
    suggestionLists.push_back(new SuggestionListImpl(thingstodo.Protocol()));
    suggestionLists.push_back(new SuggestionListImpl(popluarbrands.Protocol()));

    return suggestionLists;
}

SuggestListManager::SuggestListManager(NB_Context* context)
{
    m_private.reset(new SuggestListManagerPrivate(context));
}

SuggestListManager::~SuggestListManager()
{
}

NB_Context* SuggestListManager::GetContext() const
{
    return m_private->GetContext();
}

shared_ptr<SuggestListManager> SuggestListManager::GetInstance(NB_Context* context)
{
    return ContextBasedSingleton<SuggestListManager>::getInstance(context);
}

void SuggestListManager::InitializeDatabase(const std::string& dbFile)
{
    m_private->InitializeDatabase(dbFile);
}

bool SuggestListManager::UpdateAllSuggestListData(const std::vector<SuggestionList*>& allData)
{
    return m_private->UpdateAllSuggestListData(allData);
}

bool SuggestListManager::RemoveAllSuggestListData()
{
    return m_private->RemoveAllSuggestListData();
}

std::vector<SuggestionList*> SuggestListManager::GetAllSuggestListData() const
{
    return m_private->GetAllSuggestListData();
}

}
/*! @} */

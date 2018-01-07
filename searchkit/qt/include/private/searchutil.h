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

/*
 * (C) Copyright 2015 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

/*! @{ */


#ifndef SEARCHTRANSLATOR_H
#define SEARCHTRANSLATOR_H

#include "SingleSearchInformation.h"
#include "searchdatatypes.h"
#include "ReverseGeocodeInformation.h"

namespace locationtoolkit
{

class ResultStateImpl : public nbsearch::ResultState
{
public:
    ResultStateImpl() {}
    ~ResultStateImpl() {}

    virtual const std::string& GetSerialized() const {return mSerialized;}

    std::string mSerialized;
};

class SingelSearchInformationImpl : public nbsearch::SingleSearchInformation
{
public:
    SingelSearchInformationImpl(const SearchInformation& information);
    ~SingelSearchInformationImpl();

    virtual bool HasMore() const {return false;}

    virtual int GetResultCount() const {return 0;}
    virtual nbsearch::SingleSearchInformationResultType GetResultType() const {return nbsearch::SSIRT_None;}

    virtual const nbsearch::SearchResultBase* GetResultAtIndex(uint32 index) const {return NULL;}
    virtual const nbsearch::POI* GetPOIAtIndex(uint32 index) const {return NULL;}
    virtual const nbsearch::SuggestionMatch* GetSuggestionMatchAtIndex(uint32 index) const {return NULL;}
    virtual const nbsearch::FuelSummary* GetFuelSummary() const {return NULL;}
    virtual const std::vector<nbsearch::ResultDescription*>& GetResultDescriptions() const {return mResultDescription;}
    virtual const nbsearch::SuggestionList* GetSuggestionListAtIndex(uint32 index) const {return NULL;}

    virtual const nbsearch::ResultState* GetResultState() const {return mResultState;}
    virtual const nbsearch::ProxMatchContent* GetProxMatchContentAtIndex(uint32 index) const {return NULL;}

    ResultStateImpl* mResultState;
    std::vector<nbsearch::ResultDescription*> mResultDescription;
};

class SearchTranslator
{
public:
    static SearchInformation* toSearchInformation(const nbsearch::SingleSearchInformation& information);
    static nbsearch::SearchFilter* toSearchFilter(const SearchFilter& filter);
    static nbsearch::StringPair toStringPair(const SearchStringPair& pair);
    static nbsearch::Location* toLocation(const SearchLocation& location);
    static SearchInformation* toSearchInformation(const nbcommon::ReverseGeocodeInformation& information);
};

}

#endif // SEARCHTRANSLATOR_H

/*! @} */

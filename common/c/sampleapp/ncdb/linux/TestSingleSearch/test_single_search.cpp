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
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
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

#include <assert.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <algorithm>
#include <map>

#include "UtfString.h"
//#include "metrics.h"

#include "Session.h"
#include "DataBaseProperties.h"
#include "WorldPoint.h"
#include "test_single_search.h"
#include "SingleSearch.h"

using namespace std;
using namespace Ncdb;

static void GetLangCodeFromISO(const string& iso, SupportedLanguages& langApi)
{
    static bool bIsInitialized = false;
    static std::map<string, SupportedLanguages> gs_ISOLangMap;
    langApi = Unknown_Langauge;
    if ( false == bIsInitialized )
    {
        cout << "Note: This should be called once !!!" << endl;
        bIsInitialized = true;
        gs_ISOLangMap["ENG"] = US_English;
        gs_ISOLangMap["BAQ"] = Basque;
        gs_ISOLangMap["DUT"] = Belgian_Dutch;
        gs_ISOLangMap["POR"] = Brazilian_Portuguese;
        gs_ISOLangMap["FRE"] = Canadian_French;
        gs_ISOLangMap["CAT"] = Catalan;
        gs_ISOLangMap["CZE"] = Czech;
        gs_ISOLangMap["DAN"] = Danish;
        gs_ISOLangMap["FIN"] = Finnish;
        gs_ISOLangMap["GER"] = German;
        gs_ISOLangMap["GRE"] = Greek;
        gs_ISOLangMap["ITA"] = Italian;
        gs_ISOLangMap["SPA"] = Mexican_Spanish;
        gs_ISOLangMap["NOR"] = Norwegian;
        gs_ISOLangMap["POL"] = Polish;
        gs_ISOLangMap["RUS"] = Russian;
        gs_ISOLangMap["SWE"] = Swedish;
        gs_ISOLangMap["TUR"] = Turkish;
        gs_ISOLangMap["CHI"] = Chinese;
        gs_ISOLangMap["WEL"] = Welsh;
        gs_ISOLangMap["ARA"] = Arabic;
        gs_ISOLangMap["MAY"] = Malay;
        gs_ISOLangMap["IND"] = Indonesian;
        gs_ISOLangMap["THA"] = Thai;
        gs_ISOLangMap["VIE"] = Vietnamese;
    }
    std::map<string, SupportedLanguages>::iterator iter = gs_ISOLangMap.find(iso);
    if ( gs_ISOLangMap.end() != iter )
        langApi = iter->second;
    return;
}

static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}
bool Replace(std::string& str,  std::string findStr,  std::string replacedStr)
{
    size_t pos = 0;
    bool ret = false;
    while((pos = str.find(findStr, pos)) != std::string::npos)
    {
        str.replace(pos, findStr.length(), replacedStr);
        pos += replacedStr.length();
        ret = true;
    }
    return ret;
}
void StringsFromFlatfile(char* filename, vector<string>& strings)
{
    string currentline;
    ifstream inputfile(filename);
    assert(inputfile.is_open());
    size_t comment_pos, current_pos = 0, found = 0;
    bool isEscapedFound = false;
    while (!inputfile.eof())
    {
        getline(inputfile, currentline);
        comment_pos = currentline.find_first_of("#"); // find comments
        if (comment_pos == 0)
        {
            continue;
        }
        else if(comment_pos == std::string::npos)
        {
            strings.push_back(currentline);     
        }
        else
        {
            std::string toReplace;
            toReplace.assign(1,  ESCAPECHAR);
            toReplace += "#";
            do 
            {
                found = currentline.find("#", current_pos);
                if(std::string::npos != found)
                {
                    if(currentline[found-1] != ESCAPECHAR)
                    {
                        std::string tmp = currentline.substr(0, found);
                        Replace(tmp, toReplace, "#");
                        strings.push_back(tmp);
                        isEscapedFound = false;
                        break;
                    }
                    else
                    {
                        isEscapedFound = true;
                        current_pos = found+1;
                    }
                }
                else 
                    break;
            }while(1);
            if(isEscapedFound && Replace(currentline, toReplace, "#"))
            {
                strings.push_back(currentline); 
            }   
        }
    }
    strings.pop_back(); // get rid of the eof
    inputfile.close();
}


size_t GetNextAddressPart(size_t current_pos, string& part, const string& full)
{
    size_t found; 
    size_t request_pos = current_pos;
    bool isEscaped = false;
    do
    {
        found = full.find(DLMTR, current_pos);

        if(std::string::npos != found)
        {
            if(found != 0 && full[found-1] != ESCAPECHAR)
            {
                part = full.substr(request_pos, found - request_pos);
                if(isEscaped)
                {
                    std::string toReplace;
                    toReplace.assign(1,  ESCAPECHAR);
                    toReplace += DLMTR;
                    Replace(part, toReplace, DLMTR);
                }
                break;
            }
            else if(found != 0)
            {
                current_pos = found+1;
                isEscaped = true;
            }
            else
                break;
        }
        else
        {
            part = full.substr(current_pos, found - current_pos);
            break;
        }
    }while(1);
    return found;
}


string GetUtfStringBufferChars(UtfString& s)
{
    char* t;
    t = s.GetBuffer();
    string x = (t != 0) ? t : "";
    return x;
}

void CreateTestCaseFromInputString(string& input, Ncdb::SearchRequest& req)
{
    size_t found;
    string srchTypeStr, srchStr;
    string lat, lon;
    string radiusStr, numStr, pageStr;
    string langTemp("");

    lat="";
    lon="";

	//req.ReqType = Ncdb::POI_SEARCH;
	//req.SearchString = "starbucks";
	//req.SearchLocation.Set(-122.262726f, 37.809916f);
	//req.Radius = 10.0f;
	req.NumResults = 25; 
	req.PageID = 1;
    req.PoiSearchCriteria.searchType = 0;
    req.PoiSearchCriteria.typeToSearch.typeId = -1;
    req.PoiSearchCriteria.typeToSearch.subTypeId = -1;
    req.PoiSearchCriteria.phoneToSearch = "";
    req.PoiSearchCriteria.nameToSearch = "";
    req.PoiSearchCriteria.SearchResultId = "";
    req.WantFlags = g_WantFlags;

    found = 0;
    found = GetNextAddressPart(found, srchTypeStr, input);
	if(!srchTypeStr.empty())
	{
		int srchType = atoi(srchTypeStr.c_str());
		if(srchType == 0)
		  req.ReqType = Ncdb::SIMPLE_SUGGEST;
		else if(srchType == 1)
		  req.ReqType = Ncdb::SIMPLE_SEARCH;
		else if(srchType == 2)
		  req.ReqType = Ncdb::GEOCODE_SEARCH;
		else if (srchType == 3) 
		  req.ReqType = Ncdb::POI_SEARCH;
		else if (srchType == 7)
		  req.ReqType = Ncdb::GEOCODE_SUGGEST;
		else if(srchType == 8)
		  req.ReqType = Ncdb::POI_SUGGEST;
		else
		  req.ReqType = Ncdb::SIMPLE_SEARCH;
	}
//cout << "KV: " << req.ReqType << endl;	
    found = GetNextAddressPart(found + 1, srchStr, input);
	if(!srchStr.empty())
	{
		req.SearchString = srchStr.c_str();
	}

	found = GetNextAddressPart(found + 1, lat, input);
    if(found)
    {
    	found = GetNextAddressPart(found + 1, lon, input);
    }
    if(lat.empty() || lon.empty())
    {
        lat = "-100";
        lon = "-200";
    }

Ncdb::WorldPoint loc;
    loc.y = atof(UtfString(lat.c_str()));
    loc.x = atof(UtfString(lon.c_str()));
    
	req.SearchLocation.Set(loc.x, loc.y);
    if(found==string::npos)
        return;
        found = GetNextAddressPart(found + 1, radiusStr, input);
        if(!radiusStr.empty())
        {
	  //req.Radius = atoi(radiusStr.c_str());
        }

    if (Ncdb::POI_SEARCH == req.ReqType)
    {
        string phone, maintype, subtype;
        if (!req.SearchString.IsEmpty())
        {
            req.PoiSearchCriteria.searchType |= NAME_SEARCH;
            req.PoiSearchCriteria.nameToSearch = req.SearchString;
        }
    if(found==string::npos)
        return;
        found = GetNextAddressPart(found + 1, numStr, input);
        if(!numStr.empty())
        {
		req.NumResults = atoi(numStr.c_str());
        }

    if(found==string::npos)
        return;
        found = GetNextAddressPart(found + 1, pageStr, input);
        if(!pageStr.empty())
        {
                req.PageID = atoi(pageStr.c_str());
        }

    if(found==string::npos)
        return;
        found = GetNextAddressPart(found + 1, phone, input);
        if(!phone.empty())
        {
            req.PoiSearchCriteria.phoneToSearch = phone.c_str();
            req.PoiSearchCriteria.searchType |= PHONE_SEARCH;
        }

    if(found==string::npos)
        return;
        found = GetNextAddressPart(found + 1, maintype, input);
        req.PoiSearchCriteria.typeToSearch.typeId = 0;
        if(!maintype.empty())
        {
            req.PoiSearchCriteria.typeToSearch.typeId = atoi(maintype.c_str());
            req.PoiSearchCriteria.searchType |= TYPE_SEARCH;
        }

    if(found==string::npos)
        return;
        found = GetNextAddressPart(found + 1, subtype, input);
        req.PoiSearchCriteria.typeToSearch.subTypeId = 0;
        if(!subtype.empty())
        {
            req.PoiSearchCriteria.typeToSearch.subTypeId = atoi(subtype.c_str());
        }

    if(found==string::npos)
        return;
    string result_id("");
    found = GetNextAddressPart(found + 1, result_id, input);
    if(!result_id.empty())
    {
        std::replace( result_id.begin(), result_id.end(), ';', '|');
        req.PoiSearchCriteria.SearchResultId = result_id.c_str();
    }
    }// End if (POI)

	return;
}

void GetStreetParts(StreetComponents& StreetParts, string& strParts)
{
    strParts = " (" + GetUtfStringBufferChars(StreetParts.langAbbr) + ") => ";
    strParts = strParts + "(";
    strParts = strParts + GetUtfStringBufferChars(StreetParts.prefix) + "|";
    strParts = strParts + GetUtfStringBufferChars(StreetParts.typeBefore) + "|";
    strParts = strParts + GetUtfStringBufferChars(StreetParts.basename) + "|";
    strParts = strParts + GetUtfStringBufferChars(StreetParts.type) + "|";
    strParts = strParts + GetUtfStringBufferChars(StreetParts.suffix) + ") ";
}

string PrintExonyms(AutoArray<ExonymInfo> &arr)
{
	stringstream exonyms;
    exonyms<< "{ ";
	for(int i = 0; i < arr.size(); i++)
	{
		if(i!=0)
			exonyms << " | ";
		exonyms << GetUtfStringBufferChars(arr[i].name);
	}
	exonyms << " }";
	return exonyms.str();
}

void PrintAltNames(OutAddress &addr)
{
    if(addr.StreetList.size() > 1)
    {
        string output = "Alternate Names = ";
        for(int i = 1; i < addr.StreetList.size(); i++)
        {
            output = output + "{ " + addr.StreetList[i].fullName.GetBuffer();
            string streetParts;
            GetStreetParts(addr.StreetList[i], streetParts);
            output = output + streetParts;
            output = output + "}; ";
        }
        cout << "\t\t" << output << endl;
    }
    if(addr.CrossStreetList.size() > 1)
    {
        string output = "Alternate Cross Names = ";
        for(int i = 1; i < addr.CrossStreetList.size(); i++)
        {
            output = output + "{ " + addr.CrossStreetList[i].fullName.GetBuffer();
            string streetParts;
            GetStreetParts(addr.CrossStreetList[i], streetParts);
            output = output + streetParts;
            output = output + "}; ";
        }
        cout << "\t\t" << output << endl;
    }
	
	if(addr.NamedAreaExonymList.size() > 1)
    {
        string output = "NamedArea Exonyms = ";
		output = output + PrintExonyms(addr.NamedAreaExonymList);
        cout << "\t\t" << output << endl;
    }

	if(addr.StateExonymList.size() > 1)
    {
        string output = "State Exonyms = ";
		output = output + PrintExonyms(addr.StateExonymList);
        cout << "\t\t" << output << endl;
    }

	if(addr.CityExonymList.size() > 1)
    {
        string output = "City Exonyms = ";
		output = output + PrintExonyms(addr.CityExonymList);
        cout << "\t\t" << output << endl;
    }
	
	if(addr.ZoneExonymList.size() > 1)
    {
        string output = "Zone Exonyms = ";
		output = output + PrintExonyms(addr.ZoneExonymList);
        cout << "\t\t" << output << endl;
    }	

}

void DumpGeocodeOutput(SearchMatch& match)
{
        string res, num, streetname, crossstreet, namedarea, city, zone, county, state, zip, country;
        //string streetParts;

        num = GetUtfStringBufferChars(match.AddressOut.StartNumber);
        if(!match.AddressOut.StreetName.IsEmpty())
        {
            streetname = match.AddressOut.StreetName.GetBuffer();
            /*if(g_bVerbose)
            {
                GetStreetParts(match.AddressOut.StreetList[0], streetParts);
                streetname = streetname + streetParts;
            }*/
        }

        if(!match.AddressOut.City.IsEmpty())
            city = match.AddressOut.City.GetBuffer();
        if(!match.AddressOut.StateFullName.IsEmpty())
            state = match.AddressOut.StateFullName.GetBuffer();
        if( match.AddressOut.Postal.GetBuffer() )
            zip = match.AddressOut.Postal.GetBuffer();

        country = GetUtfStringBufferChars(match.AddressOut.Country);

        res = num + SPACE + streetname + COMMA_DLMTR + city + COMMA_DLMTR + state + COMMA_DLMTR + zip + COMMA_DLMTR + country;

        namedarea = GetUtfStringBufferChars(match.AddressOut.Subcity1);
        zone = GetUtfStringBufferChars(match.AddressOut.Subcity2);
        county = GetUtfStringBufferChars(match.AddressOut.County);
        if(!match.AddressOut.CrossStreet.IsEmpty())
        {
            crossstreet = match.AddressOut.CrossStreet.GetBuffer();
            /*if(g_bVerbose)
            {
                GetStreetParts(match.AddressOut.CrossStreetList[0], streetParts);
                crossstreet = crossstreet + streetParts;
            }*/
        }

        res = res + " | " + namedarea + COMMA_DLMTR + zone + COMMA_DLMTR + county;
        res = res + " | " + crossstreet;
        if(!match.AddressOut.AirportCode.IsEmpty())
        {
            res = res + " | AirportCode = " + match.AddressOut.AirportCode.GetBuffer();
            res = res + " | AirportName = " + match.AddressOut.AirportName.GetBuffer();
        }
        /*outstring << "<OPTION><ADDRESS><HNO>" << num << "</HNO>" \
                  << "<STREET>" << streetname << "</STREET>" \
                  << "<XSTREET>" << crossstreet << "</XSTREET>" \
                  << "<SUBCITY1>" << namedarea << "</SUBCITY1>" \
                  << "<SUBCITY2>" << zone << "</SUBCITY2>"           \
                  << "<CITY>" << city << "</CITY>" \
                  << "<COUNTY>" << county << "</COUNTY>" \
                  << "<STATE>" << state << "</STATE>" \
                  << "<POSTAL>" << zip << "</POSTAL>" \
                  << "<COUNTRY>" << country << "</COUNTRY>" \
                  << "</ADDRESS>";
        outstring.precision(9);*/

        cout.precision(9);

        cout << "\t" << res ;
        cout  << " | Distance= (" << match.MatchDistance << ")"               \
             << " | GL= (" << match.GeocodedLocation.y << ", " << match.GeocodedLocation.x << ")" \
             << " | DL= (" << match.DisplayLocation.y << ", " << match.DisplayLocation.x << ")" \
             << " | MATCHTYPE=(" << match.MatchType << ")" \
             << " | CONFIDENCE : "<< match.Confidence << " | SCORE: " << match.Score \
	     << " | BBOX= (TL=" << match.BBox.topLeft.y << ", " << match.BBox.topLeft.x \
	     << " : BR=" << match.BBox.bottomRight.y << ", " << match.BBox.bottomRight.x << ")" <<endl;

        /*outstring << "<IP>" << match.InterpolatedLocation.x << "," << match.InterpolatedLocation.y << "</IP>" \
                  << "<GP>" << match.GeocodedLocation.x << "," << match.GeocodedLocation.y << "</GP>" \
                  << "<DP>" << match.DisplayLocation.x << "," << match.DisplayLocation.y << "</DP>" \
                  << "<DISTANCE>" << match.Distance << "</DISTANCE>" \
                  << "<SCORE>" << match.Confidence << "</SCORE>" \
                  << "<COMPSCORE>" << match.Score << "</COMPSCORE>" \
                  << "<SRCID>" << match.addrSegmentId.GetSrcId() << "</SRCID>";
        
        outstring << "</OPTION>";

        if(g_bVerbose)
        {
            std::stringstream strSrcId;
            strSrcId << match.addrSegmentId.GetSrcId();
            cout << "\t\tSRCID=" << strSrcId.str() << endl;
            PrintAltNames(match.AddressOut);
        }*/
		if(!match.AddressOut.FormattedAddress.IsEmpty())
            cout << "\t\tFORMATTED_ADDRESS :" << match.AddressOut.FormattedAddress.GetBuffer() << endl;
        int compactLineCount = match.AddressOut.compactFormattedAddress.size();
        if (compactLineCount)
        {
            cout << "\t\tCOMPACT_ADDRESS : " << endl;
            for(int i = 0; i < compactLineCount; ++i)
                cout << "\t\t\t" << match.AddressOut.compactFormattedAddress[i].GetBuffer() << endl;
        }
        int extendedLineCount = match.AddressOut.extendedFormattedAddress.size();
        if (extendedLineCount)
        {
            cout << "\t\tEXTENDED_ADDRESS : " << endl;
            for(int i = 0; i < extendedLineCount; ++i)
                cout << "\t\t\t" << match.AddressOut.extendedFormattedAddress[i].GetBuffer() << endl;
        }
}

void DumpSearchOutput(std::basic_ostringstream<char>& stm, SearchMatch * match)
{
    if (PoiMatch_Name != match->MatchType)
    {
        DumpGeocodeOutput(*match);
        return;
    }

    stm << "\tPOI_NAME: [ " << match->MatchName.GetBuffer() << " ] | ";
    stm << "PHONE: [ ";
    AutoArray<UtfString>& phones = match->TelephoneNumbers;
    for (size_t i = 0; i < phones.size(); ++i)
    {
        stm << "(" << phones[i].GetBuffer() << ") ";
    }
    stm << "] | ";
    AutoArray<TYPE_INFO>& ti = match->TypeInfos;
    stm << "TYPE_INFO: [ ";
    for (size_t i = 0; i < ti.size(); i++)
    {
        stm << "(" << ti[i].typeId << ":" << ti[i].subTypeId << ") ";
    }
    stm << "]";
    // TODO 
    //AutoArray<TYPE_DATA>& td = match->TypeDatas;
    std::cout << stm.str() << endl;
    DumpGeocodeOutput(*match);
}

void SingleSearchStringsOfFlatfile(const char* inipath, char* filename)
{
    cout << "Initializing test ..." << endl;
    cout << "  Reading strings from flatfile...";

    vector<string> strings;
    StringsFromFlatfile(filename, strings);

    int numstrs = strings.size();
    cout << "  " << numstrs << " strings found.\n" << endl;

    memUsageLogger->DumpMemUsage("TestGeocoding: Before Session Object Memory: ");
    // Required for initializing UTF functionality
    Session session;
    session.Open(inipath);
    memUsageLogger->DumpMemUsage("TestGeocoding: Session Object Memory: ");

    assert(session.IsOpened());

    SimpleSearch simplesearch(session);
    memUsageLogger->DumpMemUsage("TestGeocoding: SimpleSearch Object Memory: ");
    
    // Test GetInterestList()
    if (true == g_bSuggestList)
    {
        AutoArray<AutoSharedPtr<SuggestList> > poiTypeList;
        //GLOBAL_TIMER.start();
        int retCode = simplesearch.GetInterestList(poiTypeList);
        //GLOBAL_TIMER.stop();
        for (int i = 0; i < poiTypeList.size(); ++i)
        {
            cout << "SuggestList: " << poiTypeList[i]->SuggestListName.GetBuffer() << endl;
            for (int j = 0; j < poiTypeList[i]->SuggestMatchArr.size(); ++j)
            {
                cout << "\tSuggestMatch: " << poiTypeList[i]->SuggestMatchArr[j].SuggestMatchName.GetBuffer() << endl;
            }
        }
        return;
    }


    Ncdb::AutoArray<Ncdb::AutoSharedPtr<Ncdb::SearchMatch> > geoPoiMatch;
    SupportedLanguages apiLangCode = Invalid_Language;
    GetLangCodeFromISO(g_sLangCode, apiLangCode);
    cout << "Global Language is " << g_sLangCode << " = " << apiLangCode << endl;

    for (int i = 0; i < numstrs; i++)
    {
	Ncdb::SearchRequest req;
	
        CreateTestCaseFromInputString(strings[i], req);
        req.Country = g_sCountry.c_str(); 
        req.InLanguageCode = apiLangCode;
		
        double exec_time = 0;
        std::stringstream outstring;
        outstring.str("");

        //GLOBAL_TIMER.start();
        int rc = simplesearch.Search(req, geoPoiMatch);
    //GLOBAL_TIMER.stop();

        cout << "\nINPUT: " << strings[i] << "\n";
        outstring << "<CASE_AND_RESULTS>";
        outstring << "<CASE>" << strings[i]<<"</CASE>";
        outstring << "<OPTIONS>";

        //exec_time += GLOBAL_TIMER.last_time_ms();
//        if (rc == Ncdb::NCDB_MORE_RECORDS || rc == Ncdb::NCDB_END_OF_QUERY)
        if ((req.ReqType == Ncdb::SIMPLE_SEARCH || req.ReqType == Ncdb::GEOCODE_SEARCH || req.ReqType == Ncdb::POI_SEARCH) && (rc == NCDB_OK || rc == Ncdb::NCDB_MORE_RECORDS || rc == Ncdb::NCDB_END_OF_QUERY))
        {
			//do
			//{
					for (int i = 0; i < (int)geoPoiMatch.size(); i++)
					{
							std::basic_ostringstream<char> stm;
                            DumpSearchOutput(stm, geoPoiMatch[i]);
                    }
					geoPoiMatch.clear();
			/*
					if (rc != Ncdb::NCDB_END_OF_QUERY)
					{
							req.PageID++;
							rc = search.DoSingleSearch(req, geoPoiMatch) ;
					}
			} while(rc != Ncdb::NCDB_END_OF_QUERY);
			*/
        }
//        else if (req.ReqType == Ncdb::SUGGEST && rc == NCDB_OK)
        else if ( (req.ReqType == Ncdb::SIMPLE_SUGGEST || req.ReqType == Ncdb::GEOCODE_SUGGEST || req.ReqType == Ncdb::POI_SUGGEST) && rc == NCDB_OK)
        {
            for (int i = 0; i < (int)geoPoiMatch.size(); i++)
                cout << "\tOUTPUT : " << geoPoiMatch[i]->AddressOut.FormattedAddress.GetBuffer() << 
                     "; MatchType = " <<geoPoiMatch[i]->MatchType << 
                    endl;
              //  cout << "\tCLUCENE_ADDRESS : <" << geoPoiMatch[i]->MatchDistance << " miles> : " << geoPoiMatch[i]->AddressOut.FormattedAddress.GetBuffer() << 
              //       " (LAT,LON = " << geoPoiMatch[i]->GeocodedLocation.y << "," << geoPoiMatch[i]->GeocodedLocation.x << ")" << 
              //      endl;
            geoPoiMatch.clear();
        }
        else  if (rc == NCDB_NOT_FOUND)
        {
            cout << "\t" << FAILURE_NOT_FOUND << " - " << rc << endl;
            outstring <<FAILURE_NOT_FOUND<< "-"<< rc;
        }
        else if (rc == NCDB_UNSUPPORTED)
        {
            cout << "\t Feature Not Supported." << " - " << rc << endl;
            outstring <<FAILURE<<"-"<<rc;
        }
        else if (rc == NCDB_TIME_OUT)
        {
            cout << "\t Time Out." << endl;
            outstring <<FAILURE<<"-"<<rc<<endl;
        }
        else 
        {
            cout << "\t" << FAILURE << " - " << rc << endl;
            outstring <<FAILURE<<"-"<<rc;
        }
        outstring <<"</OPTIONS>";
        outstring <<"<EXECTIME>"<<exec_time<<"</EXECTIME>";
        outstring <<"<RETCODE>"<<rc<<"</RETCODE>";
        outstring <<"</CASE_AND_RESULTS>";
        if(g_bDumpFile && outfile.is_open())
        {
            outfile << outstring.str() << endl;
        }
        cout<< "Total Time: "<< exec_time;
        cout << endl;
    }
}

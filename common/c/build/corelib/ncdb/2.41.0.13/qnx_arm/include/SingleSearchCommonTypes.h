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


#pragma once
#ifndef __SINGLE_SEARCH_COMMON_TYPES_H__
#define __SINGLE_SEARCH_COMMON_TYPES_H__
#include "UtfString.h"
#include "AutoArray.h"
namespace Ncdb {

//WANT FLAGS
#define SAERCH_DEFAULT                          0x00000000

#define WANT_SINGLELINE_ADDRESS                 0x00000001
#define WANT_COMPACT_ADDRESS                    0x00000002	//FORMATTED ADDRESS(2 LINES)
#define WANT_EXTENDED_ADDRESS                   0x00000004	//FORMATTED ADDRESS(UPTO 5 LINES)
#define WANT_COUNTRY_NAME                       0x00000008	//??

#define WANT_SEARCH_DISTANCE                    0x00000010	//PROVIDE DISTANCE FOR EVERY MATCH USING MAP CENTER
#define WANT_SUGGEST_DISTANCE                   0x00000020	//RETURN DISTANCE IN EVERY SUGGEST MATCH USING MAP CENTE

#define WANT_SUGGEST_3LINES                     0x00000100	//UPTO 3 LINES IN SUGGEST RESPONSE. DEFAULT IS 2 LINES
// End of WANT FLAGS

class   Address;
struct  WorldPoint;
class   SpatialPoi;
struct  GeocodeMatch;
typedef AutoArray<UtfString> FormattedAddressAutoArray;

// -----------------------------------------------------------------------------------
//! Enum Detailing the Geocode Match Type
enum GeoMatchType
{
    GCMatch_Undefined                           = 0,
    GCMatch_Street                              = 1,
    GCMatch_Intersection                        = 2,
    GCMatch_UNUSED                              = 3,
    GCMatch_City                                = 4,
    GCMatch_State                               = 5,
    GCMatch_Country                             = 6,
    GCMatch_Airport                             = 7,
    GCMatch_Postal                              = 8,
    GCMatch_County                              = 9,
    GCMatch_Street_Spatial                      = 11,
    GCMatch_City_MajorCity                      = 40,
    GCMatch_City_MajorCity_CountyCapital        = 41,
    GCMatch_City_MajorCity_StateCapital         = 42,
    GCMatch_City_MajorCity_CountryCapital       = 43,
    GCMatch_City_MajorCity_State_CountryCapital = 44,
    PoiMatch_Name                               = 100,
    PoiMatch_Category                           = 101,
    PoiMatch_Telephone                          = 102,
	PoiMatch_InterestList						= 103
};

struct NCDB_API BoundingBox
{
    WorldPoint         topLeft;
    WorldPoint         bottomRight;

	WorldPoint getTopLeft()
	{
		return topLeft;
	}

	WorldPoint getBottomRight()
	{
		return bottomRight;
	}

	WorldPoint getTopRight()
	{
		return WorldPoint(bottomRight.x, topLeft.y);
	}

	WorldPoint getBottomLeft()
	{
		return WorldPoint(topLeft.x, bottomRight.y);
	}
};

struct ExonymInfo
{
    SupportedLanguages langId;
    UtfString name;
};

struct NCDB_API StreetComponents
{
    UtfString prefix;
    UtfString suffix;
    UtfString basename;
    UtfString typeBefore;
    UtfString type;
    SupportedLanguages langCode;
    UtfString langAbbr;
    UtfString fullName;         //!< The combined street name with all components at their correct place

    bool typeAttach;
    bool typeBeforeAttach;

    StreetComponents()
    {
        langCode = Invalid_Language;
        typeAttach = false;
        typeBeforeAttach = false;
    }

    void clear()
    {
        prefix = "";
        suffix = "";
        basename = "";
        typeBefore = "";
        type = "";
        langCode = Invalid_Language;
        langAbbr = "";
        fullName = "";
        typeAttach = false;
        typeBeforeAttach = false;
    }
};

//! Represents type informaton object to be used for search
//! -1 in sub-type indicates ALL sub-type
struct NCDB_API TYPE_INFO
{
    int typeId;
    int subTypeId;
    TYPE_INFO():typeId(-1), subTypeId(-1)
    {
    }
    TYPE_INFO(int main, int sub): typeId(main), subTypeId(sub)
    {
    }
} ;

//! Represents type data that will be returned from API
typedef struct type_data
{
    int         typeId;
    UtfString   typeName;

    type_data():typeId(0)
    {
        typeName.Clear();
    }
    ~type_data()
    {
        typeName.Clear();
    }

    type_data(const type_data& other)
    {
        *this = other; // Revert to overload =
    }

    void operator = (const type_data& other)
    {
        if( &other == this ) return;

        this->typeId = other.typeId;
        this->typeName = other.typeName;
    }

} TYPE_DATA;

//! lists various possible search type current supported
typedef enum poi_search_type
{
    ALL_POI_SEARCH = 0x0,
    TYPE_SEARCH = 0x1,
    NAME_SEARCH = 0x2,
    PHONE_SEARCH = 0x4
} POI_SEARCH_TYPE_VALUES;

typedef int SEARCH_TYPE;
//! Defines data strcuture to take search criteria as input
struct NCDB_API SEARCH_CRITERIA
{
    SEARCH_TYPE searchType;
    TYPE_INFO typeToSearch;
    UtfString phoneToSearch;
    UtfString nameToSearch;
    UtfString SearchResultId;
    SEARCH_CRITERIA()
    {
        clear();
    }

    void clear()
    {
        searchType = 0;
        typeToSearch.typeId = -1;
        typeToSearch.subTypeId = -1;
        phoneToSearch = "";
        nameToSearch = "";
        SearchResultId = "";
    }

    bool isIncludeAllPoiSearch() const {
        return (searchType == 0);
    }

    bool isTypeSearch() const {
        return ((searchType & TYPE_SEARCH) != 0);
    }

    bool isNameSearch() const {
        return ((searchType & NAME_SEARCH) != 0);
    }

    bool isPhoneSearch() const {
        return ((searchType & PHONE_SEARCH) != 0);
    }

    bool includeAllSubType() const {
        return typeToSearch.subTypeId < 0;
    }

};


struct SuggestMatch
{
    UtfString SuggestMatchType; //ex- Category?
    UtfString SuggestMatchName; //ex- Hotels & Motels?
    UtfString Iconid;           //ex- HC? or 0001?
    UtfString SearchResultId;   // Type=category|name=Hotels & Motels|code=XHC,XHA,XHF?
                                // Type=category|brand=Walmart::WAL-MART::Walmart Neighborhood Market|code=XKU,XKT,XKK,XKL,XKR,XKD,XKF,XKH,XKI"
    SuggestMatch():SuggestMatchType(""), SuggestMatchName(""), Iconid(""), SearchResultId("")
    {
    }
    SuggestMatch(const char* type, const char* name, const char* iconid, const char* resultid):
        SuggestMatchType(type), SuggestMatchName(name), Iconid(iconid), SearchResultId(resultid)
    {
    }
    void Clear()
    {
        SuggestMatchType.Clear();
        SuggestMatchName.Clear();
        Iconid.Clear();
        SearchResultId.Clear();
    }
};

struct SuggestList
{
    UtfString SuggestListName;
    AutoArray<SuggestMatch> SuggestMatchArr;
    SuggestList(): SuggestListName("")
    {
    }
    SuggestList(const char* listName): SuggestListName(listName) 
    {
    }
    ~SuggestList()
    {
        SuggestListName.Clear();
        for(int i = 0; i < SuggestMatchArr.size(); ++i)
            SuggestMatchArr[i].Clear();
        SuggestMatchArr.clear();
    }
};

#ifdef _MSC_VER
template class NCDB_API AutoArray<SuggestMatch>;
template class NCDB_API AutoArray<SuggestList>;

template class NCDB_API Ncdb::AutoArray<Ncdb::TYPE_DATA*>;
template class NCDB_API AutoArray<ExonymInfo>;
template class NCDB_API AutoArray<StreetComponents>;
#endif

struct NCDB_API OutAddress
{
    UtfString                   StartNumber;        //!<  Start House number.
    UtfString                   EndNumber;          //!<  End House number. Only used in range geocoding.

    UtfString                   StreetName;         //!<  Searched and found street name.

    AutoArray<StreetComponents> StreetList;         //!<  Street names, including secondary names. 0th index will contain the components for Street

    UtfString                   CrossStreet;        //!<  Searched and found crossing street name.
    AutoArray<StreetComponents> CrossStreetList;    //!<  Crossing street names, including secondary names.0th index will contain the components for Cross Street

    UtfString      Subcity1;                       //!<  Named part of a city (Postal City, Neighborhood, District, Arrondissements  etc.)
    AutoArray<ExonymInfo> NamedAreaExonymList;          //!<  Exonym Names of NamedArea

    UtfString      City;                            //!<  City name
    AutoArray<ExonymInfo> CityExonymList;           //!<  Exonym Names of City

    UtfString      Subcity2;                            //!<  Zone name.
    AutoArray<ExonymInfo> ZoneExonymList;           //!<  Exonym Names of Zone

    UtfString      County;                          //!<  County name (Kreis in Germany, Commune in France, Rayon - in Russia)
    UtfString      State;                           //!<  State/Province abbr name (Department in Franse, Land in Germany, Oblast' in Russia)
    AutoArray<ExonymInfo> StateExonymList;          //!<  Exonym Names of State
    UtfString      StateFullName;                   //!<  State  name
    UtfString      Postal;                           //!<  Postal code (ZIP in US)

    UtfString      AirportCode;                     //!<  Airport code in case of airport serach
    UtfString      AirportName;                     //!<  Airport name for airport serach.

    UtfString      Country;                         //!<  Country name

    UtfString      FormattedAddress;                //!< Single Line Freeform address as per country specific postal standards.Configurable.
    UtfString      OutAdmin;
    UtfString      OutState;

    FormattedAddressAutoArray   compactFormattedAddress; //Popluate if WANT_COMPACT_ADDRESS is true
    FormattedAddressAutoArray   extendedFormattedAddress;//Populate if WANT_EXTENDED_ADDRESS is true

    OutAddress();
    OutAddress(Address& address);
    void FromAddress(Address& address);
};

};

#endif // __SINGLE_SEARCH_COMMON_TYPES_H__
